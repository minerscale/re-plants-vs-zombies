#pragma once

#include "ConstEnums.h"
constexpr const double PI = 3.141592653589793;

// ============================================================
// ■ 常数
// ============================================================
constexpr int BOARD_WIDTH = 800;
constexpr int BOARD_HEIGHT = 600;
constexpr int WIDE_BOARD_WIDTH = 800;
constexpr int BOARD_OFFSET = 220;
constexpr int BOARD_EDGE = -100;
constexpr int BOARD_IMAGE_WIDTH_OFFSET = 1180;
constexpr int BOARD_ICE_START = 800;
constexpr int LAWN_XMIN = 40;
constexpr int LAWN_YMIN = 80;
constexpr int HIGH_GROUND_HEIGHT = 30;

constexpr int SEEDBANK_MAX = 10;
constexpr int SEED_BANK_OFFSET_X = 0;
constexpr int SEED_BANK_OFFSET_X_END = 10;
constexpr int SEED_CHOOSER_OFFSET_Y = 516;
constexpr int SEED_PACKET_WIDTH = 50;
constexpr int SEED_PACKET_HEIGHT = 70;
constexpr int IMITATER_DIALOG_WIDTH = 500;
constexpr int IMITATER_DIALOG_HEIGHT = 600;

// ============================================================
// ■ 关卡相关
// ============================================================
constexpr int ADVENTURE_AREAS = 5;
constexpr int LEVELS_PER_AREA = 10;
constexpr int NUM_LEVELS = ADVENTURE_AREAS * LEVELS_PER_AREA;
constexpr int FINAL_LEVEL = NUM_LEVELS;
constexpr int FLAG_RAISE_TIME = 100;
constexpr int LAST_STAND_FLAGS = 5;
constexpr int ZOMBIE_COUNTDOWN_FIRST_WAVE = 1800;
constexpr int ZOMBIE_COUNTDOWN = 2500;
constexpr int ZOMBIE_COUNTDOWN_RANGE = 600;
constexpr int ZOMBIE_COUNTDOWN_BEFORE_FLAG = 4500;
constexpr int ZOMBIE_COUNTDOWN_BEFORE_REPICK = 5499;
constexpr int ZOMBIE_COUNTDOWN_MIN = 400;
constexpr int FOG_BLOW_RETURN_TIME = 2000;
constexpr int SUN_COUNTDOWN = 425;
constexpr int SUN_COUNTDOWN_RANGE = 275;
constexpr int SUN_COUNTDOWN_MAX = 950;
constexpr int SURVIVAL_NORMAL_FLAGS = 5;
constexpr int SURVIVAL_HARD_FLAGS = 10;

// ============================================================
// ■ 商店相关
// ============================================================
constexpr int STORESCREEN_ITEMOFFSET_1_X = 422;
constexpr int STORESCREEN_ITEMOFFSET_1_Y = 206;
constexpr int STORESCREEN_ITEMOFFSET_2_X = 372;
constexpr int STORESCREEN_ITEMOFFSET_2_Y = 310;
constexpr int STORESCREEN_ITEMSIZE = 74;
constexpr int STORESCREEN_COINBANK_X = 650;
constexpr int STORESCREEN_COINBANK_Y = 559;
constexpr int STORESCREEN_PAGESTRING_X = 470;
constexpr int STORESCREEN_PAGESTRING_Y = 500;
