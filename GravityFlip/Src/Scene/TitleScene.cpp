#include "TitleScene.h"
#include <cstdio>
#include <cmath>  
#include "../Manager/SceneManager.h"
#include "../Manager/FontManager.h"
#include <DxLib.h>

// ★ シンプルなコンストラクタ
TitleScene::TitleScene()
	:titleBgmHandle(-1)
{
}

TitleScene::~TitleScene() {
    Release();
}

void TitleScene::Init() {
    selectedMenu = MENU_START;
    fade.Start(FADE_IN, 2);
    rankingManager.Load();

    FontManager::Init();

	//タイトル画像の読み込み
    titleImg = LoadGraph("Data/Image/Title.png");

    rankingAnimCounter = 0;

    //BGMの読み込み
    titleBgmHandle = LoadSoundMem("Data/BGM/Title_bgm.wav");

    optionSE = LoadSoundMem("Data/SE/Option_se.mp3");
    optionSE2 = LoadSoundMem("Data/SE/Option_se2.mp3");
	titleSE = LoadSoundMem("Data/SE/Title_se.mp3");
	titleSE2 = LoadSoundMem("Data/SE/Title_se2.mp3");

    // BGMのループ再生
    if (titleBgmHandle != -1) {
        PlaySoundMem(titleBgmHandle, DX_PLAYTYPE_LOOP, 0);
    }
}

void TitleScene::Update() {
    input.Update();
    fade.Update();
    particleManager.Update();

    if (isShowingRanking) {
        UpdateRankingScreen();
        return;
    }
	else if (isShowingHTP) {
		UpdateHTPScreen();
		return;
	}

    if (isShowingOption) {
        UpdateOptionScreen();
        return;
    }

	if (isShowingCredit) {
		UpdateCreditScreen();
		return;
	}

    if (fade.IsActive()) {
        if (fade.IsFinished() && fade.GetLastMode() == FADE_OUT) {
            if (sceneManager) {
                switch (selectedMenu) {
                case MENU_START:
                    sceneManager->RequestChange(SceneManager::SCENE_GAME);
                    break;
                default:
                    break;
                }
            }
        }
        return;
    }

    UpdateMenuInput();
}

