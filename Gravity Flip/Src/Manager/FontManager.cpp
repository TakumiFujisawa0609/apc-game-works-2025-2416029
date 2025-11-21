#include "FontManager.h"
#include <DxLib.h>

// 静的メンバの定義
int FontManager::fontHandles[FONT_COUNT] = { -1 };
const int FontManager::fontSizes[FONT_COUNT] = {
    16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 80
};
bool FontManager::initialized = false;

void FontManager::Init() {
    if (initialized) return;

    // フォントファイルを追加
    const char* fontPath = "Data/Font/PixelMplus10-Regular.ttf";

    // フォントファイルをシステムに追加
    if (AddFontResourceEx(fontPath, FR_PRIVATE, NULL) == 0) {
        // 読み込み失敗時のデバッグ出力
        printfDx("フォントファイルの読み込みに失敗: %s\n", fontPath);
    }

    // 各サイズのフォントハンドルを作成
    for (int i = 0; i < FONT_COUNT; ++i) {
        fontHandles[i] = CreateFontToHandle(
            "PixelMplus10",  // フォント名を指定
            fontSizes[i],
            10,  // 太さ（-1だと細すぎる場合がある）
            DX_FONTTYPE_ANTIALIASING  // アンチエイリアス有効
        );

        // 読み込み失敗時のフォールバック
        if (fontHandles[i] == -1) {
            // デフォルトフォントで作成
            fontHandles[i] = CreateFontToHandle(
                NULL,  // デフォルトフォント
                fontSizes[i],
                3,
                DX_FONTTYPE_ANTIALIASING
            );
        }
    }

    initialized = true;
}

void FontManager::Release() {
    if (!initialized) return;

    for (int i = 0; i < FONT_COUNT; ++i) {
        if (fontHandles[i] != -1) {
            DeleteFontToHandle(fontHandles[i]);
            fontHandles[i] = -1;
        }
    }

    // フォントリソースを解放
    const char* fontPath = "Data/Font/PixelMplus10-Regular.ttf";
    RemoveFontResourceEx(fontPath, FR_PRIVATE, NULL);

    initialized = false;
}