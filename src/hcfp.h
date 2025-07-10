#ifndef HCF_HASHMAP_H
#define HCF_HASHMAP_H


#include <stddef.h>
/* Hash table */

// The idea is to store values using strdup for strings and  be able
// to store pointers to hash tables
typedef struct __hash_table_node {
        struct __hash_table_node *next; // allow collisions
        char *key;                      // current element key
        void *value;                    // current element value
} HashTableNode;

typedef struct
{
        size_t size;             // elements in node_arr
        HashTableNode *node_arr; // array of nodes
} __HashTable;

/* Initialize a hashmap of a given size */
void __hashmap_new(__HashTable *table, size_t size);
/* Add a value to a hashmap */
void __hashmap_add(__HashTable *table, const char *key, void *value);
/* Remove a key-value pair from a hashmap */
void __hashmap_pop(__HashTable *table, const char *key);
/* Get the value of a key in a hashmap or null if key not found*/
void *__hashmap_get(__HashTable table, const char *key, void **value);
/* Get the numeric key given a string key and a hash table */
size_t __hashmap_key(__HashTable table, const char *key);
/* */
void __hashmap_destroy(__HashTable *table);

#endif // !HCF_HASHMAP_H


#ifndef VSHCFP_H
#define VSHCFP_H

/* Type aliases */
typedef __HashTable HcfOpts;  // Options hash of hashes
typedef __HashTable HcfField; // fields hash

/* Implementation details */

/* The following numbers are just used to adjust hashmap memory efficiency.
 * The numbers dont set a maximum. The closest the number is to the reality
 * it would be more memory and time efficient. */

/* Number of fields in each file */
#define FIELDS_N 10
/* Number of key-value entries in each field */
#define OPTS_N 10

/* VSHCFP interface */

/* Load a file. File fields and options have to be defined following
 * the HCF standard. They are stored in a HcfOpts struct that have to
 * be destroyed. */
HcfOpts hcf_load(const char *file);

/* Get the field map given their name. */
HcfField *hcf_get_field(HcfOpts opts, const char *field);

/* Get the value of a given key of a field. */
char *hcf_get_value(HcfField field, const char *key);

/* Get a value given a field name and the options data structure */
char *hcf_get(HcfOpts opts, const char *field, const char *key);

/* Manually add a value */
void hcf_add(HcfField *field, const char *key, char *value);

/* destroy a yet created Options table */
void hcf_destroy(HcfOpts *opts);

#endif

#if defined(HCFP_IMPLEMENTATION)

#include <stddef.h>
#include <stdio.h> // debug print
#include <stdlib.h>
#include <string.h>

#define Malloc(...) malloc(__VA_ARGS__)
#define Calloc(...) calloc(__VA_ARGS__)

/* Initialize a hashmap of a given size */
void
__hashmap_new(__HashTable *table, size_t size)
{
        table->node_arr = (HashTableNode *) Calloc(size, sizeof(HashTableNode));
        table->size = size;
}

/* Add a value to a hashmap */
void
__hashmap_add(__HashTable *table, const char *key, void *value)
{
        HashTableNode *node;
        int index;

        index = __hashmap_key(*table, key);
        node = table->node_arr + index;

        while (node->next)
                node = node->next;

        node->key = strdup(key);
        node->value = value;
        node->next = (HashTableNode *) Calloc(1, sizeof(HashTableNode));
}

/* Remove a key-value pair from a hashmap */
void
__hashmap_pop(__HashTable *table, const char *key)
{
        /* TODO: this is a piece of shit */
        HashTableNode *node;
        HashTableNode *last;
        int index;

        index = __hashmap_key(*table, key);
        node = table->node_arr + index;
        last = table->node_arr + index;

        /* Get the node with key KEY */
        while (node->next && strcmp(node->key, key))
                node = node->next;

        /* Key is not in the list */
        if (!node)
                return;

        /* Get the last entry */
        while (last->next)
                last = last->next;

        free(node->key);
        node->key = last->key;
        node->value = last->value;

        while (node->next != last)
                node = node->next;

        node->next = NULL;

        free(last);
}

/* Get the value of a key in a hashmap or null if key not found*/
void *
__hashmap_get(__HashTable table, const char *key, void **value)
{
        HashTableNode *node;
        size_t index;

        if (!key || !table.size)
                return NULL;

        index = __hashmap_key(table, key);
        node = table.node_arr + index;

        while (node->next && strcmp(node->key, key))
                node = node->next;

        if (value)
                *value = node->value;

        return node->value;
}

/* Get the numeric key given a string key and a hash table */
size_t
__hashmap_key(__HashTable table, const char *key)
{
        unsigned sum = 0;

        if (!key || !table.size)
                return 0;

        for (int i = 0; key[i]; ++i)
                sum += i * key[i] * 7;

        return sum % table.size;
}

void
__hashmap_destroy(__HashTable *table)
{
        HashTableNode *node;
        HashTableNode *next;

        if (!table)
                return;

        for (size_t i = 0; i < table->size; i++) {
                node = table->node_arr + i;
                free(node->key);
                free(node->value);
                next = node->next;

                while ((node = next)) {
                        next = node->next;
                        free(node->key);
                        free(node->value);
                        free(node);
                }
        }

        free(table->node_arr);
        table->node_arr = NULL;
        table->size = 0;
}

inline HcfField *
hcf_get_field(HcfOpts opts, const char *field)
{
        return (HcfField *) __hashmap_get(opts, field, NULL);
}

inline char *
hcf_get_value(HcfField field, const char *key)
{
        return (char *) __hashmap_get(field, key, NULL);
}

char *
hcf_get(HcfOpts opts, const char *field, const char *key)
{
        __HashTable *table = (__HashTable *) __hashmap_get(opts, field, NULL);
        if (table)
                return (char *) __hashmap_get(*table, key, NULL);

        return NULL;
}

