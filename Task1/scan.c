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
    char cbuf;
    char strbuf[MAXSTRSIZE];
    int i;

    init_char_array(strbuf, MAXSTRSIZE);

    // 字句の1文字目を読み込む
    while ((cbuf = fgetc(fp)) != EOF) {
        if (!is_check_separator(cbuf)) {
            strbuf[0] = cbuf;
            break;
        }
    }

    // 分離子もしくは記号まで読み込む
    for (i = 1; (cbuf = fgetc(fp)) != EOF; i++) {
        if (is_check_separator(cbuf)) {
            break;
        } else {
            strbuf[i] = cbuf;
        }
    }

    return identify_token(strbuf);
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

/* 引数が
 * 空白、タブ、改行のとき1
 * 注釈のとき2
 * それ以外のとき0
 * を返す */
int is_check_separator(char c) {
    switch (c) {
        case 0x09: // 水平タブ
        case 0x0a: // 改行
        case 0x0b: // 垂直タブ
        case 0x0c: // 改頁
        case 0x0d: // 復帰
        case 0x20: // 空白文字
            return 1;
            break;
            /* todo 注釈
             * case: */
        default:
            return 0;
    }
}

/* トークンを識別する */
int identify_token(const char *tokenstr) {
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
    } else if ((tokenstr[0] >= 0x28 && tokenstr[0] <= 0x2e) || (tokenstr[0] >= 0x28 && tokenstr[0] <= 0x2e) ||
               tokenstr[0] == 0x5b || tokenstr[0] == 0x5d) {
        // todo 記号から始まるとき
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