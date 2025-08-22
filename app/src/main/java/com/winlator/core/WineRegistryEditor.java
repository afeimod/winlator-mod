package com.winlator.core;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.winlator.math.Mathf;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.Closeable;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Locale;

public class WineRegistryEditor implements Closeable {
    private final File file;
    private final File cloneFile;
    private boolean modified = false;
    private boolean createKeyIfNotExist = true;

    static {
        System.loadLibrary("winlator");
    }

    public static class Location {
        public final int offset;
        public final int start;
        public final int end;
        public int mbCount;
        private Object tag;

        public Location(int offset, int start, int end) {
            this.offset = offset;
            this.start = start;
            this.end = end;
        }

        public int length() {
            return end - start;
        }

        @NonNull
        @Override
        public String toString() {
            return offset+","+start+","+end;
        }

        public int[] toIntArray() {
            return new int[]{offset, start, end, mbCount};
        }

        @Override
        public boolean equals(@Nullable Object obj) {
            if (!(obj instanceof Location)) return false;
            Location other = (Location)obj;
            return this.offset == other.offset && this.start == other.start && this.end == other.end;
        }
    }

    public WineRegistryEditor(File file) {
        this.file = file;
        cloneFile = FileUtils.createTempFile(file.getParentFile(), FileUtils.getBasename(file.getPath()));
        if (!file.isFile()) {
            try {
                cloneFile.createNewFile();
            }
            catch (IOException e) {}
        }
        else FileUtils.copy(file, cloneFile);
    }

    private static String escape(String str) {
        return str.replace("\\", "\\\\").replace("\"", "\\\"");
    }

    private static String unescape(String str) {
        return str.replace("\\\"", "\"").replace("\\\\", "\\");
    }

    @Override
    public void close() {
        if (modified && cloneFile.exists()) {
            cloneFile.renameTo(file);
        }
        else cloneFile.delete();
    }

    public void setCreateKeyIfNotExist(boolean createKeyIfNotExist) {
        this.createKeyIfNotExist = createKeyIfNotExist;
    }

    private Location createKey(String key) {
        Location location = getParentKeyLocation(key);
        boolean success = false;

        char[] buffer = new char[StreamUtils.BUFFER_SIZE];
        File tempFile = FileUtils.createTempFile(file.getParentFile(), FileUtils.getBasename(file.getPath()));

        try (BufferedReader reader = new BufferedReader(new FileReader(cloneFile), StreamUtils.BUFFER_SIZE);
             BufferedWriter writer = new BufferedWriter(new FileWriter(tempFile), StreamUtils.BUFFER_SIZE)) {

            int length;
            if (location != null) {
                for (int i = 0, end = location.end+1; i < end; i += length) {
                    length = Math.min(buffer.length, end - i);
                    reader.read(buffer, 0, length);
                    writer.write(buffer, 0, length);
                }
            }
            else while ((length = reader.read(buffer)) != -1) writer.write(buffer, 0, length);

            long ticks1601To1970 = 86400L * (369 * 365 + 89) * 10000000;
            long currentTime = System.currentTimeMillis() + ticks1601To1970;
            String content = "\n["+escape(key)+"] "+((currentTime - ticks1601To1970) / 1000) +
                              String.format(Locale.ENGLISH, "\n#time=%x%08x", currentTime >> 32, (int)currentTime)+"\n";
            writer.write(content);

            while ((length = reader.read(buffer)) != -1) writer.write(buffer, 0, length);
            success = true;
        }
        catch (IOException e) {}

        if (success) {
            modified = true;
            tempFile.renameTo(cloneFile);
            return getKeyLocation(key);
        }
        else {
            tempFile.delete();
            return null;
        }
    }

    public String getStringValue(String key, String name) {
        return getStringValue(key, name, null);
    }

    public String getStringValue(String key, String name, String fallback) {
        String value = getRawValue(key, name);
        return value != null ? value.substring(1, value.length() - 1) : fallback;
    }

    public void setStringValue(String key, String name, String value) {
        setRawValue(key, name, value != null ? "\""+escape(value)+"\"" : "\"\"");
    }

    public void setStringValues(String key, String[]... items) {
        String[][] escapedItems = new String[items.length][];
        for (int i = 0; i < items.length; i++) {
            escapedItems[i] = new String[]{items[i][0], items[i][1] != null ? "\""+escape(items[i][1])+"\"" : "\"\""};
        }
        setRawValues(key, escapedItems);
    }

    public Integer getDwordValue(String key, String name) {
        return getDwordValue(key, name, null);
    }

    public Integer getDwordValue(String key, String name, Integer fallback) {
        String value = getRawValue(key, name);
        return value != null ? Integer.decode("0x" + value.substring(6)) : fallback;
    }

