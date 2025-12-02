// SceneManager.cpp
#include "SceneManager.h"
#include "../Scene/SceneBase.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/GameOverScene.h"
#include <DxLib.h>

SceneManager::SceneManager()
    : currentScene(nullptr), nextScene(SCENE_NO_CHANGE) {
    currentScene = new TitleScene();
    currentScene->SetSceneManager(this);
}

SceneManager::~SceneManager() {
    if (currentScene) {
        currentScene->Release();
        delete currentScene;
        currentScene = nullptr;
    }
}

void SceneManager::Init() {
    if (currentScene) {
        currentScene->Init();
    }
}

void SceneManager::Update() {
    if (currentScene) {
        currentScene->Update();
    }

    if (nextScene != SCENE_NO_CHANGE) {
        ChangeScene(nextScene);
        nextScene = SCENE_NO_CHANGE;
    }
}

void SceneManager::Draw() {
    ClearDrawScreen();

    if (currentScene) {
        currentScene->Draw();
    }

    ScreenFlip();
}

void SceneManager::RequestChange(NextScene next) {
    nextScene = next;
}

void SceneManager::ChangeScene(NextScene next) {
   

    if (next == SCENE_GAMEOVER && currentScene) {
        GameScene* gameScene = dynamic_cast<GameScene*>(currentScene);
        if (gameScene) {
            finalScore = gameScene->GetFinalScore();
            isNewRecord = gameScene->GetIsNewRecord();
            recordRank = gameScene->GetNewRecordRank();
        }
    }

    // ★ TitleScene → GameScene の設定値取得
    if (next == SCENE_GAME && currentScene) {
        TitleScene* titleScene = dynamic_cast<TitleScene*>(currentScene);
        if (titleScene) {
            // ★ TitleSceneから設定値を取得
            nextFallSpeed = titleScene->fallSpeed;
            nextGravityInterval = titleScene->gravityChangeInterval;
            nextAutoGravityFill = titleScene->autoGravityFill;
        }
    }

    // 既存のシーンを削除
    if (currentScene) {
        currentScene->Release();
        delete currentScene;
        currentScene = nullptr;
    }

    // 新しいシーンを生成
    switch (next) {
    case SCENE_TITLE:
        currentScene = new TitleScene();
        break;
    case SCENE_GAME:
    {
        GameScene* gameScene = new GameScene();
        // ★ GameSceneに設定値を渡す
        gameScene->SetGameSettings(nextFallSpeed, nextGravityInterval);
        currentScene = gameScene;
    }
    break;
    case SCENE_GAMEOVER:
    {
        GameOverScene* gameOverScene = new GameOverScene();
        gameOverScene->SetScore(finalScore, isNewRecord, recordRank);
        currentScene = gameOverScene;
    }
    break;
    default:
        currentScene = new TitleScene();
        break;
    }

    // 新しいシーンの初期化
    if (currentScene) {
        currentScene->SetSceneManager(this);
        currentScene->Init();
    }
}