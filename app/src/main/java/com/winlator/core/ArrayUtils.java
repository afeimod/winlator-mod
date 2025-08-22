package com.winlator.core;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.Arrays;

public abstract class ArrayUtils {
    public static byte[] concat(byte[]... elements) {
        byte[] result = Arrays.copyOf(elements[0], elements[0].length);
        for (int i = 1; i < elements.length; i++) {
            byte[] newArray = Arrays.copyOf(result, result.length + elements[i].length);
            System.arraycopy(elements[i], 0, newArray, result.length, elements[i].length);
            result = newArray;
        }
        return result;
    }

    @SafeVarargs
    public static <T> T[] concat(T[]... elements) {
        T[] result = Arrays.copyOf(elements[0], elements[0].length);
        for (int i = 1; i < elements.length; i++) {
            T[] newArray = Arrays.copyOf(result, result.length + elements[i].length);
            System.arraycopy(elements[i], 0, newArray, result.length, elements[i].length);
            result = newArray;
        }
        return result;
    }

    public static String[] toStringArray(JSONArray data) {
        String[] stringArray = new String[data.length()];
        for (int i = 0; i < data.length(); i++) {
            try {
                stringArray[i] = data.getString(i);
            }
            catch (JSONException e) {}
        }
        return stringArray;
    }

    public static <T extends Comparable<? super T>> boolean equals(T[] a, T[] b) {
        if (a == null && b == null) return true;
        if (a == null || b == null || a.length != b.length) return false;
        for (int i = 0; i < a.length; i++) if (!a[i].equals(b[i])) return false;
        return true;
    }

    public static boolean startsWith(byte[] prefix, byte[] array) {
        if (prefix == null || array == null || array.length < prefix.length) return false;
        for (int i = 0; i < prefix.length; i++) if (array[i] != prefix[i]) return false;
        return true;
    }
}
