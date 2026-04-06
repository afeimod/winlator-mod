package com.winlator.widget;

import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.StateListDrawable;
import android.os.Handler;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.PointerIcon;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import com.winlator.R;
import com.winlator.core.AppUtils;
import com.winlator.xserverbridge.IXServerBridge;
import com.winlator.math.Mathf;
import com.winlator.math.XForm;
import com.winlator.renderer.ViewTransformation;
import com.winlator.winhandler.MouseEventFlags;
import com.winlator.winhandler.WinHandler;
import com.winlator.xserver.Pointer;

public class TouchpadView extends View implements View.OnCapturedPointerListener {
    private static final int EFFECTIVE_TOUCH_DISTANCE = 20;
    private static final int UPDATE_FORM_DELAYED_TIME = 50;
    private static final int MOVE_TO_CLICK_DELAY_MS = 30;
    private static final int SHORT_DRAG_MAX_TIME = 100; 
    private static final int LONG_DRAG_MIN_TIME = 120;
    private static final int LONG_PRESS_MIN_TIME = 250;  

    public static final short MAX_TAP_MILLISECONDS = 200;
    public static final byte MAX_TAP_TRAVEL_DISTANCE = 10;
    public static final float CURSOR_ACCELERATION = 1.25f;
    public static final byte CURSOR_ACCELERATION_THRESHOLD = 6;

    private Finger fingerPointerButtonLeft;
    private Finger fingerPointerButtonRight;
    private final Finger[] fingers;
    private byte numFingers;

    private float sensitivity;
    private boolean pointerButtonLeftEnabled;
    private boolean pointerButtonRightEnabled;
    private boolean simTouchScreen;          
    private boolean swapMouseButtons;
    private boolean touchscreenMouseDisabled;
    private boolean pointerButtonClicksEnabled = true;

    private float scrollAccumY;
    private boolean scrolling;

    private Runnable threeFingersTapCallback;
    private Runnable fourFingersTapCallback;

    private float resolutionScale;
    private int lastTouchedPosX;
    private int lastTouchedPosY;

    private boolean isShortDrag;
    private boolean isLongDrag;
    private int initialPointerX;
    private int initialPointerY;

    private final IXServerBridge xServer;
    private final float[] xform;

    public TouchpadView(Context context, IXServerBridge xServer) {
        this(context, xServer, false);
    }

    public TouchpadView(Context context, IXServerBridge xServer, boolean capturePointerOnExternalMouse) {
        super(context);

        this.xServer = xServer;
        this.xform = XForm.getInstance();
        this.fingers = new Finger[4];
        this.numFingers = 0;
        this.sensitivity = 1.0f;
        this.pointerButtonLeftEnabled = true;
        this.pointerButtonRightEnabled = true;
        this.simTouchScreen = false;          // 原 moveCursorToTouchpoint
        this.swapMouseButtons = false;
        this.touchscreenMouseDisabled = false;
        this.scrollAccumY = 0.0f;
        this.scrolling = false;
        this.isShortDrag = false;
        this.isLongDrag = false;

        setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        setBackground(createTransparentBackground());
        setClickable(true);
        setFocusable(true);
        setFocusableInTouchMode(false);
        setPointerIcon(PointerIcon.load(getResources(), R.drawable.hidden_pointer_arrow)); // 隐藏系统指针

        updateXform(AppUtils.getScreenWidth(), AppUtils.getScreenHeight(),
                xServer.getScreenWidth(), xServer.getScreenHeight());
        resolutionScale = 1000.0f / Math.min(xServer.getScreenWidth(), xServer.getScreenHeight());

        setOnGenericMotionListener((v, event) -> {
            if (event.getToolType(0) == MotionEvent.TOOL_TYPE_STYLUS) {
                return handleStylusHoverEvent(event);
            }
            return false;
        });

        if (capturePointerOnExternalMouse) {
            setOnCapturedPointerListener(this);
            setOnClickListener(view -> requestPointerCapture());
        }
    }

    private static StateListDrawable createTransparentBackground() {
        StateListDrawable stateListDrawable = new StateListDrawable();
        ColorDrawable focusedDrawable = new ColorDrawable(0);
        ColorDrawable defaultDrawable = new ColorDrawable(0);
        stateListDrawable.addState(new int[]{16844032}, focusedDrawable); 
        stateListDrawable.addState(new int[0], defaultDrawable);
        return stateListDrawable;
    }

