#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define LINE_LENGTH 80
#define BUFFSIZE 4096
#define STACKSIZE 10

typedef enum {
    TOK_CARD,
    TOK_INTEGER,
    TOK_REAL,
    TOK_CHARACTER,
    TOK_PATH,
    TOK_BLANK,
    TOK_SCONT,
    TOK_ECONT,
    TOK_EOF
} TokenType;

typedef union {
    uint64_t i;
    char s[8];
} CardID;

typedef struct {
    TokenType type;
    union {
        CardID id;
        uint64_t i;
        double r;
        char c[8];
        char *p;
    } val;
    char *file;
    uint32_t line;
    uint8_t scol;
    uint8_t ecol;
} Token;

typedef enum {
    STATE_FIND_DATA,
    STATE_IN_CARD,
    STATE_IN_COMMENT
} State;

typedef struct {
    FILE *stream;
    char buf1[BUFFSIZE+1];
    char buf2[BUFFSIZE+1];
    uint32_t line;
    uint32_t col;
    char *beg;
    char *end;
} BDF;

typedef struct {
    size_t n;
    BDF stack[STACKSIZE];
    State state;
} Tokenizer;

void tok_init(Tokenizer *tok, FILE *stream){
    tok->n = 1;
    tok->stack[0] 
}

//void fsm(State *state, char c);

void tokenize(FILE *stream, Token *tok, size_t len);

int main() {
    
    static const char bdf_file[] = "./test/test1.dat";
    FILE* bdf;
    bdf = fopen(bdf_file, "r");

    fclose(bdf);

    CardID tmp;
    //tmp.i = 0x0000344441555143;
    tmp.i = 0;
    strncpy(tmp.s, "C", 1);

    printf("%.8s\n", tmp.s);
    printf("0x%016" PRIx64 "\n", tmp.i);
    printf("%d\n", sizeof(BDF));

    return 0;
}