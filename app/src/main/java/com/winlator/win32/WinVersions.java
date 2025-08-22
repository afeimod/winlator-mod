package com.winlator.win32;

import android.content.Context;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import androidx.annotation.NonNull;

import com.winlator.container.Container;
import com.winlator.core.WineRegistryEditor;

import java.io.File;
import java.util.concurrent.Executors;

public abstract class WinVersions {
    public static final String DEFAULT_VERSION = "win10";

    public static class WinVersion {
        public final String version;
        public final String description;
        public final String currentVersion;
        public final byte majorVersion;
        public final byte minorVersion;
        public final int buildNumber;
        public final String csdVersion;

        public WinVersion(String version, String description, String currentVersion, int majorVersion, int minorVersion, int buildNumber, String csdVersion) {
            this.version = version;
            this.description = description;
            this.currentVersion = currentVersion;
            this.majorVersion = (byte)majorVersion;
            this.minorVersion = (byte)minorVersion;
            this.buildNumber = buildNumber;
            this.csdVersion = csdVersion;
        }

        @NonNull
        @Override
        public String toString() {
            return description;
        }
    }

    public static WinVersion[] getWinVersions() {
        return new WinVersion[]{
            new WinVersion("win11", "Windows 11", "6.3", 10, 0, 22000, ""),
            new WinVersion("win10", "Windows 10", "6.3", 10, 0, 19043, ""),
            new WinVersion("win81", "Windows 8.1", null, 6, 3, 9600, ""),
            new WinVersion("win8", "Windows 8", null, 6, 2, 9200, ""),
            new WinVersion("win2008r2", "Windows 2008 R2", null, 6, 1, 7601, "Service Pack 1"),
            new WinVersion("win7", "Windows 7", null, 6, 1, 7601, "Service Pack 1"),
            new WinVersion("win2008", "Windows 2008", null, 6, 0, 6002, "Service Pack 2"),
            new WinVersion("vista", "Windows Vista", null, 6, 0, 6002, "Service Pack 2"),
            new WinVersion("win2003", "Windows 2003", null, 5, 2, 3790, "Service Pack 2"),
            new WinVersion("winxp64", "Windows XP 64", null, 5, 2, 3790, "Service Pack 2"),
            new WinVersion("winxp", "Windows XP", null, 5, 1, 2600, "Service Pack 3"),
            new WinVersion("win2k", "Windows 2000", null, 5, 0, 2195, "Service Pack 4"),
        };
    }

    public static void loadSpinner(final Container container, final Spinner sWinVersion) {
        final Context context = sWinVersion.getContext();
        final WinVersions.WinVersion[] winVersions = WinVersions.getWinVersions();

        byte oldPosition = 0;
        for (int i = 0; i < winVersions.length; i++) {
            if (winVersions[i].version.equals(WinVersions.DEFAULT_VERSION)) {
                oldPosition = (byte)i;
                break;
            }
        }

        sWinVersion.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, winVersions));
        sWinVersion.setSelection(oldPosition);
        sWinVersion.setTag(oldPosition);

        if (container != null) {
            final File systemRegFile = new File(container.getRootDir(), ".wine/system.reg");
            if (!systemRegFile.isFile()) return;

            sWinVersion.setEnabled(false);
            Executors.newSingleThreadExecutor().execute(() -> {
                byte position = (byte)sWinVersion.getSelectedItemPosition();
                try (WineRegistryEditor registryEditor = new WineRegistryEditor(systemRegFile)) {
                    String productName = registryEditor.getStringValue("Software\\Microsoft\\Windows NT\\CurrentVersion", "ProductName", "");
                    productName = productName.replaceAll("(Microsoft )|( Pro)", "");

                    for (int i = 0; i < winVersions.length; i++) {
                        if (winVersions[i].description.equals(productName)) {
                            position = (byte)i;
                            break;
                        }
                    }
                }

                final byte newPosition = position;
                sWinVersion.post(() -> {
                    sWinVersion.setSelection(newPosition);
                    sWinVersion.setTag(newPosition);
                    sWinVersion.setEnabled(true);
                });
            });
        }
    }
}
