
#ifndef _SPRITEFONT_H
#define _SPRITEFONT_H

#define ANSI_CHAR_COUNT 128

#include "sprite.h"

struct CharacterDefinition
{
    char character;
    char spriteLayer;
    struct SpriteTile data;
    char kerning;
};

struct Font
{
    struct CharacterDefinition characters[ANSI_CHAR_COUNT];
    short spaceWidth; 
    short lineHeight;
};

void fontInit(struct Font* font, int spaceWidth, int lineHeight, struct CharacterDefinition* chars, int charCount);
void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift);
int fontMeasure(struct Font* font, const char* str, int scaleShift);

#endif