package com.winlator.widget;

import android.content.Context;
import android.widget.ArrayAdapter;

import androidx.annotation.NonNull;

import com.winlator.core.FileUtils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class GPUCardAdapter extends ArrayAdapter<GPUCardAdapter.GPUCard> {
    public static class GPUCard {
        public final String name;
        public final int deviceId;
        public final int vendorId;

        public GPUCard(String name, int deviceId, int vendorId) {
            this.name = name;
            this.deviceId = deviceId;
            this.vendorId = vendorId;
        }

        @NonNull
        @Override
        public String toString() {
            return name;
        }
    }

    public GPUCardAdapter(@NonNull Context context, int resource) {
        this(context, resource, 0);
    }

    public GPUCardAdapter(@NonNull Context context, int resource, int defaultItemTextId) {
        super(context, resource, loadData(context, defaultItemTextId));
    }

    private static List<GPUCard> loadData(Context context, int defaultItemTextId) {
        try {
            JSONArray gpuCards = new JSONArray(FileUtils.readString(context, "gpu_cards.json"));

            List<GPUCard> items = new ArrayList<>();
            if (defaultItemTextId > 0) items.add(new GPUCard(context.getString(defaultItemTextId), 0, 0));

            for (int i = 0; i < gpuCards.length(); i++) {
                JSONObject item = gpuCards.getJSONObject(i);
                items.add(new GPUCard(item.getString("name"), item.getInt("deviceID"), item.getInt("vendorID")));
            }

            return items;
        }
        catch (JSONException e) {
            return Collections.emptyList();
        }
    }

    public int getPositionByDeviceId(int deviceId) {
        for (int i = 0; i < getCount(); i++) {
            GPUCard gpuCard = getItem(i);
            if (gpuCard.deviceId == deviceId) return i;
        }

        return 0;
    }
}
