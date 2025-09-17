package com.winlator.core;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.LocaleList;

import androidx.preference.PreferenceManager;

import java.util.Locale;

public class LocaleHelper {
    private static final String[] supportedLocales = {"en_US", "pt_BR", "ru_RU"};

    public static int getLocaleIndex(Context context) {
        Configuration configuration = context.getResources().getConfiguration();
        LocaleList localeList = configuration.getLocales();
        String locale = !localeList.isEmpty() ? localeList.get(0).toString() : "";

        for (int i = 0; i < supportedLocales.length; i++) {
            if (locale.startsWith(supportedLocales[i].substring(0, 2))) return i;
        }
        return 0;
    }

    public static Context setSystemLocale(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        int index = preferences.getInt("lc_index", -1);

        if (index < 0 || index >= supportedLocales.length) return context;
        Locale locale = new Locale(supportedLocales[index].substring(0, 2));
        Locale.setDefault(locale);

        Configuration configuration = context.getResources().getConfiguration();
        configuration.setLocale(locale);
        configuration.setLayoutDirection(locale);
        return context.createConfigurationContext(configuration);
    }

    public static void setEnvVars(EnvVars envVars) {
        Locale locale = Locale.getDefault();
        for (String name : supportedLocales) {
            if (locale.toString().startsWith(name.substring(0, 2))) {
                envVars.put("LC_ALL", name+".UTF-8");
                return;
            }
        }

        envVars.put("LC_ALL", "en_US.UTF-8");
    }
}
