#pragma once
#include <vector>
#include "SceneBase.h"
#include "../Object/Board.h"
#include "../Object/Block.h"
#include "../Fade.h"
#include "../Manager/InputManager.h"
#include "../Manager/RankingManager.h"
#include "../Manager/ParticleManager.h" 


class SceneManager;

class GameScene : public SceneBase {
public:
    static const int REPEAT_START = 15;
    static const int REPEAT_INTERVAL = 3;

    GameScene();
    ~GameScene();

    void Init() override;
    void Update() override;
    void Draw() override;
    void Release() override;

    void SetSceneManager(SceneManager* manager) override {
        this->sceneManager = manager;
    }

    void SetGameSettings(int fallSpeed, int gravityIntervalSeconds);

    ActiveMino activeMino;
    Board board;
    GravityDirection gravity;

    void SpawnMino();
    void RotateMino(int dir);
    bool IsGameOver();

    int GetFinalScore() const { return gameOverScore; }
    bool GetIsNewRecord() const { return isNewRecord; }
    int GetNewRecordRank() const { return newRecordRank; }

    void SetAutoGravityFill(bool enable);

private:
    SceneManager* sceneManager = nullptr;

    RankingManager rankingManager;
    ParticleManager particleManager; 

    Fade fade;
    InputManager input;

    //ブロック処理
    int fallCounter = 0;
    int fallSpeed = 30;
    int moveRepeatCounter = 0;
    int moveRepeatDir = 0;

    // ロックディレイ関連
    bool isLanding = false;                // 着地状態かどうか
    int landingTimer = 0;                  // 着地からの経過時間
    static const int LOCK_DELAY = 15;      // 着地猶予時間
    int moveCount = 0;                     // 着地中の移動/回転回数
    static const int MAX_MOVE_RESETS = 5; // 最大リセット回数

	//スコア関連
    int score = 0;
    int gameOverScore = 0;
    int combo = 0;
    int comboDisplayTimer = 0;

    //ブロック管理
    std::vector<int> nextMinos;
    ActiveMino holdMino;
    bool hasHold = false;
    bool canHold = true;

	// ランキング関連
    bool isNewRecord = false;
    int newRecordRank = -1;

	// ゲームオーバーライン描画
    int lineX1 = 0, lineY1 = 0, lineX2 = 0, lineY2 = 0;

	// 重力変更関連
    int gravityChangeCounter = 0;
    int gravityChangeInterval = 600;
    int initFallSpeed = 30;
    int initGravityIntervalSec = 10;
    int nextGravityDirection = 0;
    float gravityBonus = 1.0f;
    bool warningStarted = false;

    //自動落下フラグ
    bool autoGravityFill = false;

	//ブロック設置後に重力変化させるためのフラグ
    bool pendingGravityChange = false; 
    int pendingGravityDirection = 0;    

    // 更新処理
    void UpdatePlay();
    void UpdateCountdown();
    void UpdateGravityChange();
    void UpdateCombo();
    void UpdateMenu();

	// 入力処理
    void HandleHoldInput();
    void HandleMovement();
    void HandleAutoRepeat(int& local_dx, int& local_dy);

	// 動作処理
    void ApplyMovement(int local_dx, int local_dy);
    void HandleRotation();
    void HandleFall();
    void HandleHardDrop();

	// ランダムブロック処理
    void LandMino();

    // 描画処理
    void DrawPlay();
    void DrawActiveMino();

	// その他の描画処理
    void DrawCountdown();
    void DrawGameOverLine();
    void DrawGravityInfo();
    void DrawUI();
    void DrawGhostBlock();
	void DrawGameStats();
  
	// メニュー描画
	void DrawMenu();

	// 補助関数
    void HoldMino();
    void ChangeGravity(GravityDirection newGravity);
    void RotateBoard(GravityDirection oldGravity, GravityDirection newGravity);

	// カウントダウン関連
    bool isCountingDown = true;
    int countdownTimer = 0;
    int countdownValue = 3;

	// サウンド関連
	int BGMHandle;
    int BlockPutSE;
	int LineClearSE;
    int HoldSE;
	int HardDropSE;
	int menuSE;
	int menuSE2;
	int menuSE3;
	int BlockRotateSE;
	int ChangeGravitySE;

    // ゲームオーバー演出関連
    bool isGameOverFreeze = false;
    int gameOverFreezeTimer = 0;
    static const int GAME_OVER_FREEZE_DURATION = 90; // 1.5秒間停止

    // 背景画像関連
    int bgImageHandle = -1;

	// DrawHold関連
    int holdX = 130;
    int holdY = 300;

	// DrawManual関連
    int ManualDrawX = 900;
    int ManualDrawY = 550;

	// DrawGameStats関連
    int statsX = 50;
    int statsY = 420;

    // 背景関連
    float bgScrollX;
    float bgScrollY;
    int bgAnimTimer;
    int totalLinesCleared;  
    int totalMinosPlaced;   
    int playTime;   

	// ポーズ関連
    bool isPaused = false;
    int selectedMenuItem = 0;
    static const int MENU_ITEMS = 4;
};