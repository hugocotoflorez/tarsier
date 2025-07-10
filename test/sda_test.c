#include <stdio.h>

#include "../src/sda.h"

Sda stream = { 0 };

int
main()
{
        printf("%s\n", stream.data ?: "");
        for (int i = 0; i < 1024; i++) {
                sda_append_str(&stream, "");
                printf("%s\n", stream.data ?: "");
        }
        for (int i = 0; i < 1024; i++) {
                sda_append(&stream, 'a');
                printf("%s\n", stream.data ?: "");
        }
        for (int i = 0; i < 32; i++) {
                sda_append_str(&stream, "Hello, World!");
                printf("%s\n", stream.data ?: "");
        }
        for (int i = 0; i < 1024; i++) {
                sda_append(&stream, 'a');
                printf("%s\n", stream.data ?: "");
        }
}
