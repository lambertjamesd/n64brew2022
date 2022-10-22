#include "ui.h"

#include "../util/rom.h"
#include "../build/assets/materials/ui.h"
#include "../ui/sprite.h"

#include "../graphics/graphics.h"

void uiInitSpirtes(struct RenderState* renderState) {
    Gfx** uiMaterialList = (Gfx**)ADJUST_POINTER_FOR_SEGMENT(ui_material_list, gMaterialSegment, MATERIAL_SEGMENT);
    Gfx** uiMaterialRevertList = (Gfx**)ADJUST_POINTER_FOR_SEGMENT(ui_material_revert_list, gMaterialSegment, MATERIAL_SEGMENT);
    
    for (int i = 0; i < DEFAULT_UI_INDEX; ++i) {
        spriteSetLayer(renderState, i, uiMaterialList[i], uiMaterialRevertList[i]);
    }
}