char *
hcf_get_default(HcfOpts opts, const char *field, const char *key, char *def)
{
        return (char *) hcf_get(opts, field, key) ?: def;
}

void
hcf_add(HcfField *field, const char *key, char *value)
{
        if (field)
                __hashmap_add(field, key, value);
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINEMAX 2048

HcfField *field_table = NULL;
char key[LINEMAX] = { '\0' };
char value[LINEMAX] = { '\0' };
char field[LINEMAX] = { '\0' };

char *
__strchrnul(char *str, char c)
{
        return strchr(str, c) ?: str + strlen(str);
}

int
__is_valid_c(char c)
{
        /* If c is space or some control char as \n or \0
         * it returns false */
        return c > ' ' && c != ':';
}

char *
__get_word(char *line, char *buffer)
{
        do {
                *buffer = *line;
                ++line;
                ++buffer;
        } while (__is_valid_c(line[-1]));

        buffer[-1] = '\0';
        return line - 1;
}

char *
__remove_spaces(char *str)
{
        while (*str == ' ' || *str == '\t') {
                ++str;
        }
        return str;
}

/* STR is a string starting at '/'. This function return a pointer
 * of the first char in STR that is not part of the escape character or NULL*/
char *
__parse_escape_sequences(char *str, char *c)
{
        /* This code sucks*/
        if (str == NULL) {
                return NULL;
        }
        if (str[1] == 'e' && str[2] == '[') {
                *c = '\033';
                return str + 2;
        }
        if (str[1] == '0' && str[2] == '3' && str[3] == '3' && str[4] == '[') {
                *c = '\033';
                return str + 4;
        }

        if (str[1] == 'n') {
                *c = '\n';
                return str + 2;
        }
        return str + 1;
}

void
__parse_line(HcfOpts *opts, char *line)
{
        char buffer[LINEMAX];
        char *current;
        char *temp;
        int len;
        char *c;
        char chr;

        /* Allow identation */
        line = __remove_spaces(line);

        if (!*line) {
                // puts("[EMTY]");
                return;
        }


        current = __get_word(line, buffer);

        // printf("[C] \'%c\'\n", *c);
        // printf("[BUF] \"%s\"\n", buffer);

        switch (*current) {
        case ':':
                /* If word last char is ':', it is a field name
                 * and the colon is removed from the fieldname.
                 * Then a field hashmap is created and it is added
                 * to the options hashmap. */
                strcpy(field, buffer);
                // printf("Adding field: %s\n", field);
                field_table = (HcfField *) Malloc(sizeof(HcfField));
                __hashmap_new(field_table, OPTS_N);
                __hashmap_add(opts, field, field_table);
                break;

        case '/':
                /* first character is invalid */
                // puts("[C=/] Comment line");
                break;

        default:

                if (current == buffer) {
                        /* first character is invalid */
                        // puts("[C=BUF] empty line");
                        break;
                }

                if (!field_table) {
                        // puts("[FTBLE] field table not created");
                        break;
                }

                strcpy(key, buffer);

                current = __remove_spaces(current);

                char *curr_start = current;
                c = current;

                while ((current = __parse_escape_sequences((c = strchr(c, '\\')), &chr))) {
                        if (*current) {
                                *c++ = chr;
                                memmove(c, current, strlen(current) + 1);
                                c = current - 1;
                        } else
                                ++c;
                }
                current = curr_start;

                /* Check for // (comment introducer).
                 * If a '\' was placed before the "//", it
                 * is analized as "//" and not as a comment. */
                if (*current == '"') {
                        ++current;
                        *__strchrnul(current, '"') = '\0';
                        goto __insert__;
                }

                temp = current;
                while (*temp && !strncmp((temp = __strchrnul(temp, '/')), "//", 2)) {
                        /* I think there would be allways a char in temp-1, so I
                         * can check it without underflow */
                        if (temp[-1] != '\\')
                                *temp = '\0';

                        /* If the comment introducer has a '\' before it, it
                         * get it as a text and remove the '\' char in temp string
                         * (overlaping it with the remainig string int temp */
                        else {
                                len = strlen(temp);
                                memmove(temp - 1, temp, len);
                                temp[len - 1] = '\0';
                        }
                        ++temp;
                }

        __insert__:
                strcpy(value, current);

                // printf("Adding entry [%s] (%s): (%s)\n", field, key, value);
                __hashmap_add(field_table, key, strdup(value));
        }
}

void
__parse(HcfOpts *opts, FILE *f)
{
        char line[LINEMAX + 1];
        line[LINEMAX] = 0;

        while (fgets(line, LINEMAX, f)) {
                *__strchrnul(line, '\n') = '\0';
                __parse_line(opts, line);
        }
}

HcfOpts
hcf_load(const char *file)
{
        FILE *f;
        HcfOpts opts;

        f = fopen(file, "r");

        if (!f) {
                perror(file);
                return (__HashTable) { 0 };
        }

        __hashmap_new(&opts, FIELDS_N);
        __parse(&opts, f);

        return opts;
}

void
hcf_destroy(HcfOpts *opts)
{
        HashTableNode *node;
        HashTableNode *next;

        for (size_t i = 0; i < opts->size; i++) {
                node = opts->node_arr + i;
                __hashmap_destroy((__HashTable *) node->value);
                free(node->key);
                free(node->value);
                next = node->next;

                while ((node = next)) {
                        next = node->next;
                        __hashmap_destroy((__HashTable *) node->value);
                        free(node->key);
                        free(node->value);
                        free(node);
                }
        }

        free(opts->node_arr);
        opts->node_arr = NULL;
        opts->size = 0;
}

#endif
