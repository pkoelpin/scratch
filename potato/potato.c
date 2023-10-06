/* potato - reduce data to enveloping potato plot */
/* Written by Phil Koelpin */
#define _CRT_SECURE_NO_WARNINGS

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#define OPTPARSE_IMPLEMENTATION
#include "optparse.h"

static struct point *points;
struct pair *combo;
static int n_combo;
static size_t n_combo_allocated;
static unsigned char delimeter;
static bool merge;
static int sentinel;

#define FATAL_ERROR(Message)            \
    do                                  \
    {                                   \
        fprintf(stderr, Message);       \
        exit(EXIT_FAILURE);             \
    }                                   \
    while (0)

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

struct pair {
    uint64_t lo;
    uint64_t hi;
};

const char * getrange(uint64_t *lo, uint64_t *hi, const char *str) {
    uint64_t initial = 0;	
    uint64_t value = 0;
    bool lhs_specified = false;
    bool rhs_specified = false;
    bool dash_found = false;

    /* consume whitespace */
    while (isspace(*str))
        str++;

    for (;;) {
        if (*str == '-') {
            if (dash_found) {
                FATAL_ERROR("invalid range. Too many dashes.");
            }
            dash_found = true;
            initial = value;
            value = 0;
            str++;
        } else if (*str == ',' || isspace(*str) || *str == '\0' ) {
            if (dash_found) {
                dash_found = false;
                if (!lhs_specified || !rhs_specified)
                    FATAL_ERROR("invalid range.");
                if (value < initial)
                    FATAL_ERROR("decreasing range.");
                *lo = initial; 
                *hi = value;
            } else {
                *lo = value; 
                *hi = value; 
            }
            return *str == '\0' ? str : ++str;
        } else if (isdigit(*str)) {
            if (dash_found)
                rhs_specified = true;
            else
                lhs_specified = true;
            value = value*10 + *str - '0';
            str++;
        } else {
            FATAL_ERROR("invalid character in range.");
        }
    }
}

size_t xgetdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
    int c;
    char *line = *lineptr;
    size_t len = *n;
    size_t nread = 0;

    if (len < 2) {
        if (len)
            line[0] = 0;
        len = 256;
        if ((line = realloc(line, len)) == NULL)
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
            if ((line = realloc(line, len *= 2)) == NULL)
                return 0;
            *lineptr = line;
            *n = len;
        }

        c = fgetc(stream);
        if (c == EOF) {
            line[nread] = 0;
            return nread;
        }
        line[nread++] = (char)c;
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
    if (p == NULL) {
        return;
    }
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

void add_combo(struct pair p) {
    if (n_combo == n_combo_allocated) {
        n_combo_allocated *= 2;
        combo = realloc(combo, n_combo_allocated * sizeof (*combo));
    }

    for (int i = 0; i < n_combo; i++) {
        if (p.lo == combo[i].lo && p.hi == combo[i].hi)
            return;
    }

    combo[n_combo++] = p;
}

void set_combos(const char *str) {
    uint64_t lo = 0;
    uint64_t hi = 0;
    int n_range = 0;
    int n_range_allocated = 1;
    struct pair *range = malloc(n_range_allocated*sizeof(*range));

    while (*str != '\0') {
        str = getrange(&lo, &hi, str);
        
        /* add more space if we need it */
        if (n_range+1 == n_range_allocated) {
            n_range_allocated *= 2;
            range = realloc(range, n_range_allocated *sizeof(*range));
        }

        /* insert the range into the list of ranges */
        bool found = false;
        for (int i = 0; i < n_range; i++){
            if (lo <= range[i].lo) {
                memmove(range + i + 1, range + i, (n_range - i)*sizeof(*range));
                range[i] = (struct pair){lo, hi};
                found = true;
                break;
            }
        }
        if (!found) {
            range[n_range] = (struct pair){lo, hi};
        }
        n_range++;

        /* reduce any overlapping ranges */
        for (int i = 0; i < n_range; i++){   
            for (int j= i + 1; j < n_range; j++) {
                if (range[j].lo <= range[i].hi) {
                    range[i].hi = MAX(range[j].hi, range[i].hi);
                    memmove(range + j, range + j + 1, (n_range - j - 1)*sizeof *range);
                    n_range--;
                    j--;
                } else {
                    break;
                }
            }
        }
    }

    /* generate the combinations */
    for (int i = 0; i < n_range; i++) {
        for (uint64_t v1 = range[i].lo; v1 <= range[i].hi; v1++) {
            for (uint64_t v2 = v1 + 1; v2 <= range[i].hi; v2++)
                add_combo((struct pair){v1, v2});
            for (int j = i + 1; j < n_range; j++) {
                for (uint64_t v2 = range[j].lo; v2 <= range[j].hi; v2++)
                    add_combo((struct pair){v1, v2});
            }
        }
    } 
    free(range);
}

void potato_file(char const *file) {
    FILE *stream;

    if (strcmp(file, "-") == 0) {
        stream = stdin;
    } else {
        stream = fopen(file, "r");
        if (stream == NULL) {
            FATAL_ERROR("could not open file\n");
        }
    }

    potato_stream(stream);

    if (strcmp(file, "-") == 0) {
        clearerr(stream);
    } else {
        fclose(stream);
    }
}

int main(int argc, char **argv) {
    (void)argc;
    n_combo = 0;
    n_combo_allocated = 1;
    combo = malloc(n_combo_allocated*sizeof(*combo));
    delimeter = '\t';
    merge = false;

    /* argument parsing */
     struct optparse_long longopts[] = {
        {"delimeter", 'd', OPTPARSE_REQUIRED},
        {"fields", 'f', OPTPARSE_REQUIRED},
        {"merge", 'm', OPTPARSE_NONE},
        {"sentinel", 's', OPTPARSE_REQUIRED},
        {0}
    };
    int option;
    struct optparse options;
    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
        case 'd':
            delimeter = options.optarg[0];
            break;
        case 'f':
            set_combos(options.optarg);
            break;
        case 'm':
            merge = true;
            break;
        case 's':
            // sentinel = options.optarg;
            break;
        case '?':
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            exit(EXIT_FAILURE);
        }
    }

    /* the remaining arguments are all files */
    char *arg;
    while ((arg = optparse_arg(&options)) != '\0')
        printf("%s\n", arg);


    char *line = NULL;
    size_t n = 0;
    struct node *head = node_create(NULL);
    size_t len = 0;
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
            FATAL_ERROR("not enough variables in line\n");
        }
        if (isinf(p->f[0]) || isinf(p->f[1])){
            FATAL_ERROR("infinity in line\n");
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