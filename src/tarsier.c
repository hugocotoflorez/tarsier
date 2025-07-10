#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ascii.h"
#include "term.h"

#include "../raylib/src/raylib.h"

#define HCFP_IMPLEMENTATION
#include "hcfp.h"

#define FONT_DEFAULT_PATH "/usr/share/fonts/gnu-free/FreeMono.otf"
#define FONT_DEFAULT_SIZE "24"
#define FONT_DEFAULT_SPACING "0"

typedef struct tFont {
        Font base;
        float width, height;
        float spacing;
        float size;
} tFont;

typedef enum {
        MOD_NONE = 0,
        MOD_SHIFT = 1,
        MOD_CONTROL = 2,
        MOD_ALT = 4,
        MOD_SUPER = 8,
} ModState;

void
report(const char *restrict format, ...)
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

int
using_no_shift(int kp)
{
        if (0 <= kp && kp <= 128)
                return WITHOUT_SHIFT[kp];
        return kp;
}

int
using_shilf(int kp)
{
        if (0 <= kp && kp <= 128)
                return WITH_SHIFT[kp];
        return kp;
}

#define IsModDown(MOD) (IsKeyDown(KEY_LEFT_##MOD) || IsKeyDown(KEY_RIGHT_##MOD))

int
process_input(Term t)
{
        ModState state = 0;
        if (IsModDown(SHIFT)) state |= MOD_SHIFT;
        if (IsModDown(CONTROL)) state |= MOD_CONTROL;
        if (IsModDown(ALT)) state |= MOD_ALT;
        if (IsModDown(SUPER)) state |= MOD_SUPER;

        int kp;
        while ((kp = GetKeyPressed())) {
                kp = (state & MOD_SHIFT) ? using_shilf(kp) : using_no_shift(kp);

                if (IsKeyPressed(KEY_ENTER)) term_send(t, LF);
                if (IsKeyPressed(KEY_ESCAPE)) term_send(t, ESC);
                if (IsKeyPressed(KEY_BACKSPACE)) term_send(t, BS);
                if (IsKeyPressed(KEY_TAB)) term_send(t, HT);
                if (IsKeyPressed(KEY_DELETE)) term_send(t, DEL);

                if (kp >= 0 && kp < 128)
                        term_send(t, kp);
        }
        return 0;
}

int
get_window_size(int *h, int *w)
{
        *h = GetRenderHeight();
        *w = GetRenderWidth();
        return 0;
}

int
process_output(Term t)
{
        char buf[1024];
        while (read_fileno(t.master_fd, buf, sizeof buf)) {
                printf("%s", buf);
                fflush(stdout);
        }
        return 0;
}

void
apply_color(int s, Color *color)
{
        switch (s) {
        case 0:
                *color = WHITE;
                break;
        case 30:
                *color = BLACK;
                break;
        case 31:
                *color = RED;
                break;
        case 32:
                *color = GREEN;
                break;
        case 33:
                *color = ORANGE;
                break;
        case 34:
                *color = BLUE;
                break;
        case 35:
                *color = MAGENTA;
                break;
        case 36:
                *color = LIME;
                break;
        case 37:
                *color = WHITE;
                break;
        }
        // printf("Aplying color: %d\n", s);
}

int
consume_get_int(char **c)
{
        // printf("consume_get_int over `%5s` ", *c);
        int n = 0;
        while ('0' <= **c && **c <= '9') {
                n *= 10;
                n += **c - '0';
                ++*c;
        }
        // printf("-> %d\n", n);
        return n;
}

void
eval_escseq(char **c, Color *color)
{
        assert(**c == '\033'); /* Do not call it incorrectly */

        if (memcmp(*c, "\033[", 2) == 0) {
                // printf("Color format found\n");
                *c += 1;
                do {
                        *c += 1;
                        apply_color(consume_get_int(c), color);
                } while (**c == ';');
        }
}

void
display_text(int screen_width, int screen_height, tFont font, char *text)
{
        Vector2 position = { 0, 0 };
        Color color = GREEN;
        unsigned int max_chars = screen_width / (font.width + font.spacing);

        if (max_chars == 0) abort();

        for (char *c = text; *c; c++) {
                if (*c == '\033')
                        eval_escseq(&c, &color);
                else if (*c == '\n')
                        position.y += font.height;
                else if (*c >= ' ' && *c < 127)
                        DrawTextCodepoint(font.base, *c, position, font.size, color);

                position.x += font.width + font.spacing;

                /* Line overflow at the right -> wrap */
                if (position.x > max_chars * font.width) {
                        position.x = 0;
                        position.y += font.height;
                }
        }
}

tFont
load_font()
{
        tFont font;
        HcfOpts opts = hcf_load("_settings.hcf");
        char *fontname = hcf_get_default(opts, "font", "path", FONT_DEFAULT_PATH);
        font.size = atof(hcf_get_default(opts, "font", "size", FONT_DEFAULT_SIZE));
        font.spacing = atof(hcf_get_default(opts, "font", "spacing", FONT_DEFAULT_SPACING));
        font.base = LoadFont(fontname);
        hcf_destroy(&opts);

        float font_inc = font.size / font.base.baseSize;
        font.width = (font.base.recs->width + font.spacing) * font_inc;
        font.height = (font.base.recs->height + font.spacing) * font_inc;

        return font;
}

int
main(void)
{
        int screen_height = 800;
        int screen_width = 450;

        InitWindow(screen_width, screen_height, "Tarsier terminal emulator");

        // TODO: Load resources / Initialize variables at this point

        SetTargetFPS(30);
        SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        SetExitKey(KEY_NULL);
        //--------------------------------------------------------------------------------------

        Term t = term_start();
        /* font have to be monospace */
        tFont font = load_font();
        int a = 0;
        char text[1024];

        // Main game loop
        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
                // Update
                //----------------------------------------------------------------------------------
                // TODO: Update variables / Implement example logic at this point
                //----------------------------------------------------------------------------------

                process_input(t);

                if (IsWindowResized()) {
                        get_window_size(&screen_height, &screen_width);
                        unsigned int h = (float) screen_height / (font.height);
                        unsigned int w = (float) screen_width / (font.width + font.spacing);
                        resize_term(t, h, w, screen_height, screen_width);
                }

                // Draw
                //----------------------------------------------------------------------------------
                BeginDrawing();

                ClearBackground(BLACK);

                process_output(t);

                // TODO: Draw everything that requires to be drawn at this point:

                a++;
                sprintf(text, "%d", a);
                display_text(screen_width, screen_height, font, text);

                EndDrawing();
                //----------------------------------------------------------------------------------
        }

        // De-Initialization
        //--------------------------------------------------------------------------------------

        // TODO: Unload all loaded resources at this point

        term_close(t);

        CloseWindow(); // Close window and OpenGL context
        //--------------------------------------------------------------------------------------

        return 0;
}
