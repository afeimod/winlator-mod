package com.winlator.winhandler;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.os.BatteryManager;
import android.os.Build;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.TextView;

import com.winlator.R;
import com.winlator.XServerDisplayActivity;
import com.winlator.contentdialog.ContentDialog;
import com.winlator.core.BatteryUtils;
import com.winlator.core.CPUStatus;
import com.winlator.core.ProcessHelper;
import com.winlator.core.StringUtils;
import com.winlator.core.UnitUtils;
import com.winlator.widget.CPUListView;
import com.winlator.xserver.Window;
import com.winlator.xserver.XLock;
import com.winlator.xserver.XServer;

import java.util.ArrayList;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

public class TaskManagerDialog extends ContentDialog implements OnGetProcessInfoListener {
    private final XServerDisplayActivity activity;
    private final LayoutInflater inflater;
    private Timer timer;
    private final Object lock = new Object();
    private BroadcastReceiver batteryReceiver;
    private final BatteryInfo batteryInfo = new BatteryInfo();
    private final Panel cpuPanel;
    private final Panel memoryPanel;
    private final Panel batteryPanel;

    private static class BatteryInfo {
        private int temperature;
        private float voltage;
        private int level;
    }

    private static class Panel {
        private View view;
        private View[] itemViews;
        private PopupMenu popupMenu;

        private void setTitle(String title) {
            ((TextView)view.findViewById(R.id.TVTitle)).setText(title);
        }

        private void setIconAt(int index, int icon) {
            ((ImageView)itemViews[index].findViewById(R.id.ImageView)).setImageResource(icon);
        }

        private void setTextAt(int index, String value) {
            ((TextView)itemViews[index].findViewById(R.id.TextView)).setText(value);
        }

        private void setPopupMenuItems(final ArrayList<String> items) {
            if (popupMenu == null) {
                View menuButton = view.findViewById(R.id.BTMenu);
                popupMenu = new PopupMenu(view.getContext(), menuButton);
                menuButton.setOnClickListener((v) -> popupMenu.show());
                menuButton.setVisibility(View.VISIBLE);
            }

            Menu menu = popupMenu.getMenu();
            menu.clear();
            for (String item : items) menu.add(item);
        }
    }

