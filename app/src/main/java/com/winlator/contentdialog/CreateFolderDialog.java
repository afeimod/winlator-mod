package com.winlator.contentdialog;

import android.content.Context;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.container.Container;
import com.winlator.container.ContainerManager;

import java.util.ArrayList;

public class CreateFolderDialog extends ContentDialog {
    private OnCreateFolderListener onCreateFolderListener;

    public interface OnCreateFolderListener {
        void onCreateFolder(Container container, String name);
    }

    public CreateFolderDialog(ContainerManager manager) {
        super(manager.getContext(), R.layout.create_folder_dialog);
        final Context context = manager.getContext();
        final Spinner sContainer = findViewById(R.id.SContainer);
        final EditText etName = findViewById(R.id.ETName);

        setTitle(context.getString(R.string.new_folder));

        final ArrayList<Container> containers = manager.getContainers();
        ArrayList<String> items = new ArrayList<>();
        for (Container container : containers) items.add(container.getName());

        sContainer.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, items));

        setOnConfirmCallback(() -> {
            String name = etName.getText().toString().trim();
            Container container = containers.get(sContainer.getSelectedItemPosition());

            if (onCreateFolderListener != null && !name.isEmpty()) onCreateFolderListener.onCreateFolder(container, name);
        });
    }

    public void setOnCreateFolderListener(OnCreateFolderListener onCreateFolderListener) {
        this.onCreateFolderListener = onCreateFolderListener;
    }
}