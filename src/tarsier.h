#ifndef TARSIER_H
#define TARSIER_H

#include <stdio.h>

#include "../raylib/src/raylib.h"

#define FONT_DEFAULT_PATH "/usr/share/fonts/gnu-free/FreeMono.otf"
#define FONT_DEFAULT_SIZE "24"
#define FONT_DEFAULT_SPACING "0"

#define TERM_DEFAULT_FG GREEN
#define TERM_DEFAULT_BG BLUE

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
        Color bgcolor;
        unsigned int max_chars;

} Context;

static void
report(const char * format, ...)
{
        char buffer[1024 * 1024];
        int fd_in, fd_out;
        FILE *file = fopen("tarsier.log", "a");
        va_list ap;
        va_start(ap, format);
        vfprintf(file, format, ap);
        fprintf(file, "\n");
        fclose(file);
        va_end(ap);
}

void eval_escseq(char **c, Context *ctx);


int rowoffsetset(Context *ctx, int l) ; /* Convert L lines to y offset in pixels */
int coloffsetset(Context *ctx, int l) ; /* Convert L lines to x offset in pixels */
int rowoffsetinc(Context *ctx, int l) ; /* Increment by L lines to y offset in pixels */
int coloffsetinc(Context *ctx, int l) ; /* Convert L lines to x offset in pixels */

#endif // TARSIER_H
