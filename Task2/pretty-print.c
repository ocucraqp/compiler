#include "pretty-print.h"

int token = 0;

/* */
int parse_program(FILE *fp) {
    if (token != TPROGRAM) { return (error("Keyword 'program' is not found")); }
    token = scan(fp);
    if (token != TNAME) { return (error("Program name is not fount")); }
    token = scan(fp);
    if (token != TSEMI) { return (error("Semicolon is not found")); }
    token = scan(fp);
    if (parse_block(fp) == ERROR) { return ERROR; }
    if (token != TDOT) { return (error("Period is not found at the end of program")); }
    token = scan(fp);

    return NORMAL;
}

int parse_block(FILE *fp) {
    // todo
    return NORMAL;
}