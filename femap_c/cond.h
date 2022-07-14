/** @file cond.h
*
*/
#ifndef COND_H
#define COND_H

#include <stdbool.h>
#include <stdint.h>

typedef struct 
{
    uint32_t id;
    bool     is_active;
    char     label[128];
} cond_t;

typedef struct
{
    uint32_t count;
    cond_t*  list;
    uint32_t count_sorted;
    cond_t*  list_sorted;
    uint32_t count_filtered;
    cond_t*  list_filtered;
} condlist_t;

condlist_t condlist_create(size_t count, uint32_t *id, char **label);
void condlist_free(condlist_t *condlist);
void condlist_sort_id(condlist_t *condlist);
void condlist_sort_label(condlist_t *condlist);
void condlist_filter(condlist_t *condlist, char *str);

#endif /* COND_H */
/*** end of file ***/