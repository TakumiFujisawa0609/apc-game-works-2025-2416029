#pragma once
#include "DxLib.h"

enum FadeMode {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
};

class Fade {
public:
    Fade();

    void Start(FadeMode mode, int speed = 4); // フェード開始
    void Update();                            // フェード進行
    void Draw(int screenW, int screenH);      // 黒オーバーレイ描画
    bool IsActive() const;                    // フェード中か
    bool IsFinished() const;                  // 完了したか
    FadeMode GetMode() const;                 // 現在のモード
    FadeMode GetLastMode() const;

private:
    int alpha;        // 透明度 (0~255)
    int speed;        // 変化量
    FadeMode mode;    // 今の状態
    FadeMode lastMode;
};

