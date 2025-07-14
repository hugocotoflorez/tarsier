#ifndef TARSIER_H
#define TARSIER_H

#include <stdio.h>

#include "../raylib/src/raylib.h"

#define FONT_DEFAULT_PATH_BASE "/usr/share/fonts/gnu-free/FreeMono.otf"
#define FONT_DEFAULT_PATH_BOLD "/usr/share/fonts/gnu-free/FreeMonoBold.otf"
#define FONT_DEFAULT_PATH_ITALIC "/usr/share/fonts/gnu-free/FreeMonoOblique.otf"
#define FONT_DEFAULT_PATH_BOLD_ITALIC "/usr/share/fonts/gnu-free/FreeMonoBoldOblique.otf"
#define FONT_DEFAULT_SIZE "24"
#define FONT_DEFAULT_SPACING "0"

/* defined in getcolor.c */
extern Color TERM_DEFAULT_FG;
extern Color TERM_DEFAULT_BG;
extern Color TERM_DEFAULT_CURSOR;
extern Color color_lookup[];
void get_default_colors();

typedef struct tFont {
        Font base;
        Font bold;
        Font italic;
        Font bold_italic;
        float width, height;
        float spacing;
        float size;
} tFont;

typedef enum {
        TM_NORMAL = 0,
        TM_BOLD = 1,
        TM_DIM = 2,
        TM_ITALIC = 4,
        TM_UNDERLINE = 8,
        TM_BLINKING = 16,
        TM_INVERSE = 32,
        TM_HIDDEN = 64,
        TM_STRIKETHROUGH = 128,
} TextMode;

typedef struct Context {
        int screen_width;
        int screen_height;
        tFont font;
        Vector2 position;
        Color fgcolor;
        Color bgcolor;
        Color cursor_color;
        TextMode text_mode;
        unsigned int max_chars;

} Context;

static void
report(const char *format, ...)
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


int rowoffsetset(Context *ctx, int l); /* Convert L lines to y offset in pixels */
int coloffsetset(Context *ctx, int l); /* Convert L lines to x offset in pixels */
int rowoffsetinc(Context *ctx, int l); /* Increment by L lines to y offset in pixels */
int coloffsetinc(Context *ctx, int l); /* Convert L lines to x offset in pixels */

void erase_end_screen(Context *ctx);
void erase_begin_screen(Context *ctx);
void erase_screen(Context *ctx);
void erase_saved_lines(Context *ctx);
void erase_end_line(Context *ctx);
void erase_begin_line(Context *ctx);
void erase_line(Context *ctx);

#endif // TARSIER_H
