#include "token-list.h"

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

int scan(FILE *fp) {
}

/* トークンカウント用の配列を初期化する */
void init_array(int *array, int arraylength) {
    int i;
    for (i = 0; i < arraylength; i++) {
        array[i] = 0;
    }
}

int get_linenum(void) {
    /* todo */
}

/* スキャン終了後に呼び出しファイルを閉じる */
void end_scan(FILE *fp) {
    //ファイルを閉じる
    if(fclose(fp) == EOF){
        printf("ERROR: fclose()\n");
    };
}