void TitleScene::UpdateMenuInput() {
    int row = selectedMenu / colCount;
    int col = selectedMenu % colCount;

	if (input.IsKeyDown(KEY_INPUT_W) || input.IsKeyDown(KEY_INPUT_UP)||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_UP) || input.IsStickUpDown()) {
        row = (row - 1 + rowCount) % rowCount;
        PlaySoundMem(titleSE, DX_PLAYTYPE_BACK);
    }

	if (input.IsKeyDown(KEY_INPUT_S) || input.IsKeyDown(KEY_INPUT_DOWN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_DOWN)|| input.IsStickDownDown()) {
        row = (row + 1) % rowCount;
        PlaySoundMem(titleSE, DX_PLAYTYPE_BACK);
    }

	if (input.IsKeyDown(KEY_INPUT_A) || input.IsKeyDown(KEY_INPUT_LEFT) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_LEFT)|| input.IsStickLeftDown()) {
        col = (col - 1 + colCount) % colCount;
        PlaySoundMem(titleSE, DX_PLAYTYPE_BACK);
    }

	if (input.IsKeyDown(KEY_INPUT_D) || input.IsKeyDown(KEY_INPUT_RIGHT) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_RIGHT)|| input.IsStickRightDown()) {
        col = (col + 1) % colCount;
        PlaySoundMem(titleSE, DX_PLAYTYPE_BACK);
    }

    selectedMenu = row * colCount + col;

    if (input.IsKeyDown(KEY_INPUT_SPACE) ||
        input.IsKeyDown(KEY_INPUT_RETURN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_A)) {

        PlaySoundMem(titleSE2, DX_PLAYTYPE_BACK);

        switch (selectedMenu) {
        case MENU_START:
            fade.Start(FADE_OUT, 2);
            // ★ SceneManagerに設定を渡す
            if (sceneManager) {
                sceneManager->SetNextSceneSettings(fallSpeed, gravityChangeInterval, autoGravityFill);
                sceneManager->RequestChange(SceneManager::SCENE_GAME);
            }
            break;

        case MENU_HOWTOPLAY:
			isShowingHTP = true;
            break;

        case MENU_RANKING:
            isShowingRanking = true;
            rankingAnimCounter = 0;
            break;

        case MENU_OPTION:
            isShowingOption = true;
            break;

        case MENU_CREDIT:
			isShowingCredit = true;
            break;

        case MENU_EXIT:
            DxLib_End();
            break;
        }
    }
}
void TitleScene::UpdateOptionScreen() {

    // 項目選択処理 (W/Sキーで項目を選択)
    if (input.IsKeyDown(KEY_INPUT_W) || input.IsPadButtonDown(XINPUT_BUTTON_DPAD_UP)) {
        selectedOption = (selectedOption - 1 + OPTION_COUNT) % OPTION_COUNT;
        PlaySoundMem(optionSE, DX_PLAYTYPE_BACK);
    }
    if (input.IsKeyDown(KEY_INPUT_S) || input.IsPadButtonDown(XINPUT_BUTTON_DPAD_DOWN)) {
        selectedOption = (selectedOption + 1) % OPTION_COUNT;
        PlaySoundMem(optionSE, DX_PLAYTYPE_BACK);
    }

    // 値調整・実行処理 (A/Dキー or SPACE/ENTER)
    int valueChange = 0;
	if (CheckHitKey(KEY_INPUT_A) || input.IsKeyDown(KEY_INPUT_LEFT)||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_LEFT)
        || input.IsStickLeftDown()) {
        valueChange = -1;
    }
    if (CheckHitKey(KEY_INPUT_D) || input.IsKeyDown(KEY_INPUT_RIGHT)||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_RIGHT)
        || input.IsStickRightDown()) {
        valueChange = 1;
    }

    bool decisionInput = input.IsKeyDown(KEY_INPUT_SPACE) ||
        input.IsKeyDown(KEY_INPUT_RETURN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_A);

    bool backInput = input.IsKeyDown(KEY_INPUT_ESCAPE) ||
        input.IsPadButtonDown(XINPUT_BUTTON_B);

    // 調整可能な項目
    if (valueChange != 0) {
        if (selectedOption == OPTION_BGM_VOLUME) {
            titleBGMVolume += valueChange * 5;
            if (titleBGMVolume > 100) titleBGMVolume = 100;
            if (titleBGMVolume < 0) titleBGMVolume = 0;

            if (titleBgmHandle != -1) {
                int dxlibVolume = (int)((float)titleBGMVolume / 100.0f * 255.0f);
                ChangeVolumeSoundMem(dxlibVolume, titleBgmHandle);
            }
        }
        else if (selectedOption == OPTION_FALLSPEED) {
            fallSpeed += valueChange;
            if (fallSpeed > 100) fallSpeed = 100;
            if (fallSpeed < 1) fallSpeed = 1;
        }
        else if (selectedOption == OPTION_GRAVITY_COUNT) {
            gravityChangeInterval += valueChange;
            if (gravityChangeInterval > 60) gravityChangeInterval = 60;
            if (gravityChangeInterval < 1) gravityChangeInterval = 1;
        }
    }

    // 選択式の項目 (SPACE/ENTERまたはA/Dキーでの調整)
    if (decisionInput || valueChange != 0) {

        if (selectedOption == OPTION_BACK && decisionInput) {
            isShowingOption = false;
        }
    }

    if (backInput || (selectedOption == OPTION_BACK && decisionInput)) {
        isShowingOption = false;
    }
}

