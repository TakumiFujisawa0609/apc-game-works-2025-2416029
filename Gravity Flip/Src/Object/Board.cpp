#include "Board.h"
#include "DxLib.h"
#include "../Scene/GameScene.h"

const int BOARD_OFFSET_X = 420;
const int BOARD_OFFSET_Y = 10;

void Board::Init() {
    for (int y = 0; y < BOARD_H; ++y)
        for (int x = 0; x < BOARD_W; ++x)
            blocks[y][x].color = 0;
    gravity = GRAVITY_DOWN;
}

void Board::Draw() {
    for (int y = 0; y < BOARD_H; ++y) {
        for (int x = 0; x < BOARD_W; ++x) {
            if (blocks[y][x].color > 0) {
                int c = GetColor(100 * blocks[y][x].color, 100, 255 - 30 * blocks[y][x].color);
                DrawBox(
                    BOARD_OFFSET_X + x * BLOCK_SIZE,
                    BOARD_OFFSET_Y + y * BLOCK_SIZE,
                    BOARD_OFFSET_X + (x + 1) * BLOCK_SIZE,
                    BOARD_OFFSET_Y + (y + 1) * BLOCK_SIZE,
                    c, TRUE
                );
            }
            DrawBox(
                BOARD_OFFSET_X + x * BLOCK_SIZE,
                BOARD_OFFSET_Y + y * BLOCK_SIZE,
                BOARD_OFFSET_X + (x + 1) * BLOCK_SIZE,
                BOARD_OFFSET_Y + (y + 1) * BLOCK_SIZE,
                GetColor(50, 50, 50), FALSE
            );
        }
    }
}

void Board::FixMino(const ActiveMino& mino) {
    for (int i = 0; i < 4; ++i) {
        int x = mino.px + mino.shape[i][0];
        int y = mino.py + mino.shape[i][1];
        if (x >= 0 && x < BOARD_W && y >= 0 && y < BOARD_H)
            blocks[y][x].color = mino.color;
    }
}

bool Board::IsCollision(const ActiveMino& mino, int dx, int dy) {
    for (int i = 0; i < 4; ++i) {
        int x = mino.px + mino.shape[i][0] + dx;
        int y = mino.py + mino.shape[i][1] + dy;
        if (x < 0 || x >= BOARD_W || y < 0 || y >= BOARD_H) return true;
        if (blocks[y][x].color > 0) return true;
    }
    return false;
}

