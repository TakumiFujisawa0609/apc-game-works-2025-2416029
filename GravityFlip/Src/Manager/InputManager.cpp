#include "../Manager/InputManager.h"
#include <string.h>

void InputManager::Update() {
    // キーボード更新
    memcpy(oldKeys, keys, 256);
    GetHitKeyStateAll(keys);

    // Xboxコントローラー更新
    oldPadState = padState;
    GetJoypadXInputState(DX_INPUT_PAD1, &padState);

    // アナログスティックのリピートカウンター更新
    const int REPEAT_START = 15;  // 初回リピートまでのフレーム数
    const int REPEAT_INTERVAL = 3; // リピート間隔

    // 左スティック
    if (IsStickLeft()) {
        stickLeftRepeatCounter++;
    }
    else {
        stickLeftRepeatCounter = 0;
    }

    // 右スティック
    if (IsStickRight()) {
        stickRightRepeatCounter++;
    }
    else {
        stickRightRepeatCounter = 0;
    }

    // 下スティック
    if (IsStickUp()) {
        stickUpRepeatCounter++;
    }
    else {
        stickUpRepeatCounter = 0;
    }

    // 下スティック
    if (IsStickDown()) {
        stickDownRepeatCounter++;
    }
    else {
        stickDownRepeatCounter = 0;
    }

    // ★十字キーのリピートカウンター更新
    if (padState.Buttons[XINPUT_BUTTON_DPAD_LEFT]) {
        dpadLeftRepeatCounter++;
    }
    else {
        dpadLeftRepeatCounter = 0;
    }

    if (padState.Buttons[XINPUT_BUTTON_DPAD_RIGHT]) {
        dpadRightRepeatCounter++;
    }
    else {
        dpadRightRepeatCounter = 0;
    }

    if (padState.Buttons[XINPUT_BUTTON_DPAD_DOWN]) {
        dpadDownRepeatCounter++;
    }
    else {
        dpadDownRepeatCounter = 0;
    }

    if (padState.Buttons[XINPUT_BUTTON_DPAD_UP]) {
        dpadUpRepeatCounter++;
    }
    else {
        dpadUpRepeatCounter = 0;
    }
}

// キーボード入力
bool InputManager::IsKeyDown(int key) const {
    return (keys[key] == 1 && oldKeys[key] == 0);
}

bool InputManager::IsKeyUp(int key) const {
    return (keys[key] == 0 && oldKeys[key] == 1);
}

bool InputManager::IsKeyPressed(int key) const {
    return (keys[key] == 1);
}

// Xboxコントローラーのボタン入力
bool InputManager::IsPadButtonUp(int button) const {
    return (padState.Buttons[button] == 0 && oldPadState.Buttons[button] == 1);
}

bool InputManager::IsPadButtonPressed(int button) const {
    return (padState.Buttons[button] == 1);
}

// アナログスティックの左右上下判定
bool InputManager::IsStickLeft() const {
    return (padState.ThumbLX < -STICK_DEADZONE);
}

bool InputManager::IsStickRight() const {
    return (padState.ThumbLX > STICK_DEADZONE);
}

bool InputManager::IsStickDown() const {
    return (padState.ThumbLY < -STICK_DEADZONE);
}

bool InputManager::IsStickUp() const {
    return (padState.ThumbLY > STICK_DEADZONE);
}

// アナログスティックの押した瞬間（リピート対応）
bool InputManager::IsStickLeftDown() {
    const int REPEAT_START = 15;
    const int REPEAT_INTERVAL = 3;

    if (stickLeftRepeatCounter == 1) {
        return true; // 初回入力
    }
    if (stickLeftRepeatCounter >= REPEAT_START &&
        (stickLeftRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
        return true; // リピート入力
    }
    return false;
}

bool InputManager::IsStickRightDown() {
    const int REPEAT_START = 15;
    const int REPEAT_INTERVAL = 3;

    if (stickRightRepeatCounter == 1) {
        return true;
    }
    if (stickRightRepeatCounter >= REPEAT_START &&
        (stickRightRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
        return true;
    }
    return false;
}

bool InputManager::IsStickUpDown() {
    const int REPEAT_START = 15;
    const int REPEAT_INTERVAL = 3;

    if (stickUpRepeatCounter == 1) {
        return true;
    }
    if (stickUpRepeatCounter >= REPEAT_START &&
        (stickUpRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
        return true;
    }
    return false;
}


bool InputManager::IsStickDownDown() {
    const int REPEAT_START = 15;
    const int REPEAT_INTERVAL = 3;

    if (stickDownRepeatCounter == 1) {
        return true;
    }
    if (stickDownRepeatCounter >= REPEAT_START &&
        (stickDownRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
        return true;
    }
    return false;
}

// ★十字キーのボタンダウン判定（リピート対応版）
bool InputManager::IsPadButtonDown(int button) {  // constを外す
    // 十字キーの場合はリピート対応
    if (button == XINPUT_BUTTON_DPAD_LEFT) {
        if (dpadLeftRepeatCounter == 1) return true;
        const int REPEAT_START = 15;
        const int REPEAT_INTERVAL = 3;
        if (dpadLeftRepeatCounter >= REPEAT_START &&
            (dpadLeftRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
            return true;
        }
        return false;
    }
    if (button == XINPUT_BUTTON_DPAD_RIGHT) {
        if (dpadRightRepeatCounter == 1) return true;
        const int REPEAT_START = 15;
        const int REPEAT_INTERVAL = 3;
        if (dpadRightRepeatCounter >= REPEAT_START &&
            (dpadRightRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
            return true;
        }
        return false;
    }
    if (button == XINPUT_BUTTON_DPAD_DOWN) {
        if (dpadDownRepeatCounter == 1) return true;
        const int REPEAT_START = 15;
        const int REPEAT_INTERVAL = 3;
        if (dpadDownRepeatCounter >= REPEAT_START &&
            (dpadDownRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
            return true;
        }
        return false;
    }
    if (button == XINPUT_BUTTON_DPAD_UP) {
        if (dpadUpRepeatCounter == 1) return true;
        const int REPEAT_START = 15;
        const int REPEAT_INTERVAL = 3;
        if (dpadUpRepeatCounter >= REPEAT_START &&
            (dpadUpRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
            return true;
        }
        return false;
    }

    // それ以外のボタンは通常の押した瞬間判定
    return (padState.Buttons[button] == 1 && oldPadState.Buttons[button] == 0);
}