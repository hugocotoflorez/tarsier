#include <string.h>
#include <assert.h>

#include "tarsier.h"


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
eval_escseq(char **c, Context* ctx)
{
        assert(**c == '\033'); /* Do not call it incorrectly */

        if (memcmp(*c, "\033[", 2) == 0) {
                *c += 2;
                do {
                        apply_color(consume_get_int(c), &ctx->color);
                } while (**c == ';' && *++*c);
                if (**c == 'm') {
                        // Was a color"
                }
        }
}

