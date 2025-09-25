package com.winlator.core;

import android.content.Context;

import com.winlator.container.Container;
import com.winlator.container.Drive;
import com.winlator.win32.MSLogFont;
import com.winlator.win32.WinVersions;
import com.winlator.xenvironment.RootFS;
import com.winlator.xenvironment.XEnvironment;
import com.winlator.xenvironment.components.GuestProgramLauncherComponent;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.util.Iterator;
import java.util.Locale;

public abstract class WineUtils {
    public static void createDosdevicesSymlinks(Container container, boolean addDriveCDRom) {
        File rootDir = container.getRootDir();
        String dosdevicesPath = (new File(rootDir, ".wine/dosdevices")).getPath();
        File[] files = (new File(dosdevicesPath)).listFiles();
        if (files != null) for (File file : files) if (file.getName().matches("[a-z]:")) file.delete();

        FileUtils.symlink("../drive_c", dosdevicesPath+"/c:");
        FileUtils.symlink("../../../../", dosdevicesPath+"/z:");

        if (addDriveCDRom) {
            File driveX = new File(rootDir, ".wine/drive_x");
            if (!driveX.isDirectory()) {
                driveX.mkdir();
                FileUtils.chmod(driveX, 0771);
            }

            String serial = String.format(Locale.ENGLISH, "%-8x", (int)'X').replace(' ', '0');
            FileUtils.writeString(new File(driveX, ".windows-serial"), serial+"\n");
            FileUtils.symlink("../drive_x", dosdevicesPath+"/x:");
        }

        for (Drive drive : container.drivesIterator()) {
            File linkTarget = new File(drive.path);
            String path = linkTarget.getAbsolutePath();
            if (!linkTarget.isDirectory() && path.startsWith(AppUtils.INTERNAL_STORAGE)) {
                linkTarget.mkdirs();
                FileUtils.chmod(linkTarget, 0771);
            }
            FileUtils.symlink(path, dosdevicesPath+"/"+drive.letter.toLowerCase(Locale.ENGLISH)+":");
        }
    }

    public static void setSystemFont(WineRegistryEditor userRegistry, String faceName) {
        byte[] fontNormalData = (new MSLogFont()).setFaceName(faceName).toByteArray();
        byte[] fontBoldData = (new MSLogFont()).setFaceName(faceName).setWeight(700).toByteArray();
        userRegistry.setHexValues("Control Panel\\Desktop\\WindowMetrics", "CaptionFont", fontBoldData);
        userRegistry.setHexValues("Control Panel\\Desktop\\WindowMetrics", "IconFont", fontNormalData);
        userRegistry.setHexValues("Control Panel\\Desktop\\WindowMetrics", "MenuFont", fontNormalData);
        userRegistry.setHexValues("Control Panel\\Desktop\\WindowMetrics", "MessageFont", fontNormalData);
        userRegistry.setHexValues("Control Panel\\Desktop\\WindowMetrics", "SmCaptionFont", fontNormalData);
        userRegistry.setHexValues("Control Panel\\Desktop\\WindowMetrics", "StatusFont", fontNormalData);
    }

