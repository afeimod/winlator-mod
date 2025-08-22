package com.winlator.xserver;

import androidx.annotation.NonNull;

import com.winlator.core.ArrayUtils;
import com.winlator.core.StringUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

public class Property {
    public enum Mode {REPLACE, PREPEND, APPEND}
    public enum Format {
        BYTE_ARRAY(8), SHORT_ARRAY(16), INT_ARRAY(32);
        public final byte value;

        Format(int value) {
            this.value = (byte)value;
        }

        public static Format valueOf(int format) {
            switch (format) {
                case 8: return BYTE_ARRAY;
                case 16: return SHORT_ARRAY;
                case 32: return INT_ARRAY;
            }
            return null;
        }
    }
    public final int name;
    public final int type;
    public final Format format;
    public ByteBuffer data;
    private String cachedString;

    public Property(int name, int type, Format format, byte[] data) {
        this.name = name;
        this.type = type;
        this.format = format;
        replace(data);
    }

    public void replace(byte[] data) {
        cachedString = null;
        this.data = ByteBuffer.wrap(data != null ? data : new byte[0]).order(ByteOrder.LITTLE_ENDIAN);
    }

    public void prepend(byte[] values) {
        cachedString = null;
        replace(ArrayUtils.concat(values, this.data.array()));
    }

    public void append(byte[] values) {
        cachedString = null;
        replace(ArrayUtils.concat(this.data.array(), values));
    }

    @NonNull
    @Override
    public String toString() {
        String type = Atom.getName(this.type);
        data.rewind();
        switch (type) {
            case "UTF8_STRING":
                if (cachedString != null) return cachedString;
                cachedString = StringUtils.fromANSIString(data.array(), StandardCharsets.UTF_8);
                return cachedString;
            case "STRING":
                if (cachedString != null) return cachedString;
                cachedString = StringUtils.fromANSIString(data.array(), XServer.LATIN1_CHARSET);
                return cachedString;
            case "ATOM":
                return Atom.getName(data.getInt(0));
            default:
                StringBuilder sb = new StringBuilder();
                for (int i = 0, size = data.capacity() / (format.value >> 3); i < size; i++) {
                    if (i > 0) sb.append(",");
                    switch (format) {
                        case BYTE_ARRAY:
                            sb.append(data.get());
                            break;
                        case SHORT_ARRAY:
                            sb.append(data.getShort());
                            break;
                        case INT_ARRAY:
                            sb.append(data.getInt());
                            break;
                    }
                }
                data.rewind();
                return sb.toString();
        }
    }

    public int getInt(int index) {
        return data.getInt(index * 4);
    }

    public long getLong(int index) {
        return data.getLong(index * 8);
    }

    public String nameAsString() {
        return Atom.getName(name);
    }
}
