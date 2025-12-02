#include "GameOverScene.h"
#include "../Manager/SceneManager.h"
#include "../Manager/FontManager.h"
#include <DxLib.h>
#include <cstdlib>

GameOverScene::GameOverScene() {
}

GameOverScene::~GameOverScene() {
    Release();
}

void GameOverScene::Init() {
    fade.Start(FADE_IN, 2);
    animCounter = 0;
    isTransitioning = false;  

    FontManager::Init();

    // 背景画像を読み込み
    GameOverImg = LoadGraph("Data/Image/Finish.png");

	GameOverSE = LoadSoundMem("Data/SE/GameOver.mp3");
    PlaySoundMem(GameOverSE, DX_PLAYTYPE_BACK);

    // 新記録の場合はお祝いパーティクル
    if (isNewRecord) {
        AddCelebrationParticles();
    }

    ChangeVolumeSoundMem(150, GameOverSE);
}

void GameOverScene::Release() {
    particles.clear();

    // 画像を解放
    if (GameOverImg != -1) {
        DeleteGraph(GameOverImg);
        GameOverImg = -1;
    }
}

void GameOverScene::Update() {
    input.Update();
    fade.Update();
    UpdateParticles();

    animCounter++;


    // ★ フェードイン完了後かつ、まだ遷移していない場合のみキー入力を受け付ける
    if (!isTransitioning && fade.IsFinished()) {
        if (input.IsKeyDown(KEY_INPUT_SPACE) ||
            input.IsKeyDown(KEY_INPUT_RETURN) ||
            input.IsPadButtonDown(XINPUT_BUTTON_A) ||
            input.IsPadButtonDown(XINPUT_BUTTON_START)) {

            isTransitioning = true;  // ★ 遷移フラグを立てる
            fade.Start(FADE_OUT, 2);
        }
    }

	if (input.IsKeyDown(KEY_INPUT_R)||
        input.IsPadButtonDown(XINPUT_BUTTON_Y)) {
        if (sceneManager) {
            sceneManager->RequestChange(SceneManager::SCENE_GAME);
        }
	}

    // ★ フェードアウト完了後にシーン遷移
    if (isTransitioning && fade.IsFinished()) {
        if (sceneManager) {
            sceneManager->RequestChange(SceneManager::SCENE_TITLE);
        }
    }

}

void GameOverScene::Draw() {
    // 背景画像を描画（画面全体に拡大）
    if (GameOverImg != -1) {
        DrawExtendGraph(0, 0, 1280, 720, GameOverImg, FALSE);
    }
    else {
        // 画像が読み込めなかった場合は黒背景
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

	// スコア表示
    char scoreBuf[32];
    sprintf_s(scoreBuf, "%d", score);
    int scoreWidth = GetDrawStringWidthToHandle(scoreBuf, (int)strlen(scoreBuf), FontManager::GetFont48());
    DrawStringToHandle(620 - scoreWidth / 2, 600, scoreBuf, GetColor(255, 255, 0), FontManager::GetFont48());

    // 新記録の表示
    if (isNewRecord) {
        char newRecordText[64];
        sprintf_s(newRecordText, "NEW RECORD! Rank %d!", newRecordRank + 1);
        int textWidth = GetDrawStringWidthToHandle(newRecordText, (int)strlen(newRecordText), FontManager::GetFont36());

        int blinkInterval = 40;
        if ((animCounter / blinkInterval) % 2 == 0) {
            DrawStringToHandle(610 - textWidth / 2, 550, newRecordText, GetColor(255, 50, 50), FontManager::GetFont36());
        }
    }

	// ランク表示
    const char* rank = "D";
    int rankColor = GetColor(200, 200, 200);

    if (score >= 20000) {
        rank = "S";
        rankColor = GetColor(255, 220, 50);
    }
    else if (score >= 15000) {
        rank = "A";
        rankColor = GetColor(255, 100, 100);
    }
    else if (score >= 10000) {
        rank = "B";
        rankColor = GetColor(100, 200, 255);
    }
    else if (score >= 5000) {
        rank = "C";
        rankColor = GetColor(150, 255, 150);
    }

    char rankBuf[32];
    sprintf_s(rankBuf, "RANK: %s", rank);
    DrawStringToHandle(700, 600, rankBuf, rankColor, FontManager::GetFont48());


    // 操作説明
    const char* instruction = "SPACE/Aボタンでタイトル、R/Yボタンでリスタート";
    int instructionWidth = GetDrawStringWidthToHandle(instruction, (int)strlen(instruction), FontManager::GetFont32());
    DrawStringToHandle(640 - instructionWidth / 2, 670, instruction, GetColor(255, 255, 255), FontManager::GetFont32());

    // パーティクル描画
    DrawParticles();

    // フェード描画
    fade.Draw(1280, 720);
}

void GameOverScene::SetScore(int finalScore, bool newRecord, int recordRank) {
    score = finalScore;
    isNewRecord = newRecord;
    newRecordRank = recordRank;
}

void GameOverScene::UpdateParticles() {
    for (auto it = particles.begin(); it != particles.end(); ) {
        GameOverParticle& p = *it;
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 0.3f;
        p.life--;

        if (p.life <= 0) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void GameOverScene::DrawParticles() {
    if (particles.empty()) return;

    SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
    for (const auto& p : particles) {
        int alpha = (int)(255.0f * p.life / 50.0f);
        if (alpha > 255) alpha = 255;
        if (alpha < 0) alpha = 0;

        int r = (GetRValue(p.color) * alpha) / 255;
        int g = (GetGValue(p.color) * alpha) / 255;
        int b = (GetBValue(p.color) * alpha) / 255;

        DrawCircle((int)p.x, (int)p.y, 4, GetColor(r, g, b), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameOverScene::AddCelebrationParticles() {
    for (int i = 0; i < 50; ++i) {
        GameOverParticle p;
        p.x = (float)(640 + (rand() % 400 - 200));
        p.y = (float)(300 + (rand() % 200 - 100));
        p.vx = (float)(rand() % 200 - 100) / 20.0f;
        p.vy = (float)(rand() % 200 - 100) / 20.0f - 3.0f;
        p.life = 60 + rand() % 40;
        p.color = GetColor(255, 215, 0);
        particles.push_back(p);
    }
}