#include "token-list.h"

char string_attr[MAXSTRSIZE];

/* スキャン前に呼び出し、ファイルをオープンし、スキャンの準備 */
int init_scan(char *filename, FILE **fp) {
    /* ファイルポインタはポインタのポインタとして受け取り、値を返す */
    *fp = fopen(filename, "r");

    if (*fp == NULL) {
        // 失敗と表示し終了
        printf("ERROR: fclose()\n");
        return -1;
    }

    return 0;

}

/* トークンのコードを返す。
 * 次のトークンをスキャンできないときは、-1を返す。 */
int scan(FILE *fp) {
    char strbuf[MAXSTRSIZE];
    int i = 0, sep_type = 0;

    init_char_array(strbuf, MAXSTRSIZE);

    /* cbufに記号が入ってないか調べてなければ、他種類の字句の解析
     * EOFが入っていれば-1を返し終了*/
    if (cbuf == EOF) {
        return -1;
    } else if (is_check_symbol(cbuf)) {
        return identify_symbol(cbuf, fp);
    } else {
        /* 字句の1文字目を読み込む
         * cbufに英字又は数字が入っていればそれを1文字目に
         * 英字も数字も入ってなければ新たに読み込む */
        if (is_check_number(cbuf) || is_check_alphabet(cbuf)) {
            strbuf[0] = cbuf;
        } else {
            while ((cbuf = (char) fgetc(fp)) != EOF) {
                sep_type = is_check_separator(cbuf, fp);
                if (sep_type == 0) {
                    strbuf[0] = cbuf;
                    break;
                } else if (sep_type >= 2) {
                    if (skip_comment(fp, sep_type) == 0) {
                        error("failed to end comment.", fp);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        // todo string
        // 分離子もしくは記号まで読み込む
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            sep_type = is_check_separator(cbuf, fp);
            if (sep_type == 0) {
                if (is_check_symbol(cbuf)) {
                    break;
                } else {
                    strbuf[i] = cbuf;
                }
            } else if (sep_type >= 2) {
                if (skip_comment(fp, sep_type) == 0) {
                    error("failed to end comment.", fp);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    return identify_token(strbuf, fp);
}

/* int型配列を0で初期化する */
void init_int_array(int *array, int arraylength) {
    int i = 0;
    for (i = 0; i < arraylength; i++) {
        array[i] = 0;
    }
}

/* char型配列を\0で初期化する */
void init_char_array(char *array, int arraylength) {
    int i = 0;
    for (i = 0; i < arraylength; i++) {
        array[i] = '\0';
    }
}

/* 文字がアルファベットか確認する */
int is_check_alphabet(char c) {
    if ((c >= 0x41 && c <= 0x5a) || (c >= 0x61 && c <= 0x7a)) {
        return 1;
    }
    return 0;
}

/* 文字が数字か確認する */
int is_check_number(char c) {
    if (c >= 0x30 && c <= 0x39) {
        return 1;
    }
    return 0;
}

/* 文字が記号か確認する */
int is_check_symbol(char c) {
    if ((c >= 0x28 && c <= 0x2e) || (c >= 0x3a && c <= 0x3e) ||
        c == 0x5b || c == 0x5d) {
        return 1;
    }
    return 0;
}

/* 引数が
 * 空白、タブ、改行のとき1
 * "{"から始まる注釈のとき2
 * "/" "*"から始まる注釈のとき3
 * それ以外のとき0
 * を返す */
int is_check_separator(char c, FILE *fp) {
    switch (c) {
        case 0x09: // 水平タブ
        case 0x0a: // 改行
        case 0x0b: // 垂直タブ
        case 0x0c: // 改頁
        case 0x0d: // 復帰
        case 0x20: // 空白文字
            return 1;
        case '{':
            return 2;
        case '/':
            if ((cbuf = (char) fgetc(fp)) == '*') {
                return 3;
            }
        default:
            return 0;
    }
}

/* トークンを識別する */
int identify_token(const char *tokenstr, FILE *fp) {
    int token = 0;

    if (is_check_alphabet(tokenstr[0])) {
        // todo アルファベットから始まるとき
        if ((token = identify_keyword(tokenstr)) > 0) {
            return token;
        } else {
            return identify_name(tokenstr);
        }
    } else if (is_check_number(tokenstr[0])) {
        // todo 数字から始まるとき
    } else if (is_check_symbol(tokenstr[0])) {
        return identify_symbol(*tokenstr, fp);
    } else {
        // todo 想定されていない字句
    }
}

/* キーワードを識別する */
int identify_keyword(const char *tokenstr) {
    int i = 0;
    if (strlen(tokenstr) <= MAXKEYWORDLENGTH) {
        for (i = 0; i < KEYWORDSIZE; i++) {
            if (strcmp(tokenstr, key[i].keyword) == 0) {
                return key[i].keytoken;
            }
            printf("%s\n", key[i].keyword);
        }
    }

    return -1;
}

/* 名前に英字と数字以外が入っていないか確認し、
 * 入っていなければstring_attrに名前を格納し、TNAME(1)を返す */
int identify_name(const char *tokenstr) {
    int i = 0;

    while ((i < MAXSTRSIZE) && (tokenstr[i] != '\0')) {
        if (!(is_check_alphabet(tokenstr[i]) || is_check_number(tokenstr[i]))) {
            return -1;
        }
        i++;
    }

    /* 名前をstringattrに格納 */
    init_char_array(string_attr, MAXSTRSIZE);
    snprintf(string_attr, MAXSTRSIZE, "%s", tokenstr);

    return TNAME;
}

/* 記号を識別する */
int identify_symbol(char tokenc, FILE *fp) {
    switch (tokenc) {
        case '(':
            return TLPAREN;
        case ')':
            return TRPAREN;
        case '*':
            return TSTAR;
        case '+':
            return TPLUS;
        case ',':
            return TCOMMA;
        case '-':
            return TMINUS;
        case '.':
            return TDOT;
        case ':':
            cbuf = (char) fgetc(fp);
            switch (cbuf) {
                case '=':
                    return TASSIGN;
                case EOF:
                    return -1;
                default:
                    return TCOLON;
            }
        case ';':
            return TSEMI;
        case '<':
            cbuf = (char) fgetc(fp);
            switch (cbuf) {
                case '>':
                    return TNOTEQ;
                case '=':
                    return TLEEQ;
                case EOF:
                    return -1;
                default:
                    return TLE;
            }
        case '=':
            return TEQUAL;
        case '>':
            cbuf = (char) fgetc(fp);
            switch (cbuf) {
                case '=':
                    return TGREQ;
                case EOF:
                    return -1;
                default:
                    return TGR;
            }
        case '[':
            return TLSQPAREN;
        case ']':
            return TRSQPAREN;
        default:
            error("failed to identify symbol.", fp);
            exit(EXIT_FAILURE);
    }
}

/* 注釈をスキップする
 * 失敗した場合は0を返す */
int skip_comment(FILE *fp, int sep_type) {
    while ((cbuf = (char) fgetc(fp)) != EOF) {
        if (cbuf == '}') {
            if (sep_type == 2) {
                return 1;
            }
        } else if (cbuf == '*') {
            if ((cbuf = (char) fgetc(fp)) == '/') {
                return 2;
            } else if (cbuf == EOF) {
                return 0;
            }
        }
    }
    return 0;
}

int get_linenum(void) {
    /* todo */
}

/* スキャン終了後に呼び出しファイルを閉じる */
void end_scan(FILE *fp) {
    //ファイルを閉じる
    if (fclose(fp) == EOF) {
        printf("ERROR: fclose()\n");
    };
}