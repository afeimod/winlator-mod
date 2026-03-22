package com.winlator;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class TerminalFragment extends Fragment {
    private static final int MAX_DISPLAY_LINES = 5000;
    private static final int MAX_PENDING_CHARS = 100000; // Limit pending text to 100KB
    private static final int UI_UPDATE_INTERVAL = 100; // Update UI every 100ms
    private static final String OUTPUT_FILE_NAME = "terminal_outputs.txt";
    private static final String LATEST_OUTPUT_FILE_NAME = "terminal_latest.txt";
    private static final int STORAGE_PERMISSION_REQUEST_CODE = 100;
    
    private String outputFilePath;
    private String latestOutputFilePath;
    
    private TextView tvTerminalOutput;
    private EditText etCommandInput;
    private Button btExportOutput;
    private Button btExecuteCommand;
    private Button btInterrupt;
    private Switch swRealTimeLog;
    private ScrollView svTerminal;
    
    private final LinkedList<String> terminalLines = new LinkedList<>();
    private boolean isRealTimeLogEnabled = false;
    private Process currentProcess = null;
    private OutputStream processOutputStream = null;
    private final ExecutorService outputReaderExecutor = Executors.newSingleThreadExecutor();
    private final ExecutorService commandExecutor = Executors.newSingleThreadExecutor();
    private final ExecutorService logWriterExecutor = Executors.newSingleThreadExecutor();
    private final Handler uiHandler = new Handler(Looper.getMainLooper());
    
    // Fragment lifecycle management
    private volatile boolean isFragmentDestroyed = false;
    private volatile boolean isViewDestroyed = false;
    
    // Batch UI update
    private final StringBuilder pendingText = new StringBuilder();
    private final Runnable uiUpdateRunnable = this::updateDisplay;
    private boolean uiUpdatePending = false;
    
    // Batch log writing
    private final StringBuilder pendingLogText = new StringBuilder();
    private final Runnable logWriteRunnable = this::flushPendingLog;
    
    // Rate limiting
    private long lastOutputTime = 0;
    private final long MIN_OUTPUT_INTERVAL = 10; // Minimum 10ms between outputs
    
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(false);
        
        // Initialize file paths
        File downloadDir = android.os.Environment.getExternalStoragePublicDirectory(
            android.os.Environment.DIRECTORY_DOWNLOADS);
        File winlatorDir = new File(downloadDir, "Winlator");
        
        outputFilePath = new File(winlatorDir, OUTPUT_FILE_NAME).getAbsolutePath();
        latestOutputFilePath = new File(winlatorDir, LATEST_OUTPUT_FILE_NAME).getAbsolutePath();
        
        checkStoragePermission();
    }
    
    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        ((AppCompatActivity) getActivity()).getSupportActionBar().setTitle(R.string.terminal);
    }
    
    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.terminal_fragment, container, false);
        
        tvTerminalOutput = view.findViewById(R.id.TVTerminalOutput);
        etCommandInput = view.findViewById(R.id.ETCommandInput);
        btExportOutput = view.findViewById(R.id.BTExportOutput);
        btExecuteCommand = view.findViewById(R.id.BTExecuteCommand);
        btInterrupt = view.findViewById(R.id.BTInterrupt);
        swRealTimeLog = view.findViewById(R.id.SWRealTimeLog);
        svTerminal = view.findViewById(R.id.SVTerminal);
        
        // Set up button listeners
        btExecuteCommand.setOnClickListener(v -> executeCommand());
        btExportOutput.setOnClickListener(v -> exportOutput());
        btInterrupt.setOnClickListener(v -> interruptCommand());
        
        // Set up real-time log switch
        swRealTimeLog.setOnCheckedChangeListener((buttonView, isChecked) -> {
            isRealTimeLogEnabled = isChecked;
            if (isChecked) {
                appendToTerminal("Real-time logging enabled: " + latestOutputFilePath + "\n");
                // Test write to file
                commandExecutor.execute(() -> {
                    boolean success = writeToFile(latestOutputFilePath, 
                        "=== Terminal Log Started at " + new java.util.Date() + " ===\n", 
                        false);
                    if (!success) {
                        uiHandler.post(() -> appendToTerminal("Warning: Failed to create log file. Check permissions.\n"));
                    } else {
                        uiHandler.post(() -> appendToTerminal("Log file created successfully.\n"));
                    }
                });
            } else {
                appendToTerminal("Real-time logging disabled\n");
            }
        });
        
        // Set up enter key listener for command input
        etCommandInput.setOnEditorActionListener((v, actionId, event) -> {
            executeCommand();
            return true;
        });
        
        // Initialize terminal
        startInteractiveShell();
        
        return view;
    }
    
    private void checkStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(requireContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE) 
                != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 
                    STORAGE_PERMISSION_REQUEST_CODE);
            }
        }
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == STORAGE_PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                appendToTerminal("Warning: Storage permission denied. File export may not work.\n");
            }
        }
    }
    
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        isViewDestroyed = true;
        // Stop UI updates
        uiHandler.removeCallbacks(uiUpdateRunnable);
        // Clean up process
        cleanupProcess();
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        isFragmentDestroyed = true;
        // Shutdown all executors
        outputReaderExecutor.shutdown();
        commandExecutor.shutdown();
        logWriterExecutor.shutdown();
        // Cleanup UI handler
        uiHandler.removeCallbacksAndMessages(null);
    }
    
    private void cleanupProcess() {
        if (currentProcess != null) {
            currentProcess.destroy();
            currentProcess = null;
        }
        if (processOutputStream != null) {
            try {
                processOutputStream.close();
            } catch (IOException e) {
                // Ignore
            }
            processOutputStream = null;
        }
    }
    
    private void startInteractiveShell() {
        commandExecutor.execute(() -> {
            try {
                // Stop previous shell if running
                if (currentProcess != null && currentProcess.isAlive()) {
                    currentProcess.destroy();
                    Thread.sleep(100);
                }
                
                ProcessBuilder pb = new ProcessBuilder("sh");
                pb.redirectErrorStream(true);
                
                // Set working directory
                File imageFsDir = new File("/data/data/com.winlator/files/imagefs");
                if (imageFsDir.exists() && imageFsDir.isDirectory()) {
                    pb.directory(imageFsDir);
                }
                
                // Set environment variables
                java.util.Map<String, String> env = pb.environment();
                env.remove("LD_PRELOAD");
                
                File xuserDir = new File("/data/data/com.winlator/files/imagefs/home/xuser");
                if (xuserDir.exists() && xuserDir.isDirectory()) {
                    env.put("HOME", "/data/data/com.winlator/files/imagefs/home/xuser");
                }
                
                currentProcess = pb.start();
                processOutputStream = currentProcess.getOutputStream();
                
                uiHandler.post(() -> {
                    if (!isViewDestroyed) {
                        appendToTerminal("Shell started in: " + pb.directory() + "\n");
                    }
                });
                
                // Start reading output immediately
                startReadingOutput();
                
                // Execute initial setup commands
                Thread.sleep(200); // Wait for shell to be ready
                sendCommandToShell("unset LD_PRELOAD");
                Thread.sleep(100);
                
            } catch (IOException | InterruptedException e) {
                uiHandler.post(() -> {
                    if (!isViewDestroyed) {
                        appendToTerminal("Error starting shell: " + e.getMessage() + "\n");
                    }
                });
            }
        });
    }
    
    private void startReadingOutput() {
        outputReaderExecutor.execute(() -> {
            try {
                InputStream inputStream = currentProcess.getInputStream();
                byte[] buffer = new byte[8192];
                StringBuilder lineBuffer = new StringBuilder();
                
                while (!isFragmentDestroyed && currentProcess != null && currentProcess.isAlive()) {
                    int bytesRead = inputStream.read(buffer);
                    if (bytesRead == -1) break;
                    
                    // Rate limiting: skip if output is too fast
                    long currentTime = System.currentTimeMillis();
                    if (currentTime - lastOutputTime < MIN_OUTPUT_INTERVAL) {
                        Thread.sleep(MIN_OUTPUT_INTERVAL - (currentTime - lastOutputTime));
                    }
                    lastOutputTime = System.currentTimeMillis();
                    
                    String chunk = new String(buffer, 0, bytesRead);
                    final String[] lines = chunk.split("(?<=\n)");
                    
                    for (String line : lines) {
                        lineBuffer.append(line);
                        if (line.endsWith("\n")) {
                            final String outputLine = lineBuffer.toString();
                            lineBuffer.setLength(0);
                            uiHandler.post(() -> {
                                if (!isViewDestroyed) {
                                    appendToTerminal(outputLine);
                                }
                            });
                        }
                    }
                }
                
                // Output remaining content
                if (lineBuffer.length() > 0) {
                    final String outputLine = lineBuffer.toString();
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            appendToTerminal(outputLine + "\n");
                        }
                    });
                }
                
            } catch (IOException e) {
                if (!isFragmentDestroyed && currentProcess != null && currentProcess.isAlive()) {
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            appendToTerminal("Error reading output: " + e.getMessage() + "\n");
                        }
                    });
                }
            } catch (InterruptedException e) {
                // Thread interrupted, just exit
            }
        });
    }
    
    private void executeCommand() {
        String command = etCommandInput.getText().toString().trim();
        if (command.isEmpty()) {
            return;
        }
        
        final String displayCommand = "$ " + command + "\n";
        uiHandler.post(() -> {
            if (!isViewDestroyed) {
                appendToTerminal(displayCommand);
                etCommandInput.setText("");
            }
        });
        
        // Hide keyboard
        InputMethodManager imm = (InputMethodManager) getActivity().getSystemService(android.content.Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(etCommandInput.getWindowToken(), 0);
        
        sendCommandToShell(command);
    }
    
    private void sendCommandToShell(String command) {
        commandExecutor.execute(() -> {
            if (processOutputStream != null && currentProcess != null && currentProcess.isAlive()) {
                try {
                    processOutputStream.write((command + "\n").getBytes());
                    processOutputStream.flush();
                } catch (IOException e) {
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            appendToTerminal("Error executing command: " + e.getMessage() + "\n");
                            // Restart shell if error occurs
                            startInteractiveShell();
                        }
                    });
                }
            } else {
                uiHandler.post(() -> {
                    if (!isViewDestroyed) {
                        appendToTerminal("Shell not alive, restarting...\n");
                        startInteractiveShell();
                        // Retry command after restart
                        commandExecutor.execute(() -> {
                            try {
                                Thread.sleep(500);
                                if (processOutputStream != null) {
                                    processOutputStream.write((command + "\n").getBytes());
                                    processOutputStream.flush();
                                }
                            } catch (IOException | InterruptedException e) {
                                uiHandler.post(() -> {
                                    if (!isViewDestroyed) {
                                        appendToTerminal("Failed to execute command after restart: " + e.getMessage() + "\n");
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    }
    
    private void interruptCommand() {
        commandExecutor.execute(() -> {
            if (currentProcess != null && currentProcess.isAlive()) {
                try {
                    // Method 1: Send Ctrl+C character (0x03)
                    if (processOutputStream != null) {
                        processOutputStream.write(new byte[]{3});
                        processOutputStream.flush();
                    }
                    
                    // Method 2: Send SIGINT signal using Process.destroyForcibly()
                    // This is more reliable for interrupting long-running processes
                    Thread.sleep(100); // Give it a moment to respond to Ctrl+C
                    if (currentProcess.isAlive()) {
                        currentProcess.destroyForcibly();
                    }
                    
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            appendToTerminal("^C\nProcess interrupted\n");
                        }
                    });
                    
                    // Restart shell after interrupt
                    Thread.sleep(200);
                    startInteractiveShell();
                    
                } catch (IOException | InterruptedException e) {
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            appendToTerminal("Error sending interrupt: " + e.getMessage() + "\n");
                        }
                    });
                }
            }
        });
    }
    
    private void appendToTerminal(String line) {
        if (line == null || line.isEmpty() || isViewDestroyed) return;
        
        terminalLines.add(line);
        
        // Enforce line limit
        while (terminalLines.size() > MAX_DISPLAY_LINES) {
            terminalLines.removeFirst();
        }
        
        // Limit pending text size
        if (pendingText.length() < MAX_PENDING_CHARS) {
            pendingText.append(line);
        }
        
        // Write to log file if enabled
        if (isRealTimeLogEnabled) {
            synchronized (pendingLogText) {
                pendingLogText.append(line);
            }
            logWriterExecutor.execute(logWriteRunnable);
        }
        
        // Schedule UI update
        if (!uiUpdatePending) {
            uiUpdatePending = true;
            uiHandler.postDelayed(uiUpdateRunnable, UI_UPDATE_INTERVAL);
        }
    }
    
    private void updateDisplay() {
        uiUpdatePending = false;
        
        if (isViewDestroyed || pendingText.length() == 0) return;
        
        // Build display text only for new content
        tvTerminalOutput.append(pendingText.toString());
        pendingText.setLength(0);
        
        // Auto-scroll to bottom
        svTerminal.post(() -> svTerminal.fullScroll(ScrollView.FOCUS_DOWN));
    }
    
    private void flushPendingLog() {
        String textToWrite;
        synchronized (pendingLogText) {
            if (pendingLogText.length() == 0) return;
            textToWrite = pendingLogText.toString();
            pendingLogText.setLength(0);
        }
        
        writeToFile(latestOutputFilePath, textToWrite, true);
    }
    
    private void exportOutput() {
        commandExecutor.execute(() -> {
            try {
                // Ensure directory exists
                File outputFile = new File(outputFilePath);
                File parentDir = outputFile.getParentFile();
                if (parentDir != null && !parentDir.exists()) {
                    parentDir.mkdirs();
                }
                
                // Write all lines to file
                StringBuilder allOutput = new StringBuilder();
                for (String line : terminalLines) {
                    allOutput.append(line);
                }
                
                boolean success = writeToFile(outputFilePath, allOutput.toString(), false);
                
                if (success) {
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            android.widget.Toast.makeText(getActivity(), 
                                getString(R.string.terminal_output_exported) + " " + outputFilePath, 
                                android.widget.Toast.LENGTH_LONG).show();
                        }
                    });
                } else {
                    uiHandler.post(() -> {
                        if (!isViewDestroyed) {
                            android.widget.Toast.makeText(getActivity(), 
                                getString(R.string.export_failed), 
                                android.widget.Toast.LENGTH_LONG).show();
                        }
                    });
                }
                
            } catch (Exception e) {
                uiHandler.post(() -> {
                    if (!isViewDestroyed) {
                        android.widget.Toast.makeText(getActivity(), 
                            getString(R.string.export_failed) + ": " + e.getMessage(), 
                            android.widget.Toast.LENGTH_LONG).show();
                    }
                });
            }
        });
    }
    
    private boolean writeToFile(String filePath, String content, boolean append) {
        File file = new File(filePath);
        File parentDir = file.getParentFile();
        
        try {
            // Ensure directory exists
            if (parentDir != null && !parentDir.exists()) {
                boolean created = parentDir.mkdirs();
                if (!created && !parentDir.exists()) {
                    android.util.Log.e("TerminalFragment", "Failed to create directory: " + parentDir.getAbsolutePath());
                    return false;
                }
            }
            
            // Write to file
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(file, append))) {
                writer.write(content);
                writer.flush();
            }
            
            android.util.Log.d("TerminalFragment", "Successfully wrote to file: " + filePath);
            return true;
        } catch (IOException e) {
            android.util.Log.e("TerminalFragment", "Error writing to file: " + filePath, e);
            return false;
        }
    }
}