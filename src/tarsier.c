#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ascii.h"
#include "term.h"

#include "screen_buffer.h"

#include "../raylib/src/raylib.h"

#define HCFP_IMPLEMENTATION
#include "hcfp.h"

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

typedef enum {
        MOD_NONE = 0,
        MOD_SHIFT = 1,
        MOD_CONTROL = 2,
        MOD_ALT = 4,
        MOD_SUPER = 8,
} ModState;

Sb screen_buffer = { 0 };

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

                if (kp >= ' ' && kp < DEL)
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
                sb_append(&screen_buffer, buf);
                printf("%s", buf);
                fflush(stdout);
        }
        return 0;
}

void
apply_color(int s, Color *color)
{
        /* clang-format off */
        switch (s) {
        case 0:  *color = TERM_DEFAULT_COLOR;   break;
        case 30: *color = BLACK;                break;
        case 31: *color = RED;                  break;
        case 32: *color = GREEN;                break;
        case 33: *color = ORANGE;               break;
        case 34: *color = BLUE;                 break;
        case 35: *color = MAGENTA;              break;
        case 36: *color = LIME;                 break;
        case 37: *color = WHITE;                break;
        }
        // printf("Aplying color: %d\n", s);
        /* clang-format on */
}

int
consume_get_int(char **c)
{
        int n = 0;
        while ('0' <= **c && **c <= '9') {
                n *= 10;
                n += **c - '0';
                ++*c;
        }
        return n;
}

void
eval_escseq(char **c, Color *color)
{
        assert(**c == '\033'); /* Do not call it incorrectly */

        if (memcmp(*c, "\033[", 2) == 0) {
                *c += 2;
                do {
                        apply_color(consume_get_int(c), color);
                } while (**c == ';' && *++*c);
                if (**c == 'm') {
                        // Was a color"
                }
        }
}

int
display_append_text(Context *ctx, char *text)
{
        if (text == NULL) return 0;
        if (ctx->max_chars == 0) {
                report("Invalid line size. Aborting\n");
                abort();
        };

        for (char *c = text; *c; c++) {
                if (*c == '\033') {
                        eval_escseq(&c, &ctx->color);
                        continue;
                }

                else if (*c == '\n') {
                        ctx->position.x = 0;
                        ctx->position.y += ctx->font.height;
                        continue;
                }

                else if (*c >= ' ' && *c < 127) {
                        DrawTextCodepoint(ctx->font.base, *c, ctx->position, ctx->font.size, ctx->color);
                }

                ctx->position.x += ctx->font.width + ctx->font.spacing;

                /* Line overflow at the right -> wrap */
                if (ctx->position.x >= ctx->max_chars * ctx->font.width) {
                        ctx->position.x = 0;
                        ctx->position.y += ctx->font.height;
                }

                if (ctx->position.y + ctx->font.height > ctx->screen_height) {
                        return 1; /* Screen overflow */
                }
        }
        return 0;
}

tFont
load_font()
{
        tFont font;

        HcfOpts opts = hcf_load("_settings.hcf");
        char *fontname = hcf_get_default(opts, "font", "path", FONT_DEFAULT_PATH);
        font.size = atof(hcf_get_default(opts, "font", "size", FONT_DEFAULT_SIZE));
        assert(font.size > 0);
        font.spacing = atof(hcf_get_default(opts, "font", "spacing", FONT_DEFAULT_SPACING));
        font.base = LoadFont(fontname);
        hcf_destroy(&opts);

        float font_inc = font.size / font.base.baseSize;
        font.width = (font.base.recs->width + font.spacing) * font_inc;
        font.height = (font.base.recs->height + font.spacing) * font_inc;

        return font;
}

void
context_reset_display(Context *ctx)
{
        ctx->color = TERM_DEFAULT_COLOR;
        ctx->position.x = 0;
        ctx->position.y = 0;
}

void
context_recalc_size(Context *ctx, int screen_height, int screen_width)
{
        ctx->screen_width = screen_width;
        ctx->screen_height = screen_height;
        ctx->max_chars = screen_width / (ctx->font.width + ctx->font.spacing);
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
        char text[1024] = { "\033[31mRED\033[0mRESET\033[34mBLUE\033[37m | Frames: " };
        char text_len = strlen(text);

        Context ctx = {
                .font = font,
        };
        context_reset_display(&ctx);
        context_recalc_size(&ctx, screen_height, screen_height);

        // Main game loop
        while (!WindowShouldClose()) // Detect window close button or ESC key
        {
                // Update
                //----------------------------------------------------------------------------------
                // TODO: Update variables / Implement example logic at this point
                //----------------------------------------------------------------------------------


                if (IsWindowResized()) {
                        get_window_size(&screen_height, &screen_width);
                        unsigned int h = (float) screen_height / (font.height);
                        unsigned int w = (float) screen_width / (font.width + font.spacing);
                        resize_term(t, h, w, screen_height, screen_width);
                        context_recalc_size(&ctx, screen_height, screen_width);
                        sb_drop_all(&screen_buffer);
                }

                process_input(t);
                process_output(t);


                // Draw
                //----------------------------------------------------------------------------------
                BeginDrawing();

                ClearBackground(BLACK);
                context_reset_display(&ctx);

                // TODO: Draw everything that requires to be drawn at this point:

                // a++;
                // sprintf(text + text_len, "%d", a);

                while (display_append_text(&ctx, sb_get(screen_buffer))) {
                        /* no all text fit in screen */
                        if (sb_drop_line(&screen_buffer)) {
                                ClearBackground(BLACK);
                                context_reset_display(&ctx);
                        } else
                                break;
                }

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
