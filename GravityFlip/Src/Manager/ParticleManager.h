#pragma once
#include <vector>
#include "DxLib.h"

// パーティクル構造体
struct Particle {
    float x, y;      // 位置
    float vx, vy;    // 速度
    int life;        // 残り寿命
    int maxLife;     // 最大寿命（フェード計算用）
    int color;       // 色
    float size;      // サイズ
    int blendMode;   // ブレンドモード (DX_BLENDMODE_ADD, DX_BLENDMODE_ALPHA等)
};

// パーティクルの種類
enum ParticleType {
    PARTICLE_NORMAL,        // 通常
    PARTICLE_EXPLOSION,     // 爆発
    PARTICLE_SPARKLE,       // キラキラ
    PARTICLE_GRAVITY_CHANGE,// 重力変更
    PARTICLE_BONUS,         // ボーナス
    PARTICLE_WARNING        // 警告
};

class ParticleManager {
public:
    ParticleManager();
    ~ParticleManager();

    // 基本操作
    void Update();
    void Draw();
    void Clear();

    // パーティクル生成（汎用）
    void AddParticle(float x, float y, float vx, float vy, int life, int color, float size = 3.0f, int blendMode = DX_BLENDMODE_ADD);

    // パーティクル生成（プリセット）
    void CreateExplosion(float x, float y, int count = 50, int colorR = 255, int colorG = 100, int colorB = 50);
    void CreateSparkle(float x, float y, int count = 20);
    void CreateGravityChange(int count = 30);
    void CreateBonus(float x, float y, int count = 20);
    void CreateWarning(float x, float y, int count = 8);
    void CreateResetExplosion(float centerX, float centerY, int count = 200);
    void CreateLineClearing(int boardX, int boardY, int boardWidth, int boardHeight, int blockSize);
    void CreateLineClearEffect(int boardX, int boardY, int lineY, int boardWidth, int blockSize);

    // ゲッター
    int GetParticleCount() const { return static_cast<int>(particles.size()); }

private:
    std::vector<Particle> particles;

    // 内部ヘルパー関数
    float RandomFloat(float min, float max);
    int RandomInt(int min, int max);
};