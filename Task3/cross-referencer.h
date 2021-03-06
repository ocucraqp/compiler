#ifndef TASK3_CROSS_REFERENCER_H
#define TASK3_CROSS_REFERENCER_H

/* cross-referencer.h  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAXSTRSIZE 1024
#define MAX_IDENTIFIER_SIZE 8
#define NORMAL 0
#define ERROR -1
#define NUMOFKEYWORD    28
#define MAXKEYWORDLENGTH    9

/* Token */
#define    TNAME        1    /* Name : Alphabet { Alphabet | Digit } */
#define    TPROGRAM    2    /* program : Keyword */
#define    TVAR        3    /* var : Keyword */
#define    TARRAY        4    /* array : Keyword */
#define    TOF        5    /* of : Keyword */
#define    TBEGIN        6    /* begin : Keyword */
#define    TEND        7    /* end : Keyword */
#define    TIF        8    /* if : Keyword */
#define    TTHEN        9    /* then : Keyword */
#define    TELSE        10    /* else : Keyword */
#define    TPROCEDURE    11    /* procedure : Keyword */
#define    TRETURN        12    /* return : Keyword */
#define    TCALL        13    /* call : Keyword */
#define    TWHILE        14    /* while : Keyword */
#define    TDO        15    /* do : Keyword */
#define    TNOT        16    /* not : Keyword */
#define    TOR        17    /* or : Keyword */
#define    TDIV        18    /* div : Keyword */
#define    TAND        19    /* and : Keyword */
#define    TCHAR        20    /* char : Keyword */
#define    TINTEGER    21    /* integer : Keyword */
#define    TBOOLEAN    22    /* boolean : Keyword */
#define    TREADLN        23    /* readln : Keyword */
#define    TWRITELN    24    /* writeln : Keyword */
#define    TTRUE        25    /* true : Keyword */
#define    TFALSE        26    /* false : Keyword */
#define    TNUMBER        27    /* unsigned integer */
#define    TSTRING        28    /* String */
#define    TPLUS        29    /* + : symbol */
#define    TMINUS        30    /* - : symbol */
#define    TSTAR        31    /* * : symbol */
#define    TEQUAL        32    /* = : symbol */
#define    TNOTEQ        33    /* <> : symbol */
#define    TLE        34    /* < : symbol */
#define    TLEEQ        35    /* <= : symbol */
#define    TGR        36    /* > : symbol */
#define    TGREQ        37    /* >= : symbol */
#define    TLPAREN        38    /* ( : symbol */
#define    TRPAREN        39    /* ) : symbol */
#define    TLSQPAREN    40    /* [ : symbol */
#define    TRSQPAREN    41    /* ] : symbol */
#define    TASSIGN        42    /* := : symbol */
#define    TDOT        43    /* . : symbol */
#define    TCOMMA        44    /* , : symbol */
#define    TCOLON        45    /* : : symbol */
#define    TSEMI        46    /* ; : symbol */
#define    TREAD        47    /* read : Keyword */
#define    TWRITE        48    /* write : Keyword */
#define    TBREAK        49    /* break : Keyword */

#define NUMOFTOKEN    49

/* Type */
#define TPINT TINTEGER+100
#define TPCHAR TCHAR+100
#define TPBOOL TBOOLEAN+100
#define TPARRAYINT TINTEGER+200
#define TPARRAYCHAR TCHAR+200
#define TPARRAYBOOL TBOOLEAN+200
#define TPPROC 100

/* Structure */
extern struct KEY {
    char *keyword;
    int keytoken;
} key[NUMOFKEYWORD];

extern struct TYPE {
    int ttype;
    /* TPINT TPCHAR TPBOOL TPARRAYINT TPARRAYCHAR
    TPARRAYBOOL TPPROC */
    int arraysize;
    /* size of array, if TPARRAY */
    struct TYPE *paratp;
    /* pointer to parameter's type list if ttype is TPPROC
     * paratp is NULL if ttype is not TPROC*/
} temp_type, *end_type;

extern struct LINE {
    int linenum;
    struct LINE *nextlinep;
} *vallinenumroot;

extern struct ID {
    char *name;
    char *procname;
    /* procedure name within which this name is defined */ /* NULL if global name */
    struct TYPE *itp;
    int deflinenum;
    struct LINE *irefp;
    struct ID *nextp;
} *idroot;

extern struct NAME {
    char *name;
    struct NAME *nextnamep;
} *temp_name_root;

/* main.c */
extern int error(char *mes, ...);

/* scan.c */
extern int num_attr;

extern char string_attr[MAXSTRSIZE];

extern int linenum;

extern int init_scan(char *filename, FILE **fp);

extern int scan(FILE *fp);

extern void init_int_array(int *array, int arraylength);

extern void init_char_array(char *array, int arraylength);

extern int get_linenum(void);

extern void end_scan(FILE *fp);

/* parse-syntax.c */
extern char *tokenstr[NUMOFTOKEN + 1];

extern int token;

extern int parse_program(FILE *fp);

/* cross-referencer.c */
extern char *current_procname;

extern void init_temp_names();

extern int temp_names(char *name);

extern void release_names();

extern void init_type(struct TYPE *type);

extern int save_vallinenum();

extern void release_vallinenum();

extern int def_id(const char *name, const char *procname, const struct TYPE *itp);

extern int ref_id(const char *name, const char *procname, int refnum, struct TYPE **temp_type);

extern void print_idtab();

extern void release_idtab();

extern int check_standard_type(int type);

extern int check_standard_type_to_pointer(struct TYPE *ptype);

#endif //TASK3_CROSS_REFERENCER_H