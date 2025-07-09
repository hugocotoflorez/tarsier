#include <stdio.h>

#include "raylib/src/raylib.h"

struct Term {};

static const char *FUNC_KEY_NAME[] = {
    [KEY_SPACE] = "KEY_SPACE",                 // Key: Space
    [KEY_ESCAPE] = "KEY_ESCAPE",               // Key: Esc
    [KEY_ENTER] = "KEY_ENTER",                 // Key: Enter
    [KEY_TAB] = "KEY_TAB",                     // Key: Tab
    [KEY_BACKSPACE] = "KEY_BACKSPACE",         // Key: Backspace
    [KEY_INSERT] = "KEY_INSERT",               // Key: Ins
    [KEY_DELETE] = "KEY_DELETE",               // Key: Del
    [KEY_RIGHT] = "KEY_RIGHT",                 // Key: Cursor right
    [KEY_LEFT] = "KEY_LEFT",                   // Key: Cursor left
    [KEY_DOWN] = "KEY_DOWN",                   // Key: Cursor down
    [KEY_UP] = "KEY_UP",                       // Key: Cursor up
    [KEY_PAGE_UP] = "KEY_PAGE_UP",             // Key: Page up
    [KEY_PAGE_DOWN] = "KEY_PAGE_DOWN",         // Key: Page down
    [KEY_HOME] = "KEY_HOME",                   // Key: Home
    [KEY_END] = "KEY_END",                     // Key: End
    [KEY_CAPS_LOCK] = "KEY_CAPS_LOCK",         // Key: Caps lock
    [KEY_SCROLL_LOCK] = "KEY_SCROLL_LOCK",     // Key: Scroll down
    [KEY_NUM_LOCK] = "KEY_NUM_LOCK",           // Key: Num lock
    [KEY_PRINT_SCREEN] = "KEY_PRINT_SCREEN",   // Key: Print screen
    [KEY_PAUSE] = "KEY_PAUSE",                 // Key: Pause
    [KEY_F1] = "KEY_F1",                       // Key: F1
    [KEY_F2] = "KEY_F2",                       // Key: F2
    [KEY_F3] = "KEY_F3",                       // Key: F3
    [KEY_F4] = "KEY_F4",                       // Key: F4
    [KEY_F5] = "KEY_F5",                       // Key: F5
    [KEY_F6] = "KEY_F6",                       // Key: F6
    [KEY_F7] = "KEY_F7",                       // Key: F7
    [KEY_F8] = "KEY_F8",                       // Key: F8
    [KEY_F9] = "KEY_F9",                       // Key: F9
    [KEY_F10] = "KEY_F10",                     // Key: F10
    [KEY_F11] = "KEY_F11",                     // Key: F11
    [KEY_F12] = "KEY_F12",                     // Key: F12
    [KEY_LEFT_SHIFT] = "KEY_LEFT_SHIFT",       // Key: Shift left
    [KEY_LEFT_CONTROL] = "KEY_LEFT_CONTROL",   // Key: Control left
    [KEY_LEFT_ALT] = "KEY_LEFT_ALT",           // Key: Alt left
    [KEY_LEFT_SUPER] = "KEY_LEFT_SUPER",       // Key: Super left
    [KEY_RIGHT_SHIFT] = "KEY_RIGHT_SHIFT",     // Key: Shift right
    [KEY_RIGHT_CONTROL] = "KEY_RIGHT_CONTROL", // Key: Control right
    [KEY_RIGHT_ALT] = "KEY_RIGHT_ALT",         // Key: Alt right
    [KEY_RIGHT_SUPER] = "KEY_RIGHT_SUPER",     // Key: Super right
    [KEY_KB_MENU] = "KEY_KB_MENU",             // Key: KB menu
};