    public TaskManagerDialog(XServerDisplayActivity activity) {
        super(activity, R.layout.task_manager_dialog);
        this.activity = activity;
        setCancelable(false);
        setTitle(R.string.task_manager);
        setIcon(R.drawable.icon_task_manager);

        Button cancelButton = findViewById(R.id.BTCancel);
        cancelButton.setText(R.string.new_task);
        cancelButton.setOnClickListener((v) -> {
            dismiss();
            ContentDialog.prompt(activity, R.string.new_task, "taskmgr.exe", (command) -> activity.getWinHandler().exec(command));
        });

        setOnDismissListener((dialog) -> {
            if (batteryReceiver != null) activity.unregisterReceiver(batteryReceiver);

            if (timer != null) {
                timer.cancel();
                timer = null;
            }

            activity.getWinHandler().setOnGetProcessInfoListener(null);
        });

        inflater = LayoutInflater.from(activity);

        batteryReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action != null && action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                    int voltage = intent.getIntExtra(BatteryManager.EXTRA_VOLTAGE, 0);
                    int temperature = intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, 0);
                    int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
                    int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 0);

                    batteryInfo.voltage = voltage / 1000.0f;
                    batteryInfo.temperature = (int)(temperature / 10.0f);
                    batteryInfo.level = (int)((float)level / scale * 100);
                }
            }
        };

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        activity.registerReceiver(batteryReceiver, filter);

        cpuPanel = createPanel(2);
        cpuPanel.setIconAt(0, R.drawable.icon_cpu);
        cpuPanel.setIconAt(1, R.drawable.icon_temperature);

        memoryPanel = createPanel(1);
        memoryPanel.setIconAt(0, R.drawable.icon_memory);

        batteryPanel = createPanel(2);
        batteryPanel.setIconAt(0, R.drawable.icon_power);
        batteryPanel.setIconAt(1, R.drawable.icon_temperature);
    }

    private Panel createPanel(int numItems) {
        LinearLayout llPanelList = findViewById(R.id.LLPanelList);
        View view = inflater.inflate(R.layout.task_manager_panel, llPanelList, false);

        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        if (llPanelList.getChildCount() > 0) params.setMargins(0, (int)UnitUtils.dpToPx(6), 0, 0);

        view.setLayoutParams(params);
        llPanelList.addView(view);

        LinearLayout llItemList = view.findViewById(R.id.LLItemList);

        Panel panel = new Panel();
        panel.view = view;
        panel.itemViews = new View[numItems];

        for (int i = 0; i < numItems; i++) {
            View itemView = inflater.inflate(R.layout.task_manager_panel_item, llItemList, false);
            panel.itemViews[i] = itemView;
            llItemList.addView(itemView);
        }

        return panel;
    }

    private void update() {
        synchronized (lock) {
            activity.getWinHandler().listProcesses();

            final LinearLayout container = findViewById(R.id.LLProcessList);
            if (container.getChildCount() == 0) findViewById(R.id.TVEmptyText).setVisibility(View.VISIBLE);
        }

        updateCPUPanel();
        updateMemoryPanel();
        updateBatteryPanel();
    }

    private void showListItemMenu(final View anchorView, final ProcessInfo processInfo) {
        PopupMenu listItemMenu = new PopupMenu(activity, anchorView);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) listItemMenu.setForceShowIcon(true);

        listItemMenu.inflate(R.menu.process_popup_menu);
        listItemMenu.setOnMenuItemClickListener((menuItem) -> {
            int itemId = menuItem.getItemId();
            final WinHandler winHandler = activity.getWinHandler();
            switch (itemId) {
                case R.id.menu_item_process_affinity:
                    showProcessorAffinityDialog(processInfo);
                    break;
                case R.id.menu_item_bring_to_front:
                    winHandler.bringToFront(processInfo.name);
                    dismiss();
                    break;
                case R.id.menu_item_end_process:
                    ContentDialog.confirm(activity, R.string.do_you_want_to_end_this_process, () -> {
                        winHandler.killProcess(null, processInfo.pid);
                    });
                    break;
            }
            return true;
        });
        listItemMenu.show();
    }

    private void showProcessorAffinityDialog(final ProcessInfo processInfo) {
        ContentDialog dialog = new ContentDialog(activity, R.layout.cpu_list_dialog);
        dialog.setTitle(processInfo.name);
        dialog.setIcon(R.drawable.icon_cpu);
        final CPUListView cpuListView = dialog.findViewById(R.id.CPUListView);
        cpuListView.setCheckedCPUList(processInfo.getCPUList());
        dialog.setOnConfirmCallback(() -> {
            WinHandler winHandler = activity.getWinHandler();
            winHandler.setProcessAffinity(processInfo.pid, ProcessHelper.getAffinityMask(cpuListView.getCheckedCPUList()));
            update();
        });
        dialog.show();
    }

    @Override
    public void show() {
        update();
        activity.getWinHandler().setOnGetProcessInfoListener(this);

        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                activity.runOnUiThread(TaskManagerDialog.this::update);
            }
        }, 0, 1000);
        super.show();
    }

    @Override
    public void onGetProcessInfo(int index, int numProcesses, ProcessInfo processInfo) {
        activity.runOnUiThread(() -> {
            synchronized (lock) {
                final LinearLayout container = findViewById(R.id.LLProcessList);
                setBottomBarText(activity.getString(R.string.processes)+": " + numProcesses);

                if (numProcesses == 0) {
                    container.removeAllViews();
                    findViewById(R.id.TVEmptyText).setVisibility(View.VISIBLE);
                    return;
                }

                findViewById(R.id.TVEmptyText).setVisibility(View.GONE);

                int childCount = container.getChildCount();
                View itemView = index < childCount ? container.getChildAt(index) : inflater.inflate(R.layout.process_info_list_item, container, false);
                ((TextView)itemView.findViewById(R.id.TVName)).setText(processInfo.name+(processInfo.wow64Process ? " *32" : ""));
                ((TextView)itemView.findViewById(R.id.TVPID)).setText(String.valueOf(processInfo.pid));
                ((TextView)itemView.findViewById(R.id.TVMemoryUsage)).setText(processInfo.getFormattedMemoryUsage());
                itemView.findViewById(R.id.BTMenu).setOnClickListener((v) -> showListItemMenu(v, processInfo));

                XServer xServer = activity.getXServer();
                Window window;

                try (XLock xlock = xServer.lock(XServer.Lockable.WINDOW_MANAGER)) {
                    window = xServer.windowManager.findWindowWithProcessId(processInfo.pid);
                }

                ImageView ivIcon = itemView.findViewById(R.id.IVIcon);
                ivIcon.setImageResource(R.drawable.taskmgr_process);
                if (window != null) {
                    Bitmap icon = xServer.pixmapManager.getWindowIcon(window);
                    if (icon != null) ivIcon.setImageBitmap(icon);
                }

                if (index >= childCount) container.addView(itemView);

                if (index == numProcesses-1 && childCount > numProcesses) {
                    for (int i = childCount-1; i >= numProcesses; i--) container.removeViewAt(i);
                }
            }
        });
    }

    private void updateCPUPanel() {
        short[] clockSpeeds = CPUStatus.getCurrentClockSpeeds();
        float totalClockSpeed = 0;
        int maxClockSpeed = 0;
        int selectedClockSpeed = 0;

        ArrayList<String> popupMenuItems = new ArrayList<>();

        for (int i = 0; i < clockSpeeds.length; i++) {
            totalClockSpeed += clockSpeeds[i];
            int currentMaxClockSpeed = CPUStatus.getMaxClockSpeed(i);
            maxClockSpeed = Math.max(maxClockSpeed, currentMaxClockSpeed);
            selectedClockSpeed = Math.max(selectedClockSpeed, clockSpeeds[i]);

            popupMenuItems.add("CPU"+i+": "+CPUStatus.formatClockSpeed(clockSpeeds[i])+"/"+CPUStatus.formatClockSpeed(currentMaxClockSpeed));
        }

        float avgClockSpeed = totalClockSpeed / clockSpeeds.length;
        byte cpuUsagePercent = (byte)((avgClockSpeed / maxClockSpeed) * 100.0f);

        cpuPanel.setTitle("CPU ("+cpuUsagePercent+"%)");
        cpuPanel.setTextAt(0, CPUStatus.formatClockSpeed(selectedClockSpeed));
        cpuPanel.setTextAt(1, CPUStatus.getTemperature()+"ºC");
        cpuPanel.setPopupMenuItems(popupMenuItems);
    }

    private void updateMemoryPanel() {
        ActivityManager activityManager = (ActivityManager)activity.getSystemService(Context.ACTIVITY_SERVICE);
        ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
        activityManager.getMemoryInfo(memoryInfo);
        long usedMem = memoryInfo.totalMem - memoryInfo.availMem;
        byte memUsagePercent = (byte)(((double)usedMem / memoryInfo.totalMem) * 100.0f);

        memoryPanel.setTitle(activity.getString(R.string.memory)+" ("+memUsagePercent+"%)");
        memoryPanel.setTextAt(0, StringUtils.formatBytes(usedMem, false)+"/"+StringUtils.formatBytes(memoryInfo.totalMem));
    }

    private void updateBatteryPanel() {
        int currentMicroamperes = BatteryUtils.getCurrentMicroamperes(activity);

        batteryPanel.setTitle(activity.getString(R.string.battery)+" ("+batteryInfo.level+"%)");
        batteryPanel.setTextAt(0, String.format(Locale.ENGLISH, "%.2f", BatteryUtils.computePower(currentMicroamperes, batteryInfo.voltage))+" W");
        batteryPanel.setTextAt(1, batteryInfo.temperature+"ºC");

        ArrayList<String> popupMenuItems = new ArrayList<>();
        popupMenuItems.add(activity.getString(R.string.voltage)+": "+String.format(Locale.ENGLISH, "%.2f", batteryInfo.voltage)+" V");
        popupMenuItems.add(activity.getString(R.string.current)+": "+(currentMicroamperes / 1000)+" mA");

        int capacity = BatteryUtils.getCapacity(activity);
        if (capacity > 0) popupMenuItems.add(activity.getString(R.string.capacity)+": "+capacity+" mAh");

        batteryPanel.setPopupMenuItems(popupMenuItems);
    }
}