void TitleScene::UpdateHTPScreen()
{
	if (input.IsKeyDown(KEY_INPUT_ESCAPE) ||
		input.IsKeyDown(KEY_INPUT_SPACE) ||
		input.IsKeyDown(KEY_INPUT_RETURN) ||
		input.IsPadButtonDown(XINPUT_BUTTON_B) ||
		input.IsPadButtonDown(XINPUT_BUTTON_START)) {
		isShowingHTP = false;
	}
}

void TitleScene::UpdateRankingScreen() {
    rankingAnimCounter++;  // ★ アニメーションカウンタを増加

    if (input.IsKeyDown(KEY_INPUT_ESCAPE) ||
        input.IsKeyDown(KEY_INPUT_SPACE) ||
		input.IsKeyDown(KEY_INPUT_RETURN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_B) ||
        input.IsPadButtonDown(XINPUT_BUTTON_START)) {
        isShowingRanking = false;
    }

    if (input.IsKeyDown(KEY_INPUT_R)||
        input.IsPadButtonDown(XINPUT_BUTTON_Y)) {
        rankingManager.ResetRanking();
        rankingManager.Save();

        // ★ 画面中央に大爆発エフェクトを生成
        particleManager.CreateResetExplosion(640.0f, 360.0f, 200);

        // ★ 追加で画面全体にキラキラエフェクト
        for (int i = 0; i < 50; ++i) {
            float x = (float)(rand() % 1280);
            float y = (float)(rand() % 720);
            particleManager.CreateSparkle(x, y, 3);
        }
    }
}

void TitleScene::UpdateCreditScreen()
{
    if (input.IsKeyDown(KEY_INPUT_ESCAPE) ||
        input.IsKeyDown(KEY_INPUT_SPACE) ||
        input.IsKeyDown(KEY_INPUT_RETURN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_B) ||
        input.IsPadButtonDown(XINPUT_BUTTON_START)) {
        PlaySoundMem(optionSE2, DX_PLAYTYPE_BACK);
        isShowingCredit = false;
    }
}

void TitleScene::Draw() {
    if (isShowingRanking) {
        DrawRanking();
    }
	else if (isShowingHTP) {
		DrawHTP();
	}
    else if (isShowingOption) {
        DrawOption();
    }
	else if (isShowingCredit) {
		DrawCredit();
	}
    else {
        DrawMenu();
    }

    particleManager.Draw();

    fade.Draw(1280, 720);
}

void TitleScene::DrawMenu() const {

    // 背景描画
    if (titleImg != -1) {
        DrawExtendGraph(0, 0, screenW, screenH, titleImg, FALSE);
    }
    else {
        DrawBox(0, 0, screenW, screenH, GetColor(20, 20, 40), TRUE);
    }

    // メニュー項目
    const char* menuItems[MENU_COUNT] = {

         "ゲームスタート",
         "操作説明",
         "ランキング",
         "オプション",
         "クレジット",     
         "ゲーム終了"
    };

    for (int i = 0; i < MENU_COUNT; ++i) {
        int row = i / colCount; // 行
        int col = i % colCount; // 列

        int x = startX + col * colSpacing;
        int y = startY + row * rowSpacing;

        int color = (i == selectedMenu)
            ? GetColor(255, 255, 0)
            : GetColor(255, 255, 255);

        DrawStringToHandle(x, y, menuItems[i], color, FontManager::GetFont32());

        // 選択中項目の矢印
        if (i == selectedMenu) {
            DrawStringToHandle(x - 40, y, "→", GetColor(255, 255, 0), FontManager::GetFont32());
        }
    }

	// メニューのサブテキスト
    DrawSubText();
}

