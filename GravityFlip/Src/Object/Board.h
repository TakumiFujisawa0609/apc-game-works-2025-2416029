#pragma once
#include "Block.h"
#include <vector>  // Åö í«â¡

struct ActiveBlock;

const int BOARD_W = 14;
const int BOARD_H = 20;
const int BLOCK_SIZE = 32;

extern const int BOARD_OFFSET_X;
extern const int BOARD_OFFSET_Y;

enum GravityDirection {
    GRAVITY_DOWN,
    GRAVITY_UP,
    GRAVITY_LEFT,
    GRAVITY_RIGHT
};

class Board {
public:
    Block blocks[BOARD_H][BOARD_W];
    GravityDirection gravity;

    void Init();
    void Draw();
    void FixMino(const struct ActiveBlock& mino);
    void ApplyGravity();
    bool IsCollision(const struct ActiveBlock& mino, int dx, int dy);

    // Åö ñﬂÇËílÇ std::vector<int> Ç…ïœçX
    std::vector<int> ClearLines();

    Board() : gravity(GRAVITY_DOWN) {}

private:
};