    private void updateXform(int outerWidth, int outerHeight, int innerWidth, int innerHeight) {
        ViewTransformation viewTransformation = new ViewTransformation();
        viewTransformation.update(outerWidth, outerHeight, innerWidth, innerHeight);
        float invAspect = 1.0f / viewTransformation.aspect;
        if (!xServer.isStretchFullscreen()) {
            XForm.makeTranslation(xform, -viewTransformation.viewOffsetX, -viewTransformation.viewOffsetY);
            XForm.scale(xform, invAspect, invAspect);
        } else {
            XForm.makeScale(xform, (float) innerWidth / outerWidth, (float) innerHeight / outerHeight);
        }
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        updateXform(w, h, xServer.getScreenWidth(), xServer.getScreenHeight());
        resolutionScale = 1000.0f / Math.min(xServer.getScreenWidth(), xServer.getScreenHeight());
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int toolType = event.getToolType(0);
        
        if (touchscreenMouseDisabled && toolType != MotionEvent.TOOL_TYPE_STYLUS
                && !event.isFromSource(InputDevice.SOURCE_MOUSE)) {
            return true;
        }

        if (toolType == MotionEvent.TOOL_TYPE_STYLUS) {
            return handleStylusEvent(event);
        }

        int actionIndex = event.getActionIndex();
        int pointerId = event.getPointerId(actionIndex);
        int actionMasked = event.getActionMasked();
        if (pointerId >= fingers.length) {
            return true;
        }

        switch (actionMasked) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                handlePointerDown(event, actionIndex, pointerId);
                break;
            case MotionEvent.ACTION_MOVE:
                handlePointerMove(event);
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                handlePointerUp(event, actionIndex, pointerId);
                break;
            case MotionEvent.ACTION_CANCEL:
                handlePointerCancel();
                break;
        }
        return true;
    }

    private void handlePointerDown(MotionEvent event, int actionIndex, int pointerId) {
        if (event.isFromSource(InputDevice.SOURCE_MOUSE)) {
            return;
        }
        scrollAccumY = 0.0f;
        scrolling = false;
        fingers[pointerId] = new Finger(event.getX(actionIndex), event.getY(actionIndex));
        numFingers++;

        if (simTouchScreen && pointerId == 0) {
            isShortDrag = false;
            isLongDrag = false;
            initialPointerX = xServer.getPointerX();
            initialPointerY = xServer.getPointerY();
            if (Math.hypot(fingers[0].getX() - lastTouchedPosX,
                    fingers[0].getY() - lastTouchedPosY) * resolutionScale > EFFECTIVE_TOUCH_DISTANCE) {
                lastTouchedPosX = fingers[0].getX();
                lastTouchedPosY = fingers[0].getY();
            }
        }
    }

    private void handlePointerMove(MotionEvent event) {
        if (event.isFromSource(InputDevice.SOURCE_MOUSE)) {
            float[] transformedPoint = XForm.transformPoint(xform, event.getX(), event.getY());
            xServer.injectPointerMove((int) transformedPoint[0], (int) transformedPoint[1]);
            return;
        }
        for (int i = 0; i < fingers.length; i++) {
            Finger finger = fingers[i];
            if (finger != null) {
                int pointerIndex = event.findPointerIndex(i);
                if (pointerIndex >= 0) {
                    finger.update(event.getX(pointerIndex), event.getY(pointerIndex));
                    handleFingerMove(finger);
                } else {
                    handleFingerUp(finger);
                    fingers[i] = null;
                    numFingers--;
                }
            }
        }
    }

    private void handlePointerUp(MotionEvent event, int actionIndex, int pointerId) {
        Finger finger = fingers[pointerId];
        if (finger != null) {
            finger.update(event.getX(actionIndex), event.getY(actionIndex));
            handleFingerUp(finger);
            fingers[pointerId] = null;
            numFingers--;
        }
    }

    private void handlePointerCancel() {
        for (int i = 0; i < fingers.length; i++) {
            fingers[i] = null;
        }
        numFingers = 0;
    }

    private boolean handleStylusHoverEvent(MotionEvent event) {
        if (event.getActionMasked() == MotionEvent.ACTION_HOVER_MOVE) {
            float[] transformedPoint = XForm.transformPoint(xform, event.getX(), event.getY());
            xServer.injectPointerMove((int) transformedPoint[0], (int) transformedPoint[1]);
        }
        return true;
    }

    private boolean handleStylusEvent(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                if ((event.getButtonState() & MotionEvent.BUTTON_SECONDARY) != 0) {
                    handleStylusRightClick(event);
                } else {
                    handleStylusLeftClick(event);
                }
                break;
            case MotionEvent.ACTION_MOVE:
                handleStylusMove(event);
                break;
            case MotionEvent.ACTION_UP:
                handleStylusUp(event);
                break;
        }
        return true;
    }

    private void handleStylusLeftClick(MotionEvent event) {
        float[] point = XForm.transformPoint(xform, event.getX(), event.getY());
        xServer.injectPointerMove((int) point[0], (int) point[1]);
        xServer.injectPointerButtonPress(Pointer.Button.BUTTON_LEFT.code());
    }

    private void handleStylusRightClick(MotionEvent event) {
        float[] point = XForm.transformPoint(xform, event.getX(), event.getY());
        xServer.injectPointerMove((int) point[0], (int) point[1]);
        xServer.injectPointerButtonPress(Pointer.Button.BUTTON_RIGHT.code());
    }

    private void handleStylusMove(MotionEvent event) {
        float[] point = XForm.transformPoint(xform, event.getX(), event.getY());
        xServer.injectPointerMove((int) point[0], (int) point[1]);
    }

    private void handleStylusUp(MotionEvent event) {
        xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_LEFT.code());
        xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_RIGHT.code());
    }

    public void mouseMove(float x, float y, int action) {
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                fingers[0] = new Finger(x, y);
                numFingers = 1;
                break;
            case MotionEvent.ACTION_UP:
                Finger finger0 = fingers[0];
                if (finger0 != null) {
                    fingers[0] = null;
                    numFingers = 0;
                }
                break;
            case MotionEvent.ACTION_MOVE:
                Finger finger = fingers[0];
                if (finger != null) {
                    finger.update(x, y);
                    handleFingerMove(finger);
                }
                break;
        }
    }

    private void handleFingerUp(Finger finger) {
        if (pointerButtonClicksEnabled) {
            switch (numFingers) {
                case 1:
                    if (simTouchScreen) {
                        if (finger.isTap()) {
                            xServer.injectPointerMove(lastTouchedPosX, lastTouchedPosY);
                            postDelayed(() -> {
                                if (swapMouseButtons) {
                                    pressPointerButtonRight(finger);
                                    releasePointerButtonRight(finger);
                                } else {
                                    pressPointerButtonLeft(finger);
                                    releasePointerButtonLeft(finger);
                                }
                            }, MOVE_TO_CLICK_DELAY_MS);
                        }
                        if (finger.isLongPress()) {
                            xServer.injectPointerMove(lastTouchedPosX, lastTouchedPosY);
                            postDelayed(() -> {
                                if (!swapMouseButtons) {
                                    pressPointerButtonRight(finger);
                                    releasePointerButtonRight(finger);
                                } else {
                                    pressPointerButtonLeft(finger);
                                    releasePointerButtonLeft(finger);
                                }
                            }, MOVE_TO_CLICK_DELAY_MS);
                        }
                        if (isShortDrag) {
                            xServer.injectPointerMove(initialPointerX, initialPointerY);
                            isShortDrag = false;
                        }
                        if (isLongDrag) {
                            isLongDrag = false;
                            if (swapMouseButtons) {
                                releasePointerButtonRight(finger);
                            } else {
                                releasePointerButtonLeft(finger);
                            }
                        }
                    } else if (finger.isTap()) {
                        if (swapMouseButtons) {
                            pressPointerButtonRight(finger);
                        } else {
                            pressPointerButtonLeft(finger);
                        }
                    }
                    break;
                case 2:
                    Finger secondFinger = findSecondFinger(finger);
                    if (secondFinger != null && finger.isTap()) {
                        if (swapMouseButtons && !simTouchScreen) {
                            pressPointerButtonLeft(finger);
                        } else {
                            pressPointerButtonRight(finger);
                        }
                    }
                    break;
                case 3:
                    if (threeFingersTapCallback != null) {
                        for (Finger f : fingers) {
                            if (f != null && !f.isTap()) {
                                return;
                            }
                        }
                        threeFingersTapCallback.run();
                    }
                    break;
                case 4:
                    if (fourFingersTapCallback != null) {
                        for (Finger f : fingers) {
                            if (f != null && !f.isTap()) {
                                return;
                            }
                        }
                        fourFingersTapCallback.run();
                    }
                    break;
            }
        }
        releasePointerButtonLeft(finger);
        releasePointerButtonRight(finger);
    }

    private void handleFingerMove(Finger finger) {
        boolean skipPointerMove = false;
        Finger secondFinger = (numFingers == 2) ? findSecondFinger(finger) : null;
        if (secondFinger != null) {
            float currDistance = (float) Math.hypot(finger.getX() - secondFinger.getX(),
                    finger.getY() - secondFinger.getY()) * resolutionScale;
            if (currDistance < 350.0f) {
                scrollAccumY += ((finger.getY() + secondFinger.getY()) * 0.5f)
                        - ((finger.getLastY() + secondFinger.getLastY()) * 0.5f);
                if (scrollAccumY < -100.0f) {
                    xServer.injectPointerButtonPress(Pointer.Button.BUTTON_SCROLL_DOWN.code());
                    xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_SCROLL_DOWN.code());
                    scrollAccumY = 0.0f;
                } else if (scrollAccumY > 100.0f) {
                    xServer.injectPointerButtonPress(Pointer.Button.BUTTON_SCROLL_UP.code());
                    xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_SCROLL_UP.code());
                    scrollAccumY = 0.0f;
                }
                scrolling = true;
            } else if (!simTouchScreen && currDistance >= 350.0f
                    && !xServer.isPointerButtonPressed(Pointer.Button.BUTTON_LEFT.code())
                    && secondFinger.getTravelDistance() < MAX_TAP_TRAVEL_DISTANCE) {
                if (pointerButtonClicksEnabled) {
                    pressPointerButtonLeft(finger);
                    skipPointerMove = true;
                }
            }
        }
        if (!scrolling && numFingers <= 2 && !skipPointerMove) {
            if (simTouchScreen) {
                long touchDuration = System.currentTimeMillis() - finger.touchTime;
                if (touchDuration < SHORT_DRAG_MAX_TIME && finger.getTravelDistance() > MAX_TAP_TRAVEL_DISTANCE) {
                    if (!isShortDrag && !isLongDrag) {
                        isShortDrag = true;
                        moveCursorToEdge(finger);
                    }
                }
                else if (touchDuration >= LONG_DRAG_MIN_TIME && !isShortDrag) {
                    xServer.injectPointerMove(finger.getX(), finger.getY());
                    if (finger.getTravelDistance() > MAX_TAP_TRAVEL_DISTANCE) {
                        if (!isLongDrag && pointerButtonClicksEnabled) {
                            isLongDrag = true;
                            if (swapMouseButtons) {
                                pressPointerButtonRight(finger);
                            } else {
                                pressPointerButtonLeft(finger);
                            }
                        }
                    }
                }
                if (isShortDrag) {
                    moveCursorToEdge(finger);
                }
                return;
            }
            int dx = finger.getDeltaX();
            int dy = finger.getDeltaY();
            xServer.injectPointerMoveDelta(dx, dy);
        }
    }

    private void moveCursorToEdge(Finger finger) {
        int screenWidth = xServer.getScreenWidth();
        int screenHeight = xServer.getScreenHeight();
        int deltaX = finger.getX() - finger.getLastX();
        int deltaY = finger.getY() - finger.getLastY();
        if (Math.abs(deltaX) < 2 && Math.abs(deltaY) < 2) {
            return;
        }
        int targetX = (deltaX > 0) ? 0 : screenWidth - 1;
        int targetY = (deltaY > 0) ? 0 : screenHeight - 1;
        if (Math.abs(deltaX) > Math.abs(deltaY) * 2) {
            targetY = initialPointerY;
        } else if (Math.abs(deltaY) > Math.abs(deltaX) * 2) {
            targetX = initialPointerX;
        }
        xServer.injectPointerMove(targetX, targetY);
    }

    private Finger findSecondFinger(Finger finger) {
        for (Finger f : fingers) {
            if (f != null && f != finger) {
                return f;
            }
        }
        return null;
    }

    private void pressPointerButtonLeft(Finger finger) {
        if (isEnabled() && pointerButtonLeftEnabled
                && !xServer.isPointerButtonPressed(Pointer.Button.BUTTON_LEFT.code())) {
            xServer.injectPointerButtonPress(Pointer.Button.BUTTON_LEFT.code());
            fingerPointerButtonLeft = finger;
        }
    }

    private void pressPointerButtonRight(Finger finger) {
        if (isEnabled() && pointerButtonRightEnabled
                && !xServer.isPointerButtonPressed(Pointer.Button.BUTTON_RIGHT.code())) {
            xServer.injectPointerButtonPress(Pointer.Button.BUTTON_RIGHT.code());
            fingerPointerButtonRight = finger;
        }
    }

    private void releasePointerButtonLeft(Finger finger) {
        if (pointerButtonLeftEnabled && finger == fingerPointerButtonLeft
                && xServer.isPointerButtonPressed(Pointer.Button.BUTTON_LEFT.code())) {
            postDelayed(() -> {
                xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_LEFT.code());
                fingerPointerButtonLeft = null;
            }, MOVE_TO_CLICK_DELAY_MS);
        }
    }

    private void releasePointerButtonRight(Finger finger) {
        if (pointerButtonRightEnabled && finger == fingerPointerButtonRight
                && xServer.isPointerButtonPressed(Pointer.Button.BUTTON_RIGHT.code())) {
            postDelayed(() -> {
                xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_RIGHT.code());
                fingerPointerButtonRight = null;
            }, MOVE_TO_CLICK_DELAY_MS);
        }
    }

    @Override
    public boolean onCapturedPointer(View view, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_MOVE) {
            float dx = event.getX() * sensitivity;
            if (Math.abs(dx) > CURSOR_ACCELERATION_THRESHOLD) {
                dx *= CURSOR_ACCELERATION;
            }
            float dy = event.getY() * sensitivity;
            if (Math.abs(dy) > CURSOR_ACCELERATION_THRESHOLD) {
                dy *= CURSOR_ACCELERATION;
            }
            xServer.injectPointerMoveDelta(Mathf.roundPoint(dx), Mathf.roundPoint(dy));
            return true;
        }
        event.setSource(event.getSource() | InputDevice.SOURCE_MOUSE);
        return onExternalMouseEvent(event);
    }

    public boolean onExternalMouseEvent(MotionEvent event) {
        boolean handled = false;
        if (isEnabled() && event.isFromSource(InputDevice.SOURCE_MOUSE)) {
            int actionButton = event.getActionButton();
            switch (event.getAction()) {
                case MotionEvent.ACTION_HOVER_MOVE:
                    float[] point = XForm.transformPoint(xform, event.getX(), event.getY());
                    xServer.injectPointerMove((int) point[0], (int) point[1]);
                    handled = true;
                    break;
                case MotionEvent.ACTION_SCROLL:
                    float scrollY = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                    if (scrollY <= -1.0f) {
                        xServer.injectPointerButtonPress(Pointer.Button.BUTTON_SCROLL_DOWN.code());
                        xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_SCROLL_DOWN.code());
                    } else if (scrollY >= 1.0f) {
                        xServer.injectPointerButtonPress(Pointer.Button.BUTTON_SCROLL_UP.code());
                        xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_SCROLL_UP.code());
                    }
                    handled = true;
                    break;
                case MotionEvent.ACTION_BUTTON_PRESS:
                    if (actionButton == MotionEvent.BUTTON_PRIMARY) {
                        xServer.injectPointerButtonPress(Pointer.Button.BUTTON_LEFT.code());
                    } else if (actionButton == MotionEvent.BUTTON_SECONDARY) {
                        xServer.injectPointerButtonPress(Pointer.Button.BUTTON_RIGHT.code());
                    }
                    handled = true;
                    break;
                case MotionEvent.ACTION_BUTTON_RELEASE:
                    if (actionButton == MotionEvent.BUTTON_PRIMARY) {
                        xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_LEFT.code());
                    } else if (actionButton == MotionEvent.BUTTON_SECONDARY) {
                        xServer.injectPointerButtonRelease(Pointer.Button.BUTTON_RIGHT.code());
                    }
                    handled = true;
                    break;
            }
        }
        return handled;
    }

    public float[] computeDeltaPoint(float lastX, float lastY, float x, float y) {
        float[] result = {0.0f, 0.0f};
        XForm.transformPoint(xform, lastX, lastY, result);
        float transformedLastX = result[0];
        float transformedLastY = result[1];
        XForm.transformPoint(xform, x, y, result);
        result[0] = result[0] - transformedLastX;
        result[1] = result[1] - transformedLastY;
        return result;
    }

    public void setSensitivity(float sensitivity) {
        this.sensitivity = sensitivity;
    }

    public boolean isPointerButtonLeftEnabled() {
        return pointerButtonLeftEnabled;
    }

    public void setPointerButtonLeftEnabled(boolean pointerButtonLeftEnabled) {
        this.pointerButtonLeftEnabled = pointerButtonLeftEnabled;
    }

    public boolean isPointerButtonRightEnabled() {
        return pointerButtonRightEnabled;
    }

    public void setPointerButtonRightEnabled(boolean pointerButtonRightEnabled) {
        this.pointerButtonRightEnabled = pointerButtonRightEnabled;
    }

    public void setSimTouchScreen(boolean simTouchScreen) {
        this.simTouchScreen = simTouchScreen;
    }

    public void setSimTouchScreen() {
        this.simTouchScreen = !this.simTouchScreen;
    }

    public boolean isSimTouchScreen() {
        return simTouchScreen;
    }

    public boolean isPointerButtonClicksEnabled() {
        return pointerButtonClicksEnabled;
    }

    public void setPointerButtonClicksEnabled(boolean enabled) {
        this.pointerButtonClicksEnabled = enabled;
    }

    public void setThreeFingersTapCallback(Runnable threeFingersTapCallback) {
        this.threeFingersTapCallback = threeFingersTapCallback;
    }

    public void setFourFingersTapCallback(Runnable fourFingersTapCallback) {
        this.fourFingersTapCallback = fourFingersTapCallback;
    }

    public void setTouchscreenMouseDisabled(boolean disabled) {
        this.touchscreenMouseDisabled = disabled;
    }

    public void setSwapMouseButtons(boolean swap) {
        this.swapMouseButtons = swap;
    }

    public void setSwapMouseButtons() {
        this.swapMouseButtons = !this.swapMouseButtons;
    }

    public boolean isSwapMouseButtons() {
        return this.swapMouseButtons;
    }

    public void toggleFullscreen() {
        new Handler().postDelayed(() ->
                updateXform(getWidth(), getHeight(), xServer.getScreenWidth(), xServer.getScreenHeight()),
                UPDATE_FORM_DELAYED_TIME);
    }

    public class Finger {
        private int lastX;
        private int lastY;
        private final int startX;
        private final int startY;
        private final long touchTime;
        private int x;
        private int y;

        public Finger(float x, float y) {
            float[] transformedPoint = XForm.transformPoint(TouchpadView.this.xform, x, y);
            this.x = (int) transformedPoint[0];
            this.lastX = this.x;
            this.startX = this.x;
            this.y = (int) transformedPoint[1];
            this.lastY = this.y;
            this.startY = this.y;
            this.touchTime = System.currentTimeMillis();
        }

        public void update(float x, float y) {
            this.lastX = this.x;
            this.lastY = this.y;
            float[] transformedPoint = XForm.transformPoint(TouchpadView.this.xform, x, y);
            this.x = (int) transformedPoint[0];
            this.y = (int) transformedPoint[1];
        }

        private int getDeltaX() {
            float dx = (this.x - this.lastX) * sensitivity;
            if (Math.abs(dx) > CURSOR_ACCELERATION_THRESHOLD) {
                dx *= CURSOR_ACCELERATION;
            }
            return Mathf.roundPoint(dx);
        }

        private int getDeltaY() {
            float dy = (this.y - this.lastY) * sensitivity;
            if (Math.abs(dy) > CURSOR_ACCELERATION_THRESHOLD) {
                dy *= CURSOR_ACCELERATION;
            }
            return Mathf.roundPoint(dy);
        }

        private boolean isTap() {
            return System.currentTimeMillis() - this.touchTime < MAX_TAP_MILLISECONDS
                    && getTravelDistance() < MAX_TAP_TRAVEL_DISTANCE;
        }

        private boolean isLongPress() {
            return System.currentTimeMillis() - this.touchTime > LONG_PRESS_MIN_TIME
                    && getTravelDistance() < MAX_TAP_TRAVEL_DISTANCE;
        }

        private float getTravelDistance() {
            return (float) Math.hypot(this.x - this.startX, this.y - this.startY);
        }

        private int getX() {
            return this.x;
        }
        private int getY() {
            return this.y;
        }
        private int getLastX() {
            return this.lastX;
        }
        private int getLastY() {
            return this.lastY;
        }
    }
}
