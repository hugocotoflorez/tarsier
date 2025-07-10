/* String Dynamic Array */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 128

typedef struct Sda {
        size_t capacity;
        size_t count;
        char *data;
} Sda;

void
sda_grow(Sda *stream)
{
        if (stream->capacity == 0) {
                stream->capacity = INITIAL_CAPACITY;
                stream->data = (char *) calloc(1, stream->capacity);
                assert(stream->data);
                return;
        }
        stream->capacity *= 2;
        stream->data = (char *) realloc(stream->data, stream->capacity);
        assert(stream->data);
}

void
sda_append_str(Sda *stream, const char *buf)
{
        ssize_t len = strlen(buf);
        while (stream->capacity - stream->count <= len + 1)
                sda_grow(stream);

        strcat(stream->data, buf);
}

void
sda_append(Sda *stream, char c)
{
        if (stream->capacity <= stream->count + 1)
                sda_grow(stream);
        stream->data[stream->count] = c;
        ++stream->count;
        stream->data[stream->count] = 0;
}