    public static void applySystemTweaks(Context context, WineInfo wineInfo) {
        File rootDir = RootFS.find(context).getRootDir();

        File userCacheDir = new File(rootDir, RootFS.USER_CACHE_PATH);
        if (!userCacheDir.isDirectory()) userCacheDir.mkdirs();
        File userConfigDir = new File(rootDir, RootFS.USER_CONFIG_PATH);
        if (!userConfigDir.isDirectory()) userConfigDir.mkdirs();

        File systemRegFile = new File(rootDir, RootFS.WINEPREFIX+"/system.reg");
        File userRegFile = new File(rootDir, RootFS.WINEPREFIX+"/user.reg");

        try (WineRegistryEditor registryEditor = new WineRegistryEditor(systemRegFile)) {
            registryEditor.setStringValue("Software\\Wine\\Drives", "x:", "cdrom");
            registryEditor.setStringValue("Software\\Classes\\.reg", null, "REGfile");
            registryEditor.setStringValue("Software\\Classes\\.reg", "Content Type", "application/reg");
            registryEditor.setStringValue("Software\\Classes\\REGfile\\Shell\\Open\\command", null, "C:\\windows\\regedit.exe /C \"%1\"");

            registryEditor.setStringValue("Software\\Classes\\dllfile\\DefaultIcon", null, "shell32.dll,-154");
            registryEditor.setStringValue("Software\\Classes\\lnkfile\\DefaultIcon", null, "shell32.dll,-30");
            registryEditor.setStringValue("Software\\Classes\\inifile\\DefaultIcon", null, "shell32.dll,-151");

            File corefontsAddedFile = new File(userConfigDir, "corefonts.added");
            if (!corefontsAddedFile.isFile()) {
                setupSystemFonts(registryEditor);
                FileUtils.writeString(corefontsAddedFile, String.valueOf(System.currentTimeMillis()));
            }
        }

        final String[] direct3dLibs = {"d3d8", "d3d9", "d3d10", "d3d10_1", "d3d10core", "d3d11", "d3d12", "d3d12core", "ddraw", "dxgi", "wined3d"};
        final String[] inputLibs = {"dinput", "dinput8", "xinput1_1", "xinput1_2", "xinput1_3", "xinput1_4", "xinput9_1_0", "xinputuap"};
        final String dllOverridesKey = "Software\\Wine\\DllOverrides";

        boolean isMainWineVersion = WineInfo.isMainWineVersion(wineInfo.identifier());

        File wineSystem32Dir = new File(rootDir, "/opt/wine/lib/wine/x86_64-windows");
        File wineSysWoW64Dir = new File(rootDir, "/opt/wine/lib/wine/i386-windows");
        File containerSystem32Dir = new File(rootDir, RootFS.WINEPREFIX+"/drive_c/windows/system32");
        File containerSysWoW64Dir = new File(rootDir, RootFS.WINEPREFIX+"/drive_c/windows/syswow64");

        try (WineRegistryEditor registryEditor = new WineRegistryEditor(userRegFile)) {
            for (String name : direct3dLibs) registryEditor.setStringValue(dllOverridesKey, name, "native,builtin");
            for (String name : inputLibs) {
                if (!isMainWineVersion) {
                    registryEditor.setStringValue(dllOverridesKey, name, "native,builtin");

                    FileUtils.copy(new File(wineSysWoW64Dir, name+".dll"), new File(containerSysWoW64Dir, name+".dll"));
                    FileUtils.copy(new File(wineSystem32Dir, name+".dll"), new File(containerSystem32Dir, name+".dll"));
                }
                else registryEditor.setStringValue(dllOverridesKey, name, "builtin,native");
            }

            registryEditor.removeKey("Software\\Winlator\\WFM\\ContextMenu\\7-Zip");
            registryEditor.setStringValue("Software\\Winlator\\WFM\\ContextMenu\\7-Zip", "Open Archive", "Z:\\opt\\apps\\7-Zip\\7zFM.exe \"%FILE%\"");
            registryEditor.setStringValue("Software\\Winlator\\WFM\\ContextMenu\\7-Zip", "Extract Here", "Z:\\opt\\apps\\7-Zip\\7zG.exe x \"%FILE%\" -r -o\"%DIR%\" -y");
            registryEditor.setStringValue("Software\\Winlator\\WFM\\ContextMenu\\7-Zip", "Extract to Folder", "Z:\\opt\\apps\\7-Zip\\7zG.exe x \"%FILE%\" -r -o\"%DIR%\\%BASENAME%\" -y");
            registryEditor.setStringValue("Software\\Wine\\AddonsURL", null, "https://raw.githubusercontent.com/brunodev85/winlator/main/wine_addons/");
            registryEditor.setStringValue("Software\\Wine\\Drivers", "Graphics", "x11");
        }
    }

    public static void changeBrowsersRegistryKey(Container container, boolean useAndroidBrowser) {
        File userRegFile = new File(container.getRootDir(), ".wine/user.reg");

        try (WineRegistryEditor registryEditor = new WineRegistryEditor(userRegFile)) {
            if (useAndroidBrowser) {
                registryEditor.setStringValue("Software\\Wine\\WineBrowser", "Browsers", "C:\\windows\\winhandler.exe /url");
            }
            else registryEditor.setStringValue("Software\\Wine\\WineBrowser", "Browsers", "C:\\windows\\system32\\iexplore.exe");
        }
    }

    public static void overrideWinComponentDlls(Context context, Container container, String wincomponents) {
        final String dllOverridesKey = "Software\\Wine\\DllOverrides";
        File userRegFile = new File(container.getRootDir(), ".wine/user.reg");
        Iterator<String[]> oldWinComponentsIter = new KeyValueSet(container.getExtra("wincomponents", Container.FALLBACK_WINCOMPONENTS)).iterator();

        try (WineRegistryEditor registryEditor = new WineRegistryEditor(userRegFile)) {
            JSONObject wincomponentsJSONObject = new JSONObject(FileUtils.readString(context, "wincomponents/wincomponents.json"));

            for (String[] wincomponent : new KeyValueSet(wincomponents)) {
                if (wincomponent[1].equals(oldWinComponentsIter.next()[1])) continue;
                String identifier = wincomponent[0];
                boolean useNative = wincomponent[1].equals("1");

                JSONObject wincomponentJSONObject = wincomponentsJSONObject.getJSONObject(identifier);
                JSONArray dlnames = wincomponentJSONObject.getJSONArray("dlnames");
                for (int i = 0; i < dlnames.length(); i++) {
                    String dlname = dlnames.getString(i);
                    if (useNative) {
                        registryEditor.setStringValue(dllOverridesKey, dlname, "native,builtin");
                    }
                    else registryEditor.removeValue(dllOverridesKey, dlname);
                }
            }
        }
        catch (JSONException e) {}
    }

