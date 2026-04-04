package com.winlator.core;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.Arrays;

public abstract class ArrayUtils {
    public static final byte INDEX_NOT_FOUND = -1;

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

    public static boolean contains(int[] array, int value) {
        return indexOf(array, value) != INDEX_NOT_FOUND;
    }

    public static boolean contains(Object[] array, Object value) {
        return indexOf(array, value) != INDEX_NOT_FOUND;
    }

    public static int indexOf(int[] array, int value) {
        if (array == null) return INDEX_NOT_FOUND;
        for (int i = 0; i < array.length; i++) if (value == array[i]) return i;
        return INDEX_NOT_FOUND;
    }

    public static int indexOf(Object[] array, Object value) {
        if (array == null || value == null) return INDEX_NOT_FOUND;
        for (int i = 0; i < array.length; i++) if (value.equals(array[i])) return i;
        return INDEX_NOT_FOUND;
    }

    public static int indexOf(byte[] array, int offset, int length, byte... value) {
        byte first = value[0];
        for (int i = offset; i < length; i++) {
            if (array[i] != first) {
                while (++i < length && array[i] != first);
            }

            if (i < length) {
                int j = i + 1;
                int end = j + value.length - 1;
                for (int k = 1; j < end && array[j] == value[k]; j++, k++);
                if (j == end) return i;
            }
        }
        return INDEX_NOT_FOUND;
    }
}
