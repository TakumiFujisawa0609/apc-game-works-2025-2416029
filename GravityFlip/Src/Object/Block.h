#pragma once

struct Block {
    int color; // 0:ãÛ, 1Å`:êF
    Block() : color(0) {}
};

struct ActiveBlock {
    int type;
    int color;
    int rotation;
    int px, py;
    int shape[4][2];
};