#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#include "ascii.h"
#include "raylib/src/raylib.h"
#include "term.h"

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

int process_output(Term t) {
  char buf[1024];
  while (read_fileno(t.master_fd, buf, sizeof buf)) {
    printf("%s", buf); // Example
    fflush(stdout);
  }
  return 0;
}

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Tarsier terminal emulator");

  // TODO: Load resources / Initialize variables at this point

  SetTargetFPS(30);
  SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
  SetExitKey(KEY_NULL);
  //--------------------------------------------------------------------------------------

  Term t = term_start();

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

    ClearBackground(RAYWHITE);

    // TODO: Draw everything that requires to be drawn at this point:
    process_output(t);

    DrawText("Congrats! You created your first window!", 190, 200, 20,
             LIGHTGRAY); // Example

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------

  // TODO: Unload all loaded resources at this point

  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