    public static void setWinComponentRegistryKeys(File systemRegFile, String identifier, boolean useNative) {
        if (identifier.equals("directsound")) {
            try (WineRegistryEditor registryEditor = new WineRegistryEditor(systemRegFile)) {
                final String key64 = "Software\\Classes\\CLSID\\{083863F1-70DE-11D0-BD40-00A0C911CE86}\\Instance\\{E30629D1-27E5-11CE-875D-00608CB78066}";
                final String key32 = "Software\\Classes\\Wow6432Node\\CLSID\\{083863F1-70DE-11D0-BD40-00A0C911CE86}\\Instance\\{E30629D1-27E5-11CE-875D-00608CB78066}";

                if (useNative) {
                    registryEditor.setStringValue(key32, "CLSID", "{E30629D1-27E5-11CE-875D-00608CB78066}");
                    registryEditor.setHexValue(key32, "FilterData", "02000000000080000100000000000000307069330200000000000000010000000000000000000000307479330000000038000000480000006175647300001000800000aa00389b710100000000001000800000aa00389b71");
                    registryEditor.setStringValue(key32, "FriendlyName", "Wave Audio Renderer");

                    registryEditor.setStringValue(key64, "CLSID", "{E30629D1-27E5-11CE-875D-00608CB78066}");
                    registryEditor.setHexValue(key64, "FilterData", "02000000000080000100000000000000307069330200000000000000010000000000000000000000307479330000000038000000480000006175647300001000800000aa00389b710100000000001000800000aa00389b71");
                    registryEditor.setStringValue(key64, "FriendlyName", "Wave Audio Renderer");
                }
                else {
                    registryEditor.removeKey(key32);
                    registryEditor.removeKey(key64);
                }
            }
        }
        else if (identifier.equals("wmdecoder")) {
            try (WineRegistryEditor registryEditor = new WineRegistryEditor(systemRegFile)) {
                if (useNative) {
                    registryEditor.setStringValue("Software\\Classes\\Wow6432Node\\CLSID\\{2EEB4ADF-4578-4D10-BCA7-BB955F56320A}\\InprocServer32", null, "C:\\windows\\syswow64\\wmadmod.dll");
                    registryEditor.setStringValue("Software\\Classes\\Wow6432Node\\CLSID\\{82D353DF-90BD-4382-8BC2-3F6192B76E34}\\InprocServer32", null, "C:\\windows\\syswow64\\wmvdecod.dll");
                }
                else {
                    registryEditor.setStringValue("Software\\Classes\\Wow6432Node\\CLSID\\{2EEB4ADF-4578-4D10-BCA7-BB955F56320A}\\InprocServer32", null, "C:\\windows\\syswow64\\winegstreamer.dll");
                    registryEditor.setStringValue("Software\\Classes\\Wow6432Node\\CLSID\\{82D353DF-90BD-4382-8BC2-3F6192B76E34}\\InprocServer32", null, "C:\\windows\\syswow64\\winegstreamer.dll");
                }
            }
        }
    }

    public static void updateWineprefix(Context context, final Callback<Integer> terminationCallback) {
        RootFS rootFS = RootFS.find(context);
        final File rootDir = rootFS.getRootDir();
        File tmpDir = rootFS.getTmpDir();
        if (!tmpDir.isDirectory()) tmpDir.mkdir();

        FileUtils.writeString(new File(rootDir, RootFS.WINEPREFIX+"/.update-timestamp"), "0\n");

        EnvVars envVars = new EnvVars();
        envVars.put("WINEPREFIX", RootFS.WINEPREFIX);
        envVars.put("WINEDLLOVERRIDES", "mscoree,mshtml=d");

        XEnvironment environment = new XEnvironment(context, rootFS);
        GuestProgramLauncherComponent guestProgramLauncherComponent = new GuestProgramLauncherComponent();
        guestProgramLauncherComponent.setEnvVars(envVars);
        guestProgramLauncherComponent.setGuestExecutable("wine wineboot -u");
        guestProgramLauncherComponent.setTerminationCallback((status) -> {
            FileUtils.writeString(new File(rootDir, RootFS.WINEPREFIX+"/.update-timestamp"), "disable\n");
            if (terminationCallback != null) terminationCallback.call(status);
        });
        environment.addComponent(guestProgramLauncherComponent);
        environment.startEnvironmentComponents();
    }

