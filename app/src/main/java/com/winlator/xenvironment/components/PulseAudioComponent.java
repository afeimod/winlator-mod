package com.winlator.xenvironment.components;

import android.content.Context;
import android.os.Process;

import com.winlator.contentdialog.AudioDriverConfigDialog;
import com.winlator.core.EnvVars;
import com.winlator.core.FileUtils;
import com.winlator.core.ProcessHelper;
import com.winlator.xconnector.UnixSocketConfig;
import com.winlator.xenvironment.EnvironmentComponent;

import java.io.File;

public class PulseAudioComponent extends EnvironmentComponent {
    private final UnixSocketConfig socketConfig;
    private static int pid = -1;
    private float volume = AudioDriverConfigDialog.DEFAULT_VOLUME;
    private byte performanceMode = AudioDriverConfigDialog.DEFAULT_PERFORMANCE_MODE;
    private static final Object lock = new Object();

    public PulseAudioComponent(UnixSocketConfig socketConfig) {
        this.socketConfig = socketConfig;
    }

    @Override
    public void start() {
        synchronized (lock) {
            stop();
            pid = execPulseAudio();
        }
    }

    @Override
    public void stop() {
        synchronized (lock) {
            if (pid != -1) {
                Process.killProcess(pid);
                pid = -1;
            }
        }
    }

    public float getVolume() {
        return volume;
    }

    public void setVolume(float volume) {
        this.volume = volume;
    }

    public int getPerformanceMode() {
        return performanceMode;
    }

    public void setPerformanceMode(int performanceMode) {
        this.performanceMode = (byte)performanceMode;
    }

    private int execPulseAudio() {
        Context context = environment.getContext();
        String nativeLibraryDir = context.getApplicationInfo().nativeLibraryDir;
        File workingDir = new File(context.getFilesDir(), "/pulseaudio");
        if (!workingDir.isDirectory()) {
            workingDir.mkdirs();
            FileUtils.chmod(workingDir, 0771);
        }

        File configFile = new File(workingDir, "default.pa");
        FileUtils.writeString(configFile, String.join("\n",
            "load-module module-native-protocol-unix auth-anonymous=1 auth-cookie-enabled=0 socket=\""+socketConfig.path+"\"",
            "load-module module-aaudio-sink volume="+volume+" performance_mode="+performanceMode,
            "set-default-sink AAudioSink"
        ));

        File modulesDir = new File(workingDir, "modules");
        EnvVars envVars = new EnvVars();
        envVars.put("LD_LIBRARY_PATH", "/system/lib64:"+nativeLibraryDir+":"+modulesDir);
        envVars.put("HOME", workingDir);
        envVars.put("TMPDIR", environment.getTmpDir());

        String command = nativeLibraryDir+"/libpulseaudio.so";
        command += " --system=false";
        command += " --disable-shm=true";
        command += " --fail=false";
        command += " -n --file=default.pa";
        command += " --daemonize=false";
        command += " --use-pid-file=false";
        command += " --exit-idle-time=-1";

        return ProcessHelper.exec(command, envVars, workingDir);
    }
}
