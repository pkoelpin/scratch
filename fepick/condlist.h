#pragma once

typedef struct condlist condlist;

condlist *condlist_create(int *id, const char * const *title, int n, bool isactive);
void condlist_free(condlist *list);
void condlist_update(condlist *list, const char *filter);
int condlist_len(condlist *list);
int condlist_count(condlist *list);
void condlist_set_id(condlist *list, int *id);
int condlist_get_id(condlist *list, int index);
void condlist_flip(condlist *list, int index);
const char * condlist_get_title(condlist *list, int index);
void condlist_sort_clear(condlist *list);
void condlist_sort_id_ascending(condlist *list);
void condlist_sort_id_descending(condlist *list);
void condlist_sort_title_ascending(condlist *list);
void condlist_sort_title_descending(condlist *list);
