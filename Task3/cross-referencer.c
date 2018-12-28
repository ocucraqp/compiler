#include "cross-referencer.h"

struct LINE {
    int reflinenum;
    struct LINE *nextlinep;
};

struct ID *idroot;

struct NAME *temp_name_root = NULL;

void init_idtab() {        /* Initialise the table */
    idroot = NULL;
}

void init_temp_names() {
    temp_name_root = NULL;
}

void init_type(struct TYPE *type) {
    type->ttype = 0;
    type->arraysize = 0;
    type->paratp = NULL;
}

struct ID *search_idtab(const char *name, const char *procname) {    /* search the name pointed by np */
    struct ID *p;

    for (p = idroot; p != NULL; p = p->nextp) {
        if (strncmp(name, p->name, MAX_IDENTIFIER_SIZE) == 0) {
            if (procname != NULL && p->procname != NULL) {
                if (strncmp(procname, p->procname, MAX_IDENTIFIER_SIZE) == 0) {
                    return (p);
                }
            } else if (procname == NULL && p->procname == NULL) {
                return (p);
            }
        }
    }

    return (NULL);
}

int temp_names(char *name) {
    struct NAME *pname;
    char *temp_name;

    if ((pname = (struct NAME *) malloc(sizeof(struct NAME))) == NULL) {
        return error("can not malloc-1 in temp_names");
    }
    if ((temp_name = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
        return error("can not malloc-2 in temp_names");
    }

    init_char_array(temp_name, MAX_IDENTIFIER_SIZE + 1);
    strncpy(temp_name, name, MAX_IDENTIFIER_SIZE);

    pname->name = temp_name;
    pname->nextnamep = temp_name_root;
    temp_name_root = pname;
    return NORMAL;
}

void release_names() {    /* Release tha data structure */
    struct NAME *p, *q;

    for (p = temp_name_root; p != NULL; p = q) {
        free(p->name);
        q = p->nextnamep;
        free(p);
    }
    init_temp_names();
}

int def_id(const char *name, const char *procname, int ispara, const struct TYPE *itp) {
    struct ID *p;
    char *temp_name;
    char *temp_procname;
    struct TYPE *temp_itp;

    if ((p = search_idtab(name, procname)) != NULL) {
        return error("It is defined in duplicate");
    } else {
        if ((p = (struct ID *) malloc(sizeof(struct ID))) == NULL) {
            return error("can not malloc-1 in def_id");
        }
        if ((temp_name = (char *) malloc(MAX_IDENTIFIER_SIZE * sizeof(char))) == NULL) {
            return error("can not malloc-2 in def_id");
        }
        if ((temp_procname = (char *) malloc(MAX_IDENTIFIER_SIZE * sizeof(char))) == NULL) {
            return error("can not malloc-3 in def_id");
        }
        if ((temp_itp = (struct TYPE *) malloc(MAX_IDENTIFIER_SIZE * sizeof(struct TYPE))) == NULL) {
            return error("can not malloc-3 in def_id");
        }

        init_char_array(temp_name, MAX_IDENTIFIER_SIZE);
        init_char_array(temp_procname, MAX_IDENTIFIER_SIZE);

        strncpy(temp_name, name, MAX_IDENTIFIER_SIZE);
        if (procname != NULL) {
            strncpy(temp_procname, procname, MAX_IDENTIFIER_SIZE);
        } else {
            temp_procname = NULL;
        }
        *temp_itp = *itp;

        p->name = temp_name;
        p->procname = temp_procname;
        p->itp = temp_itp;
        p->ispara = ispara;
        p->deflinenum = linenum;
        p->irefp = NULL;
        p->nextp = idroot;
        idroot = p;
    }
    return NORMAL;
}

int ref_id(const char *name, const char *procname) {
    struct ID *p;
    struct LINE *temp_irefp;

    if ((p = search_idtab(name, procname)) == NULL) {
        return error("Variable name not defined.");
    } else {
        if ((temp_irefp = (struct LINE *) malloc((MAX_IDENTIFIER_SIZE * sizeof(struct LINE)) + 1)) == NULL) {
            return error("can not malloc-3 in def_id");
        }
        temp_irefp->reflinenum = 0;
        temp_irefp->nextlinep = NULL;
        p->irefp = temp_irefp;
    }
    return NORMAL;
}

void print_idtab() {    /* Output the registered data */
    struct ID *p;

    printf("Name\tType\t\tDef.\tRef.\n");
    for (p = idroot; p != NULL; p = p->nextp) {
        printf("%s", p->name);
        if (p->procname != NULL) {
            printf(":%s", p->procname);
        }
        printf("\t");
        switch (p->itp->ttype) {
            case TPINT:
            case TPCHAR:
            case TPBOOL:
                printf("%s\t\t", key[p->itp->ttype - 100].keyword);
                break;
            case TPARRAYINT:
            case TPARRAYCHAR:
            case TPARRAYBOOL:
                printf("array[%d] of %s\t", p->itp->arraysize, key[p->itp->ttype - 200].keyword);
                break;
            case TPPROC:
                printf("procedure");
                if (p->itp->paratp != NULL) {
                    printf("(%s", key[p->itp->paratp->ttype - 100].keyword);
                    for (p->itp->paratp = p->itp->paratp->paratp;
                         p->itp->paratp != NULL; p->itp->paratp = p->itp->paratp->paratp) {
                        printf(", %s", key[p->itp->paratp->ttype - 100].keyword);
                    }
                    printf(")\t");
                }
                break;
            default:
                error("Variable has no type");
                return;
        }
        printf("%d\t", p->deflinenum);
        if (p->irefp != NULL) {
            printf("%d", p->irefp->reflinenum);
            for (p->irefp = p->irefp->nextlinep; p->irefp != NULL; p->irefp = p->irefp->nextlinep) {
                printf(", %d", p->irefp->reflinenum);
            }
        }
        printf("\n");
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