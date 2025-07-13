#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include "ascii.h"
#include "term.h"

#include "screen_buffer.h"

#include "../raylib/src/raylib.h"

#define HCFP_IMPLEMENTATION
#include "hcfp.h"
#include "tarsier.h"

typedef enum {
        MOD_NONE = 0,
        MOD_SHIFT = 1,
        MOD_CONTROL = 2,
        MOD_ALT = 4,
        MOD_SUPER = 8,
} ModState;

Sb screen_buffer = { 0 };


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

/* Convert L lines to y offset in pixels */
int
rowoffsetset(Context *ctx, int l)
{
        return ctx->position.y = ctx->font.height * l;
}

/* Convert L lines to x offset in pixels */
int
coloffsetset(Context *ctx, int l)
{
        return ctx->position.x = (ctx->font.width + ctx->font.spacing) * l;
}

/* Increment by L lines y offset in pixels */
int
rowoffsetinc(Context *ctx, int l)
{
        return ctx->position.y += ctx->font.height * l;
}

/* Increment by L lines x offset in pixels */
int
coloffsetinc(Context *ctx, int l)
{
        return ctx->position.x += (ctx->font.width + ctx->font.spacing) * l;
}

void
print_bg(Context *ctx)
{
        DrawRectangle(ctx->position.x, ctx->position.y,
                      ctx->font.width, ctx->font.height, ctx->bgcolor);
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
                        eval_escseq(&c, ctx);
                }

                else if (*c == '\n') {
                        rowoffsetinc(ctx, 1);
                        coloffsetset(ctx, 0);
                }

                else if (*c == '\b') {
                        /* Si arreglo el texto aqui no se recalcula en cada iteracion */
                        coloffsetinc(ctx, -1);
                        print_bg(ctx);
                        DrawTextCodepoint(ctx->font.base, ' ', ctx->position,
                                          ctx->font.size, ctx->color);
                }

                else if (*c >= ' ' && *c < 127) {
                        print_bg(ctx);
                        DrawTextCodepoint(ctx->font.base, *c, ctx->position,
                                          ctx->font.size, ctx->color);
                        coloffsetinc(ctx, 1);
                }


                /* Line overflow at the right -> wrap */
                if (ctx->position.x >= ctx->max_chars * ctx->font.width) {
                        rowoffsetinc(ctx, 1);
                        coloffsetset(ctx, 0);
                }

                /* Screen overflow */
                if (ctx->position.y + ctx->font.height > ctx->screen_height) {
                        return 1;
                }
        }
        return 0;
}

tFont
load_font()
{
        /* font have to be monospace */
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
        ctx->color = TERM_DEFAULT_FG;
        ctx->bgcolor = TERM_DEFAULT_BG;
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

void
erase_end_screen(Context *ctx)
{
        /* Todo */
        DrawRectangle(ctx->position.x, ctx->position.y,
                      ctx->font.width, ctx->font.height, ctx->bgcolor);
}

void
erase_begin_screen(Context *ctx)
{
        /* Todo */
        DrawRectangle(ctx->position.x, ctx->position.y,
                      ctx->font.width, ctx->font.height, ctx->bgcolor);
}

void
erase_screen(Context *ctx)
{
        DrawRectangle(0, 0, ctx->screen_width, ctx->screen_height, ctx->bgcolor);
}

void
erase_saved_lines(Context *ctx)
{
        /* Todo */
        DrawRectangle(ctx->position.x, ctx->position.y, ctx->font.width, ctx->font.height, ctx->bgcolor);
}

void
erase_end_line(Context *ctx)
{
        /* To check */
        DrawRectangle(ctx->position.x, ctx->position.y, ctx->screen_width - ctx->font.width, ctx->font.height, ctx->bgcolor);
}

void
erase_begin_line(Context *ctx)
{
        /* should erase 1 more char? */
        DrawRectangle(0, ctx->position.y, ctx->position.x, ctx->font.height, ctx->bgcolor);
}

void
erase_line(Context *ctx)
{
        DrawRectangle(0, ctx->position.y, ctx->screen_width, ctx->font.height, ctx->bgcolor);
}

int
main(void)
{
        int screen_height = 800;
        int screen_width = 600;

        InitWindow(screen_width, screen_height, "Tarsier terminal emulator");

        // TODO: Load resources / Initialize variables at this point

        SetTargetFPS(30);
        SetWindowState(FLAG_VSYNC_HINT | FLAG_BORDERLESS_WINDOWED_MODE);
        SetExitKey(KEY_NULL);
        //--------------------------------------------------------------------------------------

        Term t = term_start();
        tFont font = load_font();
        Context ctx = { .font = font };
        context_reset_display(&ctx);

        unsigned int h = (float) screen_height / (font.height);
        unsigned int w = (float) screen_width / (font.width + font.spacing);
        resize_term(t, h, w, screen_height, screen_width);
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

                while (display_append_text(&ctx, sb_get(screen_buffer))) {
                        /* not all text fit in screen */
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
