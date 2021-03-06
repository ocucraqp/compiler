#include "compiler.h"

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

/* pointer to I/O File */
FILE *inputfp, *outputfp;

int main(int nc, char *np[]) {
    int is_success = 0;

    /* End without argument */
    if (nc < 2) {
        fprintf(stderr, "File name id not given.\n");
        return EXIT_FAILURE;
    }

    /* End if input file can not be opened */
    if (init_scan(np[1]) < 0) {
        return EXIT_FAILURE;
    }

    /* End if output file can not be opened */
    if (init_outputfile(np[1]) < 0) {
        return EXIT_FAILURE;
    }

    /* Prefetch one token */
    token = scan();

    /* Parse program */
    is_success = parse_program();

    /* Add labels and data such as character strings */
    output_label_buf();

    /* Add necessary processing */
    output_pl();

    /* end scan */
    end_scan();

//    /* Display cross reference table */
//    print_idtab();

    /* Release id table */
    release_idtab();

    /* end output */
    end_output();

    if (is_success == NORMAL) {
        printf("Compilation succeeded.\n");
        return EXIT_SUCCESS;
    } else {
        printf("Compilation failed.\n");
        return EXIT_FAILURE;
    }
}

/* Display error message
 * The arguments are the same as for printf () */
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