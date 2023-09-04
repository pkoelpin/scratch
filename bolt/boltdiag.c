#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define FDBUF(hfile, buf, cap) {buf, cap, 0, hfile, 0}
#define APPEND_STR(b, s) append(b, s, sizeof(s)-1)

struct buf {
    unsigned char *buf;
    int cap;
    int len;
    HANDLE hfile;
    _Bool error;
};

_Bool os_write(HANDLE hfile, void *b, int len) {
    WriteFile(hfile, b, len, (DWORD[]){0}, NULL);
}

void flush(struct buf *b){
    if (!b->error && b->len) {
        b->error |= !os_write(b->hfile, b->buf, b->len);
        b->len = 0;
    }
}

void append(struct buf *b, unsigned char *src, int len){
    unsigned char *end = src + len;
    while (!b->error && src<end) {
        int left = end - src;
        int avail = b->cap - b->len;
        int amount = avail<left ? avail : left;

        for (int i = 0; i < amount; i++) {
            b->buf[b->len+i] = src[i];
        }
        b->len += amount;
        src += amount;

        if (amount < left) {
            flush(b);
        }
    }
}

int main(void) {
    unsigned char mem[1<<10];  // arbitrarily-chosen 1kB buffer
    struct buf output = FDBUF( GetStdHandle(STD_OUTPUT_HANDLE), mem, sizeof(mem));
    APPEND_STR(&output, "Hello World!");
    flush(&output);

}