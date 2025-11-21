#pragma once

class FontManager {
public:
    enum FontSize {
        FONT_16 = 0,
        FONT_20,
        FONT_24,
        FONT_28,
        FONT_32,
        FONT_36,
        FONT_40,
        FONT_48,
        FONT_56,
        FONT_64,
        FONT_80,
        FONT_COUNT
    };

    static void Init();
    static void Release();

    static int GetFont16() { return fontHandles[FONT_16]; }
    static int GetFont20() { return fontHandles[FONT_20]; }
    static int GetFont24() { return fontHandles[FONT_24]; }
    static int GetFont28() { return fontHandles[FONT_28]; }
    static int GetFont32() { return fontHandles[FONT_32]; }
    static int GetFont36() { return fontHandles[FONT_36]; }
    static int GetFont40() { return fontHandles[FONT_40]; }
    static int GetFont48() { return fontHandles[FONT_48]; }
    static int GetFont56() { return fontHandles[FONT_56]; }
    static int GetFont64() { return fontHandles[FONT_64]; }
    static int GetFont80() { return fontHandles[FONT_80]; }

private:
    static int fontHandles[FONT_COUNT];
    static const int fontSizes[FONT_COUNT];
    static bool initialized;
};
