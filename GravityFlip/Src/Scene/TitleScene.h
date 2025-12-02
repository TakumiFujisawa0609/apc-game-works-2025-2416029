// TitleScene.h
#pragma once
#include <DxLib.h>
#include "SceneBase.h"
#include "GameScene.h"
#include "../Fade.h"        
#include "../Manager/InputManager.h" 
#include "../Manager/RankingManager.h" 
#include "../Manager/ParticleManager.h"

class SceneManager;


class TitleScene : public SceneBase {
public:
    enum Menu {
        MENU_START,
        MENU_HOWTOPLAY,
        MENU_RANKING,
        MENU_OPTION,
        MENU_CREDIT,
        MENU_EXIT,
        MENU_COUNT
    };

    enum OptionMenu {
        OPTION_BGM_VOLUME,
        OPTION_FALLSPEED,
        OPTION_GRAVITY_COUNT, 
        OPTION_BACK,
        OPTION_COUNT
    };

    TitleScene();
    ~TitleScene();

    void Init() override;
    void Update() override;
    void Draw() override;
    void Release() override;

    void SetSceneManager(SceneManager* manager) override {
        this->sceneManager = manager;
    }

    int selectedMenu = MENU_START;
    int selectedOption = OPTION_BGM_VOLUME;

    int fallSpeed = 30;
    int gravityChangeInterval = 10;
    bool autoGravityFill = false;  // ★ 追加: false=重力変更時のみ, true=常に隙間を埋める
    bool saveSettings = true;

private:

    SceneManager* sceneManager = nullptr;
    Fade fade;
    InputManager input;
    RankingManager rankingManager;
    ParticleManager particleManager;

    bool isShowingRanking = false;
	bool isShowingHTP = false;
    bool isShowingOption = false;
	bool isShowingCredit = false;
    int rankingAnimCounter = 0;
    int titleBgmHandle;
    int titleBGMVolume = 50;
    int titleImg;

    int optionSE;
    int optionSE2;

    int titleSE;
	int titleSE2;

	// メニュー配置定数
    const int colCount = 2;
    const int rowCount = 3;
    const int startX = 150;
    const int startY = 530;
    const int colSpacing = 300;
    const int rowSpacing = 60;

	// 画面サイズ定数
    const int screenW = 1280;
    const int screenH = 720;

    // サブテキストの描画位置
    const int subTextX = 750;
    const int subTextY = 650;

    // オプション
    int textX = 250;
    int sliderX = 650;
    int sliderWidth = 350;


	// 更新関数
    void UpdateMenuInput();
	void UpdateHTPScreen();
    void UpdateOptionScreen();
    void UpdateRankingScreen();
	void UpdateCreditScreen();

	// 描画関数
    void DrawMenu() const;
	void DrawHTP() const;
    void DrawOption() const;
    void DrawRanking() const;
    void DrawCredit() const;
    void DrawSubText() const;
};