    public static boolean isWineprefixWasUpdated(Container container) {
        File file = new File(container.getRootDir(), "/.wine/.update-timestamp");
        String content = FileUtils.readString(file);
        
        if (!content.startsWith("disable")) {
            content = content.replaceAll("[\r\n]+", "");
            try {
                int updateTimestamp = Integer.parseInt(content);
                if (updateTimestamp != 0) return FileUtils.writeString(file, "disable\n");
            }
            catch (NumberFormatException e) {}
        }
        return false;
    }

    public static void changeServicesStatus(Container container, boolean onlyEssential) {
        final byte SERVICE_DISABLED = 4;
        final String[] services = {"BITS:3", "Eventlog:2", "HTTP:3", "LanmanServer:3", "NDIS:2", "PlugPlay:2", "RpcSs:3", "scardsvr:3", "Schedule:3", "Spooler:3", "StiSvc:3", "TermService:3", "winebus:3", "winehid:3", "Winmgmt:3", "wuauserv:3", "winebth:3"};
        File systemRegFile = new File(container.getRootDir(), ".wine/system.reg");

        try (WineRegistryEditor registryEditor = new WineRegistryEditor(systemRegFile)) {
            registryEditor.setCreateKeyIfNotExist(false);

            String controlSetPath = registryEditor.getSymlinkValue("System\\CurrentControlSet", "SymbolicLinkValue");
            if (controlSetPath == null) controlSetPath = "System\\CurrentControlSet";

            for (String service : services) {
                String name = service.substring(0, service.indexOf(":"));
                int value = onlyEssential ? SERVICE_DISABLED : Character.getNumericValue(service.charAt(service.length()-1));
                registryEditor.setDwordValue(controlSetPath+"\\Services\\"+name, "Start", value);
            }
        }
    }

    public static String unixToDOSPath(String unixPath, Container container) {
        String dosPath = "";
        String driveLetter = "";

        for (Drive drive : container.drivesIterator()) {
            if (unixPath.startsWith(drive.path)) {
                driveLetter = drive.letter+":";
                dosPath = unixPath.substring(drive.path.length()).replace("/", "\\");
                break;
            }
        }

        if (dosPath.isEmpty()) {
            int index = unixPath.indexOf("/.wine/drive_c");
            if (index != -1) {
                driveLetter = "C:";
                dosPath = unixPath.substring(index + 14).replace("/", "\\");
            }
        }

        if (!dosPath.startsWith("\\")) dosPath += "\\";
        dosPath = driveLetter+StringUtils.removeEndSlash(dosPath);
        if (dosPath.equals(driveLetter)) dosPath += "\\";
        return dosPath;
    }

    public static String dosToUnixPath(String dosPath, Container container) {
        int index = dosPath.indexOf(":");
        if (index == -1) return "";

        String unixPath = "";
        String driveLetter = dosPath.substring(0, index).toUpperCase(Locale.ENGLISH);
        String relativePath = StringUtils.removeStartSlash(dosPath.substring(index+1).replace("\\", "/"));

        if (driveLetter.equals("C")) {
            unixPath = container.getRootDir()+"/.wine/drive_c/"+relativePath;
        }
        else if (driveLetter.equals("Z")) {
            File rootDir = new File(container.getRootDir(), "../../");
            try {
                unixPath = rootDir.getCanonicalPath()+"/"+relativePath;
            }
            catch (IOException e) {}
        }
        else {
            for (Drive drive : container.drivesIterator()) {
                if (drive.letter.equals(driveLetter)) {
                    unixPath = drive.path+"/"+relativePath;
                    break;
                }
            }
        }

        return unixPath;
    }

