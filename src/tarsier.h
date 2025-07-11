#ifndef TARSIER_H
#define TARSIER_H

#include "../raylib/src/raylib.h"

#define FONT_DEFAULT_PATH "/usr/share/fonts/gnu-free/FreeMono.otf"
#define FONT_DEFAULT_SIZE "24"
#define FONT_DEFAULT_SPACING "0"

#define TERM_DEFAULT_COLOR GREEN

typedef struct tFont {
        Font base;
        float width, height;
        float spacing;
        float size;
} tFont;

typedef struct Context {
        int screen_width;
        int screen_height;
        tFont font;
        Vector2 position;
        Color color;
        unsigned int max_chars;

} Context;

void eval_escseq(char **c, Context *ctx);

#endif // TARSIER_H
