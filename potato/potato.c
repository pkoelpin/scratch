/* potato - reduce data to enveloping potato plot */
/* Written by Phil Koelpin */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "getopt.h"

#define PROGRAM_NAME "potato"

static struct point *points;

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
    double *f;
    char *line;
    int ref_count;
    struct point *prev;
    struct point *next;
};

void point_delete(struct point *p) {
    if (p->next != NULL) {
        p->next->prev = p->prev;
    }
    if (p->prev != NULL)
    {
        p->prev->next = p->next;
    }
    if (p == points) {
        points = p->next;
    }
    free(p->f);
    free(p->line);
    free(p);
}

#define PRINT_POINT(p) printf("%d\t%0.6lf\t%0.6lf\n", p->idx, p->f[0], p->f[1]);

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
            point_delete(n->p);
        }
    }
    free(n);
}

struct node* node_insert_after(struct node *this, struct node *new) {
    struct node *next = this->next;
    if (this != NULL) {
        this->next = new;
    }
    if (next != NULL) {
        next->prev = new;
    }
    new->prev = this;
    new->next = next;
    return new;
}

void node_print_all(struct node *head) {
    struct node *n = head;
    do {
        PRINT_POINT(n->p);
        n = n->next;
    } while (n != head);
}

double cross(struct point p0, struct point p1, struct point p2) {
    double dx1 = p1.f[0] - p0.f[0];
    double dy1 = p1.f[1] - p0.f[1];
    double dx2 = p2.f[0] - p0.f[0];
    double dy2 = p2.f[1] - p0.f[1];
    return dx1*dy2 - dx2*dy1;
}

void node_check(struct node *head) {
    struct node *cur = head;
    do {
        double x = cross(*cur->p, *cur->next->p, *cur->next->next->p);
        assert(x > 0);
        cur = cur->next;
    } while (cur != head);
}

double distance(struct point p1, struct point p2) {
    double dx = p2.f[0] - p1.f[0];
    double dy = p2.f[1] - p1.f[1];
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

    /* go around counter clockwise looking for transition points */   
    struct node *beg = NULL;
    struct node *end = NULL;
    struct node *cur = head;
    double x1 = cross(*cur->prev->p, *cur->p, *p);
    do  {
        if (distance(*cur->p, *p) == 0.0) {
            struct node *n = node_create(p);
            node_insert_after(cur, n);
            node_delete(cur);
            return n;
        }
        double x2 = cross(*cur->p, *cur->next->p, *p);

        if (x1 > 0.0 && x2 <= 0.0) {
            beg = cur;
        } else if (x1 <= 0.0 && x2 > 0.0) {
            end = cur;           
        }
        cur = cur->next;
        x1 = x2;
    } while (cur != head);

    /* if we didn't find anything then the node is in the interior */
    if (beg == NULL) {
        point_delete(p);
        return head;
    }

    /* There is no way that we should get to this point and see a NULL end */
    assert(end != NULL);

    /* delete from beginning to end */
    while (beg->next != end) {
         node_delete(beg->next);
    };
    
    /* do one last check to see if the final node is in line with the beg & end */
    if (cross(*beg->p, *end->p, *p) == 0.0) {
        point_delete(p);
        return head;
    } else {
        return node_insert_after(beg, node_create(p));
    }
}

bool potato_file(char const *file) {
    

}

int main(int argc, char **argv) {
   
    bool have_read_stdin = false;

    /* argument parsing */
    int optc = 0;
    optind = 1;
    while ((optc = getopt(argc, argv, "d:f:h")) != -1) {
        printf("%d\n", optind);
        printf("%c\n", optopt);
        printf("%s\n", optarg);
        printf("---\n");
    }


    for (;optind < argc; optind++) {
        printf("%s\n", argv[optind]);
    }
    
   

   
    char *line = NULL;
    size_t n = 0;
    struct node *head = node_create(NULL);
    int len = 0;
    int index = 1;
    points = NULL;
    while ((len = xgetline(&line, &n, stdin)) > 0) {
        struct point *p = malloc(sizeof(struct point));
        p->f = malloc(2 * sizeof(double));
        p->line = malloc(len * sizeof(char));
        memcpy(p->line, line, len);
        p->idx = index++;
        p->ref_count = 0;
        int count = sscanf(line, "%lf,%lf", &p->f[0], &p->f[1]);
        if (count < 2) {
            fprintf(stderr, "not enough variables in line\n");
            return 1;
        }
        if (isinf(p->f[0]) || isinf(p->f[1])){
            fprintf(stderr, "infinity in line\n");
            return 1;
        }
        if (points == NULL) {
            points = p;
            p->prev = p;
            p->next = p;
        } else {
            p->next = points;
            p->prev = points->prev;
            points->prev->next = p;
            points->prev = p;
        }
        head = insert(head, p);
    }

    /* Do a sanity check to make sure this is actually a convex hull*/
    node_check(head);

    /* print all points */
    struct point *cur = points;
    do {
        PRINT_POINT(cur);
        cur = cur->next;
    } while (cur != points);

    /* free nodes */
    struct node *node = head->prev;
    do {
        struct node *tmp = node;
        node = node->prev;
        node_delete(tmp);
    } while (node != head);
    node_delete(head);

    // Clean up the line
    free(line);

    return 0;
}