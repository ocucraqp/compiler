#include "token-list.h"

char string_attr[MAXSTRSIZE];

int num_attr;

char cbuf = '\0';

int linenum = 0;

/* スキャン前に呼び出し、ファイルをオープンし、スキャンの準備
 * 成功したら0、失敗したら-1を返す */
int init_scan(char *filename, FILE **fp) {
    /* ファイルポインタはポインタのポインタとして受け取り、値を返す */
    *fp = fopen(filename, "r");

    if (*fp == NULL) {
        return -1;
    }

    /* 行番号を初期化 */
    linenum = 0;

    /* cbufに一文字読み込む */
    cbuf = (char) fgetc(*fp);


    return 0;

}

/* トークンのコードを返す。
 * 次のトークンをスキャンできないときは、-1を返す。 */
int scan(FILE *fp) {
    char strbuf[MAXSTRSIZE];
    int i = 0, temp = 0, sep_type = 0;

    //strbufを'\0'で初期化
    init_char_array(strbuf, MAXSTRSIZE);

    /* cbufに分離子かEOFが入ってないか調べ、
     * 分離子の場合は分離子がなくなるまで分離子を飛ばし
     * EOFの場合は-1を返し終了 */
    while ((sep_type = skip_separator(cbuf, fp)) != 0) {
        if (sep_type == -1) {
            return -1;
        }
    }
    if (cbuf == EOF) {
        return -1;
    }


    /* cbufに記号、数字、、文字列、英字の順で何が入っているか調べる */
    if (is_check_symbol(cbuf)) {
        /* 記号が入っていた場合は、どの記号か識別して返す */
        strbuf[0] = cbuf;
        cbuf = (char) fgetc(fp);
        return identify_symbol(strbuf, fp);
    } else if (is_check_number(cbuf)) {
        /* 数字が入っていた場合は、数字が続く限り続け、identify_numberを呼ぶ */
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break;
            }
        }
        return identify_number(strbuf);
    } else if (cbuf == '\'') {
        /* "'"が入っていた場合は、文字列と考え、identify_stringを呼ぶ */
        return identify_string(fp);
    } else if (is_check_alphabet(cbuf)) {
        /* 英字が入っていた場合はキーワードか名前なので英字か数字が続く限り、
         * strbufに入れていき、identify_keywordを呼び、
         * キーワードでなければidentify_nameを呼ぶ */
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_alphabet(cbuf) || is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break;
            }
        }
        if ((temp = identify_keyword(strbuf)) != -1) {
            return temp;
        } else {
            return identify_name(strbuf);
        }
    }

    /*何も入っていなければ想定していない終端記号が来ていたと判別する */
    error("contain unexpected token.");
    return -1;
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
int skip_separator(char c, FILE *fp) {
    switch (c) {
        case '\t': // 水平タブ
        case '\v': // 垂直タブ
        case '\f': // 改頁
        case 0x20: // 空白文字
            cbuf = (char) fgetc(fp);
            return 1;
        case '\r': // 復帰
            cbuf = (char) fgetc(fp);
            if (cbuf == '\n') {
                cbuf = (char) fgetc(fp);
            }
            linenum++;
            return 1;
        case '\n': // 改行
            cbuf = (char) fgetc(fp);
            if (cbuf == '\r') {
                cbuf = (char) fgetc(fp);
            }
            linenum++;
            return 1;
        case '{':
            return skip_comment(fp, 2);
        case '/':
            if ((cbuf = (char) fgetc(fp)) == '*') {
                return skip_comment(fp, 3);
            }
            error("contain unexpected token.");
            return -1;
        default:
            return 0;
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
        }
    }

    return -1;
}

/* string_attrに名前を格納し、TNAMEを返す */
int identify_name(const char *tokenstr) {
    /* 名前をstring_attrに格納 */
    init_char_array(string_attr, MAXSTRSIZE);
    snprintf(string_attr, MAXSTRSIZE, "%s", tokenstr);

    return TNAME;
}

/* 記号を識別する */
int identify_symbol(char *tokenstr, FILE *fp) {
    switch (tokenstr[0]) {
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
            switch (cbuf) {
                case '=':
                    cbuf = (char) fgetc(fp);
                    return TASSIGN;
                case EOF:
                    return -1;
                default:
                    return TCOLON;
            }
        case ';':
            return TSEMI;
        case '<':
            switch (cbuf) {
                case '>':
                    cbuf = (char) fgetc(fp);
                    return TNOTEQ;
                case '=':
                    cbuf = (char) fgetc(fp);
                    return TLEEQ;
                case EOF:
                    return -1;
                default:
                    return TLE;
            }
        case '=':
            return TEQUAL;
        case '>':
            switch (cbuf) {
                case '=':
                    cbuf = (char) fgetc(fp);
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
            error("failed to identify symbol.");
            return -1;
    }
}

/* num_attrに名前を格納し、TNUMBERを返す */
int identify_number(const char *tokenstr) {
    long temp = 0;

    /* 符号なし整数をnum_attrに格納 */
    temp = strtol(tokenstr, NULL, 10);
    if (temp <= 32767) {
        num_attr = (int) temp;
    } else {
        error("number is bigeer than 32767.");
        return -1;
    }

    return TNUMBER;
}

/* string_attrに文字列を格納し、TSTRINGを返す */
int identify_string(FILE *fp) {
    int i = 0;
    char tempbuf[MAXSTRSIZE];

    for (i = 0; (cbuf = (char) fgetc(fp)) != EOF; i++) {
        if (cbuf == '\'') {
            /* 文字列をstring_attrに格納 */
            init_char_array(string_attr, MAXSTRSIZE);
            snprintf(string_attr, MAXSTRSIZE, "%s", tempbuf);
            //cbufに1文字読み込む
            cbuf = (char) fgetc(fp);
            return TSTRING;
        } else {
            tempbuf[i] = cbuf;
        }
    }
    return -1;
}

/* 注釈をスキップする
 * EOFまでたどり着いた場合は-1を返す */
int skip_comment(FILE *fp, int sep_type) {
    while ((cbuf = (char) fgetc(fp)) != EOF) {
        if (cbuf == '}') {
            if (sep_type == 2) {
                cbuf = (char) fgetc(fp);
                return 2;
            }
        } else if (cbuf == '*') {
            if ((cbuf = (char) fgetc(fp)) == '/') {
                cbuf = (char) fgetc(fp);
                return 3;
            } else if (cbuf == EOF) {
                error("failed to reach comment end.");
                return -1;
            }
        }
    }
    return -1;
}

/* もっとも最近にscan()で返されたトークンが存在した番号を返す。 */
int get_linenum(void) {
    return linenum;
}

/* スキャン終了後に呼び出しファイルを閉じる */
void end_scan(FILE *fp) {
    //ファイルを閉じる
    if (fclose(fp) == EOF) {
        error("File can not close.");
    };
}