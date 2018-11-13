#include "pretty-print.h"

char string_attr[MAXSTRSIZE];

int num_attr;

char cbuf = '\0';

int linenum = 0;

/* Call before scanning, open the file, prepare for scanning
 * Success:0 Failed:-1 */
int init_scan(char *filename, FILE **fp) {
    /* The file pointer is received as a pointer to the pointer */
    *fp = fopen(filename, "r");

    if (*fp == NULL) {
        return -1;
    }

    /* Initialize linenum 1 */
    linenum = 1;

    /* Read one character in cbuf */
    cbuf = (char) fgetc(*fp);

    return 0;

}

/* Return the code of the token
 * failed:-1 */
int scan(FILE *fp) {
    char strbuf[MAXSTRSIZE];
    int i = 0, temp = 0, sep_type = 0;

    //Initialize strbuf with '\ 0'
    init_char_array(strbuf, MAXSTRSIZE);

    /* Checks whether cbuf contains a separator or EOF,
     * if it is a separator, it skips the separator
     * until there is no separator and returns -1 if it is EOF */
    while ((sep_type = skip_separator(cbuf, fp)) != 0) {
        if (sep_type == -1) {
            return -1;
        }
    }
    if (cbuf == EOF) {
        return -1;
    }


    /* Find out what cbuf contains
     * in the order of symbols, numbers, strings, letters */
    if (is_check_symbol(cbuf)) {
        /* If there is a symbol, identify which symbol and return */
        strbuf[0] = cbuf;
        cbuf = (char) fgetc(fp);
        return identify_symbol(strbuf, fp);
    } else if (is_check_number(cbuf)) {
        /* If there is a number,
         * continue as long as the number continues, call identify_number */
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_token_size(i) == -1) {
                return -1;
            }
            if (is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break;
            }
        }
        return identify_number(strbuf);
    } else if (cbuf == '\'') {
        /* If "'" is included,
         * think it as a character string and call identify_string */
        return identify_string(fp);
    } else if (is_check_alphabet(cbuf)) {
        /* If there is an alphabetic character,
         * it is a keyword or name, so long as an alphabetic character
         * or digit continues, put it in strbuf, call identify_keyword,
         * call identify_name if it is not keyword */
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_token_size(i) == -1) {
                return -1;
            }
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

    /* If there is nothing in it,
     * it is determined that a terminal symbol not supposed has come */
    error("contain unexpected token.");
    return -1;
}

/* Initialize int type array with 0 */
void init_int_array(int *array, int arraylength) {
    int i = 0;
    for (i = 0; i < arraylength; i++) {
        array[i] = 0;
    }
}

/* Initialize char array as \ 0 */
void init_char_array(char *array, int arraylength) {
    int i = 0;
    for (i = 0; i < arraylength; i++) {
        array[i] = '\0';
    }
}

/* Check whether the letters are alphabetic */
int is_check_alphabet(char c) {
    if ((c >= 0x41 && c <= 0x5a) || (c >= 0x61 && c <= 0x7a)) {
        return 1;
    }
    return 0;
}

/* Confirm whether the character is a number */
int is_check_number(char c) {
    if (c >= 0x30 && c <= 0x39) {
        return 1;
    }
    return 0;
}

/* Make sure the letters are symbols */
int is_check_symbol(char c) {
    if ((c >= 0x28 && c <= 0x2e) || (c >= 0x3a && c <= 0x3e) ||
        c == 0x5b || c == 0x5d) {
        return 1;
    }
    return 0;
}

/* Argument is
 * When blank, tab, or line feed 1
 * 2 for annotations beginning with "{"
 * 3 for annotations beginning with "/" "*"
 * Otherwise 0
 * return it */
int skip_separator(char c, FILE *fp) {
    switch (c) {
        case '\t': // Horizontal tab
        case '\v': // Vertical tab
        case 0x20: // Space
            cbuf = (char) fgetc(fp);
            return 1;
        case '\r': // Return
            cbuf = (char) fgetc(fp);
            if (cbuf == '\n') {
                cbuf = (char) fgetc(fp);
            }
            linenum++;
            return 1;
        case '\n': // new line
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

int identify_keyword(const char *tokenstr) {
    int i = 0;
    if (strlen(tokenstr) <= MAXKEYWORDLENGTH) {
        for (i = 0; i < NUMOFKEYWORD; i++) {
            if (strcmp(tokenstr, key[i].keyword) == 0) {
                return key[i].keytoken;
            }
        }
    }

    return -1;
}

/* Stores name in string_attr and returns TNAME */
int identify_name(const char *tokenstr) {
    init_char_array(string_attr, MAXSTRSIZE);
    snprintf(string_attr, MAXSTRSIZE, "%s", tokenstr);

    return TNAME;
}

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

/* Stores numbers in num_attr and returns TNUMBER */
int identify_number(const char *tokenstr) {
    long temp = 0;

    temp = strtol(tokenstr, NULL, 10);
    if (temp <= 32767) {
        num_attr = (int) temp;
        sprintf(string_attr, "%d", num_attr);
    } else {
        error("number is bigeer than 32767.");
        return -1;
    }

    return TNUMBER;
}

/* String is stored in string_attr and TSTRING is returned */
int identify_string(FILE *fp) {
    int i = 0;
    char tempbuf[MAXSTRSIZE];

    for (i = 0; (cbuf = (char) fgetc(fp)) != EOF; i++) {
        if (is_check_token_size(i + 1) == -1) {
            return -1;
        }
        if (cbuf == '\'') {
            cbuf = (char) fgetc(fp);
            if (cbuf == '\'') {
                tempbuf[i] = '\'';
                i++;
                tempbuf[i] = '\'';
            } else {
                init_char_array(string_attr, MAXSTRSIZE);
                snprintf(string_attr, MAXSTRSIZE, "%s", tempbuf);
                return TSTRING;

            }
        } else {
            tempbuf[i] = cbuf;
        }
    }
    error("failed to reach string end.");
    return -1;
}

/* Skip annotations
 * If it reaches EOF, it returns -1 */
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
            }
        }
    }
    error("failed to reach comment end.");
    return -1;
}

/* Returns the number where the token
 * returned by scan () most recently existed. */
int get_linenum(void) {
    return linenum;
}

/* Close the call file after scanning */
void end_scan(FILE *fp) {
    if (fclose(fp) == EOF) {
        error("File can not close.");
    };
}

/* Return -1 if the size of the token exceeds the maximum */
int is_check_token_size(int i) {
    if (i >= MAXSTRSIZE) {
        error("one token is too long.");
        return -1;
    }
    return 1;
}