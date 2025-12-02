#pragma once
#include "DxLib.h"

class InputManager {
private:
    char keys[256];
    char oldKeys[256];

    // Xboxコントローラーの状態
    XINPUT_STATE padState;
    XINPUT_STATE oldPadState;

    // アナログスティックのデッドゾーン
    static const int STICK_DEADZONE = 8000;

    // リピート入力用
    int stickLeftRepeatCounter;
    int stickRightRepeatCounter;
	int stickUpRepeatCounter;
    int stickDownRepeatCounter;

    int dpadLeftRepeatCounter;
    int dpadRightRepeatCounter;
    int dpadDownRepeatCounter;
    int dpadUpRepeatCounter = 0;

public:
    InputManager() : stickLeftRepeatCounter(0), stickRightRepeatCounter(0), stickDownRepeatCounter(0),
        dpadLeftRepeatCounter(0), dpadRightRepeatCounter(0), dpadDownRepeatCounter(0) {
        memset(keys, 0, sizeof(keys));
        memset(oldKeys, 0, sizeof(oldKeys));
        memset(&padState, 0, sizeof(padState));
        memset(&oldPadState, 0, sizeof(oldPadState));
    }

    void Update();

    // キーボード入力
    bool IsKeyDown(int key) const;
    bool IsKeyUp(int key) const;
    bool IsKeyPressed(int key) const;

    // Xboxコントローラーのボタン入力（押した瞬間）
    bool IsPadButtonDown(int button);  // constを削除
    bool IsPadButtonUp(int button) const;
    bool IsPadButtonPressed(int button) const;

    // アナログスティックの入力
    bool IsStickLeft() const;
    bool IsStickRight() const;
    bool IsStickDown() const;
    bool IsStickUp() const;

    // アナログスティックの押した瞬間の判定
    bool IsStickLeftDown();
    bool IsStickRightDown();
    bool IsStickDownDown();
    bool IsStickUpDown();
};