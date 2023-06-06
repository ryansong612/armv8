#include "table.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

table table_create(void) {
    table t = malloc(sizeof(struct table));
    assert(t != NULL);
    t->entries = dynarray_create();
}
bool table_add(table_entry entry);
uint32_t table_find(char* key);
bool table_remove(char* key);
