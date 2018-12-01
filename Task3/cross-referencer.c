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

void init_idtab() {        /* Initialise the table */
    globalidroot = NULL;
    localidroot = NULL;
}

struct ID *search_idtab(char *np) {    /* search the name pointed by np */
    struct ID *p;

    for (p = idroot; p != NULL; p = p->nextp) {
        if (strncmp(np, p->name, 8) == 0) return (p);
    }
    return (NULL);
}

void def_id(char *name, char *procname, int *ispara, struct TYPE *itp) {
    struct ID *p;
    char temp_name[9], temp_procname[9], *temp_itp, *temp_ispara;

    if ((p = search_idtab(name)) != NULL) {
        return error("It is defined in duplicate");
    } else {
        if ((p = (struct ID *) malloc(sizeof(struct ID))) == NULL) {
            printf("can not malloc in def_id\n");
            return;
        }
        if ((temp_name = (char *) malloc(strlen(name) + 1)) == NULL) {
            printf("can not malloc-2 in def_id\n");
            return;
        }
        if ((temp_procname = (char *) malloc(strlen(procname) + 1)) == NULL) {
            printf("can not malloc-3 in def_id\n");
            return;
        }

        init_char_array(temp_name);
        init_char_array(temp_procname);

        strcpy(temp_name, name);
        strcpy(temp_procname, procname);
        *temp_itp = *itp;
        *temp_ispara = *ispara;

        p->name = temp_name;
        p->procname = temp_procname;
        p->itp = temp_itp;
        p->ispara = temp_ispara;
        p->deflinenum = linenum;
        if (procname == NULL) {
            p->nextp = globalidroot;
            globalidroot = p;
        } else {
            p->nextp = localidroot;
            localidroot = p;
        }
    }
}

void ref_id(char *name) {
    struct ID *p;
    char temp_name[9], temp_procname[9], *temp_itp, *temp_ispara;

    if ((p = search_idtab(name)) == NULL) {
        return error("Variable name not defined.");
    } else {
        
    }
}

void release_idtab() {    /* Release tha data structure */
    struct ID *p, *q;

    for (p = idroot; p != NULL; p = q) {
        free(p->name);
        free(p->procname);
        q = p->nextp;
        free(p);
    }
    init_idtab();
}