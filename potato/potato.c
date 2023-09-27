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
    if (n->prev != NULL )
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
    struct node *head = n;
    for (;;) {
        PRINT_POINT(n->p);
        n = n->next;
        if (n == head) break;
    }
}

float cross(struct point p0, struct point p1, struct point p2){
    float dx1 = p1.f[0] - p0.f[0];
    float dy1 = p1.f[1] - p0.f[1];
    float dx2 = p2.f[0] - p0.f[0];
    float dy2 = p2.f[1] - p0.f[1];
    return dx1*dy2 - dx2*dy1;
}

float distance(struct point p1, struct point p2) {
    float dx = p2.f[0] - p1.f[0];
    float dy = p2.f[1] - p1.f[1];
    return dx*dx + dy*dy;
}

struct node* insert(struct node *head, struct point *p) {
    /* special case if we don't have any points defined */
    if (head->p == NULL) {
        head->p = p;
        head->next = head;
        head->prev = head;
        return head;
    }

    /* If there is already a point in the first node, create a new node to insert */
    struct node *n = node_create(p);

    /* Adding second node */
    if (head->next == head) {
        /* by deafult put it after the head node*/
        node_insert_after(head, n);
        float x1 = head->p->f[0];
        float y1 = head->p->f[1];
        float x2 = n->p->f[0];
        float y2 = n->p->f[1];

        /* then check to see which one should be the head */
        if ((x1 == x2) && (y1 == y2)) {
            node_delete(head);
            return n;
        } else if (y1 < y2) {
            return head;
        } else if (y1 > y2) {
            return n;
        } else if ((y1 == y2) && (x1 < x2)) {
            return head;
        } else {
            return n;
        }
    }

    /* check to see if the new node is lower and put it in */
    float x1 = head->p->f[0];
    float y1 = head->p->f[1];
    float x2 = n->p->f[0];
    float y2 = n->p->f[1];
    if (y2 <= y1) {
        if ((x1 == x2) && (y1 == y2)) {
            node_insert_after(head, n);
            node_delete(head);
            return n;
        } else if ((y1 == y2) && (x1 < x2)) {
            // do nothing
        } else {
            float x = cross(*n->p, *head->p, *head->next->p);
            if (x > 0) {
                node_insert_after(head->prev, n);
            } else if (x < 0) {
                node_insert_after(head, n);
            } else {
                node_insert_after(head, n);
                node_delete(head);
            }
            return n;
        }
    }

    /* for two nodes already defined */
    if (head->next->next == head) {
        struct node *a = head;
        struct node *b = head->next;
        struct node *c = n;
        float x = cross(*a->p, *b->p, *c->p);
        // printf("%f\n", x);
        if (x > 0){
            node_insert_after(head->next, n);
        } else if (x < 0) {
            node_insert_after(head, n);
        } else {
            float d1 = distance(*head->p, *head->next->p);
            float d2 = distance(*head->p, *p);
            // printf("%f, %f\n", d1, d2);
            if (d1 > d2) {
                node_delete(n);
            } else {
                node_insert_after(head->next, n);
                node_delete(head->next);
            }
        }
        return head;
    }

    struct node *cur = head->next->next;
    while (cur != head) {
        struct node *prev = cur->prev;
        float c1 = cross(*head->p, *prev->p, *p);
        float c2 = cross(*head->p, *cur->p, *p);
        //printf("%f. %f\n", c1, c2);
        if (c1 == 0.0 ) {
            float d1 = distance(*head->p, *prev->p);
            float d2 = distance(*head->p, *p);
             if (d1 > d2) {
                node_delete(n);
            } else {
                node_insert_after(prev, n);
                node_delete(prev);
            }
            return head;
        } else if (c1 < 0) {
            node_insert_after(head, n);
            return head;
        } else if ((c1 > 0) && (c2 < 0)) {
            node_insert_after(prev, n);
            return head;
        }
        cur = cur->next;
    }
    node_insert_after(cur->prev, n);
    return head;
}

void reduce(struct node *head) {
    struct node *c = head->next;
    while (c != head) {
        struct node *a = c->prev->prev;
        struct node *b = c->prev;
        float x = cross(*a->p, *b->p, *c->p);
        if (x < 0) {
            node_delete(b);
        }
        c = c->next;
    }
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
        reduce(head);
        DEBUG_PRINT("COMPLETE\n");
    }

    node_print_all(head);

    return 0;
}