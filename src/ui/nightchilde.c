#include "nightchilde.h"

#include "../build/assets/materials/ui.h"

struct Font gNightChilde;

struct CharacterDefinition gNightChildeDefinitions[] = {
    {'a', NIGHTCHILDE_INDEX, {5, 1, 10, 14}, 0},
    {'b', NIGHTCHILDE_INDEX, {16, 1, 12, 14}, 0},
    {'c', NIGHTCHILDE_INDEX, {30, 0, 9, 15}, 0},
    {'d', NIGHTCHILDE_INDEX, {41, 1, 11, 14}, 0},
};

void nightChildeInit() {
    fontInit(&gNightChilde, 9, 16, gNightChildeDefinitions, sizeof(gNightChildeDefinitions) / sizeof(*gNightChildeDefinitions));
}