std::vector<int> Board::ClearLines() {
    std::vector<int> clearedLines;

    // ★ 重力DOWN: 下から上へチェック、上から詰める
    if (gravity == GRAVITY_DOWN) {
        for (int y = BOARD_H - 1; y >= 0; --y) {
            bool full = true;
            for (int x = 0; x < BOARD_W; ++x) {
                if (blocks[y][x].color == 0) {
                    full = false;
                    break;
                }
            }

            if (full) {
                clearedLines.push_back(y);

                // ★ 消えた行より上のブロックを下に詰める
                for (int ty = y; ty > 0; --ty) {
                    for (int x = 0; x < BOARD_W; ++x) {
                        blocks[ty][x].color = blocks[ty - 1][x].color;
                    }
                }
                // 最上段をクリア
                for (int x = 0; x < BOARD_W; ++x) {
                    blocks[0][x].color = 0;
                }
                y++; // 詰めた行を再チェック
            }
        }
    }
    // ★ 重力UP: 上から下へチェック、下から詰める
    else if (gravity == GRAVITY_UP) {
        for (int y = 0; y < BOARD_H; ++y) {
            bool full = true;
            for (int x = 0; x < BOARD_W; ++x) {
                if (blocks[y][x].color == 0) {
                    full = false;
                    break;
                }
            }

            if (full) {
                clearedLines.push_back(y);

                // ★ 消えた行より下のブロックを上に詰める
                for (int ty = y; ty < BOARD_H - 1; ++ty) {
                    for (int x = 0; x < BOARD_W; ++x) {
                        blocks[ty][x].color = blocks[ty + 1][x].color;
                    }
                }
                // 最下段をクリア
                for (int x = 0; x < BOARD_W; ++x) {
                    blocks[BOARD_H - 1][x].color = 0;
                }
                y--; // 詰めた行を再チェック
            }
        }
    }
    // ★ 重力LEFT: 左から右へチェック、右から詰める
    else if (gravity == GRAVITY_LEFT) {
        for (int x = 0; x < BOARD_W; ++x) {
            bool full = true;
            for (int y = 0; y < BOARD_H; ++y) {
                if (blocks[y][x].color == 0) {
                    full = false;
                    break;
                }
            }

            if (full) {
                clearedLines.push_back(x);

                // ★ 消えた列より右のブロックを左に詰める
                for (int tx = x; tx < BOARD_W - 1; ++tx) {
                    for (int y = 0; y < BOARD_H; ++y) {
                        blocks[y][tx].color = blocks[y][tx + 1].color;
                    }
                }
                // 最右端をクリア
                for (int y = 0; y < BOARD_H; ++y) {
                    blocks[y][BOARD_W - 1].color = 0;
                }
                x--; // 詰めた列を再チェック
            }
        }
    }
    // ★ 重力RIGHT: 右から左へチェック、左から詰める
    else if (gravity == GRAVITY_RIGHT) {
        for (int x = BOARD_W - 1; x >= 0; --x) {
            bool full = true;
            for (int y = 0; y < BOARD_H; ++y) {
                if (blocks[y][x].color == 0) {
                    full = false;
                    break;
                }
            }

            if (full) {
                clearedLines.push_back(x);

                // ★ 消えた列より左のブロックを右に詰める
                for (int tx = x; tx > 0; --tx) {
                    for (int y = 0; y < BOARD_H; ++y) {
                        blocks[y][tx].color = blocks[y][tx - 1].color;
                    }
                }
                // 最左端をクリア
                for (int y = 0; y < BOARD_H; ++y) {
                    blocks[y][0].color = 0;
                }
                x++; // 詰めた列を再チェック
            }
        }
    }

    return clearedLines;
}

void Board::ApplyGravity() {
    switch (gravity) {
    case GRAVITY_DOWN:
        for (int x = 0; x < BOARD_W; ++x) {
            for (int y = BOARD_H - 2; y >= 0; --y) {
                if (blocks[y][x].color > 0) {
                    int ny = y;
                    while (ny + 1 < BOARD_H && blocks[ny + 1][x].color == 0) {
                        blocks[ny + 1][x].color = blocks[ny][x].color;
                        blocks[ny][x].color = 0;
                        ny++;
                    }
                }
            }
        }
        break;

    case GRAVITY_UP:
        for (int x = 0; x < BOARD_W; ++x) {
            for (int y = 1; y < BOARD_H; ++y) {
                if (blocks[y][x].color > 0) {
                    int ny = y;
                    while (ny - 1 >= 0 && blocks[ny - 1][x].color == 0) {
                        blocks[ny - 1][x].color = blocks[ny][x].color;
                        blocks[ny][x].color = 0;
                        ny--;
                    }
                }
            }
        }
        break;

    case GRAVITY_LEFT:
        for (int y = 0; y < BOARD_H; ++y) {
            for (int x = 1; x < BOARD_W; ++x) {
                if (blocks[y][x].color > 0) {
                    int nx = x;
                    while (nx - 1 >= 0 && blocks[y][nx - 1].color == 0) {
                        blocks[y][nx - 1].color = blocks[y][nx].color;
                        blocks[y][nx].color = 0;
                        nx--;
                    }
                }
            }
        }
        break;

    case GRAVITY_RIGHT:
        for (int y = 0; y < BOARD_H; ++y) {
            for (int x = BOARD_W - 2; x >= 0; --x) {
                if (blocks[y][x].color > 0) {
                    int nx = x;
                    while (nx + 1 < BOARD_W && blocks[y][nx + 1].color == 0) {
                        blocks[y][nx + 1].color = blocks[y][nx].color;
                        blocks[y][nx].color = 0;
                        nx++;
                    }
                }
            }
        }
        break;
    }
}