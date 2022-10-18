
#ifndef _SPRITE_H
#define _SPRITE_H

#include <ultra64.h>
#include "../graphics/renderstate.h"
#include "../graphics/color.h"

#define LAYER_SOLID_COLOR       0
#define LAYER_MENU_BORDER       1
#define LAYER_IMAGE_COPIES      2
#define LAYER_BUTTONS           3

#define LAYER_COMMAND_BUTTONS   4
#define LAYER_FRESH_EATERS      4

#define LAYER_HEALTH_BAR        5
#define LAYER_KICKFLIP_NUMBERS_FONT     6
#define LAYER_KICKFLIP_FONT     7

struct SpriteTile
{
    char x;
    char y;
    char w;
    char h;
};

void spriteWriteRaw(struct RenderState* renderState, int layer, Gfx* src, int count);
void spriteSetLayer(struct RenderState* renderState, int layer, Gfx* graphics, Gfx* revert);
void spritePreallocate(struct RenderState* renderState, int layer, int count);
void spriteTextureRectangle(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int dsdx, int dsdy);
void spriteSolid(struct RenderState* renderState, int layer, int x, int y, int w, int h);
void spriteCopyImage(struct RenderState* renderState, int layer, void* image, int iw, int ih, int x, int y, int w, int h, int sx, int sy);
void spriteDraw(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int scaleShiftX, int scaleShiftY);
void spriteDrawTile(struct RenderState* renderState, int layer, int x, int y, int w, int h, struct SpriteTile tile);
void spriteSetColor(struct RenderState* renderState, int layer, struct Coloru8 color);
struct Coloru8 spriteGetColor(struct RenderState* renderState, int layer);

void spriteInit(struct RenderState* renderState);
void spriteFinish(struct RenderState* renderState);

#endif