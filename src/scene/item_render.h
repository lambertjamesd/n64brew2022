#ifndef __SCENE_ITEM_RENDER_H__
#define __SCENE_ITEM_RENDER_H__

#include "../graphics/renderstate.h"
#include "item.h"

#define ITEM_RENDER_SIZE    64

void itemRenderGenerate(enum ItemType itemType, struct RenderState* renderState);
void itemRenderGenerateAll(struct RenderState* renderState);

Gfx* itemRenderUseImage(enum ItemType itemType, struct RenderState* renderState, Gfx* promptGfx);

#endif