void is_function(int kp) {
  switch (kp) {
  case KEY_SPACE:         // Key: Space
  case KEY_ESCAPE:        // Key: Esc
  case KEY_ENTER:         // Key: Enter
  case KEY_TAB:           // Key: Tab
  case KEY_BACKSPACE:     // Key: Backspace
  case KEY_INSERT:        // Key: Ins
  case KEY_DELETE:        // Key: Del
  case KEY_RIGHT:         // Key: Cursor right
  case KEY_LEFT:          // Key: Cursor left
  case KEY_DOWN:          // Key: Cursor down
  case KEY_UP:            // Key: Cursor up
  case KEY_PAGE_UP:       // Key: Page up
  case KEY_PAGE_DOWN:     // Key: Page down
  case KEY_HOME:          // Key: Home
  case KEY_END:           // Key: End
  case KEY_CAPS_LOCK:     // Key: Caps lock
  case KEY_SCROLL_LOCK:   // Key: Scroll down
  case KEY_NUM_LOCK:      // Key: Num lock
  case KEY_PRINT_SCREEN:  // Key: Print screen
  case KEY_PAUSE:         // Key: Pause
  case KEY_F1:            // Key: F1
  case KEY_F2:            // Key: F2
  case KEY_F3:            // Key: F3
  case KEY_F4:            // Key: F4
  case KEY_F5:            // Key: F5
  case KEY_F6:            // Key: F6
  case KEY_F7:            // Key: F7
  case KEY_F8:            // Key: F8
  case KEY_F9:            // Key: F9
  case KEY_F10:           // Key: F10
  case KEY_F11:           // Key: F11
  case KEY_F12:           // Key: F12
  case KEY_LEFT_SHIFT:    // Key: Shift left
  case KEY_LEFT_CONTROL:  // Key: Control left
  case KEY_LEFT_ALT:      // Key: Alt left
  case KEY_LEFT_SUPER:    // Key: Super left
  case KEY_RIGHT_SHIFT:   // Key: Shift right
  case KEY_RIGHT_CONTROL: // Key: Control right
  case KEY_RIGHT_ALT:     // Key: Alt right
  case KEY_RIGHT_SUPER:   // Key: Super right
  case KEY_KB_MENU:       // Key: KB menu
    printf("Function key: %s\n", FUNC_KEY_NAME[kp]);
    break;
  }
}

void process_input(Term t) {
  int kp;

  while ((kp = GetKeyPressed()))
    switch (kp) {
    case KEY_NULL:          // Key: NULL, used for no key
                            //            pressed Alphanumeric keys
    case KEY_APOSTROPHE:    // Key: '
    case KEY_COMMA:         // Key: ,
    case KEY_MINUS:         // Key: -
    case KEY_PERIOD:        // Key: .
    case KEY_SLASH:         // Key: /
    case KEY_ZERO:          // Key: 0
    case KEY_ONE:           // Key: 1
    case KEY_TWO:           // Key: 2
    case KEY_THREE:         // Key: 3
    case KEY_FOUR:          // Key: 4
    case KEY_FIVE:          // Key: 5
    case KEY_SIX:           // Key: 6
    case KEY_SEVEN:         // Key: 7
    case KEY_EIGHT:         // Key: 8
    case KEY_NINE:          // Key: 9
    case KEY_SEMICOLON:     // Key: ;
    case KEY_EQUAL:         // Key: =
    case KEY_LEFT_BRACKET:  // Key: [
    case KEY_BACKSLASH:     // Key: '\'
    case KEY_RIGHT_BRACKET: // Key: ]
    case KEY_GRAVE:         // Key: `
    case KEY_A:             // Key: A | a
    case KEY_B:             // Key: B | b
    case KEY_C:             // Key: C | c
    case KEY_D:             // Key: D | d
    case KEY_E:             // Key: E | e
    case KEY_F:             // Key: F | f
    case KEY_G:             // Key: G | g
    case KEY_H:             // Key: H | h
    case KEY_I:             // Key: I | i
    case KEY_J:             // Key: J | j
    case KEY_K:             // Key: K | k
    case KEY_L:             // Key: L | l
    case KEY_M:             // Key: M | m
    case KEY_N:             // Key: N | n
    case KEY_O:             // Key: O | o
    case KEY_P:             // Key: P | p
    case KEY_Q:             // Key: Q | q
    case KEY_R:             // Key: R | r
    case KEY_S:             // Key: S | s
    case KEY_T:             // Key: T | t
    case KEY_U:             // Key: U | u
    case KEY_V:             // Key: V | v
    case KEY_W:             // Key: W | w
    case KEY_X:             // Key: X | x
    case KEY_Y:             // Key: Y | y
    case KEY_Z:             // Key: Z | z
      printf("%c\n", kp);
      term_send(t, c);
      break;

    default:
      is_function(kp);
    }
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
    process_input();

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    // TODO: Draw everything that requires to be drawn at this point:

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
