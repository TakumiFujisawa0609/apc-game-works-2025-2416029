#include <DxLib.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include "../Manager/InputManager.h" 
#include "../Object/Board.h"
#include "../Fade.h"
#include "../Manager/FontManager.h"
#include "TitleScene.h"
#include "GameScene.h"

GameScene::GameScene()
{
    activeMino.px = -10;
    activeMino.py = -10;
}

GameScene::~GameScene() {
    Release();
}

void GameScene::Init() {
    gravity = GRAVITY_DOWN;
    fade.Start(FADE_IN, 2);

    board.Init();
    board.gravity = gravity;

    FontManager::Init();

    // fallCounterの初期値は落下の速さに直結するフレーム数
    fallSpeed = initFallSpeed;
    fallCounter = 0;

    // gravityChangeIntervalはフレーム数で管理
    gravityChangeInterval = initGravityIntervalSec * 60;
    gravityChangeCounter = 0;

    moveRepeatCounter = 0;
    moveRepeatDir = 0;

    score = 0;
    gameOverScore = 0;
    combo = 0;
    comboDisplayTimer = 0;

    hasHold = false;
    canHold = true;
    holdMino.px = -10;
    holdMino.py = -10;

    particleManager.Clear();

    rankingManager.Load();
    isNewRecord = false;
    newRecordRank = -1;

    nextGravityDirection = GRAVITY_DOWN;
    gravityBonus = 1.0f;
    warningStarted = false;

    pendingGravityChange = false;
    pendingGravityDirection = 0;

    srand((unsigned)time(NULL));

    nextMinos.clear();
    for (int i = 0; i < 3; ++i) {
        nextMinos.push_back(rand() % 7);
    }

    SpawnMino();

    isCountingDown = true;
    countdownTimer = 0;
    countdownValue = 3;

    bgScrollX = 0.0f;
    bgScrollY = 0.0f;
    bgAnimTimer = 0;

    totalLinesCleared = 0;
    totalMinosPlaced = 0;
    playTime = 0;

    isGameOverFreeze = false;
    gameOverFreezeTimer = 0;

    isLanding = false;
    landingTimer = 0;
    moveCount = 0;

	// 背景画像
    bgImageHandle = LoadGraph("Data/Image/GameBG.png");

	//BGM
    BGMHandle = LoadSoundMem("Data/BGM/Game_bgm.wav");
    if (BGMHandle != -1) {
        
        ChangeVolumeSoundMem(127, BGMHandle);

        
        PlaySoundMem(BGMHandle, DX_PLAYTYPE_LOOP, TRUE);
    }

	//SE
    BlockPutSE = LoadSoundMem("Data/SE/Block_put.mp3");
    HardDropSE = LoadSoundMem("Data/SE/HardDrop2.mp3");
    HoldSE = LoadSoundMem("Data/SE/Hold_SE.mp3");
    LineClearSE = LoadSoundMem("Data/SE/Line_Delete.mp3");
    menuSE = LoadSoundMem("Data/SE/Option_se.mp3");
    menuSE2 = LoadSoundMem("Data/SE/Option_se2.mp3");
	menuSE3 = LoadSoundMem("Data/SE/Title_se2.mp3");
	BlockRotateSE = LoadSoundMem("Data/SE/Block_Rotate.mp3");
	ChangeGravitySE = LoadSoundMem("Data/SE/Change_Gravity.mp3");
}

// TitleSceneから適用した設定を受け取る
void GameScene::SetGameSettings(int fallSpeed, int gravityIntervalSeconds) {
    this->initFallSpeed = fallSpeed;
    this->initGravityIntervalSec = gravityIntervalSeconds;
}

void GameScene::SetAutoGravityFill(bool enable) {
    this->autoGravityFill = enable;
}


// ミノ生成・配置
void GameScene::SpawnMino() {
    static const int minoShapes[7][4][2] = {
        { {0,1}, {1,1}, {2,1}, {3,1} },
        { {1,0}, {2,0}, {1,1}, {2,1} },
        { {1,0}, {0,1}, {1,1}, {2,1} },
        { {1,0}, {2,0}, {0,1}, {1,1} },
        { {0,0}, {1,0}, {1,1}, {2,1} },
        { {0,0}, {0,1}, {1,1}, {2,1} },
        { {2,0}, {0,1}, {1,1}, {2,1} }
    };

    int type = nextMinos.front();
    nextMinos.erase(nextMinos.begin());
    nextMinos.push_back(rand() % 7);

    activeMino.type = type;
    activeMino.color = 1 + (rand() % 6);
    activeMino.rotation = 0;

    for (int i = 0; i < 4; ++i) {
        activeMino.shape[i][0] = minoShapes[type][i][0];
        activeMino.shape[i][1] = minoShapes[type][i][1];
    }

    if (gravity == GRAVITY_DOWN) {
        activeMino.px = BOARD_W / 2 - 2;
        activeMino.py = 0;
    }
    else if (gravity == GRAVITY_UP) {
        activeMino.px = BOARD_W / 2 - 2;
        activeMino.py = BOARD_H - 4;
    }
    else if (gravity == GRAVITY_LEFT) {
        activeMino.px = BOARD_W - 4;
        activeMino.py = BOARD_H / 2 - 2;
    }
    else if (gravity == GRAVITY_RIGHT) {
        activeMino.px = 0;
        activeMino.py = BOARD_H / 2 - 2;
    }

    canHold = true;
}

// 重力・盤面回転
void GameScene::RotateBoard(GravityDirection oldGravity, GravityDirection newGravity) {
}

void GameScene::ChangeGravity(GravityDirection newGravity) {
	//効果音を再生
	PlaySoundMem(ChangeGravitySE, DX_PLAYTYPE_BACK);
    gravityBonus += 0.2f;
    if (gravityBonus > 3.0f) gravityBonus = 3.0f;

    if (fallSpeed < 40) {
        fallSpeed += 5;  // 序盤は速く加速
    }
    else if (fallSpeed < 50) {
        fallSpeed += 3;  // 中盤はそこそこ速く加速
    }
    else if (fallSpeed < 57) {
        fallSpeed += 2;  // 終盤はゆっくり加速
    }

    if (rand() % 5 == 0) {
        score += 500;
        particleManager.CreateBonus(640, 360, 30);
    }

    particleManager.CreateGravityChange(20);
    warningStarted = false;


}

