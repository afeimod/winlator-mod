package com.winlator.xenvironment.components;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Process;
import android.util.Log;

import androidx.preference.PreferenceManager;

import com.winlator.box64.Box64Preset;
import com.winlator.box64.Box64PresetManager;
import com.winlator.contents.ContentProfile;
import com.winlator.contents.ContentsManager;
import com.winlator.core.Callback;
import com.winlator.core.DefaultVersion;
import com.winlator.core.EnvVars;
import com.winlator.core.ProcessHelper;
import com.winlator.core.TarCompressorUtils;
import com.winlator.core.WineInfo;
import com.winlator.fex.FEXPresetManager;
import com.winlator.xconnector.UnixSocketConfig;
import com.winlator.xenvironment.EnvironmentComponent;
import com.winlator.xenvironment.ImageFs;

import java.io.File;

public class GlibcProgramLauncherComponent extends EnvironmentComponent {
    private final ContentsManager contentsManager;
    private final String wineVersion;
    private int fexPreset = 0;
    private String fexPresetCustom = "";
    protected String guestExecutable;
    protected static int pid = -1;
    protected String[] bindingPaths;
    protected EnvVars envVars;
    protected String box64Preset = Box64Preset.COMPATIBILITY;
    protected Callback<Integer> terminationCallback;
    protected static final Object lock = new Object();
    protected String logFilePath;

    public GlibcProgramLauncherComponent(ContentsManager contentsManager, String wineVersion) {
        this.contentsManager = contentsManager;
        this.wineVersion = wineVersion;
    }

    @Override
    public void start() {
        synchronized (lock) {
            stop();
            extractBox64Files();
            pid = execGuestProgram();
        }
    }

    @Override
    public void stop() {
        synchronized (lock) {
            if (pid != -1) {
                Process.killProcess(pid);
                pid = -1;
            }
        }
    }

    public void setFexPreset(int fexPreset) {
        this.fexPreset = fexPreset;
    }

    public void setFexPresetCustom(String fexPresetCustom) {
        this.fexPresetCustom = fexPresetCustom;
    }

    public Callback<Integer> getTerminationCallback() {
        return terminationCallback;
    }

    public void setTerminationCallback(Callback<Integer> terminationCallback) {
        this.terminationCallback = terminationCallback;
    }

    public String getGuestExecutable() {
        return guestExecutable;
    }

    public void setGuestExecutable(String guestExecutable) {
        this.guestExecutable = guestExecutable;
    }

    public String[] getBindingPaths() {
        return bindingPaths;
    }

    public void setBindingPaths(String[] bindingPaths) {
        this.bindingPaths = bindingPaths;
    }

    public EnvVars getEnvVars() {
        return envVars;
    }

    public void setEnvVars(EnvVars envVars) {
        this.envVars = envVars;
    }

    public String getBox64Preset() {
        return box64Preset;
    }

    public void setBox64Preset(String box64Preset) {
        this.box64Preset = box64Preset;
    }

    public void setLogFilePath(String logFilePath) {
        this.logFilePath = logFilePath;
    }

