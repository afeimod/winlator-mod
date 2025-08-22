package com.winlator.core;

import android.os.Process;
import android.system.Os;

import androidx.annotation.NonNull;

import com.winlator.MainActivity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public abstract class ProcessHelper {
    public enum PState {RUNNING, SLEEPING, WAITING, ZOMBIE, STOPPED, DEAD, OTHER}
    private static final ArrayList<Callback<String>> debugCallbacks = new ArrayList<>();
    private static final byte SIGCONT = 18;
    private static final byte SIGSTOP = 19;

    public static class PStat {
        public int pid = 0;
        public String name = "";
        public PState state = PState.OTHER;
        public int parentPID = 0;
        public boolean guestProcess = false;

        @NonNull
        @Override
        public String toString() {
            return pid+" "+name+" "+state+" "+parentPID+" "+guestProcess;
        }
    }

    public static void suspendProcess(int pid) {
        Process.sendSignal(pid, SIGSTOP);
    }

    public static void resumeProcess(int pid) {
        Process.sendSignal(pid, SIGCONT);
    }

    public static int exec(String command) {
        return exec(command, null);
    }

    public static int exec(String command, EnvVars envVars) {
        return exec(command, envVars, null);
    }

    public static int exec(String command, EnvVars envVars, File workingDir) {
        return exec(command, envVars, workingDir, null);
    }

    public static int exec(String command, EnvVars envVars, File workingDir, Callback<Integer> terminationCallback) {
        int pid = -1;
        try {
            ProcessBuilder processBuilder = (new ProcessBuilder(splitCommand(command))).directory(workingDir);
            if (debugCallbacks.isEmpty()) processBuilder.redirectOutput(new File("/dev/null")).redirectErrorStream(true);

            Map<String, String> environment = processBuilder.environment();
            for (String name : envVars) environment.put(name, envVars.get(name));

            java.lang.Process process = processBuilder.start();
            Field pidField = process.getClass().getDeclaredField("pid");
            pidField.setAccessible(true);
            pid = pidField.getInt(process);
            pidField.setAccessible(false);

            if (!debugCallbacks.isEmpty()) {
                createDebugThread(process.getInputStream());
                createDebugThread(process.getErrorStream());
            }

            if (terminationCallback != null) createWaitForThread(process, terminationCallback);
        }
        catch (Exception e) {}
        return pid;
    }

    private static void createDebugThread(final InputStream inputStream) {
        Executors.newSingleThreadExecutor().execute(() -> {
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    synchronized (debugCallbacks) {
                        if (!debugCallbacks.isEmpty()) {
                            for (Callback<String> callback : debugCallbacks) callback.call(line);
                        }
                        else if (MainActivity.DEBUG_MODE) System.out.println(line);
                    }
                }
            }
            catch (IOException e) {}
        });
    }

    private static void createWaitForThread(java.lang.Process process, final Callback<Integer> terminationCallback) {
        Executors.newSingleThreadExecutor().execute(() -> {
            try {
                int status = process.waitFor();
                terminationCallback.call(status);
            }
            catch (InterruptedException e) {}
        });
    }

    public static void removeAllDebugCallbacks() {
        synchronized (debugCallbacks) {
            debugCallbacks.clear();
        }
    }

    public static void addDebugCallback(Callback<String> callback) {
        synchronized (debugCallbacks) {
            if (!debugCallbacks.contains(callback)) debugCallbacks.add(callback);
        }
    }

    public static void removeDebugCallback(Callback<String> callback) {
        synchronized (debugCallbacks) {
            debugCallbacks.remove(callback);
        }
    }

    public static String[] splitCommand(String command) {
        ArrayList<String> result = new ArrayList<>();
        boolean startedQuotes = false;
        String value = "";
        char currChar, nextChar;
        for (int i = 0, count = command.length(); i < count; i++) {
            currChar = command.charAt(i);

            if (startedQuotes) {
                if (currChar == '"') {
                    startedQuotes = false;
                    if (!value.isEmpty()) {
                        value += '"';
                        result.add(value);
                        value = "";
                    }
                }
                else value += currChar;
            }
            else if (currChar == '"') {
                startedQuotes = true;
                value += '"';
            }
            else {
                nextChar = i < count-1 ? command.charAt(i+1) : '\0';
                if (currChar == ' ' || (currChar == '\\' && nextChar == ' ')) {
                    if (currChar == '\\') {
                        value += ' ';
                        i++;
                    }
                    else if (!value.isEmpty()) {
                        result.add(value);
                        value = "";
                    }
                }
                else {
                    value += currChar;
                    if (i == count-1) {
                        result.add(value);
                        value = "";
                    }
                }
            }
        }

        return result.toArray(new String[0]);
    }

    public static String getAffinityMaskAsHexString(String cpuList) {
        String[] values = cpuList.split(",");
        int affinityMask = 0;
        for (String value : values) {
            byte index = Byte.parseByte(value);
            affinityMask |= (int)Math.pow(2, index);
        }
        return Integer.toHexString(affinityMask);
    }

    public static int getAffinityMask(String cpuList) {
        if (cpuList == null || cpuList.isEmpty()) return 0;
        String[] values = cpuList.split(",");
        int affinityMask = 0;
        for (String value : values) {
            byte index = Byte.parseByte(value);
            affinityMask |= (int)Math.pow(2, index);
        }
        return affinityMask;
    }

    public static int getAffinityMask(boolean[] cpuList) {
        int affinityMask = 0;
        for (int i = 0; i < cpuList.length; i++) {
            if (cpuList[i]) affinityMask |= (int)Math.pow(2, i);
        }
        return affinityMask;
    }

    public static int getAffinityMask(int from, int to) {
        int affinityMask = 0;
        for (int i = from; i < to; i++) affinityMask |= (int)Math.pow(2, i);
        return affinityMask;
    }

    public static List<PStat> getChildProcesses() {
        File procFile = new File("/proc");
        String[] pids = procFile.list((file, name) -> (new File(file, name)).isDirectory() && name.matches("[0-9]+"));
        if (pids == null) return Collections.emptyList();
        ArrayList<PStat> result = new ArrayList<>();
        int parentPID = Os.getpid();

        for (String pid : pids) {
            try (Scanner scanner = new Scanner(new FileInputStream("/proc/"+pid+"/stat"))) {
                PStat pstat = new PStat();
                int index = 0;

                while (scanner.hasNext() && index < 4) {
                    switch (index++) {
                        case 0:
                            pstat.pid = scanner.nextInt();
                            break;
                        case 1:
                            Pattern oldDelimiter = scanner.delimiter();
                            scanner.useDelimiter("\\)");
                            pstat.name = scanner.hasNext() ? scanner.next().substring(2) : "";
                            scanner.useDelimiter(oldDelimiter);
                            if (scanner.hasNext()) scanner.next();
                            break;
                        case 2: {
                            switch (scanner.next()) {
                                case "R":
                                    pstat.state = PState.RUNNING;
                                    break;
                                case "S":
                                    pstat.state = PState.SLEEPING;
                                    break;
                                case "D":
                                    pstat.state = PState.WAITING;
                                    break;
                                case "Z":
                                    pstat.state = PState.ZOMBIE;
                                    break;
                                case "T":
                                    pstat.state = PState.STOPPED;
                                    break;
                                case "X":
                                    pstat.state = PState.DEAD;
                                    break;
                            }
                            break;
                        }
                        case 3:
                            pstat.parentPID = scanner.nextInt();
                            break;
                    }
                }

                if (pstat.parentPID == parentPID || pstat.pid > parentPID) {
                    pstat.guestProcess = pstat.name.contains("wine") || pstat.name.contains(".exe");
                    result.add(pstat);
                }
            }
            catch (Exception e) {
                return Collections.emptyList();
            }
        }

        return result;
    }
}
