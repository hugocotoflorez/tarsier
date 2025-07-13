#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "tarsier.h"


void
apply_color(int c, Color *fg, Color *bg)
{
        /* clang-format off */
        switch (c) {
        case 0:  
                *fg = TERM_DEFAULT_FG;   
                *bg = TERM_DEFAULT_BG;   
                break;

        case 90: case 30: *fg = BLACK;    break;
        case 91: case 31: *fg = RED;      break;
        case 92: case 32: *fg = GREEN;    break;
        case 93: case 33: *fg = ORANGE;   break;
        case 94: case 34: *fg = BLUE;     break;
        case 95: case 35: *fg = MAGENTA;  break;
        case 96: case 36: *fg = LIME;     break;
        case 97: case 37: *fg = WHITE;    break;

        case 100: case 40: *bg = BLACK;   break;
        case 101: case 41: *bg = RED;     break;
        case 102: case 42: *bg = GREEN;   break;
        case 103: case 43: *bg = ORANGE;  break;
        case 104: case 44: *bg = BLUE;    break;
        case 105: case 45: *bg = MAGENTA; break;
        case 106: case 46: *bg = LIME;    break;
        case 107: case 47: *bg = WHITE;   break;

/*ESC Code Sequence | Reset Sequence | Description                                                |
| ----------------- | -------------- | ---------------------------------------------------------- |
| `ESC[1;34;{...}m` |                | Set graphics modes for cell, separated by semicolon (`;`). |
| `ESC[0m`          |                | reset all modes (styles and colors)                        |
| `ESC[1m`          | `ESC[22m`      | set bold mode.                                             |
| `ESC[2m`          | `ESC[22m`      | set dim/faint mode.                                        |
| `ESC[3m`          | `ESC[23m`      | set italic mode.                                           |
| `ESC[4m`          | `ESC[24m`      | set underline mode.                                        |
| `ESC[5m`          | `ESC[25m`      | set blinking mode                                          |
| `ESC[7m`          | `ESC[27m`      | set inverse/reverse mode                                   |
| `ESC[8m`          | `ESC[28m`      | set hidden/invisible mode                                  |
| `ESC[9m`          | `ESC[29m`      | set strikethrough mode.                                   */

        }
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

static Vector2 saved_cursor = { 0 };

void
save_cursor(Context *ctx)
{
        saved_cursor = ctx->position;
}
void
restore_cursor(Context *ctx)
{
        ctx->position = saved_cursor;
}

typedef struct Immediate {
        int n;
        struct Immediate *next;
} Immd;

Immd *
get_data(Immd *data, char **c)
{
        Immd *dp = data;
        do {
                dp->n = consume_get_int(c);
                dp->next = calloc(1, sizeof *data);
                dp = dp->next;
        } while (**c == ';' && *++*c);

        return data;
}

void
free_data(Immd *data)
{
        if (data->next) {
                free_data(data->next);
                free(data->next);
        }
}

void
print_escseq(Immd data, char **c)
{
        report("\\033[");
        Immd *d = &data;
        do {
                report("%d", d->n);
                if ((d = d->next)->next) report(";");
        } while (d->next);
        report("%.5s", *c);
}

/* Return next char after match and consume char if STR start with PREFIX,
 * else return NULL and STR is not modified. */
char *
match(char **str, char *prefix)
{
        size_t l = strlen(prefix);
        if (memcmp(*str, prefix, l) == 0)
                return *str += l;
        return NULL;
}

/* Things no yet implemented (TODO)*/
/* clang-format off */
void enable_bracketed_paste() {}
void disable_bracketed_paste() {}
void make_cursor_visible(){}
void make_cursor_invisible(){}
void restore_screen(){}
void save_screen(){}
void enable_alternative_buffer(){}
void disable_alternative_buffer(){}
void request_cursor_postion(){}
/* clang-format on */

void
move_cursor_to(Immd data, Context *ctx)
{
        int line = data.n;
        int column;

        /* Only two arguments can be provided */
        if (data.next && data.next->next && !data.next->next->next) {
                column = data.next->n;
                rowoffsetset(ctx, line);
                coloffsetset(ctx, column);
        }

        else if (line == 0) {
                rowoffsetset(ctx, 0);
                coloffsetset(ctx, 0);
        }

        else {
                report("ESC[nH only accept zero or two arguments\n");
                print_escseq(data, (char **) &"ESC[nH");
        }
}

void
erase(char **c, Context *ctx)
{
}

void
eval_escseq(char **c, Context *ctx)
{
        Immd data;
        Immd *d;

        assert(**c == '\033'); /* Do not call it incorrectly */

        if (memcmp(*c, "\033[", 2) == 0) {
                *c += 2;
                get_data(&data, c);

                switch (**c) {
                case 'm': // color
                        d = &data;
                        do {
                                apply_color(d->n, &ctx->color, &ctx->bgcolor);
                        } while ((d = d->next)->next);
                        break;

                case 'H':
                case 'f':
                        move_cursor_to(data, ctx);
                        break;

                case 'A':
                        rowoffsetinc(ctx, -(data.n ?: 1));
                        break;

                case 'B':
                        rowoffsetinc(ctx, data.n ?: 1);
                        break;

                case 'C':
                        coloffsetinc(ctx, data.n ?: 1);
                        break;

                case 'D':
                        coloffsetinc(ctx, -(data.n ?: 1));
                        break;

                case 'E':
                        coloffsetset(ctx, 0);
                        rowoffsetset(ctx, data.n ?: 1);
                        break;

                case 'F':
                        coloffsetset(ctx, 0);
                        rowoffsetset(ctx, -(data.n ?: 1));
                        break;

                case 'G':
                        coloffsetset(ctx, data.n ?: 1);
                        break;

                case 'M': // should scroll of needed
                        rowoffsetinc(ctx, -(data.n ?: 1));
                        break;

                case '7':
                case 's':
                        save_cursor(ctx);
                        break;

                case '8':
                case 'u':
                        restore_cursor(ctx);
                        break;

                case 'J':
                case 'K':
                        erase(c, ctx);
                        break;

                default:
                        /* clang-format off */
                        if (0) ;
                        else if (match(c, "?25h")) make_cursor_visible();
                        else if (match(c, "?25l")) make_cursor_invisible();
                        else if (match(c, "?47h")) restore_screen();
                        else if (match(c, "?47l")) save_screen();
                        else if (match(c, "?1049h")) enable_alternative_buffer();
                        else if (match(c, "?1049l")) disable_alternative_buffer();
                        else if (match(c, "?2004h")) enable_bracketed_paste();
                        else if (match(c, "?2004l")) disable_bracketed_paste();
                        else if (match(c, "6n")) request_cursor_postion();
                        /* clang-format on */
                        else
                                print_escseq(data, c);
                }

                free_data(&data);

        }

        else if (memcmp(*c, "\033]", 2) == 0) {
                *c += 2;
                switch (**c) {
                case '0':
                        ++*c; // todo
                        break;
                default:
                        report("[FAIL]: %s\n", *c - 2);
                        assert(!1);
                }

        } else {
                report("[FAIL]: %s\n", *c);
                assert(!1);
        }
}
