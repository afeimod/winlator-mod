package com.winlator.xenvironment;

import android.content.Context;

import androidx.annotation.NonNull;

import com.winlator.core.FileUtils;

import java.io.File;
import java.io.IOException;
import java.util.Locale;

public class RootFS {
    public static final String USER = "xuser";
    public static final String HOME_PATH = "/home/"+USER;
    public static final String USER_CACHE_PATH = "/home/"+USER+"/.cache";
    public static final String USER_CONFIG_PATH = "/home/"+USER+"/.config";
    public static final String WINEPREFIX = "/home/"+USER+"/.wine";
    private final File rootDir;
    private String winePath = "/opt/wine";

    private RootFS(File rootDir) {
        this.rootDir = rootDir;
    }

    public static RootFS find(Context context) {
        File legacyDir = new File(context.getFilesDir(), "imagefs");
        File rootDir = new File(context.getFilesDir(), "rootfs");
        if (legacyDir.isDirectory()) legacyDir.renameTo(rootDir);
        return new RootFS(rootDir);
    }

    public File getRootDir() {
        return rootDir;
    }

    public boolean isValid() {
        return rootDir.isDirectory() && getRFSVersionFile().exists();
    }

    public int getVersion() {
        File rfsVersionFile = getRFSVersionFile();
        return rfsVersionFile.exists() ? Integer.parseInt(FileUtils.readLines(rfsVersionFile).get(0)) : 0;
    }

    public String getFormattedVersion() {
        return String.format(Locale.ENGLISH, "%.1f", (float)getVersion());
    }

    public void createRFSVersionFile(int version) {
        getImageInfoDir().mkdirs();
        File file = getRFSVersionFile();
        try {
            file.createNewFile();
            FileUtils.writeString(file, String.valueOf(version));
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }

    public String getWinePath() {
        return winePath;
    }

    public void setWinePath(String winePath) {
        this.winePath = FileUtils.toRelativePath(rootDir.getPath(), winePath);
    }

    private File getImageInfoDir() {
        return new File(rootDir, ".winlator");
    }

    public File getRFSVersionFile() {
        return new File(getImageInfoDir(), ".rfs_version");
    }

    public File getInstalledWineDir() {
        return new File(rootDir, "/opt/installed-wine");
    }

    public File getTmpDir() {
        return new File(rootDir, "/tmp");
    }

    public File getLibDir() {
        return new File(rootDir, "/usr/lib");
    }

    @NonNull
    @Override
    public String toString() {
        return rootDir.getPath();
    }

    public static String getDosUserCachePath() {
        return "Z:"+USER_CACHE_PATH.replace("/", "\\");
    }

    public static String getDosUserConfigPath() {
        return "Z:"+USER_CONFIG_PATH.replace("/", "\\");
    }
}
