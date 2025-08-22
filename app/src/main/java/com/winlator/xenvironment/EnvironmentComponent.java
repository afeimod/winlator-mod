package com.winlator.xenvironment;

public abstract class EnvironmentComponent {
    protected XEnvironment environment;

    public abstract void start();

    public abstract void stop();

    public void onPause() {}

    public void onResume() {}
}