// ホールド機能
void GameScene::HoldMino() {
    if (!canHold) return;

    //効果音を再生
    PlaySoundMem(HoldSE, DX_PLAYTYPE_BACK);

    static const int minoShapes[7][4][2] = {
        { {0,1}, {1,1}, {2,1}, {3,1} },
        { {1,0}, {2,0}, {1,1}, {2,1} },
        { {1,0}, {0,1}, {1,1}, {2,1} },
        { {1,0}, {2,0}, {0,1}, {1,1} },
        { {0,0}, {1,0}, {1,1}, {2,1} },
        { {0,0}, {0,1}, {1,1}, {2,1} },
        { {2,0}, {0,1}, {1,1}, {2,1} }
    };

    if (!hasHold) {
        holdMino = activeMino;
        hasHold = true;
        SpawnMino();
    }
    else {
        ActiveMino temp = activeMino;
        activeMino = holdMino;
        holdMino = temp;

        if (gravity == GRAVITY_DOWN) {
            activeMino.px = BOARD_W / 2 - 2;
            activeMino.py = 0;
        }
        else if (gravity == GRAVITY_UP) {
            activeMino.px = BOARD_W / 2 - 2;
            activeMino.py = BOARD_H - 4;
        }
        else if (gravity == GRAVITY_LEFT) {
            activeMino.px = BOARD_W - 4;
            activeMino.py = BOARD_H / 2 - 2;
        }
        else if (gravity == GRAVITY_RIGHT) {
            activeMino.px = 0;
            activeMino.py = BOARD_H / 2 - 2;
        }

        activeMino.rotation = 0;

        for (int i = 0; i < 4; ++i) {
            activeMino.shape[i][0] = minoShapes[activeMino.type][i][0];
            activeMino.shape[i][1] = minoShapes[activeMino.type][i][1];
        }
    }

    canHold = false;
}

