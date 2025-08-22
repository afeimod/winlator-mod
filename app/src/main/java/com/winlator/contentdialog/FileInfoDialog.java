package com.winlator.contentdialog;

import android.content.Context;
import android.graphics.Typeface;
import android.util.TypedValue;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.winlator.R;
import com.winlator.container.Container;
import com.winlator.container.FileInfo;
import com.winlator.core.AppUtils;
import com.winlator.core.FileUtils;
import com.winlator.core.StringUtils;
import com.winlator.core.UnitUtils;
import com.winlator.core.WineUtils;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.concurrent.atomic.AtomicLong;

public class FileInfoDialog extends ContentDialog {
    public FileInfoDialog(Context context, FileInfo file, Container container) {
        super(context, R.layout.file_info_dialog);
        findViewById(R.id.BTCancel).setVisibility(View.GONE);
        setTitle(R.string.information);
        setIcon(R.drawable.icon_info);

        String formattedType = "";
        switch (file.type) {
            case FILE:
                formattedType = context.getString(R.string.file);
                break;
            case DRIVE:
                formattedType = context.getString(R.string.drive);
                break;
            case DIRECTORY:
                formattedType = context.getString(R.string.folder);
                break;
        }

        ArrayList<String[]> lines = new ArrayList<>();
        lines.add(new String[]{context.getString(R.string.type), formattedType});

        if (file.type == FileInfo.Type.DIRECTORY) {
            lines.add(new String[]{context.getString(R.string.contains), file.getItemCount()+" "+context.getString(R.string.items)});
        }

        if (file.type == FileInfo.Type.FILE) {
            lines.add(new String[]{context.getString(R.string.size), StringUtils.formatBytes(file.getSize())});
        }
        else lines.add(new String[]{context.getString(R.string.size), "?"});

        if (file.type != FileInfo.Type.DRIVE) {
            lines.add(new String[]{context.getString(R.string.location), WineUtils.unixToDOSPath(FileUtils.getDirname(file.path), container)});
        }

        Date date = new Date(file.toFile().lastModified());
        String modified = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT).format(date);
        lines.add(new String[]{context.getString(R.string.modified), modified});

        LinearLayout llContent = findViewById(R.id.LLContent);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params.width = AppUtils.getPreferredDialogWidth(context);
        llContent.setLayoutParams(params);

        params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        int paddingRight = (int) UnitUtils.dpToPx(8);

        for (String[] columns : lines) {
            LinearLayout row = new LinearLayout(context);
            row.setLayoutParams(params);
            row.setOrientation(LinearLayout.HORIZONTAL);

            TextView tvLabel = new TextView(context);
            tvLabel.setLayoutParams(params);
            tvLabel.setPadding(0, 0, paddingRight, 0);
            tvLabel.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 16);
            tvLabel.setText(columns[0]+":");
            tvLabel.setTypeface(tvLabel.getTypeface(), Typeface.BOLD);
            row.addView(tvLabel);

            final TextView tvValue = new TextView(context);
            tvValue.setLayoutParams(params);
            tvValue.setTextSize(TypedValue.COMPLEX_UNIT_DIP, 16);
            tvValue.setMaxLines(1);
            tvValue.setText(columns[1]);

            if (columns[1].equals("?")) {
                final AtomicLong lastTime = new AtomicLong();
                final AtomicLong totalSize = new AtomicLong();

                FileUtils.getSizeAsync(file.toFile(), (size) -> {
                    totalSize.addAndGet(size);
                    long currTime = System.currentTimeMillis();
                    int elapsedTime = (int)(currTime - lastTime.get());
                    if (lastTime.get() == 0 || elapsedTime > 30) {
                        tvValue.post(() -> tvValue.setText(StringUtils.formatBytes(totalSize.get())));
                        lastTime.set(currTime);
                    }
                });
            }

            row.addView(tvValue);

            llContent.addView(row);
        }
    }
}
