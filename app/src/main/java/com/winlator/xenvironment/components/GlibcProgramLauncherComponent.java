package com.winlator.xenvironment.components;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Process;
import android.util.Log;

import androidx.preference.PreferenceManager;

import com.winlator.box86_64.Box86_64Preset;
import com.winlator.box86_64.Box86_64PresetManager;
import com.winlator.contents.ContentProfile;
import com.winlator.contents.ContentsManager;
import com.winlator.core.Callback;
import com.winlator.core.DefaultVersion;
import com.winlator.core.EnvVars;
import com.winlator.core.ProcessHelper;
import com.winlator.core.TarCompressorUtils;
import com.winlator.core.WineInfo;
import com.winlator.xconnector.UnixSocketConfig;
import com.winlator.xenvironment.ImageFs;

import java.io.File;

public class GlibcProgramLauncherComponent extends GuestProgramLauncherComponent {
    private String guestExecutable;
    private static int pid = -1;
    private String[] bindingPaths;
    private EnvVars envVars;
    private String box86Preset = Box86_64Preset.COMPATIBILITY;
    private String box64Preset = Box86_64Preset.COMPATIBILITY;
    private int fexPreset = 0;
    private Callback<Integer> terminationCallback;
    private static final Object lock = new Object();
    private boolean wow64Mode = true;
    private final ContentsManager contentsManager;
    private final String wineVersion;
    private final ContentProfile wineProfile;
    private String logFilePath;

    public GlibcProgramLauncherComponent(ContentsManager contentsManager, String wineVersion) {
        this.contentsManager = contentsManager;
        this.wineVersion = wineVersion;
        this.wineProfile = contentsManager.getProfileByEntryName(wineVersion);
    }

