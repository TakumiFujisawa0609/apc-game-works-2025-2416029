#include <DxLib.h>
#include "Manager/SceneManager.h"
#include "Manager/FontManager.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    ChangeWindowMode(FALSE);
    SetGraphMode(1280, 720, 32);

    SetWaitVSyncFlag(FALSE);

    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    // ★ フォント初期化
    FontManager::Init();

    const int FPS = 60;
    const int FRAME_TIME = 1000 / FPS;

    static int frameCount = 0;
    static int startTime = GetNowCount();
    static float fps = 0.0f;

    SetMouseDispFlag(TRUE);
    SceneManager manager;
    manager.Init(); //

    bool escLock = true; // ESCロック状態

    while (ProcessMessage() == 0) {
        int frameStart = GetNowCount();
        ClearDrawScreen();

        // FPS 計測ロジック
        frameCount++;
        if (GetNowCount() - startTime >= 1000) {
            fps = frameCount * 1000.0f / (GetNowCount() - startTime);
            frameCount = 0;
            startTime = GetNowCount();
        }

        // ゲームの更新と描画
        manager.Update();
        manager.Draw();

        SetFontSize(24);
        // DrawFormatString(10, 10, GetColor(255, 255, 0), "FPS: %.2f", fps);

        ScreenFlip();

        int frameEnd = GetNowCount();
        int deltaTime = frameEnd - frameStart;
        if (deltaTime < FRAME_TIME) {
            WaitTimer(FRAME_TIME - deltaTime);
        }

    }

    // ★ フォント解放
    FontManager::Release();

    DxLib_End();
    return 0;
}
