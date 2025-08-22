package com.winlator.win32;

import com.winlator.core.ArrayUtils;
import com.winlator.core.FileUtils;
import com.winlator.core.StringUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public abstract class MSLink {
    public static final byte SW_SHOWNORMAL = 1;
    public static final byte SW_SHOWMAXIMIZED = 3;
    public static final byte SW_SHOWMINNOACTIVE = 7;

    private static final int HasLinkTargetIDList = 1<<0;
    private static final int HasLinkInfo = 1<<1;
    private static final int HasName = 1<<2;
    private static final int HasRelativePath = 1<<3;
    private static final int HasWorkingDir = 1<<4;
    private static final int HasArguments = 1<<5;
    private static final int HasIconLocation = 1<<6;
    private static final int IsUnicode = 1<<7;
    private static final int ForceNoLinkInfo = 1<<8;

    private static final int VolumeIDAndLocalBasePath = 1<<0;
    private static final int CommonNetworkRelativeLinkAndPathSuffix = 1<<1;

    public static final class LinkInfo {
        public String targetPath;
        public String arguments;
        public String iconLocation;
        public int iconIndex;
        public int fileSize;
        public int showCommand = SW_SHOWNORMAL;
        public boolean isDirectory;
    }

    private static int charToHexDigit(char chr) {
        return chr >= 'A' ? chr - 'A' + 10 : chr - '0';
    }

    private static byte twoCharsToByte(char chr1, char chr2) {
        return (byte)(charToHexDigit(Character.toUpperCase(chr1)) * 16 + charToHexDigit(Character.toUpperCase(chr2)));
    }

    private static byte[] convertCLSIDtoDATA(String str) {
        return new byte[]{
            twoCharsToByte(str.charAt(6), str.charAt(7)),
            twoCharsToByte(str.charAt(4), str.charAt(5)),
            twoCharsToByte(str.charAt(2), str.charAt(3)),
            twoCharsToByte(str.charAt(0), str.charAt(1)),
            twoCharsToByte(str.charAt(11), str.charAt(12)),
            twoCharsToByte(str.charAt(9), str.charAt(10)),
            twoCharsToByte(str.charAt(16), str.charAt(17)),
            twoCharsToByte(str.charAt(14), str.charAt(15)),
            twoCharsToByte(str.charAt(19), str.charAt(20)),
            twoCharsToByte(str.charAt(21), str.charAt(22)),
            twoCharsToByte(str.charAt(24), str.charAt(25)),
            twoCharsToByte(str.charAt(26), str.charAt(27)),
            twoCharsToByte(str.charAt(28), str.charAt(29)),
            twoCharsToByte(str.charAt(30), str.charAt(31)),
            twoCharsToByte(str.charAt(32), str.charAt(33)),
            twoCharsToByte(str.charAt(34), str.charAt(35))
        };
    }

    private static byte[] stringToByteArray(String str) {
        byte[] bytes = new byte[str.length()];
        for (int i = 0; i < bytes.length; i++) bytes[i] = (byte)str.charAt(i);
        return bytes;
    }

    private static byte[] intToByteArray(int value) {
        return ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(value).array();
    }

    private static byte[] generateStringData(String str) {
        ByteBuffer buffer = ByteBuffer.allocate(str.length() + 2).order(ByteOrder.LITTLE_ENDIAN);
        buffer.putShort((short)str.length());
        for (int i = 0; i < str.length(); i++) buffer.put((byte)str.charAt(i));
        return buffer.array();
    }

    private static byte[] generateIDLIST(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN).putShort((short)(bytes.length + 2));
        return ArrayUtils.concat(buffer.array(), bytes);
    }

    private static String readStringData(ByteBuffer data, boolean isUnicode) {
        short CountCharacters = data.getShort();
        if (CountCharacters == 0) return null;

        byte[] bytes = new byte[CountCharacters * (isUnicode ? 2 : 1)];
        data.get(bytes);
        String string = isUnicode ? new String(bytes, StandardCharsets.UTF_16LE) : new String(bytes);
        int indexOfNull = string.indexOf('\0');
        return indexOfNull != -1 ? string.substring(0, indexOfNull) : string;
    }

    private static String readNullTerminatedString(ByteBuffer data) {
        byte[] bytes = new byte[256];
        int i = 0;
        byte value;
        while ((value = data.get()) != 0) bytes[i++] = value;
        return new String(Arrays.copyOf(bytes, i));
    }

    public static boolean createFile(String targetPath, File outputFile) {
        LinkInfo linkInfo = new LinkInfo();
        linkInfo.targetPath = targetPath;
        return createFile(linkInfo, outputFile);
    }

    public static boolean createFile(LinkInfo linkInfo, File outputFile) {
        byte[] HeaderSize = new byte[]{0x4c, 0x00, 0x00, 0x00};
        byte[] LinkCLSID = convertCLSIDtoDATA("00021401-0000-0000-c000-000000000046");

        int linkFlags = HasLinkTargetIDList | ForceNoLinkInfo;
        if (linkInfo.arguments != null && !linkInfo.arguments.isEmpty()) linkFlags |= HasArguments;
        if (linkInfo.iconLocation != null && !linkInfo.iconLocation.isEmpty()) linkFlags |= HasIconLocation;

        byte[] LinkFlags = intToByteArray(linkFlags);

        byte[] FileAttributes, prefixOfTarget;
        linkInfo.targetPath = linkInfo.targetPath.replaceAll("/+", "\\\\");
        if (linkInfo.isDirectory) {
            FileAttributes = new byte[]{0x10, 0x00, 0x00, 0x00};
            prefixOfTarget = new byte[]{0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        }
        else {
            FileAttributes = new byte[]{0x20, 0x00, 0x00, 0x00};
            prefixOfTarget = new byte[]{0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        }

        byte[] CreationTime, AccessTime, WriteTime;
        CreationTime = AccessTime = WriteTime = new byte[]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        byte[] FileSize = intToByteArray(linkInfo.fileSize);
        byte[] IconIndex = intToByteArray(linkInfo.iconIndex);
        byte[] ShowCommand = intToByteArray(linkInfo.showCommand);
        byte[] Hotkey = new byte[]{0x00, 0x00};
        byte[] Reserved1 = new byte[]{0x00, 0x00};
        byte[] Reserved2 = new byte[]{0x00, 0x00, 0x00, 0x00};
        byte[] Reserved3 = new byte[]{0x00, 0x00, 0x00, 0x00};

        byte[] CLSIDComputer = convertCLSIDtoDATA("20d04fe0-3aea-1069-a2d8-08002b30309d");
        byte[] CLSIDNetwork = convertCLSIDtoDATA("208d2c60-3aea-1069-a2d7-08002b30309d");

        byte[] itemData, prefixRoot, targetRoot, targetLeaf;
        if (linkInfo.targetPath.startsWith("\\")) {
            prefixRoot = new byte[]{(byte)0xc3, 0x01, (byte)0x81};
            targetRoot = stringToByteArray(linkInfo.targetPath);
            targetLeaf = !linkInfo.targetPath.endsWith("\\") ? stringToByteArray(linkInfo.targetPath.substring(linkInfo.targetPath.lastIndexOf("\\") + 1)) : null;
            itemData = ArrayUtils.concat(new byte[]{0x1f, 0x58}, CLSIDNetwork);
        }
        else {
            prefixRoot = new byte[]{0x2f};
            int index = linkInfo.targetPath.indexOf("\\");
            targetRoot = stringToByteArray(linkInfo.targetPath.substring(0, index+1));
            targetLeaf = stringToByteArray(linkInfo.targetPath.substring(index+1));
            itemData = ArrayUtils.concat(new byte[]{0x1f, 0x50}, CLSIDComputer);
        }

        targetRoot = ArrayUtils.concat(targetRoot, new byte[21]);

        byte[] endOfString = new byte[]{0x00};
        byte[] IDListItems = ArrayUtils.concat(generateIDLIST(itemData), generateIDLIST(ArrayUtils.concat(prefixRoot, targetRoot, endOfString)));
        if (targetLeaf != null) IDListItems = ArrayUtils.concat(IDListItems, generateIDLIST(ArrayUtils.concat(prefixOfTarget, targetLeaf, endOfString)));
        byte[] IDList = generateIDLIST(IDListItems);

        byte[] TerminalID = new byte[]{0x00, 0x00};

        byte[] StringData = new byte[0];
        if ((linkFlags & HasArguments) != 0) StringData = ArrayUtils.concat(StringData, generateStringData(linkInfo.arguments));
        if ((linkFlags & HasIconLocation) != 0) StringData = ArrayUtils.concat(StringData, generateStringData(linkInfo.iconLocation));

        try (FileOutputStream os = new FileOutputStream(outputFile)) {
            os.write(HeaderSize);
            os.write(LinkCLSID);
            os.write(LinkFlags);
            os.write(FileAttributes);
            os.write(CreationTime);
            os.write(AccessTime);
            os.write(WriteTime);
            os.write(FileSize);
            os.write(IconIndex);
            os.write(ShowCommand);
            os.write(Hotkey);
            os.write(Reserved1);
            os.write(Reserved2);
            os.write(Reserved3);
            os.write(IDList);
            os.write(TerminalID);

            if (StringData.length > 0) os.write(StringData);
            return true;
        }
        catch (IOException e) {
            return false;
        }
    }

    public static LinkInfo extractLinkInfo(File linkFile) {
        byte[] bytes = FileUtils.read(linkFile);
        if (bytes == null) return null;

        ByteBuffer data = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
        if (data.get() != 0x4c || data.get() != 0x00 || data.get() != 0x00 || data.get() != 0x00) return null;

        int LinkFlags = data.getInt(0x0014);
        int IconIndex = data.getInt(0x0038);
        data.position(0x004c);

        String driveLetter = "";
        String path = "";
        boolean isDirectory = false;
        LinkInfo linkInfo = null;

        if ((LinkFlags & HasLinkTargetIDList) != 0) {
            short IDListSize = data.getShort();
            final byte[] prefixOfDirectory = new byte[]{0x31, 0x00, 0x00, 0x00, 0x00, 0x00};
            final byte[] prefixOfArchive = new byte[]{0x32, 0x00, 0x00, 0x00, 0x00, 0x00};

            while (IDListSize > 2) {
                short ItemIDSize = data.getShort();
                byte[] ItemData = new byte[ItemIDSize - 2];
                data.get(ItemData);

                if (ArrayUtils.startsWith(prefixOfDirectory, ItemData)) {
                    String filename = StringUtils.fromANSIString(Arrays.copyOfRange(ItemData, 12, ItemData.length));
                    path += (!path.isEmpty() ? "\\" : "")+filename;
                    isDirectory = true;
                }
                else if (ArrayUtils.startsWith(prefixOfArchive, ItemData)) {
                    String filename = StringUtils.fromANSIString(Arrays.copyOfRange(ItemData, 12, ItemData.length));
                    path += (!path.isEmpty() ? "\\" : "")+filename;
                    isDirectory = false;
                }
                else if (ItemData[0] == 0x2f || ItemData[0] == 0x23) {
                    driveLetter = StringUtils.fromANSIString(Arrays.copyOfRange(ItemData, 1, ItemData.length));
                }

                IDListSize -= ItemIDSize;
            }

            data.getShort();
        }

        if ((LinkFlags & HasLinkInfo) != 0) {
            int oldPosition = data.position();

            int LinkInfoSize = data.getInt();
            int LinkInfoHeaderSize = data.getInt();
            int LinkInfoFlags = data.getInt();

            if (LinkInfoHeaderSize < 0x00000024 && (LinkInfoFlags & CommonNetworkRelativeLinkAndPathSuffix) == 0) {
                int VolumeIDOffset = data.getInt();
                int LocalBasePathOffset = data.getInt();
                int CommonNetworkRelativeLinkOffset = data.getInt();
                int CommonPathSuffixOffset = data.getInt();

                if ((LinkInfoFlags & VolumeIDAndLocalBasePath) != 0) {
                    data.position(data.position() + 17);
                    String LocalBasePath = readNullTerminatedString(data);
                    if (linkInfo == null) linkInfo = new LinkInfo();
                    linkInfo.targetPath = LocalBasePath;
                }

                data.get();
            }
            else data.position(oldPosition + LinkInfoSize);
        }

        if (driveLetter.matches("[A-Za-z]:\\\\?")) {
            if (!driveLetter.endsWith("\\")) driveLetter += "\\";
            if (linkInfo == null) linkInfo = new LinkInfo();
            linkInfo.targetPath = driveLetter+path;
            linkInfo.isDirectory = isDirectory;
            linkInfo.iconIndex = IconIndex != 0 ? Math.abs(IconIndex) + 1 : -1;
        }

        boolean isUnicode = (LinkFlags & IsUnicode) != 0;

        if ((LinkFlags & HasName) != 0) {
            String name = readStringData(data, isUnicode);
        }

        if ((LinkFlags & HasRelativePath) != 0) {
            String relativePath = readStringData(data, isUnicode);
        }

        if ((LinkFlags & HasWorkingDir) != 0) {
            String workingDir = readStringData(data, isUnicode);
        }

        if ((LinkFlags & HasArguments) != 0) {
            String arguments = readStringData(data, isUnicode);
            if (linkInfo != null) linkInfo.arguments = arguments;
        }

        if ((LinkFlags & HasIconLocation) != 0) {
            String iconLocation = readStringData(data, isUnicode);
            if (linkInfo != null) {
                if (iconLocation != null && iconLocation.equals("shell32.dll")) iconLocation = "C:/windows/system32/shell32.dll";
                linkInfo.iconLocation = iconLocation;
            }
        }

        return linkInfo;
    }
}
