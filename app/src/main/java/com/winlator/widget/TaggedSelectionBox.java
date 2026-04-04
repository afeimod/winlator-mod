package com.winlator.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.widget.ListPopupWindow;

import com.winlator.R;

public class TaggedSelectionBox extends LinearLayout {
    private String[] items;
    private OnButtonClickListener onButtonClickListener;
    private OnItemSelectedListener onItemSelectedListener;

    public interface OnButtonClickListener {
        void onClick();
    }

    public interface OnItemSelectedListener {
        void onItemSelected(String item);
    }

    public TaggedSelectionBox(Context context) {
        this(context, null);
    }

    public TaggedSelectionBox(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public TaggedSelectionBox(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public TaggedSelectionBox(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);

        setOrientation(HORIZONTAL);
        setLayoutParams(new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        LayoutInflater inflater = LayoutInflater.from(context);
        View contentView = inflater.inflate(R.layout.tagged_selection_box, this, false);
        LinearLayout llDisplayBox = contentView.findViewById(R.id.LLDisplayBox);

        llDisplayBox.setOnClickListener((v) -> {
            if (items == null) return;
            final ListPopupWindow popupWindow = new ListPopupWindow(context);
            popupWindow.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, items));
            popupWindow.setAnchorView(v);
            popupWindow.setWidth(v.getWidth());

            popupWindow.setOnItemClickListener((parent, view, position, id) -> {
                ((TextView)findViewById(R.id.TVName)).setText(items[position]);
                if (onItemSelectedListener != null) onItemSelectedListener.onItemSelected(items[position]);
                popupWindow.dismiss();
            });
            popupWindow.show();
        });

        contentView.findViewById(R.id.BTSettings).setOnClickListener((view) -> {
            if (onButtonClickListener != null) onButtonClickListener.onClick();
        });
        addView(contentView);
    }

    public String getSelectedItem() {
        return ((TextView)findViewById(R.id.TVName)).getText().toString();
    }

    public void setSelectedItem(String selectedItem) {
        ((TextView)findViewById(R.id.TVName)).setText(selectedItem);
    }

    public void setLabel(String label) {
        ((TextView)findViewById(R.id.TVLabel)).setText(label);
    }

    public String[] getItems() {
        return items;
    }

    public void setItems(String... items) {
        this.items = items;
    }

    public OnButtonClickListener getOnButtonClickListener() {
        return onButtonClickListener;
    }

    public void setOnButtonClickListener(OnButtonClickListener onButtonClickListener) {
        this.onButtonClickListener = onButtonClickListener;
    }

    public OnItemSelectedListener getOnItemSelectedListener() {
        return onItemSelectedListener;
    }

    public void setOnItemSelectedListener(OnItemSelectedListener onItemSelectedListener) {
        this.onItemSelectedListener = onItemSelectedListener;
    }
}
