#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifdef DEBUG
# define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#else
# define DEBUG_PRINT(...) do {} while (0)
#endif

size_t xgetdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
    int c;
    char *line = *lineptr;
    size_t len = *n;
    size_t nread = 0;

    if (len < 2) {
        if (len)
            line[0] = 0;
        len = 256;
        if (!(line = realloc(line, len)))
            return 0;
        *lineptr = line;
        *n = len;
    }

    for (;;) {
        if (nread == len - 2) {
            /* Need space for at least two characters before fgetc(). */
            line[nread] = 0;
            if (len * 2 < len)
                return 0;
            if (!(line = realloc(line, len *= 2)))
                return 0;
            *lineptr = line;
            *n = len;
        }

        c = fgetc(stream);
        if (c == EOF) {
            line[nread] = 0;
            return nread;
        }
        line[nread++] = c;
        if (c == delim) {
            line[nread] = 0;
            return nread;
        }
    }
}

size_t xgetline(char **lineptr, size_t *n, FILE *stream) {
    return xgetdelim(lineptr, n, '\n', stream);
}

struct point {
    int idx;
    float *f;
    char *line;
    int ref_count;
};

#define PRINT_POINT(p) printf("%d\t%0.3f\t%0.3f\n", p->idx, p->f[0], p->f[1]);

struct node {
    struct point *p;
    struct node *prev;
    struct node *next;
};

struct node* node_create(struct point *p){
    struct node* n = (struct node*)malloc(sizeof(struct node));
    n->p = p;
    n->prev = NULL;
    n->next = NULL;
    if (p != NULL) {
        p->ref_count += 1;
    }
    return n;
}

void node_delete(struct node *n) {
    if (n->next != NULL) {
        n->next->prev = n->prev;
    }
    if (n->prev != NULL)
    {
        n->prev->next = n->next;
    }

    /* decrement the point ref count */
    if (n->p != NULL) {
        n->p->ref_count -= 1;
        if (n->p->ref_count < 1) {
            free(n->p);
        }
    }

    free(n);
}

void node_insert_after(struct node *this, struct node *new) {
    struct node *next = this->next;
    if (this != NULL) {
        this->next = new;
    }
    if (next != NULL) {
        next->prev = new;
    }
    new->prev = this;
    new->next = next;
}

void node_print_all(struct node *n) {
    printf("-----\n");
    struct node *head = n;
    for (;;) {
        PRINT_POINT(n->p);
        n = n->next;
        if (n == head) break;
    }
    printf("-----\n");
}

float cross(struct point p0, struct point p1, struct point p2) {
    float dx1 = p1.f[0] - p0.f[0];
    float dy1 = p1.f[1] - p0.f[1];
    float dx2 = p2.f[0] - p0.f[0];
    float dy2 = p2.f[1] - p0.f[1];
    return dx1*dy2 - dx2*dy1;
}

void node_check(struct node *head) {
    struct node *cur = head;
    do {
        float x = cross(*cur->p, *cur->next->p, *cur->next->next->p);
        assert(x > 0);
        cur = cur->next;
    } while (cur->next == head);
}

float distance(struct point p1, struct point p2) {
    float dx = p2.f[0] - p1.f[0];
    float dy = p2.f[1] - p1.f[1];
    return dx*dx + dy*dy;
}

struct node* insert(struct node *head, struct point *p) {
    /* Adding first node */
    if (head->p == NULL) {
        head->p = p;
        head->next = head;
        head->prev = head;
        return head;
    }

    /* Adding second node */
    if (head->next == head) {
        struct node *n = node_create(p);
        if (distance(*head->p, *p) == 0.0) {
            node_insert_after(head, n);
            node_delete(head);
            return n;
        } else {
            node_insert_after(head, n);
            return head;
        }
    }

    /* Adding third node */
    if (head->next->next == head) {
        struct node *n = node_create(p);
        /* first check to see if we are on top of another node */
        if (distance(*head->p, *p) == 0.0) {
            node_insert_after(head, n);
            node_delete(head);
            return n;
        } 
        if (distance(*head->next->p, *p) == 0.0) {
            node_insert_after(head->next, n);
            node_delete(head->next);
            return head;
        } 

        /* now check to see where we should place the new node */
        float x = cross(*head->p, *head->next->p, *p);
        if (x > 0){
            node_insert_after(head->next, n);
        } else if (x < 0) {
            node_insert_after(head, n);
        } else {
            float d1 = distance(*head->p, *head->next->p);
            float d2 = distance(*head->p, *p);
            float d3 = distance(*head->next->p, *p);
            if ((d1 > d2) && (d1 > d3)) {
                node_delete(n);
            } else if ((d2 > d1) && (d2 > d3)) {
                node_insert_after(head->next, n);
                node_delete(head->next);
            } else {
                node_insert_after(head, n);
                node_delete(head);
                return n;
            }
        }
        return head;
    }

    /* go around counter clockwise looking for a non-left turn */   
    struct node *beg = NULL;
    struct node *end = NULL;
    struct node *cur = head;
    do  {
        float x = cross(*cur->p, *cur->next->p, *p);
        if (x <= 0.0) {
            beg = cur;
            break;
        }
        cur = cur->next;
    } while (cur != head);


    /* if we didn't find anything then the node is in the interior */
    if (beg == NULL) {
        return head;
    }

    /* now go back around clockwise looking for non-right turn */
    cur = head;
    do  {
        float x = cross(*cur->p, *cur->prev->p, *p);
        if (x >= 0.0) {
            end = cur;
            break;
        }
        cur = cur->prev;
    } while (cur != head);
    assert (end != NULL);

        if (p->idx == 3213) {
            PRINT_POINT(p);
            PRINT_POINT(beg->p);
            PRINT_POINT(end->p);
            node_print_all(head);
            exit(1);
         }

    if (beg == end) {
        struct node *n = node_create(p);
        node_insert_after(beg, n);
        node_delete(beg);
        return n;
    }


        // if (p->idx == 3213) {
        //     PRINT_POINT(p);
        //     PRINT_POINT(beg->p);
        //     PRINT_POINT(end->p);
        //     node_print_all(cur);
        //     exit(1);
        //  }

    /* delete from beginning to end */
    cur = beg;
    while (cur->next != end) {
         node_delete(cur->next);
    };
    node_insert_after(cur, node_create(p));
    



    return cur;
}

int main(int argc, char **argv) {
    DEBUG_PRINT("START\n");

    char *line = NULL;
    size_t n = 0;
    struct node *head = node_create(NULL);

    int len = 0;
    int index = 1;
    while ((len = xgetline(&line, &n, stdin)) > 0) {
        struct point *p = malloc(sizeof(struct point));
        p->f = malloc(2 * sizeof(float));
        p->line = malloc(len * sizeof(char));
        p->idx = index++;
        p->ref_count = 0;
        memcpy(p->line, line, len);

        DEBUG_PRINT("Reading\n");
        int count = sscanf(line, "%f,%f", &p->f[0], &p->f[1]);
        if (count < 2) {
            printf("not enough variables in line\n");
            return 1;
        }
        if (isinf(p->f[0]) || isinf(p->f[1])){
            fprintf(stderr, "infinity in line\n");
            return 1;
        }
        DEBUG_PRINT("insert\n");
        head = insert(head, p);
        DEBUG_PRINT("reduce\n");
        // reduce(head);
        DEBUG_PRINT("COMPLETE\n");
    }

    node_print_all(head);
        node_check(head);


    return 0;
}