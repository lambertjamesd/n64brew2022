#include "nightchilde.h"

#include "../build/assets/materials/ui.h"

struct Font gNightChilde;

struct CharacterDefinition gNightChildeDefinitions[] = {
    {'a', NIGHTCHILDE_INDEX, {5, 1, 10, 14}, 0, 0},
    {'b', NIGHTCHILDE_INDEX, {16, 1, 12, 14}, 0, 0},
    {'c', NIGHTCHILDE_INDEX, {30, 0, 9, 15}, 0, -1},
    {'d', NIGHTCHILDE_INDEX, {41, 1, 11, 14}, 0, 0},
    {'e', NIGHTCHILDE_INDEX, {54, 1, 9, 15}, 0, 0},
    {'f', NIGHTCHILDE_INDEX, {64, 1, 10, 14}, 0, 0},
    {'g', NIGHTCHILDE_INDEX, {74, 1, 10, 15}, 0, 0},
    {'h', NIGHTCHILDE_INDEX, {88, 1, 10, 15}, 0, 0},
    {'i', NIGHTCHILDE_INDEX, {101, 1, 5, 14}, 0, 0},
    {'j', NIGHTCHILDE_INDEX, {108, 1, 11, 14}, 0, 1},
    {'k', NIGHTCHILDE_INDEX, {5, 16, 10, 15}, 0, 0},
    {'l', NIGHTCHILDE_INDEX, {16, 16, 9, 15}, 0, 0},
    {'m', NIGHTCHILDE_INDEX, {27, 16, 12, 15}, 0, 1},
    {'n', NIGHTCHILDE_INDEX, {42, 16, 9, 16}, 0, 0},
    {'o', NIGHTCHILDE_INDEX, {52, 17, 11, 14}, 0, 0},
    {'p', NIGHTCHILDE_INDEX, {64, 17, 11, 14}, 0, 0},
    {'q', NIGHTCHILDE_INDEX, {77, 16, 10, 17}, 0, -1},
    {'r', NIGHTCHILDE_INDEX, {88, 17, 13, 14}, 0, 0},
    {'s', NIGHTCHILDE_INDEX, {101, 16, 11, 15}, 0, 0},
    {'t', NIGHTCHILDE_INDEX, {114, 16, 9, 15}, 0, 0},
    {'u', NIGHTCHILDE_INDEX, {4, 32, 11, 14}, 0, 0},
    {'v', NIGHTCHILDE_INDEX, {16, 32, 12, 15}, 0, 0},
    {'w', NIGHTCHILDE_INDEX, {30, 32, 14, 15}, 0, -1},
    {'x', NIGHTCHILDE_INDEX, {46, 33, 10, 14}, 0, 0},
    {'y', NIGHTCHILDE_INDEX, {57, 32, 11, 15}, 0, 0},
    {'z', NIGHTCHILDE_INDEX, {69, 33, 10, 14}, 0, 0},

    {'A', NIGHTCHILDE_INDEX, {5, 1, 10, 14}, 0, 0},
    {'B', NIGHTCHILDE_INDEX, {16, 1, 12, 14}, 0, 0},
    {'C', NIGHTCHILDE_INDEX, {30, 0, 9, 15}, 0, -1},
    {'D', NIGHTCHILDE_INDEX, {41, 1, 11, 14}, 0, 0},
    {'E', NIGHTCHILDE_INDEX, {54, 1, 9, 15}, 0, 0},
    {'F', NIGHTCHILDE_INDEX, {64, 1, 10, 14}, 0, 0},
    {'G', NIGHTCHILDE_INDEX, {74, 1, 10, 15}, 0, 0},
    {'H', NIGHTCHILDE_INDEX, {88, 1, 10, 15}, 0, 0},
    {'I', NIGHTCHILDE_INDEX, {101, 1, 5, 14}, 0, 0},
    {'J', NIGHTCHILDE_INDEX, {108, 1, 11, 14}, 0, 1},
    {'K', NIGHTCHILDE_INDEX, {5, 16, 10, 15}, 0, 0},
    {'L', NIGHTCHILDE_INDEX, {16, 16, 9, 15}, 0, 0},
    {'M', NIGHTCHILDE_INDEX, {27, 16, 12, 15}, 0, 1},
    {'N', NIGHTCHILDE_INDEX, {42, 16, 9, 16}, 0, 0},
    {'O', NIGHTCHILDE_INDEX, {52, 17, 11, 14}, 0, 0},
    {'P', NIGHTCHILDE_INDEX, {64, 17, 11, 14}, 0, 0},
    {'Q', NIGHTCHILDE_INDEX, {77, 16, 10, 17}, 0, -1},
    {'R', NIGHTCHILDE_INDEX, {88, 17, 13, 14}, 0, 0},
    {'S', NIGHTCHILDE_INDEX, {101, 16, 11, 15}, 0, 0},
    {'T', NIGHTCHILDE_INDEX, {114, 16, 9, 15}, 0, 0},
    {'U', NIGHTCHILDE_INDEX, {4, 32, 11, 14}, 0, 0},
    {'V', NIGHTCHILDE_INDEX, {16, 32, 12, 15}, 0, 0},
    {'W', NIGHTCHILDE_INDEX, {30, 32, 14, 15}, 0, -1},
    {'X', NIGHTCHILDE_INDEX, {46, 33, 10, 14}, 0, 0},
    {'Y', NIGHTCHILDE_INDEX, {57, 32, 11, 15}, 0, 0},
    {'Z', NIGHTCHILDE_INDEX, {69, 33, 10, 14}, 0, 0},

    {'0', NIGHTCHILDE_INDEX, {4, 49, 10, 14}, 0, 0},
    {'1', NIGHTCHILDE_INDEX, {15, 48, 7, 15}, 0, 0},
    {'2', NIGHTCHILDE_INDEX, {23, 48, 11, 15}, 0, 0},
    {'3', NIGHTCHILDE_INDEX, {35, 49, 11, 14}, 0, 1},
    {'4', NIGHTCHILDE_INDEX, {49, 48, 10, 15}, 0, 0},
    {'5', NIGHTCHILDE_INDEX, {60, 47, 12, 16}, 0, 0},
    {'6', NIGHTCHILDE_INDEX, {72, 48, 11, 15}, 0, 0},
    {'7', NIGHTCHILDE_INDEX, {85, 48, 10, 15}, 0, 0},
    {'8', NIGHTCHILDE_INDEX, {98, 48, 11, 15}, 0, 0},
    {'9', NIGHTCHILDE_INDEX, {110, 48, 12, 15}, 0, 0},

    {'.', NIGHTCHILDE_INDEX, {79, 33, 5, 14}, 1, 0},
};

void nightChildeInit() {
    fontInit(&gNightChilde, 9, 24, gNightChildeDefinitions, sizeof(gNightChildeDefinitions) / sizeof(*gNightChildeDefinitions));
}