    public void setDwordValue(String key, String name, int value) {
        setRawValue(key, name, "dword:"+String.format("%08x", value));
    }

    public void setHexValue(String key, String name, String value) {
        int start = (int)Mathf.roundTo(name.length(), 2) + 7;
        StringBuilder lines = new StringBuilder();
        for (int i = 0, j = start; i < value.length(); i++) {
            if (i > 0 && (i % 2) == 0) lines.append(",");
            if (j++ > 56) {
                lines.append("\\\n  ");
                j = 8;
            }
            lines.append(value.charAt(i));
        }
        setRawValue(key, name, "hex:"+lines);
    }

    public void setHexValues(String key, String name, byte[] bytes) {
        StringBuilder data = new StringBuilder();
        for (byte b : bytes) data.append(String.format(Locale.ENGLISH, "%02x", Byte.toUnsignedInt(b)));
        setHexValue(key, name, data.toString());
    }

    public byte[] getHexValues(String key, String name) {
        String value = getRawValue(key, name);
        if (value != null && value.startsWith("hex:")) {
            String[] items = value.replace("hex:", "").replace("\\\n  ", "").split(",");
            byte[] bytes = new byte[items.length];
            for (int i = 0; i < items.length; i++) {
                try {
                    bytes[i] = Integer.decode("0x"+items[i]).byteValue();
                }
                catch (NumberFormatException e) {}
            }
            return bytes;
        }
        return null;
    }

    private String getRawValue(String key, String name) {
        Location keyLocation = getKeyLocation(key);
        if (keyLocation == null) return null;

        Location valueLocation = getValueLocation(keyLocation, name);
        if (valueLocation == null) return null;
        boolean success = false;
        char[] buffer = new char[valueLocation.length()];

        try (BufferedReader reader = new BufferedReader(new FileReader(cloneFile), StreamUtils.BUFFER_SIZE)) {
            reader.skip(valueLocation.start);
            success = reader.read(buffer) == buffer.length;
        }
        catch (IOException e) {}
        return success ? unescape(new String(buffer)) : null;
    }

    private void setRawValue(String key, String name, String value) {
        Location keyLocation = getKeyLocation(key);
        if (keyLocation == null) {
            if (createKeyIfNotExist) {
                keyLocation = createKey(key);
            }
            else return;
        }

        Location valueLocation = getValueLocation(keyLocation, name);
        char[] buffer = new char[StreamUtils.BUFFER_SIZE];
        boolean success = false;

        File tempFile = FileUtils.createTempFile(file.getParentFile(), FileUtils.getBasename(file.getPath()));

        try (BufferedReader reader = new BufferedReader(new FileReader(cloneFile), StreamUtils.BUFFER_SIZE);
             BufferedWriter writer = new BufferedWriter(new FileWriter(tempFile), StreamUtils.BUFFER_SIZE)) {

            int length;
            for (int i = 0, end = valueLocation != null ? valueLocation.start : keyLocation.end; i < end; i += length) {
                length = Math.min(buffer.length, end - i);
                reader.read(buffer, 0, length);
                writer.write(buffer, 0, length);
            }

            if (valueLocation == null) {
                writer.write("\n"+(name != null ? "\""+escape(name)+"\"" : "@")+"="+value);
            }
            else {
                writer.write(value);
                reader.skip(valueLocation.length());
            }

            while ((length = reader.read(buffer)) != -1) writer.write(buffer, 0, length);
            success = true;
        }
        catch (IOException e) {}

        if (success) {
            modified = true;
            tempFile.renameTo(cloneFile);
        }
        else tempFile.delete();
    }