void TitleScene::DrawOption() const {
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(180, 80, 1100, 680, GetColor(20, 20, 40), TRUE);
    DrawBox(180, 80, 1100, 680, GetColor(100, 100, 150), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawStringToHandle(550, 100, "オプション", GetColor(255, 255, 255), FontManager::GetFont48());

    const char* optionItems[OPTION_COUNT] = {
        "BGMの音量",
        "落下スピード",
        "Gravity Timer",
        "タイトルへ"
    };

    int itemY[OPTION_COUNT] = { 200, 250, 300, 400 };
   

    for (int i = 0; i < OPTION_COUNT; ++i) {
        int color = (i == selectedOption) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
        int item_Y = itemY[i];

        DrawStringToHandle(textX, item_Y, optionItems[i], color, FontManager::GetFont36());

        if (i == selectedOption) {
            DrawStringToHandle(textX - 40, item_Y, "→", GetColor(255, 255, 0), FontManager::GetFont36());
        }

        // 値の描画
        if (i == OPTION_BGM_VOLUME || i == OPTION_FALLSPEED || i == OPTION_GRAVITY_COUNT) {
            int currentValue = 0;
            int maxVal = 0;
            const char* suffix = "";
            int barColor = GetColor(50, 200, 255);

            if (i == OPTION_BGM_VOLUME) {
                currentValue = titleBGMVolume;
                maxVal = 100;
                suffix = "%";

            }
            else if (i == OPTION_FALLSPEED) {
                currentValue = fallSpeed;
                maxVal = 100;
            }
            else if (i == OPTION_GRAVITY_COUNT) {
                currentValue = gravityChangeInterval;
                maxVal = 30;
                suffix = " 秒";
            }

            float ratio = (float)currentValue / maxVal;
            int barLength = (int)(ratio * sliderWidth);

            DrawBox(sliderX, item_Y + 8, sliderX + sliderWidth, item_Y + 28, GetColor(50, 50, 50), TRUE);
            DrawBox(sliderX, item_Y + 8, sliderX + barLength, item_Y + 28, barColor, TRUE);

            char valBuf[32];
            sprintf_s(valBuf, "%d%s", currentValue, suffix);
            DrawStringToHandle(sliderX + sliderWidth + 20, item_Y, valBuf, color, FontManager::GetFont32());
        }
        else if (i == OPTION_BACK) {
            PlaySoundMem(optionSE2, DX_PLAYTYPE_BACK);
        }
    }

    DrawStringToHandle(300, 600, "W/S(上下ボタン)で項目選択、A/D(左右ボタン)で調整 (細かな調整は十字キー)", GetColor(200, 200, 200), FontManager::GetFont24());
    DrawStringToHandle(300, 650, "SPACE/ENTER or /Bボタンで戻る", GetColor(200, 200, 200), FontManager::GetFont24());
}

void TitleScene::DrawHTP()const {


    DrawStringToHandle(500, 180, "< 操作説明 >", GetColor(180, 220, 255), FontManager::GetFont40());
    DrawStringToHandle(420, 220 + 40, "移動: WASD : 十字ボタン", GetColor(200, 200, 200), FontManager::GetFont36());
    DrawStringToHandle(420, 240 + 70, "回転: N/M : A/Bボタン", GetColor(200, 200, 200), FontManager::GetFont36());
    DrawStringToHandle(420, 260 + 100, "高速落下: SPACE : Yボタン", GetColor(200, 200, 200), FontManager::GetFont36());
    DrawStringToHandle(420, 280 + 130, "ホールド: C : Xボタン", GetColor(200, 200, 200), FontManager::GetFont36());

    DrawStringToHandle(370, 450 + 130, "テ〇リスではありません", GetColor(200, 200, 200), FontManager::GetFont56());

    DrawStringToHandle(300, 650, "SPACE/ENTER or /Bボタンでタイトルへ", GetColor(200, 200, 200), FontManager::GetFont24());
}

void TitleScene::DrawRanking() const {
    const auto& scores = rankingManager.GetRanking();

    // 点滅アニメーション
    int titleAlpha = (int)(200 + 55 * sin(rankingAnimCounter * 0.05));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, titleAlpha);
    DrawStringToHandle(530, 100, "RANKING", GetColor(255, 215, 0), FontManager::GetFont56());
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // タイトルを再描画
    DrawStringToHandle(530, 100, "RANKING", GetColor(255, 200, 0), FontManager::GetFont56());

    // ★ ランキング表示（元の仕様に戻す）
    for (size_t i = 0; i < scores.size() && i < 5; ++i) {
        char rankBuf[64];
        sprintf_s(rankBuf, "%zd.  %d", i + 1, scores[i].score);

        // ★ 順位ごとの色分け
        int rankColor;
        if (i == 0) {
            int pulse = (int)(255 * (0.8 + 0.2 * sin(rankingAnimCounter * 0.1)));
            rankColor = GetColor(pulse, (int)(pulse * 0.8), 0);  // ゴールド（点滅）
        }
        else if (i == 1) {
            rankColor = GetColor(190, 190, 190);  // シルバー
        }
        else if (i == 2) {
            rankColor = GetColor(205, 125, 50);   // ブロンズ
        }
        else {
            rankColor = GetColor(200, 200, 200);  // 通常
        }

        // アニメーション
        int slideOffset = 0;
        if (rankingAnimCounter < 60) {
            slideOffset = (int)((60 - rankingAnimCounter) * (i + 1) * 3);
        }

        DrawStringToHandle(450 + slideOffset, 220 + (int)i * 70, rankBuf, rankColor, FontManager::GetFont40());

        // マークを表示
        if (i == 0 && scores[0].score > 0) {
            DrawStringToHandle(400 + slideOffset, 215, "★", GetColor(255, 215, 0), FontManager::GetFont48());
        }

        // トップ3にコメントを表示
        if (i < 3 && scores[i].score > 0) {
            const char* medals[3] = { "すごい！！！！！", "すごい！！！", "すごい！" };
            DrawStringToHandle(750 + slideOffset, 225 + (int)i * 70, medals[i], rankColor, FontManager::GetFont36());
        }
    }

    // 操作説明
    DrawStringToHandle(380, 600, "R/Yボタンでランキングをリセットできます。", GetColor(255, 100, 100), FontManager::GetFont24());
    DrawStringToHandle(460, 650, "SPACE/Aボタンでタイトルへ", GetColor(200, 200, 200), FontManager::GetFont28());
}

