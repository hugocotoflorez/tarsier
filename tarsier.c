#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ascii.h"
#include "raylib/src/raylib.h"
#include "term.h"

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

int using_no_shift(int kp) {
  if (0 <= kp && kp <= 128)
    return WITHOUT_SHIFT[kp];
  return kp;
}

int using_shilf(int kp) {
  if (0 <= kp && kp <= 128)
    return WITH_SHIFT[kp];
  return kp;
}

#define IsModDown(MOD) (IsKeyDown(KEY_LEFT_##MOD) || IsKeyDown(KEY_LEFT_##MOD))

int process_input(Term t) {
  ModState state = 0;
  if (IsModDown(SHIFT))
    state |= MOD_SHIFT;
  if (IsModDown(CONTROL))
    state |= MOD_CONTROL;
  if (IsModDown(ALT))
    state |= MOD_ALT;
  if (IsModDown(SUPER))
    state |= MOD_SUPER;

  int kp;
  while ((kp = GetKeyPressed())) {
    kp = (state & MOD_SHIFT) ? using_shilf(kp) : using_no_shift(kp);

    /* clang-format off */
    if (IsKeyPressed(KEY_ENTER)) term_send(t, LF);
    if (IsKeyPressed(KEY_ESCAPE)) term_send(t, ESC);
    if (IsKeyPressed(KEY_BACKSPACE)) term_send(t, BS);
    if (IsKeyPressed(KEY_TAB)) term_send(t, HT);
    if (IsKeyPressed(KEY_DELETE)) term_send(t, DEL);
    /* clang-format on */

    if (kp >= 0 && kp < 128)
      term_send(t, kp);
  }
  return 0;
}

int get_window_size(int *h, int *w) {
  *h = GetRenderHeight();
  *w = GetRenderWidth();
  return 0;
}

int process_output(Term t) {
  char buf[1024];
  while (read_fileno(t.master_fd, buf, sizeof buf)) {
    printf("%s", buf); // Example
    fflush(stdout);
  }
  return 0;
}

void display_text(int screen_width, int screen_height, tFont font, char *text) {
  int max_chars = screen_width / font.width;

  char *m = text;
  int row = 0;

  do {
    if (max_chars <= 0)
      break;
    int length = (strlen(text) < max_chars) ? strlen(text) : max_chars;
    char prev = text[length];
    text[length] = 0;
    DrawTextEx(font.base, text, (Vector2){0.0f, (float)row}, font.size,
               font.spacing, GREEN);
    text[length] = prev;
    text += length;
    row += font.height;
  } while (text && *text && row <= screen_height);
}

int main(void) {

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
  tFont font;
  font.base = LoadFont("/usr/share/fonts/TTF/IosevkaNerdFontMono-Bold.ttf");
  font.size = 36;
  font.spacing = 2;
  float font_inc = font.size / font.base.baseSize;
  font.width = (font.base.recs->width + font.spacing) * font_inc;
  font.height = (font.base.recs->height + font.spacing) * font_inc;

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    //----------------------------------------------------------------------------------

    process_input(t);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(BLACK);

    process_output(t);
    get_window_size(&screen_height, &screen_width);

    // TODO: Draw everything that requires to be drawn at this point:

    char *text =
        strdup("Congrats! You created your first window! "
               "fakshfhfjjfjfjfjfjshshdklldiowruoweurowieurwpoierupwoqi"
               "eurpqwoiuerpoqiw0");

    display_text(screen_width,screen_height, font, text);
    EndDrawing();
    //----------------------------------------------------------------------------------
    free(text);
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------

  // TODO: Unload all loaded resources at this point

  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
