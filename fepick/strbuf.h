#ifndef STRBUF_H
#define STRBUF_H

#define MEMBUF(buf, cap) {buf, cap, 0, 0}
struct strbuf {
    wchar_t *buf;
    int cap;
    int len;
    _Bool error;
};

void append(struct strbuf *b, wchar_t *src, int len)
{
    int avail = b->cap - b->len;
    int amount = avail<len ? avail : len;
    for (int i = 0; i < amount; i++) {
        b->buf[b->len+i] = src[i];
    }
    b->len += amount;
    b->error |= amount < len;
}

#define APPEND_STR(b, s) append(b, s, sizeof(s)/sizeof(wchar_t)-1)

void append_long(struct strbuf *b, long x)
{
    wchar_t tmp[64];
    wchar_t *end = tmp + sizeof(tmp)/sizeof(wchar_t);
    wchar_t *beg = end;
    long t = x>0 ? -x : x;
    do {
        *--beg = L'0' - t%10;
    } while (t /= 10);
    if (x < 0) {
        *--beg = L'-';
    }
    append(b, beg, end-beg);
}

#endif