// 落下地点に透明なブロックを描画
void GameScene::DrawGhostBlock() {
    int ddx = 0, ddy = 0;
    if (gravity == GRAVITY_DOWN)      ddy = 1;
    else if (gravity == GRAVITY_UP)   ddy = -1;
    else if (gravity == GRAVITY_LEFT) ddx = -1;
    else if (gravity == GRAVITY_RIGHT)ddx = 1;

    ActiveMino ghost = activeMino;
    while (!board.IsCollision(ghost, ddx, ddy)) {
        ghost.px += ddx;
        ghost.py += ddy;
    }

    extern const int BOARD_OFFSET_X;
    extern const int BOARD_OFFSET_Y;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    for (int i = 0; i < 4; ++i) {
        int gx = ghost.px + ghost.shape[i][0];
        int gy = ghost.py + ghost.shape[i][1];
        if (gx >= 0 && gx < BOARD_W && gy >= 0 && gy < BOARD_H) {
            DrawBox(
                BOARD_OFFSET_X + gx * BLOCK_SIZE,
                BOARD_OFFSET_Y + gy * BLOCK_SIZE,
                BOARD_OFFSET_X + (gx + 1) * BLOCK_SIZE,
                BOARD_OFFSET_Y + (gy + 1) * BLOCK_SIZE,
                GetColor(255, 255, 255),
                TRUE
            );
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

//DrawUIが多くなるので分割
void GameScene::DrawGameStats()
{
    DrawStringToHandle(statsX, statsY, "< 統計 >",
        GetColor(180, 220, 255), FontManager::GetFont24());

    // プレイ時間(タイマー式で表示する)
    int minutes = (playTime / 60) / 60;
    int seconds = (playTime / 60) % 60;
    char timeBuf[32];
    sprintf_s(timeBuf, "プレイ時間: %02d:%02d", minutes, seconds);
    DrawStringToHandle(statsX, statsY + 35, timeBuf,
        GetColor(200, 200, 200), FontManager::GetFont20());

    // 累計ライン消去数
    char linesBuf[32];
    sprintf_s(linesBuf, "消去ライン: %d", totalLinesCleared);
    DrawStringToHandle(statsX, statsY + 60, linesBuf,
        GetColor(200, 200, 200), FontManager::GetFont20());

    // 累計ブロック設置数
    char minosBuf[32];
    sprintf_s(minosBuf, "設置ブロック: %d", totalMinosPlaced);
    DrawStringToHandle(statsX, statsY + 85, minosBuf,
        GetColor(200, 200, 200), FontManager::GetFont20());


    // --- ランク表示 ---
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
    DrawStringToHandle(110, 530, rankBuf, rankColor, FontManager::GetFont32());

    // 装飾的な枠線
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(statsX - 5, statsY - 5, statsX + 250, statsY + 145,
        GetColor(50, 100, 150), FALSE);
    DrawBox(statsX - 4, statsY - 4, statsX + 250, statsY + 145,
        GetColor(50, 100, 150), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// ブロック回転
void GameScene::RotateMino(int dir) {
    int tmp[4][2];


    // 回転後の座標を計算
    for (int i = 0; i < 4; ++i) {
        int x = activeMino.shape[i][0];
        int y = activeMino.shape[i][1];
        if (dir == 1) {  // 右回転
            tmp[i][0] = 3 - y;
            tmp[i][1] = x;
        }
        else {  // 左回転
            tmp[i][0] = y;
            tmp[i][1] = 3 - x;
        }
    }

    ActiveMino test = activeMino;
    for (int i = 0; i < 4; ++i) {
        test.shape[i][0] = tmp[i][0];
        test.shape[i][1] = tmp[i][1];
    }

    // 壁蹴りのオフセットテーブル(基本的なパターン)
    // 試行順: そのまま → 右 → 左 → 上 → 下 → 右上 → 左上 → 右下 → 左下
    VECTOR offsets[] = {
        VGet(0.0f, 0.0f, 0.0f),   // そのまま
        VGet(1.0f, 0.0f, 0.0f),   // 右
        VGet(-1.0f, 0.0f, 0.0f),  // 左
        VGet(0.0f, -1.0f, 0.0f),  // 上
        VGet(0.0f, 1.0f, 0.0f),   // 下
        VGet(1.0f, -1.0f, 0.0f),  // 右上
        VGet(-1.0f, -1.0f, 0.0f), // 左上
        VGet(1.0f, 1.0f, 0.0f),   // 右下
        VGet(-1.0f, 1.0f, 0.0f),  // 左下
        VGet(2.0f, 0.0f, 0.0f),   // 右2
        VGet(-2.0f, 0.0f, 0.0f),  // 左2
    };

    // 各オフセットを試す
    for (int i = 0; i < 11; ++i) {
        test.px = activeMino.px + (int)offsets[i].x;
        test.py = activeMino.py + (int)offsets[i].y;

        if (!board.IsCollision(test, 0, 0)) {
            // 回転成功
            activeMino = test;

            // ★ 着地中に回転した場合、タイマーをリセット（回数制限あり）
            if (isLanding && moveCount < MAX_MOVE_RESETS) {
                landingTimer = 0;
                moveCount++;
            }

            return;
        }
    }

    // すべての試行が失敗した場合は回転しない
}
// ゲームオーバー判定
bool GameScene::IsGameOver() {
    return board.IsCollision(activeMino, 0, 0);
}

// 更新処理
void GameScene::Update() {
    fade.Update();
    input.Update();

    // ESCキーでメニュー
    if (input.IsKeyDown(KEY_INPUT_ESCAPE) ||
        input.IsPadButtonDown(XINPUT_BUTTON_START)) {
		PlaySoundMem(menuSE, DX_PLAYTYPE_BACK);
        isPaused = !isPaused;
        selectedMenuItem = 0;
    }

    // ポーズ中はメニュー操作のみ
    if (isPaused) {
        UpdateMenu();
        return;
    }

    particleManager.Update();

    // カウントダウン中はゲームを進めない
    if (isCountingDown) {
        UpdateCountdown();
        return;
    }


    UpdatePlay();
}

void GameScene::UpdateCountdown() {
    countdownTimer++;

    // 1秒ごとにカウントダウン
    if (countdownTimer >= 60) {
        countdownTimer = 0;
        countdownValue--;

        // カウントが0未満になったらゲーム開始
        if (countdownValue < 0) {
            isCountingDown = false;
        }
    }
}

void GameScene::UpdateMenu() {
    // メニュー項目の選択
    PlaySoundMem(menuSE2, DX_PLAYTYPE_BACK);
    if (input.IsKeyDown(KEY_INPUT_W) ||
		input.IsKeyDown(KEY_INPUT_UP) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_UP) ||
        input.IsStickUpDown()) {
        PlaySoundMem(menuSE3, DX_PLAYTYPE_BACK);
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = MENU_ITEMS - 1;
    }

    if (input.IsKeyDown(KEY_INPUT_S) ||
		input.IsKeyDown(KEY_INPUT_DOWN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_DOWN) ||
        input.IsStickDownDown()) {
        PlaySoundMem(menuSE3, DX_PLAYTYPE_BACK);
        selectedMenuItem++;
        if (selectedMenuItem >= MENU_ITEMS) selectedMenuItem = 0;
    }

    // 決定
    if (input.IsKeyDown(KEY_INPUT_SPACE) ||
        input.IsKeyDown(KEY_INPUT_RETURN) ||
        input.IsPadButtonDown(XINPUT_BUTTON_A)) {

        switch (selectedMenuItem) {
        case 0:  // 再開
            isPaused = false;
            break;

        case 1:  // リスタート
            if (sceneManager) {
                sceneManager->RequestChange(SceneManager::SCENE_GAME);
            }
            break;

        case 2:  // タイトル
            if (sceneManager) {
                sceneManager->RequestChange(SceneManager::SCENE_TITLE);
            }
            break;
		case 3:  // 終了
			DxLib_End();
			break;
        }
    }
}

void GameScene::UpdatePlay() {

    if (isGameOverFreeze) {
        gameOverFreezeTimer++;
        if (gameOverFreezeTimer >= GAME_OVER_FREEZE_DURATION) {
            // 演出終了後にゲームオーバーシーンへ遷移
            if (sceneManager) {
                sceneManager->RequestChange(SceneManager::SCENE_GAMEOVER);
            }
        }
        return;
    }


    

    //プレイ時間カウント
    playTime++;

    bgScrollX += 0.3f;  // 横スクロール速度
    bgScrollY += 0.2f;  // 縦スクロール速度

    UpdateGravityChange();
    UpdateCombo();
    HandleHoldInput();
    HandleMovement();
    HandleRotation();
    HandleFall();
    HandleHardDrop();
}

//重力変化
void GameScene::UpdateGravityChange() {
    gravityChangeCounter++;

    int remainingTime = (gravityChangeInterval - gravityChangeCounter) / 60;

    // 残り3秒になったら次の重力を決定
    // DrawChangeGravityで表示
    if (remainingTime == 3 && nextGravityDirection == gravity) {
        int newGravity;
        do {
            newGravity = rand() % 4;
        } while (newGravity == gravity);
        nextGravityDirection = newGravity;
    }

    if (remainingTime <= 3 && remainingTime > 0 && !warningStarted) {
        warningStarted = true;
        particleManager.CreateWarning(640, 100, 8);
    }

    if (gravityChangeCounter >= gravityChangeInterval) {
        gravityChangeCounter = 0;

        pendingGravityChange = true;
        pendingGravityDirection = nextGravityDirection;
        fallCounter = 61 - fallSpeed;
    }
}

void GameScene::UpdateCombo() {
    if (comboDisplayTimer > 0) comboDisplayTimer--;
}

void GameScene::HandleHoldInput() {
    if (input.IsKeyDown(KEY_INPUT_C) ||
        input.IsKeyDown(KEY_INPUT_LSHIFT) ||
        input.IsPadButtonDown(XINPUT_BUTTON_X)) {
        HoldMino();
    }
}

void GameScene::HandleMovement() {
    int local_dx = 0, local_dy = 0;
    bool moved = false;

    // 左移動 (重力がRIGHTの時は除外)
    if ((input.IsKeyDown(KEY_INPUT_A) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_LEFT) || input.IsStickLeftDown()) &&
        gravity != GRAVITY_RIGHT) { 
        PlaySoundMem(menuSE3, DX_PLAYTYPE_BACK);
        local_dx = -1;
        moveRepeatDir = -1;
        moveRepeatCounter = 0;
        moved = true;
    }
    // 右移動 (重力がLEFTの時は除外)
    else if ((input.IsKeyDown(KEY_INPUT_D) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_RIGHT) || input.IsStickRightDown()) &&
        gravity != GRAVITY_LEFT) {  
        PlaySoundMem(menuSE3, DX_PLAYTYPE_BACK);
        local_dx = 1;
        moveRepeatDir = 1;
        moveRepeatCounter = 0;
        moved = true;
    }
    // 下移動 (重力がUPの時は除外)
    else if ((input.IsKeyDown(KEY_INPUT_S) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_DOWN) || input.IsStickDownDown()) &&
        gravity != GRAVITY_UP) {  
        PlaySoundMem(menuSE3, DX_PLAYTYPE_BACK);
        local_dy = 1;
        moveRepeatDir = 2;
        moveRepeatCounter = 0;
        moved = true;
    }
    // 上移動 (重力がDOWNの時は除外)
    else if ((input.IsKeyDown(KEY_INPUT_W) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_UP) ||input.IsStickUpDown()) &&
        gravity != GRAVITY_DOWN) {  
        PlaySoundMem(menuSE3, DX_PLAYTYPE_BACK);
        local_dy = -1;
        moveRepeatDir = 3;
        moveRepeatCounter = 0;
        moved = true;
    }

    else {
        HandleAutoRepeat(local_dx, local_dy);
        moved = (local_dx != 0 || local_dy != 0);
    }

    if (local_dx != 0 || local_dy != 0) {
        ApplyMovement(local_dx, local_dy);
    }
}

