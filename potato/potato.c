#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return n;
}

void node_delete(struct node *n) {
    n->next->prev = n->prev;
    n->prev->next = n->next;
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

    /* if we already have one point  */
    if (head->next == head) {
        node_insert_after(head, n);
        return (n->p->f[1] < head->p->f[1]) ? n : head;

    }

    /* if this node is lower it takes the place of the head node */
    if (n->p->f[1] < head->p->f[1]) {
        if (n->p->f[0] < head->p->f[0]) {
            node_insert_after(head->prev, n);
        } else {
            node_insert_after(head, n);
        }
        return n;
    }

    struct node *cur = head->next->next;
    while (cur != head) {
        struct node *prev = cur->prev;
        float c1 = cross(*head->p, *prev->p, *p);
        float c2 = cross(*head->p, *cur->p, *p);
        if (c1 < 0) {
            node_insert_after(head, n);
            return head;
        }
        if ((c1 > 0) && (c2 < 0)) {
            node_insert_after(prev, n);
            return head;
        }
        cur = cur->next;
    }
    node_insert_after(cur->prev, n);
    return head;
}

void reduce(struct node *head) {
    /* If we have 3 or fewer then return */
    if ((head->next == head) || (head->next->next == head)) {
        return;
    }

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
        sscanf(line, "%f\t%f", &p->f[0], &p->f[1]);

        head = insert(head, p);
        reduce(head);
    }

    node_print_all(head);

    return 0;
}