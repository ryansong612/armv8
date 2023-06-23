#include <stdint.h>
#include <stdbool.h>

#ifndef ARMV8_32_DYNMAP_H
#define ARMV8_32_DYNMAP_H

#define DYNMAP_DEFAULT_LIMIT 30

struct map_entry {
    char* key;
    uint32_t value;
};

typedef struct map_entry *map_entry;

struct dynmap {
    map_entry *entries;
    int cap;
    int size;
};

typedef struct dynmap *dynmap;

dynmap dynmap_create(void);
void dynmap_add(dynmap map, char* key, uint8_t value);
uint32_t dynmap_get(dynmap map, char* key);

#endif //ARMV8_32_DYNMAP_H