// キー長押しに対応
void GameScene::HandleAutoRepeat(int& local_dx, int& local_dy) {

    // 左
    bool leftHeld = (CheckHitKey(KEY_INPUT_A) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_LEFT) || input.IsStickLeftDown()) &&
        gravity != GRAVITY_RIGHT;  

    // 右
    bool rightHeld = (CheckHitKey(KEY_INPUT_D) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_RIGHT) || input.IsStickRightDown()) &&
        gravity != GRAVITY_LEFT;

    // 下
    bool downHeld = (CheckHitKey(KEY_INPUT_S) ||
        input.IsPadButtonDown(XINPUT_BUTTON_DPAD_DOWN) || input.IsStickDownDown()) &&
        gravity != GRAVITY_UP; 

    // 上
    bool upHeld = (CheckHitKey(KEY_INPUT_W) ||
       input.IsPadButtonDown(XINPUT_BUTTON_DPAD_UP) || input.IsStickUpDown()) &&
        gravity != GRAVITY_DOWN;

    // 左移動のリピート処理
    if (leftHeld) {
        if (moveRepeatDir != -1) {
            moveRepeatDir = -1;
            moveRepeatCounter = 0;
        }
        else {
            moveRepeatCounter++;
            if (moveRepeatCounter >= REPEAT_START &&
                (moveRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
                local_dx = -1;
            }
        }
    }
    // 右移動のリピート処理
    else if (rightHeld) {
        if (moveRepeatDir != 1) {
            moveRepeatDir = 1;
            moveRepeatCounter = 0;
        }
        else {
            moveRepeatCounter++;
            if (moveRepeatCounter >= REPEAT_START &&
                (moveRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
                local_dx = 1;
            }
        }
    }
    // 下移動のリピート処理
    else if (downHeld) {
        if (moveRepeatDir != 2) {
            moveRepeatDir = 2;
            moveRepeatCounter = 0;
        }
        else {
            moveRepeatCounter++;
            if (moveRepeatCounter >= REPEAT_START &&
                (moveRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
                local_dy = 1;
            }
        }
    }
    // 上移動のリピート処理
    else if (upHeld) {
        if (moveRepeatDir != 3) {
            moveRepeatDir = 3;
            moveRepeatCounter = 0;
        }
        else {
            moveRepeatCounter++;
            if (moveRepeatCounter >= REPEAT_START &&
                (moveRepeatCounter - REPEAT_START) % REPEAT_INTERVAL == 0) {
                local_dy = -1;
            }
        }
    }
    else {
        moveRepeatDir = 0;
        moveRepeatCounter = 0;
    }
}

void GameScene::ApplyMovement(int local_dx, int local_dy) {
    int mdx = 0, mdy = 0;

    mdx = local_dx;
    mdy = local_dy;

    ActiveMino test = activeMino;
    test.px += mdx;
    test.py += mdy;

    if (!board.IsCollision(test, 0, 0)) {
        activeMino = test;

        if (isLanding && moveCount < MAX_MOVE_RESETS) {
            landingTimer = 0;
            moveCount++;
        }
    }
}

// ブロック回転処理
void GameScene::HandleRotation() {
    if (input.IsKeyDown(KEY_INPUT_N) || input.IsPadButtonDown(XINPUT_BUTTON_A)) {
		PlaySoundMem(BlockRotateSE, DX_PLAYTYPE_BACK);
        RotateMino(-1);
    }
    if (input.IsKeyDown(KEY_INPUT_M) || input.IsPadButtonDown(XINPUT_BUTTON_B)) {
        PlaySoundMem(BlockRotateSE, DX_PLAYTYPE_BACK);
        RotateMino(1);
    }
}

// ブロック落下処理
void GameScene::HandleFall() {
    fallCounter++;
    int fallInterval = 61 - fallSpeed;
    if (fallInterval < 1) fallInterval = 1;

    if (fallCounter >= fallInterval) {
        int ddx = 0, ddy = 0;
        if (gravity == GRAVITY_DOWN) ddy = 1;
        else if (gravity == GRAVITY_UP) ddy = -1;
        else if (gravity == GRAVITY_LEFT) ddx = -1;
        else if (gravity == GRAVITY_RIGHT) ddx = 1;

        // まず移動できるかチェック
        if (!board.IsCollision(activeMino, ddx, ddy)) {
            // 移動できる場合
            activeMino.px += ddx;
            activeMino.py += ddy;

            // 着地状態を解除
            isLanding = false;
            landingTimer = 0;
            moveCount = 0;
        }
        else {
            // 移動できない（着地している）場合
            if (!isLanding) {
                // 着地状態に入る
                isLanding = true;
                landingTimer = 0;
                moveCount = 0;
            }
        }

        fallCounter = 0;
    }

    // 着地中のタイマー処理（fallCounter とは別に毎フレーム更新）
    if (isLanding) {
        landingTimer++;

        // 猶予時間が過ぎたら設置
        if (landingTimer >= LOCK_DELAY) {
            LandMino();
            isLanding = false;
            landingTimer = 0;
            moveCount = 0;
        }
    }
}

void GameScene::LandMino() {

    PlaySoundMem(BlockPutSE, DX_PLAYTYPE_BACK);

	//統計のために設置数をカウント
    totalMinosPlaced++;

	// パーティクル生成
    for (int i = 0; i < 4; ++i) {
        int x = activeMino.px + activeMino.shape[i][0];
        int y = activeMino.py + activeMino.shape[i][1];
        float px = (float)(BOARD_OFFSET_X + x * BLOCK_SIZE + BLOCK_SIZE / 2);
        float py = (float)(BOARD_OFFSET_Y + y * BLOCK_SIZE + BLOCK_SIZE / 2);

        particleManager.CreateSparkle(px, py, 5);
    }

	// ブロックを盤面に固定
    board.FixMino(activeMino);
    score += (int)(10 * gravityBonus);

    std::vector<int> clearedLines = board.ClearLines();
    int linesCleared = static_cast<int>(clearedLines.size());
    if (linesCleared > 0) {
        totalLinesCleared += linesCleared;
        PlaySoundMem(LineClearSE, DX_PLAYTYPE_BACK);
        combo++;
        comboDisplayTimer = 120;

        int baseScore = linesCleared * linesCleared * 100;
        int comboBonus = combo > 1 ? (combo - 1) * 50 : 0;
        score += (int)((baseScore + comboBonus) * gravityBonus);

        for (int lineIndex : clearedLines) {
            particleManager.CreateLineClearEffect(BOARD_OFFSET_X, BOARD_OFFSET_Y,
                lineIndex, BOARD_W, BLOCK_SIZE);
        }
    }
    else {
        combo = 0;
    }

    //  重力変更の処理
    if (pendingGravityChange) {
        // 重力方向を変更
        gravity = (GravityDirection)pendingGravityDirection;
        board.gravity = (GravityDirection)pendingGravityDirection;

        // 重力適用(バグを防ぐために３回適応しておく)
        for (int i = 0; i < 3; ++i) {
            board.ApplyGravity();
        }

        // 重力適用後にラインをチェック＆消去
        std::vector<int> gravityLines = board.ClearLines();
        if (!gravityLines.empty()) {
            // ライン消去によるスコア追加
            int gravityLinesCleared = static_cast<int>(gravityLines.size());
            int gravityScore = gravityLinesCleared * gravityLinesCleared * 100;
            score += (int)(gravityScore * gravityBonus);

            // パーティクル生成
            for (int lineIndex : gravityLines) {
                particleManager.CreateLineClearEffect(BOARD_OFFSET_X, BOARD_OFFSET_Y,
                    lineIndex, BOARD_W, BLOCK_SIZE);
            }

            // ライン消去後再度重力を適用（浮いたブロックを落とす）
            for (int i = 0; i < 3; ++i) {
                board.ApplyGravity();
            }
        }

        // ボーナスやエフェクトの処理
        ChangeGravity((GravityDirection)pendingGravityDirection);

        // フラグをリセット
        pendingGravityChange = false;

        nextGravityDirection = gravity;
    }
    // 通常の隙間埋め処理（保留重力がない場合のみ）
    else if (autoGravityFill) {
        board.ApplyGravity();
    }


    SpawnMino();

    if (IsGameOver()) {
        gameOverScore = score;
        isNewRecord = rankingManager.AddScore(score);
        rankingManager.Save();

        if (isNewRecord) {
            const auto& rankings = rankingManager.GetRanking();
            for (size_t i = 0; i < rankings.size(); ++i) {
                if (rankings[i].score == score) {
                    newRecordRank = (int)i;
                    break;
                }
            }
        }
        isGameOverFreeze = true;
        gameOverFreezeTimer = 0;
    }
}

void GameScene::HandleHardDrop() {
    bool hardDropInput = input.IsPadButtonDown(XINPUT_BUTTON_Y) ||
        input.IsKeyDown(KEY_INPUT_SPACE);

    if (input.IsPadButtonDown(XINPUT_BUTTON_Y) || input.IsKeyDown(KEY_INPUT_SPACE)) {
        hardDropInput = hardDropInput;
    }
    
    if (hardDropInput) {

        isLanding = false;
        landingTimer = 0;
        moveCount = 0;

        PlaySoundMem(HardDropSE, DX_PLAYTYPE_BACK);
        totalMinosPlaced++;
        int ddx = 0, ddy = 0;
        if (gravity == GRAVITY_DOWN) ddy = 1;
        else if (gravity == GRAVITY_UP) ddy = -1;
        else if (gravity == GRAVITY_LEFT) ddx = -1;
        else if (gravity == GRAVITY_RIGHT) ddx = 1;

        ActiveMino dropMino = activeMino;
        int dropDistance = 0;
        while (!board.IsCollision(dropMino, ddx, ddy)) {
            dropMino.px += ddx;
            dropMino.py += ddy;
            dropDistance++;
        }
        activeMino = dropMino;

        for (int i = 0; i < 4; ++i) {
            int x = activeMino.px + activeMino.shape[i][0];
            int y = activeMino.py + activeMino.shape[i][1];
            float px = (float)(BOARD_OFFSET_X + x * BLOCK_SIZE + BLOCK_SIZE / 2);
            float py = (float)(BOARD_OFFSET_Y + y * BLOCK_SIZE + BLOCK_SIZE / 2);

            particleManager.CreateExplosion(px, py, 20, 255, 150, 50);
        }

        board.FixMino(activeMino);
        score += (int)((10 + dropDistance * 2) * gravityBonus);

        std::vector<int> clearedLines = board.ClearLines();
        int linesCleared = static_cast<int>(clearedLines.size());
        if (linesCleared > 0) {
			//ライン消去数を加算
            totalLinesCleared += linesCleared;

			// 効果音を再生
            PlaySoundMem(LineClearSE, DX_PLAYTYPE_BACK);
            combo++;
            comboDisplayTimer = 120;
            int baseScore = linesCleared * linesCleared * 100;
            int comboBonus = combo > 1 ? (combo - 1) * 50 : 0;
            score += (int)((baseScore + comboBonus) * gravityBonus);

            for (int lineIndex : clearedLines) {
                particleManager.CreateLineClearEffect(BOARD_OFFSET_X, BOARD_OFFSET_Y,
                    lineIndex, BOARD_W, BLOCK_SIZE);
            }
        }
        else {
            combo = 0;
        }

        // ★ 保留中の重力変更があれば実行
        if (pendingGravityChange) {
            gravity = (GravityDirection)pendingGravityDirection;
            board.gravity = (GravityDirection)pendingGravityDirection;

            // 重力適用
            for (int i = 0; i < 3; ++i) {
                board.ApplyGravity();
            }

            // ★ 重力適用後にラインをチェック＆消去
            std::vector<int> gravityLines = board.ClearLines();
            if (!gravityLines.empty()) {
                int gravityLinesCleared = static_cast<int>(gravityLines.size());
                int gravityScore = gravityLinesCleared * gravityLinesCleared * 100;
                score += (int)(gravityScore * gravityBonus);

                for (int lineIndex : gravityLines) {
                    particleManager.CreateLineClearEffect(BOARD_OFFSET_X, BOARD_OFFSET_Y,
                        lineIndex, BOARD_W, BLOCK_SIZE);
                }

                // ★ ライン消去後、再度重力を適用
                for (int i = 0; i < 3; ++i) {
                    board.ApplyGravity();
                }
            }

            ChangeGravity((GravityDirection)pendingGravityDirection);
            pendingGravityChange = false;
            nextGravityDirection = gravity;
        }
        // 通常の隙間埋め処理
        else if (autoGravityFill) {
            board.ApplyGravity();
        }

        SpawnMino();

        // ★ ゲームオーバー判定を追加
        if (IsGameOver()) {
            gameOverScore = score;
            isNewRecord = rankingManager.AddScore(score);
            rankingManager.Save();

            if (isNewRecord) {
                const auto& rankings = rankingManager.GetRanking();
                for (size_t i = 0; i < rankings.size(); ++i) {
                    if (rankings[i].score == score) {
                        newRecordRank = (int)i;
                        break;
                    }
                }
            }
            isGameOverFreeze = true;
            gameOverFreezeTimer = 0;
        }
      
    }
}

void GameScene::Draw() {
    DrawPlay();

    // ★ カウントダウン表示
    if (isCountingDown) {
        DrawCountdown();
    }

    if (isPaused) {
        DrawMenu();
    }

    fade.Draw(1280, 720);
}

void GameScene::DrawMenu() {
    // 背景を半透明で暗くする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // メニュータイトル
    const char* title = "PAUSE";
    int titleWidth = GetDrawStringWidthToHandle(title, (int)strlen(title), FontManager::GetFont80());
    DrawStringToHandle(640 - titleWidth / 2, 150, title, GetColor(255, 255, 255), FontManager::GetFont80());

    // メニュー項目
    const char* menuItems[MENU_ITEMS] = {
        "再開",
        "リスタート",
        "タイトルへ",
        "ゲーム終了"
    };

    int menuY = 320;
    int menuSpacing = 80;

    for (int i = 0; i < MENU_ITEMS; ++i) {
        int color = (i == selectedMenuItem) ? GetColor(255, 255, 0) : GetColor(200, 200, 200);
        int font = (i == selectedMenuItem) ? FontManager::GetFont48() : FontManager::GetFont40();

        int textWidth = GetDrawStringWidthToHandle(menuItems[i], (int)strlen(menuItems[i]), font);
        DrawStringToHandle(640 - textWidth / 2, menuY + i * menuSpacing, menuItems[i], color, font);

        // 選択中の項目に矢印を表示
        if (i == selectedMenuItem) {
            DrawStringToHandle(640 - textWidth / 2 - 50, menuY + i * menuSpacing, "→", GetColor(255, 255, 0), font);
        }
    }

    // 操作説明
    const char* hint = "W/S:選択  SPACE:決定  ESC:閉じる";
    int hintWidth = GetDrawStringWidthToHandle(hint, (int)strlen(hint), FontManager::GetFont20());
    DrawStringToHandle(640 - hintWidth / 2, 640, hint, GetColor(150, 150, 150), FontManager::GetFont20());
}

void GameScene::DrawPlay() {

    // 背景画像を半透明で描画
    if (bgImageHandle != -1) {
        int scrollX = (int)bgScrollX % 1280;
        int scrollY = (int)bgScrollY % 720;

        // 半透明にして描画（透明度を50%に設定）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        DrawGraph(scrollX - 1280, scrollY - 720, bgImageHandle, TRUE);
        DrawGraph(scrollX, scrollY - 720, bgImageHandle, TRUE);
        DrawGraph(scrollX - 1280, scrollY, bgImageHandle, TRUE);
        DrawGraph(scrollX, scrollY, bgImageHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // さらに暗いオーバーレイをかける
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    else {
        DrawBox(0, 0, 1280, 720, GetColor(20, 20, 40), TRUE);
    }


    // ボード部分を黒で塗りつぶす
    extern const int BOARD_OFFSET_X;
    extern const int BOARD_OFFSET_Y;
    DrawBox(BOARD_OFFSET_X, BOARD_OFFSET_Y,
        BOARD_OFFSET_X + BOARD_W * BLOCK_SIZE,
        BOARD_OFFSET_Y + BOARD_H * BLOCK_SIZE,
        GetColor(0, 0, 0), TRUE);

    // ゲームオーバー演出表示
    if (isGameOverFreeze) {
        // 点滅
        int alpha = (int)(80 + 20 * sin(gameOverFreezeTimer * 0.2f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, 1280, 720, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 盤面描画
    board.Draw();

    // ブロック設置場所の可視化
	DrawGhostBlock();

	// ブロックの描画
    DrawActiveMino();

	// ゲームオーバーラインの描画
    DrawGameOverLine();

    // パーティクル描画、ちらつき防止のリセット
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    particleManager.Draw();
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 情報表示
    DrawGravityInfo();
    DrawUI();
    DrawGameStats();

   
}


void GameScene::DrawActiveMino() {
    extern const int BOARD_OFFSET_X;
    extern const int BOARD_OFFSET_Y;

    for (int i = 0; i < 4; ++i) {
        int x = activeMino.px + activeMino.shape[i][0];
        int y = activeMino.py + activeMino.shape[i][1];
        if (x >= 0 && x < BOARD_W && y >= 0 && y < BOARD_H) {
            int c = GetColor(255, 100, 100);
            DrawBox(
                BOARD_OFFSET_X + x * BLOCK_SIZE,
                BOARD_OFFSET_Y + y * BLOCK_SIZE,
                BOARD_OFFSET_X + (x + 1) * BLOCK_SIZE,
                BOARD_OFFSET_Y + (y + 1) * BLOCK_SIZE,
                c, TRUE
            );
            DrawBox(
                BOARD_OFFSET_X + x * BLOCK_SIZE,
                BOARD_OFFSET_Y + y * BLOCK_SIZE,
                BOARD_OFFSET_X + (x + 1) * BLOCK_SIZE,
                BOARD_OFFSET_Y + (y + 1) * BLOCK_SIZE,
                GetColor(50, 50, 50), FALSE
            );
        }
    }
}

//ゲームオーバーラインの表示
void GameScene::DrawGameOverLine() {
    extern const int BOARD_OFFSET_X;
    extern const int BOARD_OFFSET_Y;

    
    int lineThickness = 3;
    int lineColor = GetColor(255, 0, 0);

    int boardDrawX1 = BOARD_OFFSET_X;
    int boardDrawY1 = BOARD_OFFSET_Y;
    int boardDrawX2 = BOARD_OFFSET_X + BOARD_W * BLOCK_SIZE;
    int boardDrawY2 = BOARD_OFFSET_Y + BOARD_H * BLOCK_SIZE;

    switch (gravity) {
    case GRAVITY_DOWN:
        lineX1 = boardDrawX1;
        lineY1 = BOARD_OFFSET_Y + 2 * BLOCK_SIZE;
        lineX2 = boardDrawX2;
        lineY2 = lineY1;
        break;
    case GRAVITY_UP:
        lineX1 = boardDrawX1;
        lineY1 = BOARD_OFFSET_Y + BOARD_H * BLOCK_SIZE - 2 * BLOCK_SIZE;
        lineX2 = boardDrawX2;
        lineY2 = lineY1;
        break;
    case GRAVITY_LEFT:
        lineX1 = BOARD_OFFSET_X + BOARD_W * BLOCK_SIZE - 2 * BLOCK_SIZE;
        lineY1 = boardDrawY1;
        lineX2 = lineX1;
        lineY2 = boardDrawY2;
        break;
    case GRAVITY_RIGHT:
        lineX1 = BOARD_OFFSET_X + 2 * BLOCK_SIZE;
        lineY1 = boardDrawY1;
        lineX2 = lineX1;
        lineY2 = boardDrawY2;
        break;
    }

    for (int i = 0; i < lineThickness; ++i) {
        if (gravity == GRAVITY_DOWN || gravity == GRAVITY_UP) {
            DrawLine(lineX1, lineY1 + i - lineThickness / 2, lineX2, lineY2 + i - lineThickness / 2, lineColor);
        }
        else {
            DrawLine(lineX1 + i - lineThickness / 2, lineY1, lineX2 + i - lineThickness / 2, lineY2, lineColor);
        }
    }
}

void GameScene::DrawGravityInfo() {
    const char* gravityText[4] = { "↓DOWN", "↑UP", "←LEFT", "→RIGHT" };
    
	// 重力方向ごとの色設定
    int gravityColors[4] = {
    GetColor(100, 200, 255), // DOWN（水色）
    GetColor(255, 80, 80),   // UP（赤）
    GetColor(255, 180, 60),  // LEFT（オレンジ）
    GetColor(80, 255, 120)   // RIGHT（緑）
    };

    int gravityColor = gravityColors[gravity];

    // 現在の重力方向表示
    DrawStringToHandle(50, 30, "Current Gravity", GetColor(255, 255, 255), FontManager::GetFont40());
    DrawStringToHandle(100, 85, gravityText[gravity], gravityColor, FontManager::GetFont40());

    // 次の重力方向表示
    int remainingTime = (gravityChangeInterval - gravityChangeCounter) / 60;
    if (remainingTime <= 5 && nextGravityDirection != gravity) {
        DrawStringToHandle(530, 200, "NEXT", GetColor(200, 200, 200), FontManager::GetFont40());
        int nextColor = gravityColors[nextGravityDirection];
        DrawStringToHandle(620, 200, gravityText[nextGravityDirection], nextColor, FontManager::GetFont40());
    }

	// 重力変化までの時間表示
    char timeBuf[32];
    sprintf_s(timeBuf, "Gravity Change: %ds", remainingTime);
    DrawStringToHandle(50, 145, timeBuf, GetColor(200, 200, 200), FontManager::GetFont32());

	// スコアボーナス表示
    char bonusBuf[32];
    sprintf_s(bonusBuf, "スコアボーナス: x%.1f", gravityBonus);
    int bonusColor = gravityBonus >= 2.0f ? GetColor(255, 215, 0) :
        gravityBonus >= 1.5f ? GetColor(255, 165, 0) :
        GetColor(200, 200, 200);
    DrawStringToHandle(50, 185, bonusBuf, bonusColor, FontManager::GetFont28());
}

void GameScene::DrawUI() {
	// スコア表示
    DrawStringToHandle(900, 30, "SCORE", GetColor(255, 255, 255), FontManager::GetFont40());
    char scoreBuf[32];
    sprintf_s(scoreBuf, "%d", score);
    DrawStringToHandle(920, 70, scoreBuf, GetColor(255, 255, 0), FontManager::GetFont40());

    if (comboDisplayTimer > 0 && combo > 1) {
        char comboBuf[32];
        sprintf_s(comboBuf, "コンボ x%d!", combo);
        int alpha = comboDisplayTimer > 60 ? 255 : (comboDisplayTimer * 255 / 60);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawStringToHandle(500, 660, comboBuf, GetColor(255, 150, 0), FontManager::GetFont48());
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

	// スピード表示
    char speedBuf[32];
    sprintf_s(speedBuf, "スピード: %d", fallSpeed);
    int speedColor = fallSpeed >= 50 ? GetColor(255, 50, 50) :
        fallSpeed >= 40 ? GetColor(255, 150, 0) :
        GetColor(200, 200, 200);
    DrawStringToHandle(900, 120, speedBuf, speedColor, FontManager::GetFont28());


	// HOLD表示
    DrawStringToHandle(150, 230, "HOLD", GetColor(255, 255, 255), FontManager::GetFont32());
    DrawStringToHandle(100, 265, "(Cキー/Xボタン)", GetColor(180, 180, 180), FontManager::GetFont32());

    //操作説明表示
    DrawStringToHandle(ManualDrawX, ManualDrawY, "< 操作説明 >", GetColor(180, 220, 255), FontManager::GetFont28());
    DrawStringToHandle(ManualDrawX, ManualDrawY + 40, "移動: WASD : 十字ボタン", GetColor(200, 200, 200), FontManager::GetFont24());
    DrawStringToHandle(ManualDrawX, ManualDrawY + 70, "回転: N/M : A/Bボタン", GetColor(200, 200, 200), FontManager::GetFont24());
    DrawStringToHandle(ManualDrawX, ManualDrawY + 100, "高速落下: SPACE : Yボタン", GetColor(200, 200, 200), FontManager::GetFont24());
    DrawStringToHandle(ManualDrawX, ManualDrawY + 130, "ホールド: C : Xボタン", GetColor(200, 200, 200), FontManager::GetFont24());

	// 重力変化カウントダウン表示
    int remainingTime = (gravityChangeInterval - gravityChangeCounter) / 60;
    if (remainingTime <= 3 && remainingTime > 0) {
        char warningBuf[32];
        sprintf_s(warningBuf, "%d", remainingTime);
        //int warningAlpha = ((gravityChangeCounter % 30) < 15) ? 255 : 128;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawStringToHandle(630, 30, warningBuf, GetColor(255, 50, 50), FontManager::GetFont80());
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        DrawStringToHandle(520, 120, "GRAVITY CHANGE!", GetColor(255, 50, 50), FontManager::GetFont32());
    }


	// ホールドブロック表示
    if (hasHold) {
        static const int minoShapes[7][4][2] = {
            { {0,1}, {1,1}, {2,1}, {3,1} },
            { {1,0}, {2,0}, {1,1}, {2,1} },
            { {1,0}, {0,1}, {1,1}, {2,1} },
            { {1,0}, {2,0}, {0,1}, {1,1} },
            { {0,0}, {1,0}, {1,1}, {2,1} },
            { {0,0}, {0,1}, {1,1}, {2,1} },
            { {2,0}, {0,1}, {1,1}, {2,1} }
        };
        int color = canHold ? GetColor(200, 200, 200) : GetColor(100, 100, 100);
        for (int i = 0; i < 4; ++i) {
            int x = minoShapes[holdMino.type][i][0];
            int y = minoShapes[holdMino.type][i][1];
            DrawBox(
                holdX + x * BLOCK_SIZE,
                holdY + y * BLOCK_SIZE,
                holdX + (x + 1) * BLOCK_SIZE,
                holdY + (y + 1) * BLOCK_SIZE,
                color, TRUE
            );
            DrawBox(
                holdX + x * BLOCK_SIZE,
                holdY + y * BLOCK_SIZE,
                holdX + (x + 1) * BLOCK_SIZE,
                holdY + (y + 1) * BLOCK_SIZE,
                GetColor(50, 50, 50), FALSE
            );
        }
    }


    //次のブロック表示
    DrawStringToHandle(950, 200, "NEXT", GetColor(255, 255, 255), FontManager::GetFont32());
    int previewX = 1000;
    int previewY = 250;
    static const int minoShapes[7][4][2] = {
        { {0,1}, {1,1}, {2,1}, {3,1} },
        { {1,0}, {2,0}, {1,1}, {2,1} },
        { {1,0}, {0,1}, {1,1}, {2,1} },
        { {1,0}, {2,0}, {0,1}, {1,1} },
        { {0,0}, {1,0}, {1,1}, {2,1} },
        { {0,0}, {0,1}, {1,1}, {2,1} },
        { {2,0}, {0,1}, {1,1}, {2,1} }
    };
    for (int n = 0; n < 3; ++n) {
        int type = nextMinos[n];
        int color = GetColor(200, 200, 200);
        for (int i = 0; i < 4; ++i) {
            int x = minoShapes[type][i][0];
            int y = minoShapes[type][i][1];
            DrawBox(
                previewX + x * BLOCK_SIZE,
                previewY + y * BLOCK_SIZE + n * 100,
                previewX + (x + 1) * BLOCK_SIZE,
                previewY + (y + 1) * BLOCK_SIZE + n * 100,
                color, TRUE
            );
            DrawBox(
                previewX + x * BLOCK_SIZE,
                previewY + y * BLOCK_SIZE + n * 100,
                previewX + (x + 1) * BLOCK_SIZE,
                previewY + (y + 1) * BLOCK_SIZE + n * 100,
                GetColor(50, 50, 50), FALSE
            );
        }
    }
}

void GameScene::DrawCountdown() {
    // 背景を半透明で暗くする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // カウントダウン表示
    if (countdownValue > 0) {
        // 数字を表示
        char countBuf[8];
        sprintf_s(countBuf, "%d", countdownValue);

        // 中央に大きく表示
        int textWidth = GetDrawStringWidthToHandle(countBuf, (int)strlen(countBuf), FontManager::GetFont80());

        // 点滅効果
        int alpha = (int)(255 * (1.0f - (countdownTimer / 60.0f) * 0.3f));
        int scale = (int)(80 + (countdownTimer / 60.0f) * 20);  // 徐々に大きく

        DrawStringToHandle(640 - textWidth / 2, 300, countBuf, GetColor(255, 255, 0), FontManager::GetFont80());
    }
    else {
        // "START!" を表示
        const char* startText = "START!!!";
        int textWidth = GetDrawStringWidthToHandle(startText, (int)strlen(startText), FontManager::GetFont64());

        // 虹色効果
        int frame = countdownTimer;
        int r = (int)(255 * (0.5 + 0.5 * sin(frame * 0.2)));
        int g = (int)(255 * (0.5 + 0.5 * sin(frame * 0.2 + 2.0)));
        int b = (int)(255 * (0.5 + 0.5 * sin(frame * 0.2 + 4.0)));

        DrawStringToHandle(640 - textWidth / 2, 300, startText, GetColor(r, g, b), FontManager::GetFont64());
    }
}

void GameScene::Release()
{
    // BGMの停止
    if (BGMHandle != -1) {
        StopSoundMem(BGMHandle);
    }

    // BGMの解放
    DeleteSoundMem(BGMHandle);

	// 効果音の解放
    DeleteSoundMem(BlockPutSE);
    DeleteSoundMem(HardDropSE);
    DeleteSoundMem(HoldSE);
    DeleteSoundMem(LineClearSE);

	// 背景画像の解放
    if (bgImageHandle != -1) {
        DeleteGraph(bgImageHandle);
    }

    // パーティクルのクリア
    particleManager.Clear();
    
}