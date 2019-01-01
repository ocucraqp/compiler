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
    struct ID *p, **pp, **prevpp;
    char *temp_name;
    char *temp_procname;
    struct TYPE *temp_itp;
    int name_cmp_result = 0, procname_cmp_result = 0;

    /* If it is not already registered name, secure memory and register */
    if ((p = search_idtab(name, procname)) != NULL) {
        return error("%s is defined in duplicate", name, procname);
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

        /* Initialization of each area */
        init_char_array(temp_name, MAX_IDENTIFIER_SIZE);
        init_char_array(temp_procname, MAX_IDENTIFIER_SIZE);

        /* Assign information */
        strncpy(temp_name, name, MAX_IDENTIFIER_SIZE);
        if (procname != NULL) {
            strncpy(temp_procname, procname, MAX_IDENTIFIER_SIZE);
        } else {
            temp_procname = NULL;
        }
        *temp_itp = *itp;

        /* Stored in structure */
        p->name = temp_name;
        p->procname = temp_procname;
        p->itp = temp_itp;
        p->ispara = ispara;
        p->deflinenum = linenum;
        p->irefp = NULL;

        /* Insert in list from name and procname lexicographically */
        if (idroot != NULL) {
            for (pp = &idroot; (*pp) != NULL; pp = &((*pp)->nextp)) {
                name_cmp_result = strcmp(p->name, (*pp)->name);
                if (name_cmp_result < 0) {
                    p->nextp = (*pp);
                    if (pp == &idroot) {
                        idroot = p;
                    } else {
                        (*prevpp)->nextp = p;
                    }
                    return NORMAL;
                } else if (name_cmp_result == 0) {
                    if (p->procname == NULL) {
                        p->nextp = (*pp);
                        if (pp == &idroot) {
                            idroot = p;
                        } else {
                            (*prevpp)->nextp = p;
                        }
                        return NORMAL;
                    } else if ((*pp)->procname == NULL) {
                        prevpp = &(*pp);
                        continue;
                    } else {
                        procname_cmp_result = strcmp(p->procname, (*pp)->procname);
                        if (procname_cmp_result < 0) {
                            p->nextp = (*pp);
                            if (pp == &idroot) {
                                idroot = p;
                            } else {
                                (*prevpp)->nextp = p;
                            }
                            return NORMAL;
                        } else if (procname_cmp_result == 0) {
                            return error("%s in %s is defined in duplicate", p->name, p->procname);
                        } else {
                            prevpp = &(*pp);
                            continue;
                        }
                    }
                } else {
                    prevpp = &(*pp);
                    continue;
                }
            }
            if ((*pp) != NULL) {
                (*pp)->nextp = p;
            } else {
                (*pp) = p;
            }
        } else {
            idroot = p;
        }
    }
    return NORMAL;
}

int ref_id(const char *name, const char *procname, int refnum, struct TYPE **parameter_type) {
    /* If the name is not in procedure compound statement, procname is NULL.
     * refnum is Element number of array.
     * Initial refnum is -1. */
    struct ID *p;
    struct LINE *temp_irefp;

    if ((p = search_idtab(name, procname)) == NULL) {
        if (procname == NULL) {
            return error("%s is not defined.", name);
        } else {
            return error("%s:%s is not defined.", name, procname);
        }
    } else {
        if (refnum >= 0) {
            if (p->itp->arraysize == 0) {
                return error("Variable %s is not array type");
            } else if (refnum >= p->itp->arraysize) {
                return error("The number of subscripts is too large");
            }
        }
        if ((temp_irefp = (struct LINE *) malloc((MAX_IDENTIFIER_SIZE * sizeof(struct LINE)) + 1)) == NULL) {
            return error("can not malloc-3 in def_id");
        }
        temp_irefp->reflinenum = linenum;
        temp_irefp->nextlinep = p->irefp;
        p->irefp = temp_irefp;
    }
    *parameter_type = p->itp;
    return p->itp->ttype;
}

void print_idtab() {    /* Output the registered data */
    struct ID *p;
    int num_space = 0, i = 0;
    char buf[1024];

    for (i = 0; i < 80; i++) {
        printf("-");
    }
    printf("\n");

    /* print row name */
    printf("Name");
    make_space(16);
    printf("Type");
    make_space(28);
    printf("Def.");
    make_space(4);
    printf("Ref.\n");

    for (p = idroot; p != NULL; p = p->nextp) {
        /* print Name */
        printf("%s", p->name);
        num_space = 20 - (int) strlen(p->name);
        if (p->procname != NULL) {
            printf(":%s", p->procname);
            num_space = num_space - 1 - (int) strlen(p->procname);
        }
        make_space(num_space);

        /* print Type */
        switch (p->itp->ttype) {
            case TPINT:
            case TPCHAR:
            case TPBOOL:
                printf("%s", tokenstr[p->itp->ttype - 100]);
                num_space = 32 - (int) strlen(tokenstr[p->itp->ttype - 100]);
                break;
            case TPARRAYINT:
            case TPARRAYCHAR:
            case TPARRAYBOOL:
                printf("array[%d] of %s", p->itp->arraysize, tokenstr[p->itp->ttype - 200]);
                init_char_array(buf, 1024);
                snprintf(buf, 1024, "%d", p->itp->arraysize);
                num_space = 32 - 11 - (int) strlen(buf) - (int) strlen(tokenstr[p->itp->ttype - 200]);
                break;
            case TPPROC:
                printf("procedure");
                num_space = 32 - 9;
                if (p->itp->paratp != NULL) {
                    printf("(%s", tokenstr[p->itp->paratp->ttype - 100]);
                    num_space = num_space - 1 - (int) strlen(tokenstr[p->itp->paratp->ttype - 100]);
                    for (p->itp->paratp = p->itp->paratp->paratp;
                         p->itp->paratp != NULL; p->itp->paratp = p->itp->paratp->paratp) {
                        printf(", %s", tokenstr[p->itp->paratp->ttype - 100]);
                        num_space = num_space - 2 - (int) strlen(tokenstr[p->itp->paratp->ttype - 100]);
                    }
                    printf(")");
                    num_space -= 1;
                }
                break;
            default:
                error("Variable has no type");
                return;
        }
        make_space(num_space);

        /* print Def. */
        init_char_array(buf, 1024);
        snprintf(buf, 1024, "%d", p->deflinenum);
        num_space = 5 - (int) strlen(buf);
        make_space(num_space);
        printf("%d | ", p->deflinenum);

        /* print Ref. */
        if (p->irefp != NULL) {
            printf("%d", p->irefp->reflinenum);
            for (p->irefp = p->irefp->nextlinep; p->irefp != NULL; p->irefp = p->irefp->nextlinep) {
                printf(", %d", p->irefp->reflinenum);
            }
        }

        printf("\n");
    }
    for (i = 0; i < 80; i++) {
        printf("-");
    }
    printf("\n");
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

void make_space(int n) {
    int i = 0;

    for (i = 0; i < n; i++) {
        printf(" ");
    }
}

int check_standard_type(int type) {
    switch (type) {
        case TPINT:
        case TPCHAR:
        case TPBOOL:
            return NORMAL;
        default:
            return error("The type is not a standard type");
    }
}

int check_standard_type_to_pointer(struct TYPE *ptype) {
    switch (ptype->ttype) {
        case TPINT:
        case TPCHAR:
        case TPBOOL:
            return NORMAL;
        default:
            return error("The type is not a standard type");
    }
}