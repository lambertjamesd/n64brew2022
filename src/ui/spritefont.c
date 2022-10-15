
#include "spritefont.h"
#include "sprite.h"

void fontInit(struct Font* font, int spaceWidth, int lineHeight, struct CharacterDefinition* chars, int charCount)
{
    font->spaceWidth = spaceWidth;
    font->lineHeight = lineHeight;

    for (int i = 0; i < ANSI_CHAR_COUNT; ++i)
    {
        font->characters[i].data.w = 0;
    }

    for (int i = 0; i < charCount; ++i)
    {
        font->characters[(unsigned)chars[i].character] = chars[i];
    }
}

void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift)
{
    int startX = x;

    while (*str)
    {
        unsigned charValue = (unsigned)*str;
        struct CharacterDefinition* curr = &font->characters[charValue];
        if (curr->data.w)
        {
            spriteDraw(
                renderState, 
                curr->spriteLayer, 
                x, y, 
                curr->data.w, curr->data.h, 
                curr->data.x, curr->data.y, 
                scaleShift, scaleShift
            );
            if (scaleShift >= 0) {
                x += (curr->data.w + curr->kerning) << scaleShift;
            } else {
                x += (curr->data.w + curr->kerning) >> -scaleShift;
            }
        }
        else if (*str == ' ')
        {
            if (scaleShift >= 0) {
                x += (font->spaceWidth) << scaleShift;
            } else {
                x += font->spaceWidth >> -scaleShift;
            }
        }
        else if (*str == '\n')
        {
            x = startX;

            if (scaleShift >= 0) {
                y += (font->lineHeight) << scaleShift;
            } else {
                y += font->lineHeight >> -scaleShift;
            }
        }

        ++str;
    }
}

int fontMeasure(struct Font* font, const char* str, int scaleShift) {
    int result = 0;
    int currentRow = 0;

    while (*str)
    {
        struct CharacterDefinition* curr = &font->characters[(unsigned)*str];

        if (*str == ' ') {
            if (scaleShift >= 0) {
                currentRow += font->spaceWidth << scaleShift;
            } else {
                currentRow += font->spaceWidth >> -scaleShift;
            }
        } else if (*str == '\n') {
            result = MAX(result, currentRow);
            currentRow = 0;
        } else {
            if (scaleShift >= 0) {
                currentRow += (curr->data.w + curr->kerning) << scaleShift;
            } else {
                currentRow += (curr->data.w + curr->kerning) >> -scaleShift;
            }
        }

        ++str;
    }

    return MAX(result, currentRow);
}