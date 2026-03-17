package com.winlator.core;

import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;

import androidx.annotation.NonNull;

import com.winlator.xenvironment.ImageFs;

import java.io.File;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class WineInfo implements Parcelable {
    public static final WineInfo WINE_X86_64 = new WineInfo("9.16", null, "x86_64", "/opt/x86_64-wine");
    public static final WineInfo WINE_ARM64EC = new WineInfo("11.14", null, "arm64ec", "/opt/arm64ec-wine");
    public static final WineInfo MAIN_WINE_VERSION = WINE_X86_64;
    private static final Pattern pattern = Pattern.compile("^wine\\-([0-9\\.]+)\\-?([0-9\\.]+)?\\-(x86|x86_64|arm64ec)$");
    public final String version;
    public final String subversion;
    public final String path;
    private String arch;

    public WineInfo(String version, String arch) {
        this.version = version;
        this.subversion = null;
        this.arch = arch;
        this.path = null;
    }

    public WineInfo(String version, String subversion, String arch, String path) {
        this.version = version;
        this.subversion = subversion != null && !subversion.isEmpty() ? subversion : null;
        this.arch = arch;
        this.path = path;
    }

    private WineInfo(Parcel in) {
        version = in.readString();
        subversion = in.readString();
        arch = in.readString();
        path = in.readString();
    }

    public String getArch() {
        return arch;
    }

    public void setArch(String arch) {
        this.arch = arch;
    }

    public boolean isWin64() {
        return arch.equals("x86_64") || arch.equals("arm64ec");
    }

    public String getExecutable(Context context, boolean wow64Mode) {
        if (isDefaultWine()) {
            File wineBinDir = new File(ImageFs.find(context).getRootDir(), path + "/bin");
            File wineBinFile = new File(wineBinDir, "wine");
            File winePreloaderBinFile = new File(wineBinDir, "wine-preloader");
            FileUtils.copy(new File(wineBinDir, wow64Mode ? "wine-wow64" : "wine32"), wineBinFile);
            FileUtils.copy(new File(wineBinDir, wow64Mode ? "wine-preloader-wow64" : "wine32-preloader"), winePreloaderBinFile);
            FileUtils.chmod(wineBinFile, 0771);
            FileUtils.chmod(winePreloaderBinFile, 0771);
            return wow64Mode ? "wine" : "wine64";
        }
        else return (new File(path, "/bin/wine64")).isFile() ? "wine64" : "wine";
    }

    public boolean isDefaultWine() {
        return this == WINE_X86_64 || this == WINE_ARM64EC || (path != null && (path.equals("/opt/x86_64-wine") || path.equals("/opt/arm64ec-wine")));
    }

    public String identifier() {
        if (this == WINE_X86_64) return "Wine-9.2-x86_64";
        if (this == WINE_ARM64EC) return "Wine-11.14-arm64ec";
        return "wine-"+fullVersion()+"-"+arch;
    }

    public String fullVersion() {
        return version+(subversion != null ? "-"+subversion : "");
    }

    @NonNull
    @Override
    public String toString() {
        return identifier();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<WineInfo> CREATOR = new Parcelable.Creator<WineInfo>() {
        public WineInfo createFromParcel(Parcel in) {
            return new WineInfo(in);
        }

        public WineInfo[] newArray(int size) {
            return new WineInfo[size];
        }
    };

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(version);
        dest.writeString(subversion);
        dest.writeString(arch);
        dest.writeString(path);
    }

    @NonNull
    public static WineInfo fromIdentifier(Context context, String identifier) {
        if (identifier.equals(WINE_X86_64.identifier())) return WINE_X86_64;
        if (identifier.equals(WINE_ARM64EC.identifier())) return WINE_ARM64EC;
        Matcher matcher = pattern.matcher(identifier);
        if (matcher.find()) {
            File installedWineDir = ImageFs.find(context).getInstalledWineDir();
            String path = (new File(installedWineDir, identifier)).getPath();
            return new WineInfo(matcher.group(1), matcher.group(2), matcher.group(3), path);
        }
        else return MAIN_WINE_VERSION;
    }

    public static boolean isMainWineVersion(String wineVersion) {
        return wineVersion == null || wineVersion.equals(WINE_X86_64.identifier()) || wineVersion.equals(WINE_ARM64EC.identifier());
    }
}
