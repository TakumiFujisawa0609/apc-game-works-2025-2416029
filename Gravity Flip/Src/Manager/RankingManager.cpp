#include <fstream>
#include <iostream>
#include "RankingManager.h"

RankingManager::RankingManager() {
    rankings.clear();
}


//ランキングデータをソートする
void RankingManager::Sort() {
    //スコア降順に基づいてソート
    std::sort(rankings.begin(), rankings.end());

    //超えた分を削除
    if (rankings.size() > MAX_RANKINGS) {
        rankings.resize(MAX_RANKINGS);
    }
}


//ランキングデータをファイルから読み込む
void RankingManager::Load() {
    std::ifstream ifs(FILENAME, std::ios::binary);
    rankings.clear();

    if (ifs.is_open()) {
        RankingEntry entry;
        // ファイルから最大数ののエントリーを読み込む
        for (int i = 0; i < MAX_RANKINGS; ++i) {
            if (ifs.read(reinterpret_cast<char*>(&entry), sizeof(RankingEntry))) {
                rankings.push_back(entry);
            }
            else {
                break; // 読み込み失敗
            }
        }
        ifs.close();

        //念のためソート
        Sort();
    }
    else {
        // ファイルが存在しない、または開けなかった場合は、空のランキングで開始
        std::cerr << "Ranking file not found. Starting with empty ranking." << std::endl;
    }
}

//ランキングデータをファイルに保存する
void RankingManager::Save() const {
    std::ofstream ofs(FILENAME, std::ios::binary | std::ios::trunc); // truncate: 上書き

    if (ofs.is_open()) {
        for (const auto& entry : rankings) {
            ofs.write(reinterpret_cast<const char*>(&entry), sizeof(RankingEntry));
        }
        ofs.close();
    }
    else {
        std::cerr << "Error: Failed to open ranking file for saving." << std::endl;
    }
}

//新しいスコアがランキング入りしたかどうか
bool RankingManager::AddScore(int newScore) {
    if (newScore <= 0) return false;

    // 現在のランキング数が最大数未満、または新しいスコアが最下位のスコアより高い場合
    if (rankings.size() < MAX_RANKINGS || newScore > rankings.back().score) {

        RankingEntry newEntry = { newScore };
        rankings.push_back(newEntry);

        // ソートとサイズの調整
        Sort();

        // ランキング入りしたかどうかをチェック
        for (const auto& entry : rankings) {
            if (entry.score == newScore) {
                
                return true;
            }
        }
        return false; 
    }

    // ランキング入りしなかった場合
    return false;
}

//ランキングデータをリセット
void RankingManager::ResetRanking() {
    rankings.clear();
    Save();
}