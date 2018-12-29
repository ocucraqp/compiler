#include "cross-referencer.h"

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
        fprintf(stderr, "File %s can not open.\n", np[1]);
        return EXIT_FAILURE;
    }

    /* Prefetch one token */
    token = scan(fp);

    /* Parse program */
    is_success = parse_program(fp);

    end_scan(fp);

    print_idtab();
    release_idtab();

    if (is_success == NORMAL) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

/* Display error message */
int error(char *mes, ...) {
    va_list args;
    char output[1024];

    va_start(args, mes);
    vsprintf(output, mes, args);
    va_end(args);

    fflush(stdout);
    fprintf(stderr, "\nline%d ERROR: %s\n", get_linenum(), output);
    return (ERROR);
}