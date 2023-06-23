#include "dynmap.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Creates a new dynamic map
dynmap dynmap_create() {
    dynmap new = malloc(sizeof(struct dynmap));
    assert (new != NULL);
    new->cap = DYNMAP_DEFAULT_LIMIT;
    new->entries = malloc(DYNMAP_DEFAULT_LIMIT * sizeof(struct map_entry));
    assert (new->entries != NULL);
    new->size = 0;
    return new;
}

// pre: when rebuild is being called, the size of map >= the cap of map
static void dynmap_rebuild(dynmap map) {
    map->cap += DYNMAP_DEFAULT_LIMIT;
    map->entries = realloc(map->entries, (map->cap) * sizeof(struct map_entry));
    assert (map->entries != NULL);
}

// frees all memory allocated to create the dynmap
void dynmap_free(dynmap map) {
    for (int i = 0; i < map->size; i++) {
        free(map->entries[i]->key);
        free(map->entries[i]);
    }
    free(map->entries);
    free(map);
}

// adds a key-value entry
// pre: key is not already in the map
void dynmap_add(dynmap map, char* key, uint8_t value) {
    if (map->size >= map->cap) {
        dynmap_rebuild(map);
    }
    map_entry new_entry = malloc(sizeof(struct map_entry));
    new_entry->key = strdup(key);
    new_entry->value = value;
    map->entries[map->size] = new_entry;
    (map->size)++;
}

// Retrieves a value from the map using the key
uint32_t dynmap_get(dynmap map, char* key) {
    for (int i = 0; i < map->size; i++) {
        if (strcmp(key, map->entries[i]->key) == 0) {
            return map->entries[i]->value;
        }
    }
    perror("Error: Key not found\n");
    exit(EXIT_FAILURE);
}