    public static void setWinVersion(Container container, int winVersionIdx) {
        WinVersions.WinVersion winVersion = WinVersions.getWinVersions()[winVersionIdx];
        String currentBuild = String.valueOf(winVersion.buildNumber);
        String currentVersion = winVersion.currentVersion != null ? winVersion.currentVersion : winVersion.majorVersion+"."+winVersion.minorVersion;

        File systemRegFile = new File(container.getRootDir(), ".wine/system.reg");
        try (WineRegistryEditor registryEditor = new WineRegistryEditor(systemRegFile)) {
            String key64 = "Software\\Microsoft\\Windows NT\\CurrentVersion";
            String key32 = "Software\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion";

            registryEditor.setStringValue(key32, "CurrentVersion", currentVersion);
            registryEditor.setDwordValue(key32, "CurrentMajorVersionNumber", winVersion.majorVersion);
            registryEditor.setDwordValue(key32, "CurrentMinorVersionNumber", winVersion.minorVersion);
            registryEditor.setStringValue(key32, "CSDVersion", winVersion.csdVersion);
            registryEditor.setStringValue(key32, "CurrentBuild", currentBuild);
            registryEditor.setStringValue(key32, "CurrentBuildNumber", currentBuild);
            registryEditor.setStringValue(key32, "ProductName", "Microsoft "+winVersion.description);

            registryEditor.setStringValue(key64, "CurrentVersion", currentVersion);
            registryEditor.setDwordValue(key64, "CurrentMajorVersionNumber", winVersion.majorVersion);
            registryEditor.setDwordValue(key64, "CurrentMinorVersionNumber", winVersion.minorVersion);
            registryEditor.setStringValue(key64, "CSDVersion", winVersion.csdVersion);
            registryEditor.setStringValue(key64, "CurrentBuild", currentBuild);
            registryEditor.setStringValue(key64, "CurrentBuildNumber", currentBuild);
            registryEditor.setStringValue(key64, "ProductName", "Microsoft "+winVersion.description);
        }
    }

    private static void setupSystemFonts(WineRegistryEditor registryEditor) {
        final String[][] corefonts = {
            {"Andale Mono (TrueType)", "andalemo.ttf"},
            {"Arial (TrueType)", "arial.ttf"},
            {"Arial Black (TrueType)", "ariblk.ttf"},
            {"Arial Bold (TrueType)", "arialbd.ttf"},
            {"Arial Bold Italic (TrueType)", "arialbi.ttf"},
            {"Arial Italic (TrueType)", "ariali.ttf"},
            {"Comic Sans MS (TrueType)", "comic.ttf"},
            {"Comic Sans MS Bold (TrueType)", "comicbd.ttf"},
            {"Courier New (TrueType)", "cour.ttf"},
            {"Courier New Bold (TrueType)", "courbd.ttf"},
            {"Courier New Bold Italic (TrueType)", "courbi.ttf"},
            {"Courier New Italic (TrueType)", "couri.ttf"},
            {"Georgia (TrueType)", "georgia.ttf"},
            {"Georgia Bold (TrueType)", "georgiab.ttf"},
            {"Georgia Bold Italic (TrueType)", "georgiaz.ttf"},
            {"Georgia Italic (TrueType)", "georgiai.ttf"},
            {"Impact (TrueType)", "impact.ttf"},
            {"Times New Roman (TrueType)", "times.ttf"},
            {"Times New Roman Bold (TrueType)", "timesbd.ttf"},
            {"Times New Roman Bold Italic (TrueType)", "timesbi.ttf"},
            {"Times New Roman Italic (TrueType)", "timesi.ttf"},
            {"Trebuchet MS (TrueType)", "trebuc.ttf"},
            {"Trebuchet MS Bold (TrueType)", "trebucbd.ttf"},
            {"Trebuchet MS Bold Italic (TrueType)", "trebucbi.ttf"},
            {"Trebuchet MS Italic (TrueType)", "trebucit.ttf"},
            {"Verdana (TrueType)", "verdana.ttf"},
            {"Verdana Bold (TrueType)", "verdanab.ttf"},
            {"Verdana Bold Italic (TrueType)", "verdanaz.ttf"},
            {"Verdana Italic (TrueType)", "verdanai.ttf"},
            {"Webdings (TrueType)", "webdings.ttf"}
        };

        registryEditor.setStringValues("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts", corefonts);
        registryEditor.setStringValues("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", corefonts);

        final String[][] wineFonts = {
            {"Marlett (TrueType)", "Z:\\opt\\wine\\share\\wine\\fonts\\marlett.ttf"},
            {"Symbol (TrueType)", "Z:\\opt\\wine\\share\\wine\\fonts\\symbol.ttf"},
            {"Tahoma (TrueType)", "Z:\\opt\\wine\\share\\wine\\fonts\\tahoma.ttf"},
            {"Tahoma Bold (TrueType)", "Z:\\opt\\wine\\share\\wine\\fonts\\tahomabd.ttf"},
            {"Wingdings (TrueType)", "Z:\\opt\\wine\\share\\wine\\fonts\\wingding.ttf"}
        };

        registryEditor.setStringValues("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts", wineFonts);
        registryEditor.setStringValues("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", wineFonts);
    }
}
