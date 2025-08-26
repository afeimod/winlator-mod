package com.winlator.core;

import android.content.Context;

import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Locale;

public class StringUtils {
    public static String removeStartSlash(String value) {
        while (value.startsWith("/") || value.startsWith("\\")) value = value.substring(1);
        return value;
    }

    public static String removeEndSlash(String value) {
        while (value.endsWith("/") || value.endsWith("\\")) value = value.substring(0, value.length()-1);
        return value;
    }

    public static String addEndSlash(String value) {
        return value.endsWith("/") ? value : value+"/";
    }

    public static String insert(String text, int index, String value) {
        return text.substring(0, index) + value + text.substring(index);
    }

    public static String replace(String text, int start, int end, String value) {
        return text.substring(0, start) + value + text.substring(end);
    }

    public static String escapeDOSPath(String path) {
        return path.replace("\\", "\\\\").replace(" ", "\\ ");
    }

    public static String unescapeDOSPath(String path) {
        return path.replaceAll("\\\\([^\\\\]+)", "$1").replaceAll("\\\\([^\\\\]+)", "$1").replaceAll("\\\\\\\\", "\\\\").trim();
    }

    public static String parseIdentifier(Object text) {
        return text.toString().toLowerCase(Locale.ENGLISH).replaceAll(" *\\(([^\\)]+)\\)$", "").replaceAll("( \\+ )+| +", "-");
    }

    public static String parseNumber(Object text) {
        return parseNumber(text, "");
    }

    public static String parseMemorySize(Object text) {
        return parseMemorySize(text, "MB");
    }

    public static String parseMemorySize(Object text, String targetUnit) {
        final String[] units = new String[]{"bytes", "KB", "MB", "GB", "TB"};
        String value = text.toString();
        int targetIndex = -1;
        for (int i = 0; i < units.length; i++) {
            if (units[i].equalsIgnoreCase(targetUnit)) {
                targetIndex = i;
                break;
            }
        }

        if (targetIndex != -1) {
            try {
                for (int i = 0; i < units.length; i++) {
                    if (value.endsWith(" "+units[i])) {
                        long number = Long.parseLong(value.replace(" "+units[i], ""));
                        int diff = targetIndex - i;
                        if (diff < 0) {
                            return String.valueOf((long)(number * Math.pow(1024, Math.abs(diff))));
                        }
                        else if (diff > 0) {
                            return String.valueOf(number / Math.pow(1024, diff));
                        }
                        else return String.valueOf(number);
                    }
                }
            }
            catch (NumberFormatException e) {
                return "0";
            }
        }
        return value.matches("[0-9\\.]+") ? value : "0";
    }

    public static String parseNumber(Object text, String fallback) {
        String result = text != null ? text.toString().replaceAll("[^0-9\\.]+", "") : "";
        return !result.isEmpty() ? result : fallback;
    }

    public static String getString(Context context, String resName) {
        try {
            resName = resName.toLowerCase(Locale.ENGLISH);
            int resID = context.getResources().getIdentifier(resName, "string", context.getPackageName());
            return context.getString(resID);
        }
        catch (Exception e) {
            return null;
        }
    }

    public static String formatBytes(long bytes) {
        return formatBytes(bytes, true);
    }

    public static String formatBytes(long bytes, boolean withSuffix) {
        if (bytes <= 0) return "0 bytes";
        final String[] units = new String[]{"bytes", "KB", "MB", "GB", "TB"};
        int digitGroups = (int)(Math.log10(bytes) / Math.log10(1024));
        String suffix = withSuffix ? " "+units[digitGroups] : "";
        return String.format(Locale.ENGLISH, "%.2f", bytes / Math.pow(1024, digitGroups))+suffix;
    }

    public static String fromANSIString(byte[] bytes) {
        return fromANSIString(bytes, null);
    }

    public static String fromANSIString(byte[] bytes, Charset charset) {
        String value = charset != null ? new String(bytes, charset) : new String(bytes);
        int indexOfNull = value.indexOf('\0');
        return indexOfNull != -1 ? value.substring(0, indexOfNull) : value;
    }

    public static String clearReservedChars(String name) {
        if (name == null || name.isEmpty()) return "";
        return name.replaceAll("[\\\\/:*?\"<>\\|]+", "");
    }

    public static String repeat(char chr, int count) {
        final char[] buffer = new char[count];
        Arrays.fill(buffer, chr);
        return new String(buffer);
    }
}
