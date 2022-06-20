#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define LINE_LENGTH 80

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

typedef struct {
    enum {
        STATE_FIND_DATA,
        STATE_IN_CARD,
        STATE_IN_COMMENT
    } state;
    char token[8];
    char *file;
    uint32_t line;
    uint8_t scol;
    uint8_t ecol;
    uint8_t field;
} State;

void fsm(State *state, char c);

void bdftok(FILE *stream, Token *tok, size_t len);

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
    printf("%d\n", sizeof(size_t));

    return 0;
}