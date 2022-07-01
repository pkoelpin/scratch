/*
spmatch - String Pattern Match
Good resource on a simple regex implementation in C 
    https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
*/
#include <stdbool.h>

bool spmatch(const char *pattern, const char *string) {
    char c;
    for (;;) {
        switch (c = *pattern++) {
        case '\0':
        case '?':
        case '*':
        default:

        }
    }

}

int main() {
    return 0;
}