    @Override
    public void start() {
        synchronized (lock) {
            stop();
            extractBox86_64Files();
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

    public boolean isWoW64Mode() {
        return wow64Mode;
    }

    public void setWoW64Mode(boolean wow64Mode) {
        this.wow64Mode = wow64Mode;
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

    public String getBox86Preset() {
        return box86Preset;
    }

    public void setBox86Preset(String box86Preset) {
        this.box86Preset = box86Preset;
    }

    public String getBox64Preset() {
        return box64Preset;
    }

    public void setBox64Preset(String box64Preset) {
        this.box64Preset = box64Preset;
    }

    public void setFexPreset(int fexPreset) {
        this.fexPreset = fexPreset;
    }

    public void setLogFilePath(String logFilePath) {
        this.logFilePath = logFilePath;
    }

    private int execGuestProgram() {
        Context context = environment.getContext();
        ImageFs imageFs = environment.getImageFs();
        File rootDir = imageFs.getRootDir();

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        boolean enableBox86_64Logs = preferences.getBoolean("enable_box86_64_logs", false);

        EnvVars envVars = new EnvVars();
        
        WineInfo wineInfo = WineInfo.fromIdentifier(context, wineVersion);
        boolean isArm64EC = wineInfo.getArch().equals("arm64ec");

        if (!isArm64EC) {
            if (!wow64Mode) addBox86EnvVars(envVars, enableBox86_64Logs);
            addBox64EnvVars(envVars, enableBox86_64Logs);
        } else {
            addFEXEnvVars(envVars);
        }

        envVars.put("HOME", imageFs.home_path);
        envVars.put("USER", ImageFs.USER);
        envVars.put("TMPDIR", imageFs.getRootDir().getPath() + "/tmp");
        envVars.put("DISPLAY", ":0");

        String winePathStr = wineInfo.path;
        if (winePathStr.startsWith("/")) winePathStr = winePathStr.substring(1);
        File wineDirAbs = new File(rootDir, winePathStr);
        File wineBinDirAbs = new File(wineDirAbs, "bin");
        
        envVars.put("PATH", wineBinDirAbs.getPath() + ":" +
                new File(rootDir, "/usr/bin").getPath() + ":" +
                new File(rootDir, "/usr/local/bin").getPath());

        String ldLibraryPath = new File(rootDir, "/usr/lib").getPath();
        if (isArm64EC) {
            File wineUnixLibDir = new File(wineDirAbs, "lib/wine/aarch64-unix");
            ldLibraryPath = wineUnixLibDir.getPath() + ":" + ldLibraryPath;
            envVars.put("WINEDLLPATH", new File(wineDirAbs, "lib/wine").getPath());
        }
        
        envVars.put("LD_LIBRARY_PATH", ldLibraryPath);
        envVars.put("BOX64_LD_LIBRARY_PATH", new File(rootDir, "/usr/lib/x86_64-linux-gnu").getPath());
        envVars.put("ANDROID_SYSVSHM_SERVER", new File(rootDir, UnixSocketConfig.SYSVSHM_SERVER_PATH).getPath());
        envVars.put("FONTCONFIG_PATH", new File(rootDir, "/usr/etc/fonts").getPath());

        if ((new File(imageFs.getGlibc64Dir(), "libandroid-sysvshm.so")).exists() ||
                (new File(imageFs.getGlibc32Dir(), "libandroid-sysvshm.so")).exists())
            envVars.put("LD_PRELOAD", "libandroid-sysvshm.so");
        if (this.envVars != null) envVars.putAll(this.envVars);

        String finalArgs = guestExecutable;
        if (finalArgs.startsWith("wine ")) finalArgs = finalArgs.substring(5);

        String command = "";
        if (!isArm64EC) {
            String wineExecutableName = wineInfo.getExecutable(context, wow64Mode);
            File wineAbsPath = new File(wineBinDirAbs, wineExecutableName);
            command = new File(rootDir, "/usr/local/bin/box64").getPath() + " " + wineAbsPath.getPath() + " " + finalArgs;
        } else {
            // 核心修复：显式使用 ld-linux 加载器启动 wine，绕过 ELF 内部硬编码路径导致的闪退
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
        if (fexPreset == 1) {
            envVars.put("HODLL", "libwow64fex.dll");
        } else {
            envVars.remove("HODLL");
        }
    }

    private void extractBox86_64Files() {
        ImageFs imageFs = environment.getImageFs();
        Context context = environment.getContext();
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String box86Version = preferences.getString("box86_version", DefaultVersion.BOX86);
        String box64Version = preferences.getString("box64_version", DefaultVersion.BOX64);
        String currentBox86Version = preferences.getString("current_box86_version", "");
        String currentBox64Version = preferences.getString("current_box64_version", "");
        File rootDir = imageFs.getRootDir();

        if (wow64Mode) {
            File box86File = new File(rootDir, "/usr/local/bin/box86");
            if (box86File.isFile()) {
                box86File.delete();
                preferences.edit().putString("current_box86_version", "").apply();
            }
        } else if (!box86Version.equals(currentBox86Version)) {
            TarCompressorUtils.extract(TarCompressorUtils.Type.ZSTD, context, "box86_64/box86-" + box86Version + ".tzst", rootDir);
            preferences.edit().putString("current_box86_version", box86Version).apply();
        }

        if (!box64Version.equals(currentBox64Version)) {
            ContentProfile profile = contentsManager.getProfileByEntryName("box64-" + box64Version);
            if (profile != null)
                contentsManager.applyContent(profile);
            else
                TarCompressorUtils.extract(TarCompressorUtils.Type.ZSTD, context, "box86_64/box64-" + box64Version + ".tzst", rootDir);
            preferences.edit().putString("current_box64_version", box64Version).apply();
        }
    }

    private void addBox86EnvVars(EnvVars envVars, boolean enableLogs) {
        envVars.put("BOX86_NOBANNER", ProcessHelper.PRINT_DEBUG && enableLogs ? "0" : "1");
        envVars.put("BOX86_DYNAREC", "1");

        if (enableLogs) {
            envVars.put("BOX86_LOG", "1");
            envVars.put("BOX86_DYNAREC_MISSING", "1");
        }

        envVars.putAll(Box86_64PresetManager.getEnvVars("box86", environment.getContext(), box86Preset));
        envVars.put("BOX86_X11GLX", "1");
    }

    private void addBox64EnvVars(EnvVars envVars, boolean enableLogs) {
        envVars.put("BOX64_NOBANNER", ProcessHelper.PRINT_DEBUG && enableLogs ? "0" : "1");
        envVars.put("BOX64_DYNAREC", "1");
        if (wow64Mode) envVars.put("BOX64_MMAP32", "1");

        if (enableLogs) {
            envVars.put("BOX64_LOG", "1");
            envVars.put("BOX64_DYNAREC_MISSING", "1");
        }

        envVars.putAll(Box86_64PresetManager.getEnvVars("box64", environment.getContext(), box64Preset));
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