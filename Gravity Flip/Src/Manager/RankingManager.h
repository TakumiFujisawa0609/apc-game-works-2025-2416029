// RankingManager.h
#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include "ParticleManager.h"

// ランキングデータ構造体
struct RankingEntry {
    int score;

    bool operator<(const RankingEntry& other) const {
        return score > other.score; // スコアが高い順 (降順) にソート
    }
};

class RankingManager {
public:
    // ★ static constexpr に変更（コンパイル時定数）
    static constexpr int MAX_RANKINGS = 10;

    RankingManager();
    ~RankingManager() = default;

    void Load();
    void Save() const;
    bool AddScore(int newScore);
    const std::vector<RankingEntry>& GetRanking() const { return rankings; }
    void ResetRanking();

private:
    std::vector<RankingEntry> rankings;
    const std::string FILENAME = "ranking.dat";

    void Sort();
};