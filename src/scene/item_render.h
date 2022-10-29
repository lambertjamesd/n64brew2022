#ifndef __SCENE_ITEM_RENDER_H__
#define __SCENE_ITEM_RENDER_H__

#include "../graphics/renderstate.h"
#include "item.h"

#define ITEM_RENDER_SIZE    64

#define MAX_ITEM_REQUESTERS 8

void itemRenderGenerate(int itemIndex, enum ItemType itemType, float progress, float timeLeft, struct RenderState* renderState);

Gfx* itemRenderUseImage(int itemIndex, struct RenderState* renderState, Gfx* promptGfx);

#endif