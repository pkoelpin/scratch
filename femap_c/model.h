/** @file model.h
*
*/

#ifndef MODEL_H
#define MODEL_H

typedef struct model_t *h_model;

h_model model_create();
void model_destroy(h_model model);

#endif /* MODEL_H */

/*** end of file ***/