#include "ParticleManager.h"
#include <cstdlib>
#include <cmath>

ParticleManager::ParticleManager() {
    particles.clear();
}

ParticleManager::~ParticleManager() {
    Clear();
}

void ParticleManager::Update() {
    for (auto it = particles.begin(); it != particles.end();) {
        Particle& p = *it;

        // 位置更新
        p.x += p.vx;
        p.y += p.vy;

        // 重力適用
        p.vy += 0.3f;

        // 寿命減少
        p.life--;

        // 寿命が尽きたら削除
        if (p.life <= 0) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ParticleManager::Draw() {
    if (particles.empty()) return;

    // まずADDブレンドのパーティクル
    SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
    for (const auto& p : particles) {
        if (p.blendMode != DX_BLENDMODE_ADD) continue;

        int alpha = (int)(255.0f * p.life / (float)p.maxLife);
        int r = (GetRValue(p.color) * alpha) / 255;
        int g = (GetGValue(p.color) * alpha) / 255;
        int b = (GetBValue(p.color) * alpha) / 255;
        DrawCircle((int)p.x, (int)p.y, (int)p.size, GetColor(r, g, b), TRUE);
    }

    // 次にALPHAブレンドのパーティクル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    for (const auto& p : particles) {
        if (p.blendMode != DX_BLENDMODE_ALPHA) continue;

        int alpha = (int)(255.0f * p.life / (float)p.maxLife);
        int r = (GetRValue(p.color) * alpha) / 255;
        int g = (GetGValue(p.color) * alpha) / 255;
        int b = (GetBValue(p.color) * alpha) / 255;
        DrawCircle((int)p.x, (int)p.y, (int)p.size, GetColor(r, g, b), TRUE);
    }

    // 最後に戻す
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}


void ParticleManager::Clear() {
    particles.clear();
}

void ParticleManager::AddParticle(float x, float y, float vx, float vy, int life, int color, float size, int blendMode) {
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.life = life;
    p.maxLife = life;
    p.color = color;
    p.size = size;
    p.blendMode = blendMode;

    particles.push_back(p);
}

// 爆発エフェクト
void ParticleManager::CreateExplosion(float x, float y, int count, int colorR, int colorG, int colorB) {
    for (int i = 0; i < count; ++i) {
        float angle = RandomFloat(0.0f, 6.28318f); // 0~2π
        float speed = RandomFloat(1.0f, 8.0f);

        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;

        int life = RandomInt(20, 50);
        int color = GetColor(
            colorR + RandomInt(-30, 30),
            colorG + RandomInt(-30, 30),
            colorB + RandomInt(-30, 30)
        );

        AddParticle(x, y, vx, vy, life, color, RandomFloat(2.0f, 5.0f), DX_BLENDMODE_ADD);
    }
}

// キラキラエフェクト
void ParticleManager::CreateSparkle(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        float vx = RandomFloat(-3.0f, 3.0f);
        float vy = RandomFloat(-6.0f, -2.0f);
        int life = RandomInt(30, 50);

        int color = GetColor(
            200 + RandomInt(0, 55),
            200 + RandomInt(0, 55),
            50 + RandomInt(0, 100)
        );

        AddParticle(x, y, vx, vy, life, color, RandomFloat(2.0f, 4.0f), DX_BLENDMODE_ADD);
    }
}

// 重力変更エフェクト（画面全体）
void ParticleManager::CreateGravityChange(int count) {
    for (int i = 0; i < count; ++i) {
        float x = RandomFloat(0.0f, 1280.0f);
        float y = RandomFloat(0.0f, 720.0f);
        float vx = RandomFloat(-5.0f, 5.0f);
        float vy = RandomFloat(-5.0f, 5.0f);
        int life = RandomInt(30, 60);

        int color = GetColor(
            200 + RandomInt(0, 55),
            100 + RandomInt(0, 155),
            50 + RandomInt(0, 100)
        );

        AddParticle(x, y, vx, vy, life, color, RandomFloat(2.0f, 5.0f), DX_BLENDMODE_ADD);
    }
}

// ボーナスエフェクト（金色）
void ParticleManager::CreateBonus(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        float vx = RandomFloat(-6.0f, 6.0f);
        float vy = RandomFloat(-10.0f, -5.0f);
        int life = RandomInt(40, 60);
        int color = GetColor(255, 215, 0); // 金色

        AddParticle(x, y, vx, vy, life, color, RandomFloat(3.0f, 6.0f), DX_BLENDMODE_ADD);
    }
}

