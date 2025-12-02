// GameOverScene.h
#pragma once
#include "SceneBase.h"
#include "../Fade.h"
#include "../Manager/InputManager.h"
#include <vector>

class SceneManager;

// パーティクル構造体（GameSceneと共通）
struct GameOverParticle {
    float x, y;
    float vx, vy;
    int life;
    int color;
};

class GameOverScene : public SceneBase {
public:
    GameOverScene();
    ~GameOverScene();

    // SceneBase インターフェース
    void Init() override;
    void Update() override;
    void Draw() override;
    void Release() override;
    void SetSceneManager(SceneManager* manager) override {
        this->sceneManager = manager;
    }

    // ★ スコアをセットするメソッド
    void SetScore(int finalScore, bool newRecord, int recordRank);

private:
    SceneManager* sceneManager = nullptr;

    Fade fade;
    InputManager input;

    bool isTransitioning = false;

    int score = 0;
    bool isNewRecord = false;
    int newRecordRank = -1;

    std::vector<GameOverParticle> particles;

    // 演出用
    int animCounter = 0;

    int GameOverImg = -1;

	int GameOverSE = -1;

    // パーティクル管理
    void UpdateParticles();
    void DrawParticles();
    void AddCelebrationParticles();
};