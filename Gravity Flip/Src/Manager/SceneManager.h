#pragma once

class SceneBase;
class TitleScene;

class SceneManager {
public:
    enum NextScene {
        SCENE_NO_CHANGE,
        SCENE_TITLE,
        SCENE_GAME,
        SCENE_GAMEOVER
    };

    SceneManager();
    ~SceneManager();

    void Init();
    void Update();
    void Draw();

    int finalScore = 0;
    bool isNewRecord = false;
    int recordRank = -1;

    int nextFallSpeed = 30; //GameSceneに渡す落下スピード (デフォルト 30)
    int nextGravityInterval = 10; //GameSceneに渡す重力インターバル (デフォルト 10)

	bool nextAutoGravityFill = false; //GameSceneに渡すオートグラビティ設定
   
    //TitleSceneから設定を受け取るためのメソッド
    void SetNextSceneSettings(int fallSpeed, int gravityChangeInterval, bool autoGravity) {
        nextFallSpeed = fallSpeed;
        nextGravityInterval = gravityChangeInterval;
        nextAutoGravityFill = autoGravity;
    }

    // ★ 追加: 現在のシーンを取得
    SceneBase* GetCurrentScene() const {
        return currentScene;
    }

    // シーン遷移
    void RequestChange(NextScene next);
    NextScene nextScene;

private:
    SceneBase* currentScene;  //各シーンを管理
    void ChangeScene(NextScene next);
};