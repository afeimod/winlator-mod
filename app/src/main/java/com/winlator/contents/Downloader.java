package com.winlator.contents;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.atomic.AtomicBoolean;

public class Downloader {
    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public static boolean downloadFile(String address, File file) {
        return downloadFile(address, file, null, null);
    }

    public static boolean downloadFile(String address, File file, OnProgressListener listener, AtomicBoolean isCancelled) {
        HttpURLConnection connection = null;
        InputStream input = null;
        OutputStream output = null;
        try {
            URL url = new URL(address);
            connection = (HttpURLConnection) url.openConnection();
            connection.connect();

            if (connection.getResponseCode() != HttpURLConnection.HTTP_OK) {
                return false;
            }

            int fileLength = connection.getContentLength();
            input = connection.getInputStream();
            output = new FileOutputStream(file);

            byte[] data = new byte[4096];
            long total = 0;
            int count;
            while ((count = input.read(data)) != -1) {
                if (isCancelled != null && isCancelled.get()) {
                    output.close();
                    input.close();
                    if (file.exists()) file.delete();
                    return false;
                }
                total += count;
                if (fileLength > 0 && listener != null) {
                    listener.onProgress((int) (total * 100 / fileLength));
                }
                output.write(data, 0, count);
            }

            output.flush();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            if (file.exists()) file.delete();
            return false;
        } finally {
            try {
                if (output != null) output.close();
                if (input != null) input.close();
            } catch (Exception ignored) {}
            if (connection != null) connection.disconnect();
        }
    }

    public static String downloadString(String address) {
        try {
            URL url = new URL(address);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.connect();

            InputStream input = connection.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(input));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }
            reader.close();
            return sb.toString();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
