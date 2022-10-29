
#include "spritefont.h"
#include "sprite.h"

int fontParseHexDigit(char digit) {
    if (digit >= '0' && digit <= '9') {
        return digit - '0';
    }
    
    if (digit >= 'a' && digit <= 'f') {
        return digit - 'a' + 10;
    }

    if (digit >= 'A' && digit <= 'F') {
        return digit - 'A' + 10;
    }

    return 0;
}

int fontParseHexByte(const char* at) {
    return (fontParseHexDigit(at[0]) << 4) | fontParseHexDigit(at[1]);
}

void fontParseColor(const char* at, struct Coloru8* output) {
    output->r = fontParseHexByte(&at[1]);
    output->g = fontParseHexByte(&at[3]);
    output->b = fontParseHexByte(&at[5]);
    output->a = fontParseHexByte(&at[7]);
}

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

int fontRenderWordWidth(struct Font* font, const char* str, int scaleShift) {
    int len = 0;

    while (*str == ' ') {
        ++str;
    }

    while (str[len] != 0 && str[len] != ' ') {
        ++len;
    }

    return fontMeasure(font, str, scaleShift, len);
}

void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift, int maxWidth, void* data, CharacterRenderModifier characterModifier)
{
    int startX = x;

    int index = 0;
    int maxX = x + maxWidth;

    if (maxWidth <= 0) {
        maxX = 0xffffff;
    }

    if (!*str) {
        return;
    }

    struct CharacterDefinition* firstChar = &font->characters[(int)*str];
    struct Coloru8 color = spriteGetColor(renderState, firstChar->spriteLayer);

    while (*str)
    {
        unsigned charValue = (unsigned)*str;
        struct CharacterDefinition* curr = &font->characters[charValue];
        if (curr->data.w)
        {
            int finalX = x;
            int finalY = y;

            struct Coloru8 colorCopy = color;

            if (characterModifier) {
                characterModifier(data, index, charValue, &finalX, &finalY, &colorCopy);
            }

            if (colorCopy.a) {
                spriteSetColor(renderState, curr->spriteLayer, colorCopy);

                spriteDraw(
                    renderState, 
                    curr->spriteLayer, 
                    finalX, finalY + curr->yOffset, 
                    curr->data.w, curr->data.h, 
                    curr->data.x, curr->data.y, 
                    scaleShift, scaleShift
                );
            }

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

            int wordWidth = fontRenderWordWidth(font, str, scaleShift);

            if (x + wordWidth > maxX) {
                x = startX;

                if (scaleShift >= 0) {
                    y += (font->lineHeight) << scaleShift;
                } else {
                    y += font->lineHeight >> -scaleShift;
                }
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
        } else if (*str == '#')
        {
            fontParseColor(str, &color);
            str += 8;
        }

        ++str;
        ++index;
    }
}

int fontMeasure(struct Font* font, const char* str, int scaleShift, int maxLength) {
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
        } else if (*str == '#') {
            str += 8;
        } else {
            if (scaleShift >= 0) {
                currentRow += (curr->data.w + curr->kerning) << scaleShift;
            } else {
                currentRow += (curr->data.w + curr->kerning) >> -scaleShift;
            }
        }

        ++str;
        --maxLength;

        if (maxLength == 0) {
            break;
        }
    }

    return MAX(result, currentRow);
}