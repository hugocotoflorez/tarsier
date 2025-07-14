#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "tarsier.h"

typedef struct Immediate {
        int n;
        struct Immediate *next;
} Immd;

void
apply_color(int c, Context *ctx)
{
        /* clang-format off */
        switch (c) {
        case 0:  
                ctx->fgcolor = TERM_DEFAULT_FG;   
                ctx->bgcolor = TERM_DEFAULT_BG;   
                ctx->text_mode = TM_NORMAL;
                break;

        case  90: case 30: ctx->fgcolor = color_lookup[0];      break;
        case  91: case 31: ctx->fgcolor = color_lookup[1];      break;
        case  92: case 32: ctx->fgcolor = color_lookup[2];      break;
        case  93: case 33: ctx->fgcolor = color_lookup[3];      break;
        case  94: case 34: ctx->fgcolor = color_lookup[4];      break;
        case  95: case 35: ctx->fgcolor = color_lookup[5];      break;
        case  96: case 36: ctx->fgcolor = color_lookup[6];      break;
        case  97: case 37: ctx->fgcolor = color_lookup[7];      break;

        case 100: case 40: ctx->bgcolor = color_lookup[0];      break;
        case 101: case 41: ctx->bgcolor = color_lookup[1];      break;
        case 102: case 42: ctx->bgcolor = color_lookup[2];      break;
        case 103: case 43: ctx->bgcolor = color_lookup[3];      break;
        case 104: case 44: ctx->bgcolor = color_lookup[4];      break;
        case 105: case 45: ctx->bgcolor = color_lookup[5];      break;
        case 106: case 46: ctx->bgcolor = color_lookup[6];      break;
        case 107: case 47: ctx->bgcolor = color_lookup[7];      break;

        case   1: ctx->text_mode |=  TM_BOLD;                   break; 
        case   2: ctx->text_mode |=  TM_DIM;                    break; 
        case   3: ctx->text_mode |=  TM_ITALIC;                 break; 
        case   4: ctx->text_mode |=  TM_UNDERLINE;              break; 
        case   5: ctx->text_mode |=  TM_BLINKING;               break; 
        case   7: ctx->text_mode |=  TM_INVERSE;                break; 
        case   8: ctx->text_mode |=  TM_HIDDEN;                 break; 
        case   9: ctx->text_mode |=  TM_STRIKETHROUGH;          break; 

        case  21: ctx->text_mode &=~ TM_BOLD;                   break; 
        case  22: ctx->text_mode &=~ TM_DIM;                    break; 
        case  23: ctx->text_mode &=~ TM_ITALIC;                 break; 
        case  24: ctx->text_mode &=~ TM_UNDERLINE;              break; 
        case  25: ctx->text_mode &=~ TM_BLINKING;               break; 
        case  27: ctx->text_mode &=~ TM_INVERSE;                break; 
        case  28: ctx->text_mode &=~ TM_HIDDEN;                 break; 
        case  29: ctx->text_mode &=~ TM_STRIKETHROUGH;          break; 
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

        /* 0 is == 1? should it? */

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
erase(Immd n, char **c, Context *ctx)
{
        switch (**c) {
        case 'J':
                if (n.n == 0) erase_end_screen(ctx);
                if (n.n == 1) erase_begin_screen(ctx);
                if (n.n == 2) erase_screen(ctx);
                if (n.n == 3) erase_saved_lines(ctx);
                break;
        case 'K':
                if (n.n == 0) erase_end_line(ctx);
                if (n.n == 1) erase_begin_line(ctx);
                if (n.n == 2) erase_line(ctx);
                break;
        }
        /*ESC Code Sequence | Description                               |
        | :---------------- | :---------------------------------------- |
        | `ESC[J`           | erase in display (same as `ESC[0J`)       |
        | `ESC[0J`          | erase from cursor until end of screen     |
        | `ESC[1J`          | erase from cursor to beginning of screen  |
        | `ESC[2J`          | erase entire screen                       |
        | `ESC[3J`          | erase saved lines                         |
        | `ESC[K`           | erase in line (same as `ESC[0K`)          |
        | `ESC[0K`          | erase from cursor to end of line          |
        | `ESC[1K`          | erase start of line to the cursor         |
        | `ESC[2K`          | erase the entire line                    */
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
                                apply_color(d->n, ctx);
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
                        erase(data, c, ctx);
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
