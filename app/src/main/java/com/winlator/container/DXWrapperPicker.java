package com.winlator.container;

import android.content.Context;
import android.view.View;
import android.widget.LinearLayout;

import com.winlator.contentdialog.DXVKConfigDialog;
import com.winlator.contentdialog.VKD3DConfigDialog;
import com.winlator.contentdialog.WineD3DConfigDialog;
import com.winlator.core.KeyValueSet;
import com.winlator.core.StringUtils;
import com.winlator.widget.TaggedSelectionBox;

public class DXWrapperPicker {
    private final LinearLayout container;

    public DXWrapperPicker(LinearLayout container, final GraphicsDriverPicker graphicsDriverPicker, String selectedDXWrapper, String dxwrapperConfig) {
        this.container = container;
        final Context context = container.getContext();
        container.removeAllViews();
        KeyValueSet[] configs = DXWrappers.parseConfigs(selectedDXWrapper, dxwrapperConfig);
        selectedDXWrapper = DXWrappers.parseIdentifier(selectedDXWrapper);

        final String[] apiNames = {"Direct3D", "DirectX 12"};
        for (int i = 0; i < apiNames.length; i++) {
            final TaggedSelectionBox taggedSelectionBox = new TaggedSelectionBox(context);
            taggedSelectionBox.setLabel(apiNames[i]);
            taggedSelectionBox.setTag(configs[i].toString());

            if (i == 0) {
                taggedSelectionBox.setItems(DXWrappers.getName(DXWrappers.WINED3D), DXWrappers.getName(DXWrappers.DXVK));
                if (selectedDXWrapper.equals(DXWrappers.WINED3D) || selectedDXWrapper.equals(DXWrappers.DXVK)) {
                    taggedSelectionBox.setSelectedItem(DXWrappers.getName(selectedDXWrapper));
                }
                else taggedSelectionBox.setSelectedItem(DXWrappers.getName(Container.DEFAULT_DXWRAPPER));

                final String[] oldDXWrapper = {selectedDXWrapper};
                taggedSelectionBox.setOnItemSelectedListener((item) -> {
                    String dxwrapper = StringUtils.parseIdentifier(item);
                    if (!oldDXWrapper[0].equals(dxwrapper)) taggedSelectionBox.setTag("");
                    oldDXWrapper[0] = dxwrapper;
                });
            }
            else {
                taggedSelectionBox.setItems(DXWrappers.getName(DXWrappers.VKD3D));
                taggedSelectionBox.setSelectedItem(taggedSelectionBox.getItems()[0]);
            }

            taggedSelectionBox.setOnButtonClickListener(() -> {
                String dxwrapper = StringUtils.parseIdentifier(taggedSelectionBox.getSelectedItem());
                String graphicsDriver = GraphicsDrivers.parseIdentifiers(graphicsDriverPicker.getGraphicsDriver())[0];
                showDXWrapperConfigDialog(dxwrapper, graphicsDriver, taggedSelectionBox);
            });
            container.addView(taggedSelectionBox);
        }
    }

    public String getDXWrapper() {
        TaggedSelectionBox taggedSelectionBox = (TaggedSelectionBox)container.getChildAt(0);
        return StringUtils.parseIdentifier(taggedSelectionBox.getSelectedItem());
    }

    public String getDXWrapperConfig() {
        StringBuilder dxwrapperConfig = new StringBuilder();
        for (int i = 0; i < container.getChildCount(); i++) {
            TaggedSelectionBox taggedSelectionBox = (TaggedSelectionBox)container.getChildAt(i);
            if (dxwrapperConfig.length() > 0) dxwrapperConfig.append('|');
            dxwrapperConfig.append(taggedSelectionBox.getTag().toString());
        }
        return dxwrapperConfig.toString();
    }

    private static void showDXWrapperConfigDialog(String dxwrapper, String graphicsDriver, View anchor) {
        switch (dxwrapper) {
            case DXWrappers.DXVK:
                (new DXVKConfigDialog(graphicsDriver, anchor)).show();
                break;
            case DXWrappers.VKD3D:
                (new VKD3DConfigDialog(anchor)).show();
                break;
            case DXWrappers.WINED3D:
                (new WineD3DConfigDialog(anchor)).show();
                break;
        }
    }
}
