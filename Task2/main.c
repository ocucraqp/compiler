#include "pretty-print.h"

/* keyword list */
struct KEY key[NUMOFKEYWORD] = {
        {"and",       TAND},
        {"array",     TARRAY},
        {"begin",     TBEGIN},
        {"boolean",   TBOOLEAN},
        {"break",     TBREAK},
        {"call",      TCALL},
        {"char",      TCHAR},
        {"div",       TDIV},
        {"do",        TDO},
        {"else",      TELSE},
        {"end",       TEND},
        {"false",     TFALSE},
        {"if",        TIF},
        {"integer",   TINTEGER},
        {"not",       TNOT},
        {"of",        TOF},
        {"or",        TOR},
        {"procedure", TPROCEDURE},
        {"program",   TPROGRAM},
        {"read",      TREAD},
        {"readln",    TREADLN},
        {"return",    TRETURN},
        {"then",      TTHEN},
        {"true",      TTRUE},
        {"var",       TVAR},
        {"while",     TWHILE},
        {"write",     TWRITE},
        {"writeln",   TWRITELN}
};

int main(int nc, char *np[]) {
    FILE *fp;
    int is_success = 0;

    /* End without argument */
    if (nc < 2) {
        fprintf(stderr, "File name id not given.\n");
        return EXIT_FAILURE;
    }

    /* End if file can not be opened */
    if (init_scan(np[1], &fp) < 0) {
        fprintf(stderr, "File %s can not open.", np[1]);
        return EXIT_FAILURE;
    }

    /* Prefetch one token */
    token = scan(fp);

    /* Parse program */
    is_success = parse_program(fp);

    end_scan(fp);

#ifdef DEBUG
    if (token == -1) {
        printf("finished\n");
    } else {
        printf("didn't finished program\n");
    }
#endif

    if (is_success == 1) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

/* Display error message */
int error(char *mes) {
    fprintf(stderr, "\nline%d ERROR: %s\n", get_linenum(), mes);
    return (ERROR);
}