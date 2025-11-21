#pragma once

struct Block {
    int color; // 0:‹ó, 1`:F
    Block() : color(0) {}
};

struct ActiveMino {
    int type;
    int color;
    int rotation;
    int px, py;
    int shape[4][2];
};