#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define LINE_LENGTH 80

struct buf {
    size_t max_size;
    int64_t len;
    char buf[];
};

int32_t buf_push(struct buf *b, char *s)
{
    int32_t off = b->len;
    int32_t len = strlen(s);
    if (b->len+len > BUF_MAX) {
        return -1;  // out of memory
    }
    memcpy(b->buf+off, s, len*sizeof(*s));
    b->len += len;
    return len<<22 | off;
}

int main() {
    static const char bdf_file[] = "./test/test1.dat";
    FILE* bdf;
    FILE* ofile;

    bdf = fopen(bdf_file, "r");
    ofile = fopen("./test/ofile.dat", "w");

    if (!bdf) {
        perror(bdf_file);
        exit(EXIT_FAILURE);
    }

    char line[LINE_LENGTH+1];
    while (fgets(line, sizeof(line), bdf)) {
        if ( strchr(line, '\n') == NULL) {
            int c;
            while ((c = fgetc(bdf)) != EOF && c != '\n'){}
        }
        /* trim the trailing newline */
        line[strcspn(line, "\n")] = 0;
        fprintf(ofile, "%-80s%d\n", line, strlen(line));
    }

    fclose(bdf);
    fclose(ofile);


    return 0;
}