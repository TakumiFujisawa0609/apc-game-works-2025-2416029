#include "Fade.h"

Fade::Fade() {
    alpha = 0;
    speed = 4;
    mode = FADE_NONE;
}

void Fade::Start(FadeMode m, int s) {
    mode = m;
    lastMode = m;   // Å© äJénéûÇ…ãLò^
    speed = s;
    if (mode == FADE_IN) alpha = 255;
    if (mode == FADE_OUT) alpha = 0;
}

void Fade::Update() {
    if (mode == FADE_IN) {
        alpha -= speed;
        if (alpha <= 0) {
            alpha = 0;
            mode = FADE_NONE;
        }
    }
    else if (mode == FADE_OUT) {
        alpha += speed;
        if (alpha >= 255) {
            alpha = 255;
            mode = FADE_NONE;
        }
    }
}

FadeMode Fade::GetLastMode() const {
    return lastMode;   
}

void Fade::Draw(int screenW, int screenH) {
    if (mode != FADE_NONE) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

bool Fade::IsActive() const {
    return mode != FADE_NONE;
}

bool Fade::IsFinished() const {
    return mode == FADE_NONE;
}

FadeMode Fade::GetMode() const {
    return mode;
}