void TitleScene::DrawCredit() const
{
    DrawStringToHandle(480, 180, "Gravity Flip", GetColor(255, 255, 255), FontManager::GetFont48());
    DrawStringToHandle(520, 300, "制作者：西村明洋", GetColor(255, 255, 255), FontManager::GetFont24());
    DrawStringToHandle(490, 350, "キャラ：DOT ILLUST", GetColor(255, 255, 255), FontManager::GetFont24());
    DrawStringToHandle(490, 400, "BGM/SE：Audio Stock", GetColor(255, 255, 255), FontManager::GetFont24());

}

void TitleScene::DrawSubText() const
{
    const char* subTexts[MENU_COUNT] = {
         "ゲームを開始します。", 
         "操作方法を確認できます。",
         "過去のハイスコアを確認できます。",
         "ブロックの落下速度など、設定を変更できます。", 
         "制作者情報や使用素材のクレジットを表示します。", 
         "ゲームを終了します。" 
    };

    
    // 選択されているメニューに対応するテキストを取得
    const char* text = subTexts[selectedMenu];

    // 背景（透過ボックス）を暗くしてテキストを目立たせる
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    // DrawBoxのX座標を (screenW / 2) から screenW に変更
    DrawBox(screenW / 2, subTextY - 10, screenW, screenH, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // サブテキスト本体の描画
    DrawStringToHandle(subTextX, subTextY, text, GetColor(200, 200, 255), FontManager::GetFont20());
}

void TitleScene::Release() {

	// タイトル画像の解放
    DeleteGraph(titleImg);

    // BGMの停止
    if (titleBgmHandle != -1) {
        StopSoundMem(titleBgmHandle);
    }
    
    DeleteSoundMem(titleBgmHandle);
    particleManager.Clear();
}