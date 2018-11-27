#include "cross-referencer.h"

struct TYPE {
    int ttype;
    /* TPINT TPCHAR TPBOOL TPARRAY TPARRAYINT TPARRAYCHAR
    TPARRAYBOOL TPPROC */
    int arraysize;
    /* size of array, if TPARRAY */
    struct TYPE *etp;
    /* pointer to element type if TPARRAY */
    struct TYPE *paratp;
    /* pointer to parameter's type list if ttype is TPPROC */
};

struct LINE {
    int reflinenum;
    struct LINE *nextlinep;
};

struct ID {
    char *name;
    char *procname;
    /* procedure name within which this name is defined */ /* NULL if global name */
    struct TYPE *itp;
    int ispara;
    /* 1:formal parameter, 0:else(variable) */
    int deflinenum;
    struct LINE *irefp;
    struct ID *nextp;
} *globalidroot, *localidroot;
/* Pointers to root of global & local symbol tables */

//struct ID {
//    char *name;
//    int count;
//    struct ID *nextp;
//} *idroot;
//
//void init_idtab() {        /* Initialise the table */
//    idroot = NULL;
//}
//
//struct ID *search_idtab(char *np) {    /* search the name pointed by np */
//    struct ID *p;
//
//    for (p = idroot; p != NULL; p = p->nextp) {
//        if (strcmp(np, p->name) == 0) return (p);
//    }
//    return (NULL);
//}
//
//void id_countup(char *np) {    /* Register and count up the name pointed by np */
//    struct ID *p;
//    char *cp;
//
//    if ((p = search_idtab(np)) != NULL) p->count++;
//    else {
//        if ((p = (struct ID *) malloc(sizeof(struct ID))) == NULL) {
//            printf("can not malloc in id_countup\n");
//            return;
//        }
//        if ((cp = (char *) malloc(strlen(np) + 1)) == NULL) {
//            printf("can not malloc-2 in id_countup\n");
//            return;
//        }
//        strcpy(cp, np);
//        p->name = cp;
//        p->count = 1;
//        p->nextp = idroot;
//        idroot = p;
//    }
//}
//
//void print_idtab() {    /* Output the registered data */
//    struct ID *p;
//
//    for (p = idroot; p != NULL; p = p->nextp) {
//        if (p->count != 0)
//            printf("\t\"Identifier\" \"%s\"\t%d\n", p->name, p->count);
//    }
//}
//
//void release_idtab() {    /* Release tha data structure */
//    struct ID *p, *q;
//
//    for (p = idroot; p != NULL; p = q) {
//        free(p->name);
//        q = p->nextp;
//        free(p);
//    }
//    init_idtab();
//}