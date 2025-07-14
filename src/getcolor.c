#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "hcfp.h"
#include "tarsier.h"

#define COLOR_DEFAULT_BG "#0000FF"
#define COLOR_DEFAULT_FG "#FFFFFF"
#define COLOR_DEFAULT_BLACK "#000000"
#define COLOR_DEFAULT_RED "#FF0000"
#define COLOR_DEFAULT_GREEN "#00FF00"
#define COLOR_DEFAULT_YELLOW "#FFFF00"
#define COLOR_DEFAULT_BLUE "#0000FF"
#define COLOR_DEFAULT_MAGENTA "#FF00FF"
#define COLOR_DEFAULT_CYAN "#00FFFF"
#define COLOR_DEFAULT_WHITE "#FFFFFF"
#define COLOR_DEFAULT_CURSOR "#FFFFFF"

Color color_lookup[8] = {
        /* [0] = Color for black */
        /* [1] = Color for red */
        /* ... */
};

Color TERM_DEFAULT_FG;
Color TERM_DEFAULT_BG;
Color TERM_DEFAULT_CURSOR;

char *
rtrim(char *s)
{
        char *c = s + strlen(s) - 1;
        while (isspace(*c) && c != s) {
                *c = 0;
                --c;
        }
        return s;
}

unsigned char
to_hex(char c)
{
        if ('0' <= c && c <= '9') return c - '0';
        if ('a' <= c && c <= 'f') return c - 'a' + 10;
        if ('A' <= c && c <= 'F') return c - 'A' + 10;
        if ('O' == c || 'o' == c) return '0'; // just in case
        return 0;
}

/* Convert a string like #00FFee to a Color */
Color
hex_to_Color(char *s)
{
        rtrim(s);
        if (strlen(s) != 7 || *s != '#') {
                printf("Invalid color: `%s`\n", s);
                exit(1);
        }

        printf("Color: %s -> ", s);

        Color c = {
                .r = 16 * to_hex(s[1]) + to_hex(s[2]),
                .g = 16 * to_hex(s[3]) + to_hex(s[4]),
                .b = 16 * to_hex(s[5]) + to_hex(s[6]),
                .a = 255,
        };

        printf("%d ", c.r);
        printf("%d ", c.g);
        printf("%d;\n", c.b);
        return c;
}

/* Have to be called */
void
get_default_colors()
{
        HcfOpts opts = hcf_load("settings.hcf");

        char *bg = hcf_get_default(opts, "colors", "background", COLOR_DEFAULT_BG);
        char *fg = hcf_get_default(opts, "colors", "foreground", COLOR_DEFAULT_FG);
        char *cur = hcf_get_default(opts, "colors", "cursor", COLOR_DEFAULT_CURSOR);
        char *c0 = hcf_get_default(opts, "colors", "color0", COLOR_DEFAULT_BLACK);
        char *c1 = hcf_get_default(opts, "colors", "color1", COLOR_DEFAULT_RED);
        char *c2 = hcf_get_default(opts, "colors", "color2", COLOR_DEFAULT_GREEN);
        char *c3 = hcf_get_default(opts, "colors", "color3", COLOR_DEFAULT_YELLOW);
        char *c4 = hcf_get_default(opts, "colors", "color4", COLOR_DEFAULT_BLUE);
        char *c5 = hcf_get_default(opts, "colors", "color5", COLOR_DEFAULT_MAGENTA);
        char *c6 = hcf_get_default(opts, "colors", "color6", COLOR_DEFAULT_CYAN);
        char *c7 = hcf_get_default(opts, "colors", "color7", COLOR_DEFAULT_WHITE);

        TERM_DEFAULT_BG = hex_to_Color(bg);
        TERM_DEFAULT_FG = hex_to_Color(fg);
        TERM_DEFAULT_CURSOR = hex_to_Color(cur);
        color_lookup[0] = hex_to_Color(c0);
        color_lookup[1] = hex_to_Color(c1);
        color_lookup[2] = hex_to_Color(c2);
        color_lookup[3] = hex_to_Color(c3);
        color_lookup[4] = hex_to_Color(c4);
        color_lookup[5] = hex_to_Color(c5);
        color_lookup[6] = hex_to_Color(c6);
        color_lookup[7] = hex_to_Color(c7);

        hcf_destroy(&opts);
}
