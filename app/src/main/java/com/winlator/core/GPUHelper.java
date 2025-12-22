package com.winlator.core;

import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.EGL14;

import androidx.collection.ArrayMap;
import androidx.preference.PreferenceManager;

import java.util.Locale;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import dalvik.annotation.optimization.CriticalNative;

public abstract class GPUHelper {
    public static int VK_API_VERSION_1_3 = GPUHelper.vkMakeVersion(1, 3, 0);

    static {
        System.loadLibrary("winlator");
    }

    private static ArrayMap<String, String> loadGPUInformation(Context context) {
        final Thread thread = Thread.currentThread();
        final ArrayMap<String, String> gpuInfo = new ArrayMap<>();
        gpuInfo.put("renderer", "");
        gpuInfo.put("vendor", "");
        gpuInfo.put("version", "");

        (new Thread(() -> {
            int[] attribList = new int[] {
                EGL10.EGL_SURFACE_TYPE, EGL10.EGL_PBUFFER_BIT,
                EGL10.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_GREEN_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 8,
                EGL10.EGL_ALPHA_SIZE, 0,
                EGL10.EGL_NONE
            };
            EGLConfig[] configs = new EGLConfig[1];
            int[] configCounts = new int[1];

            EGL10 egl = (EGL10)EGLContext.getEGL();
            EGLDisplay eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

            int[] version = new int[2];
            egl.eglInitialize(eglDisplay, version);
            egl.eglChooseConfig(eglDisplay, attribList, configs, 1, configCounts);

            attribList = new int[]{EGL14.EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
            EGLContext eglContext = egl.eglCreateContext(eglDisplay, configs[0], EGL10.EGL_NO_CONTEXT, attribList);

            egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, eglContext);

            GL10 gl = (GL10)eglContext.getGL();
            String gpuRenderer = Objects.toString(gl.glGetString(GL10.GL_RENDERER), "");
            String gpuVendor = Objects.toString(gl.glGetString(GL10.GL_VENDOR), "");
            String gpuVersion = Objects.toString(gl.glGetString(GL10.GL_VERSION), "");

            gpuInfo.put("renderer", gpuRenderer);
            gpuInfo.put("vendor", gpuVendor);
            gpuInfo.put("version", gpuVersion);

            final SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
            preferences.edit()
                .putString("gpu_renderer", gpuRenderer)
                .putString("gpu_vendor", gpuVendor)
                .putString("gpu_version", gpuVersion)
                .apply();

            synchronized (thread) {
                thread.notify();
            }
        })).start();

        synchronized (thread) {
            try {
                thread.wait();
            }
            catch (InterruptedException e) {}
        }
        return gpuInfo;
    }

    public static String glGetRenderer(Context context) {
        final SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String value = preferences.getString("gpu_renderer", "");
        if (!value.isEmpty()) return value;

        ArrayMap<String, String> gpuInfo = loadGPUInformation(context);
        return gpuInfo.get("renderer");
    }

    public static String glGetVendor(Context context) {
        final SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String value = preferences.getString("gpu_vendor", "");
        if (!value.isEmpty()) return value;

        ArrayMap<String, String> gpuInfo = loadGPUInformation(context);
        return gpuInfo.get("vendor");
    }

    public static String glGetVersion(Context context) {
        final SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String value = preferences.getString("gpu_version", "");
        if (!value.isEmpty()) return value;

        ArrayMap<String, String> gpuInfo = loadGPUInformation(context);
        return gpuInfo.get("version");
    }

    public static boolean isAdreno6xx(Context context) {
        return glGetRenderer(context).toLowerCase(Locale.ENGLISH).matches(".*adreno[^6]+6[0-9]{2}.*");
    }

    public static int vkMakeVersion(String value) {
        final Pattern pattern = Pattern.compile("([0-9]+)\\.([0-9]+)\\.?([0-9]+)?");
        Matcher matcher = pattern.matcher(value);
        if (matcher.find()) {
            try {
                int major = matcher.group(1) != null ? Integer.parseInt(matcher.group(1)) : 0;
                int minor = matcher.group(2) != null ? Integer.parseInt(matcher.group(2)) : 0;
                int patch = matcher.group(3) != null ? Integer.parseInt(matcher.group(3)) : 0;
                if (matcher.group(1) == null && patch == 0) patch = minor;
                return vkMakeVersion(major, minor, patch);
            }
            catch (NumberFormatException e) {
                return 0;
            }
        }
        else return 0;
    }

    public static int vkMakeVersion(int major, int minor, int patch) {
        return  ((major) << 22) | ((minor) << 12) | (patch);
    }

    public static int vkVersionMajor(int version) {
        return (version) >> 22;
    }

    public static int vkVersionMinor(int version) {
        return ((version) >> 12) & 0x3FF;
    }

    public static int vkVersionPatch(int version) {
        return (version) & 0xFFF;
    }

    public static native String[] vkGetDeviceExtensions();

    @CriticalNative
    public static native int vkGetApiVersion();

    public static native void setGlobalEGLContext();
}
