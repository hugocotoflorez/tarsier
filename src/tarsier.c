#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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
        MS_NONE = 0,
        MS_SHIFT = 1,
        MS_CONTROL = 2,
        MS_ALT = 4,
        MS_SUPER = 8,
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

int
using_control(int kp)
{
        if (64 <= kp && kp <= 94) {
                printf("CONTROL %c\n", kp);
                return kp - 64;
        }
        return kp;
}


#define IsModDown(MOD) (IsKeyDown(KEY_LEFT_##MOD) || IsKeyDown(KEY_RIGHT_##MOD))

int
process_input(Term t)
{
        ModState state = 0;
        if (IsModDown(SHIFT)) state |= MS_SHIFT;
        if (IsModDown(CONTROL)) state |= MS_CONTROL;
        if (IsModDown(ALT)) state |= MS_ALT;
        if (IsModDown(SUPER)) state |= MS_SUPER;

        int kp;
        while ((kp = GetKeyPressed())) {
                kp = (state & MS_CONTROL) ? using_control(kp) : kp;
                kp = (state & MS_SHIFT) ? using_shilf(kp) : using_no_shift(kp);

                if (IsKeyPressed(KEY_ENTER)) term_send(t, LF);
                if (IsKeyPressed(KEY_ESCAPE)) term_send(t, ESC);
                if (IsKeyPressed(KEY_BACKSPACE)) term_send(t, BS);
                if (IsKeyPressed(KEY_TAB)) term_send(t, HT);
                if (IsKeyPressed(KEY_DELETE)) term_send(t, DEL);

                if (kp <= 127) term_send(t, kp);
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
                      ctx->font.width + ctx->font.spacing, ctx->font.height, ctx->bgcolor);
}

int
display_append_text(Context *ctx, char *text)
{
        if (text == NULL) return 0;
        if (ctx->max_chars == 0) {
                report("Invalid line size. Aborting\n");
                abort();
        };

        Font font = ctx->font.base;

        if (ctx->text_mode & TM_BOLD & TM_ITALIC)
                font = ctx->font.bold_italic;
        else if (ctx->text_mode & TM_BOLD)
                font = ctx->font.bold;
        else if (ctx->text_mode & TM_ITALIC)
                font = ctx->font.italic;

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
                                          ctx->font.size, ctx->fgcolor);
                }

                else if (*c >= ' ' && *c < 127) {
                        print_bg(ctx);
                        DrawTextCodepoint(ctx->font.base, *c, ctx->position,
                                          ctx->font.size, ctx->fgcolor);
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

        HcfOpts opts = hcf_load("settings.hcf");
        char *base = hcf_get_default(opts, "font", "base", FONT_DEFAULT_PATH_BASE);
        char *italic = hcf_get_default(opts, "font", "italic", FONT_DEFAULT_PATH_ITALIC);
        char *bold = hcf_get_default(opts, "font", "bold", FONT_DEFAULT_PATH_BOLD);
        char *bold_italic = hcf_get_default(opts, "font", "bold-italic", FONT_DEFAULT_PATH_BOLD_ITALIC);
        font.size = atof(hcf_get_default(opts, "font", "size", FONT_DEFAULT_SIZE));
        assert(font.size > 0);
        font.spacing = atof(hcf_get_default(opts, "font", "spacing", FONT_DEFAULT_SPACING));
        font.base = LoadFont(base);
        font.bold = LoadFont(bold);
        font.italic = LoadFont(italic);
        font.bold_italic = LoadFont(bold_italic);
        hcf_destroy(&opts);

        float font_inc = font.size / font.base.baseSize;
        font.width = (font.base.recs->width + font.spacing) * font_inc;
        font.height = (font.base.recs->height + font.spacing) * font_inc;

        return font;
}

void
context_reset_display(Context *ctx)
{
        ctx->fgcolor = TERM_DEFAULT_FG;
        ctx->bgcolor = TERM_DEFAULT_BG;
        ctx->text_mode = TM_NORMAL;
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
        DrawRectangle(ctx->position.x, ctx->position.y, ctx->screen_width - ctx->position.x, ctx->screen_height - ctx->position.y, ctx->bgcolor);
}

void
erase_begin_screen(Context *ctx)
{
        DrawRectangle(0, 0, ctx->position.x, ctx->position.y, ctx->bgcolor);
}

void
erase_screen(Context *ctx)
{
        DrawRectangle(0, 0, ctx->screen_width, ctx->screen_height, ctx->bgcolor);
}

void
erase_saved_lines(Context *ctx)
{
        assert("Wtf is this" && 0);
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

void
print_cursor(Context *ctx)
{
        DrawRectangle(ctx->position.x, ctx->position.y,
                      ctx->font.width + ctx->font.spacing,
                      ctx->font.height, TERM_DEFAULT_CURSOR);
}

int
main(void)
{
        int screen_height = 800;
        int screen_width = 600;

        InitWindow(screen_width, screen_height, "Tarsier terminal emulator");

        // TODO: Load resources / Initialize variables at this point

        SetTargetFPS(30);
        SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE );
        SetExitKey(KEY_NULL);
        //--------------------------------------------------------------------------------------

        Term t = term_start();
        tFont font = load_font();
        Context ctx = { .font = font };

        get_default_colors();
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

                ClearBackground(TERM_DEFAULT_BG);
                context_reset_display(&ctx);

                // TODO: Draw everything that requires to be drawn at this point:

                while (display_append_text(&ctx, sb_get(screen_buffer))) {
                        /* not all text fit in screen */
                        if (sb_drop_line(&screen_buffer)) {
                                ClearBackground(TERM_DEFAULT_BG);
                                context_reset_display(&ctx);
                        } else
                                break;
                }

                print_cursor(&ctx);

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
