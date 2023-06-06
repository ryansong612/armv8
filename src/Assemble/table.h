#include <stdint.h>
#include <stdbool.h>
#include "dynarray.h"

#ifndef ARMV8_32_TABLE_H
#define ARMV8_32_TABLE_H

struct table_entry{
    char* key;
    uint32_t value;
};

typedef struct table_entry *table_entry;

struct table {
    dynarray *entries;
};

typedef struct table *table;

table table_create(void);
bool table_add(table t, table_entry entry);
uint32_t table_find(char* key);
bool table_remove(char* key);

#endif //ARMV8_32_TABLE_H