    protected int execGuestProgram() {
        Context context = environment.getContext();
        ImageFs imageFs = environment.getImageFs();
        File rootDir = imageFs.getRootDir();

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        boolean enableBox64Logs = preferences.getBoolean("enable_box64_logs", false);

        EnvVars envVars = new EnvVars();
        
        WineInfo wineInfo = WineInfo.fromIdentifier(context, wineVersion);
        boolean isArm64EC = wineInfo.getArch().equalsIgnoreCase("arm64ec");

        if (!isArm64EC) {
            addBox64EnvVars(envVars, enableBox64Logs);
        } else {
            addFEXEnvVars(envVars);
        }

        envVars.put("HOME", imageFs.home_path);
        envVars.put("USER", ImageFs.USER);
        envVars.put("TMPDIR", imageFs.getRootDir().getPath() + "/tmp");
        envVars.put("DISPLAY", ":0");
        envVars.put("WINE_HOST_XDG_CURRENT_DESKTOP", "1");

        ContentProfile profile = contentsManager.getProfileByEntryName(wineVersion);
        File wineDirAbs;
        File wineBinDirAbs;
        File wineLibDirAbs;

        if (profile != null && profile.type == ContentProfile.ContentType.CONTENT_TYPE_WINE) {
            wineDirAbs = ContentsManager.getInstallDir(context, profile);
            wineBinDirAbs = new File(wineDirAbs, profile.wineBinPath);
            wineLibDirAbs = new File(wineDirAbs, profile.wineLibPath);
        } else {
            String winePathStr = wineInfo.path;
            if (winePathStr != null && winePathStr.startsWith("/")) winePathStr = winePathStr.substring(1);
            wineDirAbs = new File(rootDir, winePathStr != null ? winePathStr : "opt/wine");
            wineBinDirAbs = new File(wineDirAbs, "bin");
            wineLibDirAbs = new File(wineDirAbs, "lib");
        }

        envVars.put("PATH", wineBinDirAbs.getPath() + ":" +
                new File(rootDir, "/usr/bin").getPath() + ":" +
                new File(rootDir, "/usr/local/bin").getPath());

        String ldLibraryPath = new File(rootDir, "/usr/lib").getPath();
        File wineLib64Dir = new File(wineDirAbs, "lib64");
        
        if (isArm64EC) {
            File wineUnixLibDir = new File(wineLibDirAbs, "wine/aarch64-unix");
            ldLibraryPath = wineUnixLibDir.getPath() + ":" + wineLibDirAbs.getPath() + ":" + ldLibraryPath;
            envVars.put("WINEDLLPATH", wineLibDirAbs.getPath() + "/wine");
        } else {
            ldLibraryPath = wineLib64Dir.getPath() + ":" + wineLibDirAbs.getPath() + ":" + ldLibraryPath;
            File wineDllDir = new File(wineLibDirAbs, "wine");
            if (!wineDllDir.exists()) wineDllDir = new File(wineLib64Dir, "wine");
            
            if (wineDllDir.exists()) {
                envVars.put("WINEDLLPATH", wineDllDir.getPath());
                File unix64 = new File(wineDllDir, "x86_64-unix");
                if (unix64.exists()) ldLibraryPath = unix64.getPath() + ":" + ldLibraryPath;
            }
        }

        envVars.put("LD_LIBRARY_PATH", ldLibraryPath);
        envVars.put("BOX64_LD_LIBRARY_PATH", new File(rootDir, "/usr/lib/x86_64-linux-gnu").getPath() + ":" + ldLibraryPath);
        envVars.put("ANDROID_SYSVSHM_SERVER", new File(rootDir, UnixSocketConfig.SYSVSHM_SERVER_PATH).getPath());
        envVars.put("FONTCONFIG_PATH", new File(rootDir, "/usr/etc/fonts").getPath());

        if ((new File(imageFs.getGlibc64Dir(), "libandroid-sysvshm.so")).exists())
            envVars.put("LD_PRELOAD", "libandroid-sysvshm.so");
            
        if (this.envVars != null) envVars.putAll(this.envVars);

        String finalArgs = guestExecutable;
        finalArgs = finalArgs.trim();
        if (finalArgs.startsWith("wine64 ")) finalArgs = finalArgs.substring(7).trim();
        else if (finalArgs.startsWith("wine ")) finalArgs = finalArgs.substring(5).trim();

        String command = "";
        if (!isArm64EC) {
            File wineAbsPath = new File(wineBinDirAbs, "wine64");
            if (!wineAbsPath.exists()) wineAbsPath = new File(wineBinDirAbs, "wine");
            command = new File(rootDir, "/usr/local/bin/box64").getPath() + " " + wineAbsPath.getPath() + " " + finalArgs;
        } else {
            File ldLoader = new File(rootDir, "usr/lib/ld-linux-aarch64.so.1");
            File wineAbsPath = new File(wineBinDirAbs, "wine");
            command = ldLoader.getPath() + " " + wineAbsPath.getPath() + " " + finalArgs;
        }

        Log.d("Winlator", "Executing command: " + command);

        return ProcessHelper.exec(command, envVars.toStringArray(), rootDir, (status) -> {
            synchronized (lock) {
                pid = -1;
            }
            if (terminationCallback != null) terminationCallback.call(status);
        }, logFilePath);
    }

    private void addFEXEnvVars(EnvVars envVars) {
        if (fexPreset == 0) {
            envVars.put("HODLL", "libwow64fex.dll");
        } else if (fexPreset == 1) {
            envVars.put("HODLL", "wowbox64.dll");
        } else {
            envVars.remove("HODLL");
        }
        
        if (fexPresetCustom != null && !fexPresetCustom.isEmpty()) {
            envVars.putAll(FEXPresetManager.getEnvVars(environment.getContext(), fexPresetCustom));
        }
    }

    protected void extractBox64Files() {
        ImageFs imageFs = environment.getImageFs();
        Context context = environment.getContext();
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String box64Version = preferences.getString("box64_version", DefaultVersion.BOX64);
        String currentBox64Version = preferences.getString("current_box64_version", "");
        File rootDir = imageFs.getRootDir();

        if (!box64Version.equals(currentBox64Version)) {
            ContentProfile profile = contentsManager.getProfileByEntryName("box64-" + box64Version);
            if (profile != null)
                contentsManager.applyContent(profile);
            else
                TarCompressorUtils.extract(TarCompressorUtils.Type.ZSTD, context, "box64/box64-" + box64Version + ".tzst", rootDir);
            preferences.edit().putString("current_box64_version", box64Version).apply();
        }
    }

    protected void addBox64EnvVars(EnvVars envVars, boolean enableLogs) {
        envVars.put("BOX64_NOBANNER", ProcessHelper.PRINT_DEBUG && enableLogs ? "0" : "1");
        envVars.put("BOX64_DYNAREC", "1");
        envVars.put("BOX64_MMAP32", "1");

        if (enableLogs) {
            envVars.put("BOX64_LOG", "1");
            envVars.put("BOX64_DYNAREC_MISSING", "1");
        }

        envVars.putAll(Box64PresetManager.getEnvVars(environment.getContext(), box64Preset));
        envVars.put("BOX64_X11GLX", "1");
    }

    public void suspendProcess() {
        synchronized (lock) {
            if (pid != -1) ProcessHelper.suspendProcess(pid);
        }
    }

    public void resumeProcess() {
        synchronized (lock) {
            if (pid != -1) ProcessHelper.resumeProcess(pid);
        }
    }
}
