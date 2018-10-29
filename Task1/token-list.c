#include "token-list.h"

/* keyword list */
struct KEY key[KEYWORDSIZE] = {
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
    int token, i;
    FILE *fp;

    /* 引数がなければ終了 */
    if (nc < 2) {
        printf("File name id not given.\n");
        return EXIT_FAILURE;
    }

    /* ファイルが開けなければ終了 */
    if (init_scan(np[1], &fp) < 0) {
        printf("File %s can not open.\n", np[1]);
        return EXIT_FAILURE;
    }

    /* トークンカウント用の配列を初期化する */
    init_array(numtoken, NUMOFTOKEN + 1);

    /* トークンをカウントする */
    while ((token = scan(fp)) >= 0) {
        /* todo：トークンをカウントする */
    }

    /* スキャン終了 */
    end_scan(fp);

    /* todo:カウントした結果を出力する */

    return EXIT_SUCCESS;
}

/* errorの表示とスキャン終了処理 */
void error(char *mes, FILE *fp) {
    printf("\n ERROR: %s\n", mes);
    end_scan(fp);
}