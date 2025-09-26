package com.winlator.widget;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;

import androidx.annotation.Nullable;

import com.winlator.MainActivity;
import com.winlator.R;
import com.winlator.core.AppUtils;
import com.winlator.core.FileUtils;
import com.winlator.core.ImageUtils;
import com.winlator.core.UnitUtils;
import com.winlator.core.WineThemeManager;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class ImagePickerView extends View implements View.OnClickListener {
    private final Bitmap icon;
    private final List<String> defaultSources = Arrays.asList("wallpaper-1", "wallpaper-2", "wallpaper-3");
    private String selectedSource = WineThemeManager.DEFAULT_WALLPAPER_ID;

    public ImagePickerView(Context context) {
        this(context, null);
    }

    public ImagePickerView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ImagePickerView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        icon = BitmapFactory.decodeResource(context.getResources(), R.drawable.icon_image_picker);

        setBackgroundResource(R.drawable.combo_box);
        setClickable(true);
        setFocusable(true);
        setOnClickListener(this);
    }

    public String getSelectedSource() {
        return selectedSource;
    }

    public void setSelectedSource(String selectedSource) {
        this.selectedSource = selectedSource;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        int width = getWidth();
        int height = getHeight();
        if (width == 0 || height == 0) return;

        float rectSize = height - UnitUtils.dpToPx(12);
        float startX = (width - rectSize) * 0.5f - UnitUtils.dpToPx(16);
        float startY = (height - rectSize) * 0.5f;

        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        Rect srcRect = new Rect(0, 0, icon.getWidth(), icon.getHeight());
        RectF dstRect = new RectF(startX, startY, startX + rectSize, startY + rectSize);
        canvas.drawBitmap(icon, srcRect, dstRect, paint);
    }

    @Override
    public void onClick(View anchor) {
        final Context context = getContext();
        final File userWallpaperFile = WineThemeManager.getUserWallpaperFile(context);

        LayoutInflater inflater = LayoutInflater.from(context);
        View view = inflater.inflate(R.layout.image_picker_view, null);
        LinearLayout llImageList = view.findViewById(R.id.LLImageList);

        final PopupWindow[] popupWindow = {null};
        ArrayList<String> sources = new ArrayList<>(defaultSources);
        if (userWallpaperFile.isFile()) sources.add("user-wallpaper");

        for (final String source : sources) {
            View itemView = inflater.inflate(R.layout.image_picker_list_item, llImageList, false);
            ImageView imageView = itemView.findViewById(R.id.ImageView);

            if (source.startsWith("wallpaper-")) {
                imageView.setImageBitmap(ImageUtils.getBitmapFromAsset(context, "wallpapers/"+source+"/image.png"));
            }
            else if (source.equals("user-wallpaper")) {
                imageView.setImageBitmap(BitmapFactory.decodeFile(userWallpaperFile.getPath()));
                View removeButton = itemView.findViewById(R.id.BTRemove);
                removeButton.setVisibility(View.VISIBLE);
                removeButton.setOnClickListener((v) -> {
                    FileUtils.delete(userWallpaperFile);
                    selectedSource = WineThemeManager.DEFAULT_WALLPAPER_ID;
                    popupWindow[0].dismiss();
                });
            }

            if (source.equals(selectedSource)) itemView.setBackgroundResource(R.drawable.bordered_panel);

            itemView.setOnClickListener((v) -> {
                selectedSource = source;
                popupWindow[0].dismiss();
            });
            llImageList.addView(itemView);
        }

        View browseButton = view.findViewById(R.id.BTBrowse);
        browseButton.setOnClickListener((v) -> {
            MainActivity activity = (MainActivity)context;
            Intent intent = new Intent(Intent.ACTION_PICK);
            intent.setType("image/*");
            activity.setOpenFileCallback((data) -> {
                Bitmap bitmap = ImageUtils.getBitmapFromUri(context, data, 1280);
                if (bitmap == null) return;

                ImageUtils.save(bitmap, userWallpaperFile, Bitmap.CompressFormat.PNG, 100);
                popupWindow[0].dismiss();
            });
            activity.startActivityForResult(intent, MainActivity.OPEN_FILE_REQUEST_CODE);
        });

        popupWindow[0] = AppUtils.showPopupWindow(anchor, view, 0, 200);
    }
}
