/** @file lcpick.h
*
*/
#ifndef LCPICK_H
#define LCPICK_H

#include <stdint.h>

typedef struct lcpick_t *h_lcpick;

h_lcpick lcpick_create(HWND hwnd);
HWND lcpick_hwnd(h_lcpick lcpick);
void lcpick_set_cond(h_lcpick lcpick, size_t count, uint32_t *lcid, char **desc);
size_t lcpick_active_count(h_lcpick lcpick);
void lcpick_get_active(h_lcpick lcpick, uint32_t *active);
void lcpick_destroy(h_lcpick lcpick);

#endif /* LCPICK_H */
/*** end of file ***/