    private void setRawValues(String key, String[]... items) {
        Location keyLocation = getKeyLocation(key);
        if (keyLocation == null) {
            if (createKeyIfNotExist) {
                keyLocation = createKey(key);
            }
            else return;
        }

        ArrayList<Location> valueLocations = new ArrayList<>();
        for (int i = 0; i < items.length; i++) {
            Location valueLocation = getValueLocation(keyLocation, items[i][0]);
            if (valueLocation == null) valueLocation = new Location(0, Integer.MAX_VALUE - items.length + i, -1);
            valueLocation.tag = items[i];
            valueLocations.add(valueLocation);
        }

        valueLocations.sort(Comparator.comparingInt((o) -> o.start));

        char[] buffer = new char[StreamUtils.BUFFER_SIZE];
        boolean success = false;

        File tempFile = FileUtils.createTempFile(file.getParentFile(), FileUtils.getBasename(file.getPath()));

        try (BufferedReader reader = new BufferedReader(new FileReader(cloneFile), StreamUtils.BUFFER_SIZE);
             BufferedWriter writer = new BufferedWriter(new FileWriter(tempFile), StreamUtils.BUFFER_SIZE)) {

            int length;
            int position = 0;
            for (Location valueLocation : valueLocations) {
                if (valueLocation.end == -1) continue;
                for (int i = position; i < valueLocation.start; i += length) {
                    length = Math.min(buffer.length, valueLocation.start - i);
                    reader.read(buffer, 0, length);
                    writer.write(buffer, 0, length);
                    position += length;
                }

                String[] item = (String[])valueLocation.tag;
                writer.write(item[1]);
                reader.skip(valueLocation.length());
                position += valueLocation.length();
            }

            for (int i = position; i < keyLocation.end; i += length) {
                length = Math.min(buffer.length, keyLocation.end - i);
                reader.read(buffer, 0, length);
                writer.write(buffer, 0, length);
            }

            for (Location valueLocation : valueLocations) {
                if (valueLocation.end != -1) continue;
                String[] item = (String[])valueLocation.tag;
                writer.write("\n"+(item[0] != null ? "\""+escape(item[0])+"\"" : "@")+"="+item[1]);
            }

            while ((length = reader.read(buffer)) != -1) writer.write(buffer, 0, length);
            success = true;
        }
        catch (IOException e) {}

        if (success) {
            modified = true;
            tempFile.renameTo(cloneFile);
        }
        else tempFile.delete();
    }

    public void removeValue(String key, String name) {
        Location keyLocation = getKeyLocation(key);
        if (keyLocation == null) return;

        Location valueLocation = getValueLocation(keyLocation, name);
        if (valueLocation == null) return;
        removeRegion(valueLocation);
    }

    public boolean removeKey(String key) {
        return removeKey(key, false);
    }

    public boolean removeKey(String key, boolean removeTree) {
        boolean removed = false;
        if (removeTree) {
            Location location;
            while ((location = getKeyLocation(key, true)) != null) {
                if (removeRegion(location)) removed = true;
            }
        }
        else {
            Location location = getKeyLocation(key, false);
            if (location != null && removeRegion(location)) removed = true;
        }
        return removed;
    }

    private boolean removeRegion(Location location) {
        char[] buffer = new char[StreamUtils.BUFFER_SIZE];
        boolean success = false;

        File tempFile = FileUtils.createTempFile(file.getParentFile(), FileUtils.getBasename(file.getPath()));

        try (BufferedReader reader = new BufferedReader(new FileReader(cloneFile), StreamUtils.BUFFER_SIZE);
             BufferedWriter writer = new BufferedWriter(new FileWriter(tempFile), StreamUtils.BUFFER_SIZE)) {

            int length = 0;
            for (int i = 0; i < location.offset; i += length) {
                length = Math.min(buffer.length, location.offset - i);
                reader.read(buffer, 0, length);
                writer.write(buffer, 0, length);
            }

            boolean skipLine = length > 1 && buffer[length-1] == '\n';
            reader.skip(location.end - location.offset + (skipLine ? 1 : 0));
            while ((length = reader.read(buffer)) != -1) writer.write(buffer, 0, length);
            success = true;
        }
        catch (IOException e) {}

        if (success) {
            modified = true;
            tempFile.renameTo(cloneFile);
        }
        else tempFile.delete();
        return success;
    }

    private Location getParentKeyLocation(String key) {
        String[] parts = key.split("\\\\");
        ArrayList<String> stack = new ArrayList<>(Arrays.asList(parts).subList(0, parts.length - 1));

        while (!stack.isEmpty()) {
            String currentKey = String.join("\\", stack);
            Location location = getKeyLocation(currentKey, true);
            if (location != null) return location;
            stack.remove(stack.size()-1);
        }

        return null;
    }

    private Location getKeyLocation(String key) {
        return getKeyLocation(key, false);
    }

    private Location getKeyLocation(String key, boolean keyAsPrefix) {
        if (!cloneFile.isFile()) return null;
        key = "["+escape(key)+(!keyAsPrefix ? "]" : "");
        int[] result = getKeyLocation(cloneFile.getPath(), key);
        Location location = new Location(result[0], result[1], result[2]);
        location.mbCount = result[3];
        return location.start != -1 ? location : null;
    }

    private Location getValueLocation(Location keyLocation, String name) {
        if (!cloneFile.isFile() || keyLocation.start == keyLocation.end) return null;
        name = name != null ? "\""+escape(name)+"\"=" : "@=";
        int[] result = getValueLocation(cloneFile.getPath(), keyLocation.toIntArray(), name);
        Location location = new Location(result[0], result[1], result[2]);
        return location.start != -1 ? location : null;
    }

    private native int[] getKeyLocation(String filename, String key);

    private native int[] getValueLocation(String filename, int[] keyLocation, String name);
}
