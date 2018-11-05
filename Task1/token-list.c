#include "token-list.h"

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

/* Token counter */
int numtoken[NUMOFTOKEN + 1];

/* string of each token */
char *tokenstr[NUMOFTOKEN + 1] = {
        "",
        "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
        "else", "procedure", "return", "call", "while", "do", "not", "or",
        "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
        "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
        ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read", "write", "break"
};

int main(int nc, char *np[]) {
    int token = 0, i = 0;
    FILE *fp;
    char *temp;

    /* 引数がなければ終了 */
    if (nc < 2) {
        error("File name id not given.");
        return EXIT_FAILURE;
    }

    /* ファイルが開けなければ終了 */
    if (init_scan(np[1], &fp) < 0) {
        sprintf(temp, "File %s can not open.\n", np[1]);
        error(temp);
        return EXIT_FAILURE;
    }

    /* トークンカウント用の配列と識別子カウント用の構造体を初期化する */
    init_int_array(numtoken, NUMOFTOKEN + 1);
    init_idtab();

    /* トークンをカウントする */
    while ((token = scan(fp)) >= 0) {
        numtoken[token]++;
        if (token == TNAME) {
            /* 名前のトークンだった場合は、識別子もカウント */
            id_countup(string_attr);
        }
    }

    /* スキャン終了 */
    end_scan(fp);

    /* カウントした結果を出力する */
    for (i = 1; i < NUMOFTOKEN + 1; i++) {
        if (numtoken[i] > 0) {
            printf("\t\"%s\" \t%d\n", tokenstr[i], numtoken[i]);
        }
    }

    /* カウントした識別子を出力する */
    print_idtab();
    release_idtab();

    return EXIT_SUCCESS;
}

/* errorの表示とスキャン終了処理 */
void error(char *mes) {
    fprintf(stderr, "\nline%d ERROR: %s\n", get_linenum(), mes);
}