// 警告エフェクト（赤）
void ParticleManager::CreateWarning(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
        float vx = 0;
        float vy = RandomFloat(1.0f, 3.0f);
        int life = 20;
        int color = GetColor(255, 50, 50);

        AddParticle(
            x + RandomFloat(-50.0f, 50.0f),
            y + RandomFloat(-25.0f, 25.0f),
            vx, vy, life, color, 4.0f, DX_BLENDMODE_ALPHA
        );
    }
}

// リセット時の大爆発
void ParticleManager::CreateResetExplosion(float centerX, float centerY, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = RandomFloat(0.0f, 6.28318f);
        float speed = RandomFloat(100.0f, 400.0f);

        float vx = cos(angle) * speed / 60.0f; // 60FPS想定で割る
        float vy = sin(angle) * speed / 60.0f - 3.3f;

        int colorType = RandomInt(0, 4);
        int color;
        switch (colorType) {
        case 0: color = GetColor(255, 50, 50); break;   // 赤
        case 1: color = GetColor(255, 200, 50); break;  // 黄
        case 2: color = GetColor(50, 255, 50); break;   // 緑
        case 3: color = GetColor(50, 200, 255); break;  // 青
        case 4: color = GetColor(255, 50, 255); break;  // 紫
        default: color = GetColor(255, 255, 255);
        }

        int life = RandomInt(60, 120);
        float size = RandomFloat(3.0f, 8.0f);

        AddParticle(centerX, centerY, vx, vy, life, color, size, DX_BLENDMODE_ADD);
    }
}

// ライン消去エフェクト（ボード全体）
void ParticleManager::CreateLineClearing(int boardX, int boardY, int boardWidth, int boardHeight, int blockSize) {
    for (int x = 0; x < boardWidth; ++x) {
        for (int y = 0; y < boardHeight; ++y) {
            int count = RandomInt(2, 4);
            for (int i = 0; i < count; ++i) {
                float px = (float)(boardX + x * blockSize + blockSize / 2);
                float py = (float)(boardY + y * blockSize + blockSize / 2);
                float vx = RandomFloat(-3.0f, 3.0f);
                float vy = RandomFloat(-3.0f, 3.0f);
                int life = RandomInt(30, 50);

                int color = GetColor(
                    150 + RandomInt(0, 105),
                    150 + RandomInt(0, 105),
                    50 + RandomInt(0, 100)
                );

                AddParticle(px, py, vx, vy, life, color, 3.0f, DX_BLENDMODE_ADD);
            }
        }
    }
}

// ★ 新規追加: 特定のラインだけにパーティクルを生成
void ParticleManager::CreateLineClearEffect(int boardX, int boardY, int lineY, int boardWidth, int blockSize) {
    // そのライン上の全ブロックにパーティクル生成
    for (int x = 0; x < boardWidth; ++x) {
        int count = RandomInt(8, 15);  // 消去ラインは多めのパーティクル
        for (int i = 0; i < count; ++i) {
            float px = (float)(boardX + x * blockSize + blockSize / 2);
            float py = (float)(boardY + lineY * blockSize + blockSize / 2);

            // ランダムな方向に飛び散る
            float angle = RandomFloat(0.0f, 6.28318f);
            float speed = RandomFloat(2.0f, 8.0f);
            float vx = cos(angle) * speed;
            float vy = sin(angle) * speed - 2.0f;  // 少し上方向にバイアス

            int life = RandomInt(40, 60);

            // カラフルなパーティクル
            int colorType = RandomInt(0, 3);
            int color;
            switch (colorType) {
            case 0: color = GetColor(255, 200, 100); break;  // オレンジ
            case 1: color = GetColor(100, 200, 255); break;  // 青
            case 2: color = GetColor(255, 100, 200); break;  // ピンク
            case 3: color = GetColor(200, 255, 100); break;  // 黄緑
            default: color = GetColor(255, 255, 255);
            }

            float size = RandomFloat(3.0f, 6.0f);
            AddParticle(px, py, vx, vy, life, color, size, DX_BLENDMODE_ADD);
        }
    }
}

// ヘルパー関数
float ParticleManager::RandomFloat(float min, float max) {
    return min + (max - min) * (rand() % 1000) / 1000.0f;
}

int ParticleManager::RandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}