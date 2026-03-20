package com.winlator.xenvironment.components;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Process;
import android.util.Log;

import androidx.preference.PreferenceManager;

import com.winlator.box86_64.Box86_64PresetManager;
import com.winlator.contents.ContentProfile;
import com.winlator.contents.ContentsManager;
import com.winlator.core.DefaultVersion;
import com.winlator.core.EnvVars;
import com.winlator.core.ProcessHelper;
import com.winlator.core.TarCompressorUtils;
import com.winlator.core.WineInfo;
import com.winlator.fex.FEXPresetManager;
import com.winlator.xconnector.UnixSocketConfig;
import com.winlator.xenvironment.ImageFs;

import java.io.File;

public class GlibcProgramLauncherComponent extends GuestProgramLauncherComponent {
    private final ContentsManager contentsManager;
    private final String wineVersion;
    private int fexPreset = 0;
    private String fexPresetCustom = "";

    public GlibcProgramLauncherComponent(ContentsManager contentsManager, String wineVersion) {
        this.contentsManager = contentsManager;
        this.wineVersion = wineVersion;
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

    public void setFexPreset(int fexPreset) {
        this.fexPreset = fexPreset;
    }

    public void setFexPresetCustom(String fexPresetCustom) {
        this.fexPresetCustom = fexPresetCustom;
    }

    @Override
    protected int execGuestProgram() {
        Context context = environment.getContext();
        ImageFs imageFs = environment.getImageFs();
        File rootDir = imageFs.getRootDir();

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        boolean enableBox86_64Logs = preferences.getBoolean("enable_box86_64_logs", false);

        EnvVars envVars = new EnvVars();
        
        WineInfo wineInfo = WineInfo.fromIdentifier(context, wineVersion);
        boolean isArm64EC = wineInfo.getArch().equalsIgnoreCase("arm64ec");

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
            // 针对 WCP Wine 深度优化库路径顺序
            ldLibraryPath = wineLib64Dir.getPath() + ":" + wineLibDirAbs.getPath() + ":" + ldLibraryPath;
            
            File wineDllDir = new File(wineLibDirAbs, "wine");
            if (!wineDllDir.exists()) wineDllDir = new File(wineLib64Dir, "wine");
            
            if (wineDllDir.exists()) {
                envVars.put("WINEDLLPATH", wineDllDir.getPath());
                File unix64 = new File(wineDllDir, "x86_64-unix");
                if (unix64.exists()) ldLibraryPath = unix64.getPath() + ":" + ldLibraryPath;
                File unix32 = new File(wineDllDir, "i386-unix");
                if (unix32.exists()) ldLibraryPath = unix32.getPath() + ":" + ldLibraryPath;
            }
        }

        envVars.put("LD_LIBRARY_PATH", ldLibraryPath);
        envVars.put("BOX64_LD_LIBRARY_PATH", new File(rootDir, "/usr/lib/x86_64-linux-gnu").getPath() + ":" + ldLibraryPath);
        envVars.put("ANDROID_SYSVSHM_SERVER", new File(rootDir, UnixSocketConfig.SYSVSHM_SERVER_PATH).getPath());
        envVars.put("FONTCONFIG_PATH", new File(rootDir, "/usr/etc/fonts").getPath());

        if ((new File(imageFs.getGlibc64Dir(), "libandroid-sysvshm.so")).exists() ||
                (new File(imageFs.getGlibc32Dir(), "libandroid-sysvshm.so")).exists())
            envVars.put("LD_PRELOAD", "libandroid-sysvshm.so");
            
        if (this.envVars != null) envVars.putAll(this.envVars);

        String finalArgs = guestExecutable;
        String wineExecutableName = wineInfo.getExecutable(context, wow64Mode);
        
        // 修正参数截取逻辑，增加鲁棒性
        finalArgs = finalArgs.trim();
        if (finalArgs.startsWith("wine64 ")) finalArgs = finalArgs.substring(7).trim();
        else if (finalArgs.startsWith("wine ")) finalArgs = finalArgs.substring(5).trim();

        String command = "";
        if (!isArm64EC) {
            File wineAbsPath = new File(wineBinDirAbs, wineExecutableName);
            // 兜底检查：如果 wine64 不存在，尝试 wine
            if (!wineAbsPath.exists() && wineExecutableName.equals("wine64")) {
                wineAbsPath = new File(wineBinDirAbs, "wine");
            }
            command = new File(rootDir, "/usr/local/bin/box64").getPath() + " " + wineAbsPath.getPath() + " " + finalArgs;
        } else {
            File ldLoader = new File(rootDir, "usr/lib/ld-linux-aarch64.so.1");
            File wineAbsPath = new File(wineBinDirAbs, "wine");
            command = ldLoader.getPath() + " " + wineAbsPath.getPath() + " " + finalArgs;
        }

        Log.d("Winlator", "Executing command: " + command);
        Log.d("Winlator", "LD_LIBRARY_PATH: " + ldLibraryPath);
        Log.d("Winlator", "WINEDLLPATH: " + envVars.get("WINEDLLPATH"));

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
        
        if (fexPresetCustom != null && !fexPresetCustom.isEmpty()) {
            envVars.putAll(FEXPresetManager.getEnvVars(environment.getContext(), fexPresetCustom));
        }
    }

    @Override
    protected void extractBox86_64Files() {
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

    @Override
    protected void addBox86EnvVars(EnvVars envVars, boolean enableLogs) {
        envVars.put("BOX86_NOBANNER", ProcessHelper.PRINT_DEBUG && enableLogs ? "0" : "1");
        envVars.put("BOX86_DYNAREC", "1");

        if (enableLogs) {
            envVars.put("BOX86_LOG", "1");
            envVars.put("BOX86_DYNAREC_MISSING", "1");
        }

        envVars.putAll(Box86_64PresetManager.getEnvVars("box86", environment.getContext(), box86Preset));
        envVars.put("BOX86_X11GLX", "1");
    }

    @Override
    protected void addBox64EnvVars(EnvVars envVars, boolean enableLogs) {
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
}
