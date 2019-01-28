#include "compiler.h"

/* A variable holding the label number */
int labelnum = 0;

/* An array of variables that holds which libraries are needed */
int pl_flag[NUMOFPL + 1];

/* A buffer to hold the code to be added later */
char label_buf[MAX_OUTPUT_BUF_SIZE];

/* A pointer holding the label to which the exit statement jumps */
char *exit_label = NULL;

/* Call before output, open the file,
 * Success:0 Failed:-1 */
int init_outputfile(char *inputfilename) {
    char *outputfilename;
    char outputfilename_extension[32];

    outputfilename = strtok(inputfilename, ".");
    sprintf(outputfilename_extension, "%.27s.csl", outputfilename);

    /* The file pointer is received as a pointer to the pointer */
    outputfp = fopen(outputfilename_extension, "w");

    if (outputfp == NULL) {
        fprintf(stderr, "\nERROR: File %s can not open.\n", outputfilename_extension);
        return -1;
    }

    init_int_array(pl_flag, NUMOFPL);

    return 0;

}

/* Close the call file after output */
void end_output() {
    fprintf(outputfp, "\tEND\n");
    if (fclose(outputfp) == EOF) {
        fprintf(stderr, "\nERROR: Output file can not close.\n");
    };
}

/* Create a new label from lebelnum */
int create_newlabel(char **labelname) {
    char *newlabel;

    if ((newlabel = (char *) malloc((LABELSIZE + 1) * sizeof(char))) == NULL) {
        return error("can not malloc in create_newlabel");
    }

    labelnum++;
    if (labelnum > 9999) {
        return error("Too many labels");
    }
    snprintf(newlabel, LABELSIZE, "L%04d", labelnum);
    *labelname = newlabel;
    return NORMAL;
}

/* Create a label for each id name */
int create_id_label(struct ID *p) {
    /* id_label output */
    if (p != NULL) {
        if (p->name != NULL) {
            fprintf(outputfp, "$%s", p->name);
        } else {
            return error("Output id name is not fount");
        }
        if (p->procname != NULL) {
            fprintf(outputfp, "%%%s", p->procname);
        }
        if (p->itp != NULL) {
            switch (p->itp->ttype) {
                case TPINT:
                case TPCHAR:
                case TPBOOL:
                    fprintf(outputfp, "\tDC\t0");
                    break;
                case TPARRAYINT:
                case TPARRAYCHAR:
                case TPARRAYBOOL:
                    fprintf(outputfp, "\tDS\t%d", p->itp->arraysize);
                    break;
                default:
                    break;
            }
        } else {
            return error("Output id type is not fount");
        }
        fprintf(outputfp, "\n");
        return NORMAL;
    } else {
        return error("Output id is not found");
    }
}

/* Generate label */
void command_label(char *labelname) {
    fprintf(outputfp, "%s\n", labelname);
}

/* Generate code at program start */
int command_start(char *program_name, char **labelname) {
    /* program start */
    fprintf(outputfp, "$$%s\tSTART\n", program_name);
    /* Initialize gr 0 to 0 */
    fprintf(outputfp, "\tLAD \tgr0, 0\n");
    /* Call a label indicating compound statement of main program */
    if (create_newlabel(labelname) == ERROR) { return ERROR; }
    fprintf(outputfp, "\tCALL\t%s\n", *labelname);
    /* End processing */
    fprintf(outputfp, "\tCALL\tFLUSH\n");
    on_pl_flag(PLFLUSH);
    fprintf(outputfp, "\tSVC \t0\n");

    return NORMAL;
}

/* Generate code to process arguments */
void command_process_arguments() {
    struct PARAID **paraidp, *tempp;
    paraidp = &(paraidroot.nextparaidp);

    if ((*paraidp) != NULL) {
        fprintf(outputfp, "\tPOP \tgr2\n");
        while ((*paraidp) != NULL) {
            fprintf(outputfp, "\tPOP \tgr1\n");
            fprintf(outputfp, "\tST  \tgr1, $%s%%%s\n",
                    (*paraidp)->paraidp->name, (*paraidp)->paraidp->procname);
            tempp = (*paraidp);
            paraidp = &((*paraidp)->nextparaidp);
            free(tempp);
            tempp = NULL;
        }
        fprintf(outputfp, "\tPUSH\t0, gr2\n");
    }
}

/* Generate code of if and while statement */
void command_condition_statement(char *labelname) {
    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
    fprintf(outputfp, "\tJZE \t%s\n", labelname);
};

/* Generate code of assign statement */
void command_assign(int is_insubproc, struct ID *p) {
    if (is_insubproc) {
        fprintf(outputfp, "\tPOP \tgr2\n");
        fprintf(outputfp, "\tST  \tgr1, 0, gr2\n");
    } else {
        if (p->itp->ttype == TPARRAYINT || p->itp->ttype == TPARRAYCHAR || p->itp->ttype == TPARRAYBOOL) {
            fprintf(outputfp, "\tPOP \tgr2\n");
        }
        fprintf(outputfp, "\tST  \tgr1, $%s", p->name);
        if (p->procname != NULL) {
            fprintf(outputfp, "%%%s", p->procname);
        }
        if (p->itp->ttype == TPARRAYINT || p->itp->ttype == TPARRAYCHAR || p->itp->ttype == TPARRAYBOOL) {
            fprintf(outputfp, ", gr2");
        }
        fprintf(outputfp, "\n");
    }
}

/* Generate code for outputting character string
 * When this function is called in a call statement,
 * it reads the address to gr1 */
int command_variable(struct ID *p, int is_incall, int is_ininput, int is_index) {
    if (is_incall == 1 || (is_ininput && !(p->ispara))) {
        fprintf(outputfp, "\tLAD  \tgr1, $%s", p->name);
    } else {
        fprintf(outputfp, "\tLD  \tgr1, $%s", p->name);
    }
    if (p->procname != NULL) {
        fprintf(outputfp, "%%%s", p->procname);
    }
    if (is_index) {
        fprintf(outputfp, ", gr1");
    }
    fprintf(outputfp, "\n");

    return NORMAL;
}

/* Generate code for determining whether the number of elements is within the range */
int command_judge_index(int arraysize) {
    char *labelname = NULL;
    if (create_newlabel(&labelname) == ERROR) { return ERROR; }

    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
    fprintf(outputfp, "\tJMI \tEROV\n");
    fprintf(outputfp, "\tCPA \tgr1, %d\n", arraysize);
    fprintf(outputfp, "\tJMI \t%s\n", labelname);
    fprintf(outputfp, "\tJUMP\tEROV\n");
    fprintf(outputfp, "%s\n", labelname);
    on_pl_flag(PLEROV);

    return NORMAL;
}

/* Generate code for expressions */
void command_expressions(char *procname) {
    fprintf(outputfp, "\tCALL\t$%s\n", procname);
}

/* Generate code to calculate expression */
int command_expression(int opr) {
    char *ok_labelname = NULL, *ng_labelname = NULL;

    fprintf(outputfp, "\tPOP \tgr2\n");
    fprintf(outputfp, "\tCPA \tgr2, gr1\n");
    if (create_newlabel(&ok_labelname) == ERROR) { return ERROR; }
    if (create_newlabel(&ng_labelname) == ERROR) { return ERROR; }
    switch (opr) {
        case TEQUAL:
            fprintf(outputfp, "\tJZE \t%s\n", ok_labelname);
            break;
        case TNOTEQ:
            fprintf(outputfp, "\tJNZ \t%s\n", ok_labelname);
            break;
        case TLE:
            fprintf(outputfp, "\tJMI \t%s\n", ok_labelname);
            break;
        case TLEEQ:
            fprintf(outputfp, "\tJPL \t%s\n", ok_labelname);
            break;
        case TGR:
            fprintf(outputfp, "\tJPL \t%s\n", ok_labelname);
            break;
        case TGREQ:
            fprintf(outputfp, "\tJMI \t%s\n", ok_labelname);
            break;
        default:
            break;
    }
    if (opr == TLEEQ || opr == TGREQ) {
        fprintf(outputfp, "\tLAD \tgr1, 1\n");
    } else {
        fprintf(outputfp, "\tLD  \tgr1, gr0\n");
    }
    fprintf(outputfp, "\tJUMP\t%s\n", ng_labelname);
    fprintf(outputfp, "%s\n", ok_labelname);
    if (opr == TLEEQ || opr == TGREQ) {
        fprintf(outputfp, "\tLD  \tgr1, gr0\n");
    } else {
        fprintf(outputfp, "\tLAD \tgr1, 1\n");
    }
    fprintf(outputfp, "%s\n", ng_labelname);

    return NORMAL;
};

/* Generate code when it is necessary to hold the calculation result of the expression on the label */
int command_expression_by_call() {
    char *labelname = NULL;
    char output_buf_add[MAX_OUTPUT_BUF_SIZE];

    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
    fprintf(outputfp, "\tLAD \tgr2, %s\n", labelname);
    fprintf(outputfp, "\tST  \tgr1, 0, gr2\n");
    fprintf(outputfp, "\tPUSH\t0, gr2\n");
    init_char_array(output_buf_add, MAX_OUTPUT_BUF_SIZE);
    snprintf(output_buf_add, MAX_OUTPUT_BUF_SIZE, "%s\tDC  \t0\n", labelname);
    if ((strlen(label_buf) + strlen(output_buf_add)) >= MAX_OUTPUT_BUF_SIZE) {
        return error("Over BUF_SIZE for output");
    } else {
        strcat(label_buf, output_buf_add);
    }
    return NORMAL;
}

/* Generate code to calculate simple expression */
void command_simple_expression(int opr) {
    fprintf(outputfp, "\tPOP \tgr2\n");
    switch (opr) {
        case TPLUS:
            fprintf(outputfp, "\tADDA\tgr1, gr2\n");
            break;
        case TMINUS:
            fprintf(outputfp, "\tSUBA\tgr2, gr1\n");
            break;
        case TOR:
            fprintf(outputfp, "\tOR  \tgr1, gr2\n");
            break;
        default:
            break;
    }
    fprintf(outputfp, "\tJOV \tEOVF\n");
    if (opr == TMINUS) {
        fprintf(outputfp, "\tLD  \tgr1, gr2\n");
    }
    on_pl_flag(PLEOVF);
};

/* Generate code when '-' was attached before the term */
void command_minus() {
    fprintf(outputfp, "\tLAD \tgr2, -1\n");
    fprintf(outputfp, "\tMULA\tgr1, gr2\n");
    fprintf(outputfp, "\tJOV \tEOVF\n");
    on_pl_flag(PLEOVF);
}

/* Generate code to calculate terms */
void command_term(int opr) {
    fprintf(outputfp, "\tPOP \tgr2\n");
    switch (opr) {
        case TSTAR:
            fprintf(outputfp, "\tMULA\tgr1, gr2\n");
            break;
        case TDIV:
            fprintf(outputfp, "\tDIVA\tgr2, gr1\n");
            break;
        case TAND:
            fprintf(outputfp, "\tAND \tgr1, gr2\n");
            break;
        default:
            break;
    }
    if (opr == TDIV) {
        fprintf(outputfp, "\tJOV \tE0DIV\n");
        on_pl_flag(PLE0DIV);
        fprintf(outputfp, "\tLD  \tgr1, gr2\n");
    } else {
        fprintf(outputfp, "\tJOV \tEOVF\n");
        on_pl_flag(PLEOVF);
    }
};

/* Generate code to cast the type of factor */
int command_factor_cast(int cast_type, int expression_type) {
    char *labelname = NULL;

    switch (expression_type) {
        case TPINT:
            switch (cast_type) {
                case TPBOOL:
                    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
                    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
                    fprintf(outputfp, "\tJZE \t%s\n", labelname);
                    fprintf(outputfp, "\tLAD \tgr1, 1\n");
                    fprintf(outputfp, "%s\n", labelname);
                    break;
                case TPCHAR:
                    fprintf(outputfp, "\tLAD \tgr2, 127\n");
                    fprintf(outputfp, "\tAND \tgr1, gr2\n");
                    break;
                default:
                    break;
            }
            break;
        case TPBOOL:
            switch (cast_type) {
                case TPINT:
                    /* No processing required */
                    break;
                case TPCHAR:
                    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
                    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
                    fprintf(outputfp, "\tJZE \t%s\n", labelname);
                    fprintf(outputfp, "\tLAD \tgr1, 1\n");
                    fprintf(outputfp, "%s\n", labelname);
                    break;
                default:
                    break;
            }
            break;
        case TPCHAR:
            switch (cast_type) {
                case TPINT:
                    /* No processing required */
                    break;
                case TPBOOL:
                    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
                    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
                    fprintf(outputfp, "\tJZE \t%s\n", labelname);
                    fprintf(outputfp, "\tLAD \tgr1, 1\n");
                    fprintf(outputfp, "%s\n", labelname);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return NORMAL;
}

/* Generate code with logical negation */
void command_factor_not_factor() {
    fprintf(outputfp, "\tLAD \tgr2, 1\n");
    fprintf(outputfp, "\tXOR \tgr1, gr2\n");
}

/* Generate code indicating constant */
void command_constant_num(int num) {
    fprintf(outputfp, "\tLAD \tgr1, %d\n", num);
}

/* Generate code for reading character string */
void command_read_int() {
    fprintf(outputfp, "\tCALL\tREADINT\n");
    on_pl_flag(PLREADINT);
}

/* Generate code for reading character string */
void command_read_char() {
    fprintf(outputfp, "\tCALL\tREADCHAR\n");
    on_pl_flag(PLREADCHAR);
}

/* Generate code for reading new line */
void command_read_line() {
    fprintf(outputfp, "\tCALL\tREADLINE\n");
    on_pl_flag(PLREADLINE);
}

/* Generate code when output specification is expression */
void command_write_expression(int type, int length) {
    if (length > 0) {
        fprintf(outputfp, "\tLD  \tgr2, %d\n", length);
    } else {
        fprintf(outputfp, "\tLD  \tgr2, gr0\n");
    }
    fprintf(outputfp, "\tCALL\t");
    switch (type) {
        case TPINT:
            fprintf(outputfp, "WRITEINT\n");
            on_pl_flag(PLWRITEINT);
            break;
        case TPCHAR:
            fprintf(outputfp, "WRITECHAR\n");
            on_pl_flag(PLWRITECHAR);
            break;
        case TPBOOL:
            fprintf(outputfp, "WRITEBOOL\n");
            on_pl_flag(PLWRITEBOOL);
            break;
        default:
            break;
    }
};

/* Generate code for outputting character string */
int command_write_string(char *string) {
    char *labelname = NULL;
    char output_buf_add[MAX_OUTPUT_BUF_SIZE];

    /* Create a label and call WRITESTR */
    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
    fprintf(outputfp, "\tLAD \tgr1, %s\n", labelname);
    fprintf(outputfp, "\tLD  \tgr2, gr0\n");
    fprintf(outputfp, "\tCALL\tWRITESTR\n");
    on_pl_flag(PLWRITESTR);

    /* Define string at label location */
    init_char_array(output_buf_add, MAX_OUTPUT_BUF_SIZE);
    snprintf(output_buf_add, MAX_OUTPUT_BUF_SIZE, "%s\tDC  \t'%s'\n", labelname, string);
    if ((strlen(label_buf) + strlen(output_buf_add)) >= MAX_OUTPUT_BUF_SIZE) {
        return error("Over BUF_SIZE for output");
    } else {
        strcat(label_buf, output_buf_add);
    }

    free(labelname);
    labelname = NULL;

    return NORMAL;
}

/* Generate code for outputting new line */
void command_write_line() {
    fprintf(outputfp, "\tCALL\tWRITELINE\n");
    on_pl_flag(PLWRITELINE);
}

/* Generate code of return */
void command_return() {
    fprintf(outputfp, "\tRET\n");
}

/* Generate code of jump */
void command_jump(char *labelname) {
    fprintf(outputfp, "\tJUMP\t%s\n", labelname);
}

/* Generate code of push */
void command_push_gr1() {
    fprintf(outputfp, "\tPUSH\t0, gr1\n");
}

/* Generate code of load */
void command_ld_gr1_0_gr1() {
    fprintf(outputfp, "\tLD  \tgr1, 0, gr1\n");
}

/* Add label_buf to outputfp */
void output_label_buf() {
    fprintf(outputfp, "%s", label_buf);
}

/* Output Processing label */
void output_pl() {
    if (pl_flag[PLE0DIV - PL]) {
        fprintf(outputfp, "E0DIV\n");
        fprintf(outputfp, "\tJNZ \tEOVF\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tLAD \tgr1, E0DIV1\n");
        fprintf(outputfp, "\tLD  \tgr2, gr0\n");
        fprintf(outputfp, "\tCALL\tWRITESTR\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tSVC \t2\n");
        fprintf(outputfp, "E0DIV1  DC  '***** Run-Time Error : Zero-Divide *****'\n");
        on_pl_flag(PLEOVF);
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLEOVF - PL]) {
        fprintf(outputfp, "EOVF\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tLAD \tgr1, EOVF1\n");
        fprintf(outputfp, "\tLD  \tgr2, gr0\n");
        fprintf(outputfp, "\tCALL\tWRITESTR\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tSVC \t1\n");
        fprintf(outputfp, "EOVF1   DC  '***** Run-Time Error : Overfrow *****'\n");
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLEROV - PL]) {
        fprintf(outputfp, "EROV\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tLAD \tgr1, EROV1\n");
        fprintf(outputfp, "\tLD  \tgr2, gr0\n");
        fprintf(outputfp, "\tCALL\tWRITESTR\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tSVC \t3\n");
        fprintf(outputfp, "EROV1   DC  '***** Run-Time Error : Range-Over in Array Index *****'\n");
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLWRITECHAR - PL]) {
        fprintf(outputfp, "WRITECHAR\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tLD  \tgr6, SPACE\n");
        fprintf(outputfp, "\tLD  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "WC1\n");
        fprintf(outputfp, "\tSUBA\tgr2, ONE\n");
        fprintf(outputfp, "\tJZE \tWC2\n");
        fprintf(outputfp, "\tJMI \tWC2\n");
        fprintf(outputfp, "\tST  \tgr6, OBUF, gr7\n");
        fprintf(outputfp, "\tCALL\tBOVFCHECK\n");
        fprintf(outputfp, "\tJUMP\tWC1\n");
        fprintf(outputfp, "WC2\n");
        fprintf(outputfp, "\tST  \tgr1, OBUF, gr7\n");
        fprintf(outputfp, "\tCALL\tBOVFCHECK\n");
        fprintf(outputfp, "\tST  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLSPACE);
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLONE);
        on_pl_flag(PLOBUF);
        on_pl_flag(PLBOVFCHECK);
    }
    if (pl_flag[PLWRITEINT - PL]) {
        fprintf(outputfp, "WRITEINT\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tLD  \tgr7, gr0\n");
        fprintf(outputfp, "\tCPA \tgr1, gr0\n");
        fprintf(outputfp, "\tJPL \tWI1\n");
        fprintf(outputfp, "\tJZE \tWI1\n");
        fprintf(outputfp, "\tLD  \tgr4, gr0\n");
        fprintf(outputfp, "\tSUBA\tgr4, gr1\n");
        fprintf(outputfp, "\tCPA \tgr4, gr1\n");
        fprintf(outputfp, "\tJZE \tWI6\n");
        fprintf(outputfp, "\tLD  \tgr1, gr4\n");
        fprintf(outputfp, "\tLD  \tgr7, ONE\n");
        fprintf(outputfp, "WI1\n");
        fprintf(outputfp, "\tLD  \tgr6, SIX\n");
        fprintf(outputfp, "\tST  \tgr0, INTBUF, gr6\n");
        fprintf(outputfp, "\tSUBA\tgr6, ONE\n");
        fprintf(outputfp, "\tCPA \tgr1, gr0\n");
        fprintf(outputfp, "\tJNZ \tWI2\n");
        fprintf(outputfp, "\tLD  \tgr4, ZERO\n");
        fprintf(outputfp, "\tST  \tgr4, INTBUF, gr6\n");
        fprintf(outputfp, "\tJUMP\tWI5\n");
        fprintf(outputfp, "WI2\n");
        fprintf(outputfp, "\tCPA \tgr1, gr0\n");
        fprintf(outputfp, "\tJZE \tWI3\n");
        fprintf(outputfp, "\tLD  \tgr5, gr1\n");
        fprintf(outputfp, "\tDIVA\tgr1, TEN\n");
        fprintf(outputfp, "\tLD  \tgr4, gr1\n");
        fprintf(outputfp, "\tMULA\tgr4, TEN\n");
        fprintf(outputfp, "\tSUBA\tgr5, gr4\n");
        fprintf(outputfp, "\tADDA\tgr5, ZERO\n");
        fprintf(outputfp, "\tST  \tgr5, INTBUF, gr6\n");
        fprintf(outputfp, "\tSUBA\tgr6, ONE\n");
        fprintf(outputfp, "\tJUMP\tWI2\n");
        fprintf(outputfp, "WI3\n");
        fprintf(outputfp, "\tCPA \tgr7, gr0\n");
        fprintf(outputfp, "\tJZE \tWI4\n");
        fprintf(outputfp, "\tLD  \tgr4, MINUS\n");
        fprintf(outputfp, "\tST  \tgr4, INTBUF, gr6\n");
        fprintf(outputfp, "\tJUMP\tWI5\n");
        fprintf(outputfp, "WI4\n");
        fprintf(outputfp, "\tADDA\tgr6, ONE\n");
        fprintf(outputfp, "WI5\n");
        fprintf(outputfp, "\tLAD \tgr1, INTBUF, gr6\n");
        fprintf(outputfp, "\tCALL\tWRITESTR\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        fprintf(outputfp, "WI6\n");
        fprintf(outputfp, "\tLAD \tgr1, MMINT\n");
        fprintf(outputfp, "\tCALL\tWRITESTR\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        fprintf(outputfp, "MMINT   DC  '-32768'\n");
        on_pl_flag(PLONE);
        on_pl_flag(PLSIX);
        on_pl_flag(PLINTBUF);
        on_pl_flag(PLZERO);
        on_pl_flag(PLTEN);
        on_pl_flag(PLMINUS);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLWRITEBOOL - PL]) {
        fprintf(outputfp, "WRITEBOOL\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tCPA \tgr1, gr0\n");
        fprintf(outputfp, "\tJZE \tWB1\n");
        fprintf(outputfp, "\tLAD \tgr1, WBTRUE\n");
        fprintf(outputfp, "\tJUMP\tWB2\n");
        fprintf(outputfp, "WB1\n");
        fprintf(outputfp, "\tLAD \tgr1, WBFALSE\n");
        fprintf(outputfp, "WB2\n");
        fprintf(outputfp, "\tCALL\tWRITESTR\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        fprintf(outputfp, "WBTRUE  DC  'TRUE'\n");
        fprintf(outputfp, "WBFALSE DC  'FALSE'\n");
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLWRITESTR - PL]) {
        fprintf(outputfp, "WRITESTR\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tLD  \tgr6, gr1\n");
        fprintf(outputfp, "WS1\n");
        fprintf(outputfp, "\tLD  \tgr4, 0, gr6\n");
        fprintf(outputfp, "\tJZE \tWS2\n");
        fprintf(outputfp, "\tADDA\tgr6, ONE\n");
        fprintf(outputfp, "\tSUBA\tgr2, ONE\n");
        fprintf(outputfp, "\tJUMP\tWS1\n");
        fprintf(outputfp, "WS2\n");
        fprintf(outputfp, "\tLD  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "\tLD  \tgr5, SPACE\n");
        fprintf(outputfp, "WS3\n");
        fprintf(outputfp, "\tSUBA\tgr2, ONE\n");
        fprintf(outputfp, "\tJMI \tWS4\n");
        fprintf(outputfp, "\tST  \tgr5, OBUF, gr7\n");
        fprintf(outputfp, "\tCALL\tBOVFCHECK\n");
        fprintf(outputfp, "\tJUMP\tWS3\n");
        fprintf(outputfp, "WS4\n");
        fprintf(outputfp, "\tLD  \tgr4, 0, gr1\n");
        fprintf(outputfp, "\tJZE \tWS5\n");
        fprintf(outputfp, "\tST  \tgr4, OBUF, gr7\n");
        fprintf(outputfp, "\tADDA\tgr1, ONE\n");
        fprintf(outputfp, "\tCALL\tBOVFCHECK\n");
        fprintf(outputfp, "\tJUMP\tWS4\n");
        fprintf(outputfp, "WS5\n");
        fprintf(outputfp, "\tST  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLONE);
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLSPACE);
        on_pl_flag(PLOBUF);
        on_pl_flag(PLBOVFCHECK);
    }
    if (pl_flag[PLBOVFCHECK - PL]) {
        fprintf(outputfp, "BOVFCHECK\n");
        fprintf(outputfp, "\tADDA\tgr7, ONE\n");
        fprintf(outputfp, "\tCPA \tgr7, BOVFLEVEL\n");
        fprintf(outputfp, "\tJMI \tBOVF1\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "\tLD  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "BOVF1\n");
        fprintf(outputfp, "\tRET\n");
        fprintf(outputfp, "BOVFLEVEL   DC  256\n");
        on_pl_flag(PLONE);
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLOBUFSIZE);
    }
    if (pl_flag[PLFLUSH - PL]) {
        fprintf(outputfp, "FLUSH\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tLD  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "\tJZE \tFL1\n");
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        fprintf(outputfp, "FL1\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLWRITELINE);
    }
    if (pl_flag[PLWRITELINE - PL]) {
        fprintf(outputfp, "WRITELINE\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tLD  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "\tLD  \tgr6, NEWLINE\n");
        fprintf(outputfp, "\tST  \tgr6, OBUF, gr7\n");
        fprintf(outputfp, "\tADDA\tgr7, ONE\n");
        fprintf(outputfp, "\tST  \tgr7, OBUFSIZE\n");
        fprintf(outputfp, "\tOUT \tOBUF, OBUFSIZE\n");
        fprintf(outputfp, "\tST  \tgr0, OBUFSIZE\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLNEWLINE);
        on_pl_flag(PLOBUF);
        on_pl_flag(PLONE);
    }
    if (pl_flag[PLREADINT - PL]) {
        fprintf(outputfp, "READINT\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "RI1\n");
        fprintf(outputfp, "\tCALL\tREADCHAR\n");
        fprintf(outputfp, "\tLD  \tgr7, 0, gr1\n");
        fprintf(outputfp, "\tCPA \tgr7, SPACE\n");
        fprintf(outputfp, "\tJZE \tRI1\n");
        fprintf(outputfp, "\tCPA \tgr7, TAB\n");
        fprintf(outputfp, "\tJZE \tRI1\n");
        fprintf(outputfp, "\tCPA \tgr7, NEWLINE\n");
        fprintf(outputfp, "\tJZE \tRI1\n");
        fprintf(outputfp, "\tLD  \tgr5, ONE\n");
        fprintf(outputfp, "\tCPA \tgr7, MINUS\n");
        fprintf(outputfp, "\tJNZ \tRI4\n");
        fprintf(outputfp, "\tLD  \tgr5, gr0\n");
        fprintf(outputfp, "\tCALL\tREADCHAR\n");
        fprintf(outputfp, "\tLD  \tgr7, 0, gr1\n");
        fprintf(outputfp, "RI4\n");
        fprintf(outputfp, "\tLD  \tgr6, gr0\n");
        fprintf(outputfp, "RI2\n");
        fprintf(outputfp, "\tCPA \tgr7, ZERO\n");
        fprintf(outputfp, "\tJMI \tRI3\n");
        fprintf(outputfp, "\tCPA \tgr7, NINE\n");
        fprintf(outputfp, "\tJPL \tRI3\n");
        fprintf(outputfp, "\tMULA\tgr6, TEN\n");
        fprintf(outputfp, "\tADDA\tgr6, gr7\n");
        fprintf(outputfp, "\tSUBA\tgr6, ZERO\n");
        fprintf(outputfp, "\tCALL\tREADCHAR\n");
        fprintf(outputfp, "\tLD  \tgr7, 0, gr1\n");
        fprintf(outputfp, "\tJUMP\tRI2\n");
        fprintf(outputfp, "RI3\n");
        fprintf(outputfp, "\tST  \tgr7, RPBBUF\n");
        fprintf(outputfp, "\tST  \tgr6, 0, gr1\n");
        fprintf(outputfp, "\tCPA \tgr5, gr0\n");
        fprintf(outputfp, "\tJNZ \tRI5\n");
        fprintf(outputfp, "\tSUBA\tgr5, gr6\n");
        fprintf(outputfp, "\tST  \tgr5, 0, gr1\n");
        fprintf(outputfp, "RI5\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLREADCHAR);
        on_pl_flag(PLSPACE);
        on_pl_flag(PLTAB);
        on_pl_flag(PLNEWLINE);
        on_pl_flag(PLONE);
        on_pl_flag(PLMINUS);
        on_pl_flag(PLZERO);
        on_pl_flag(PLNINE);
        on_pl_flag(PLTEN);
        on_pl_flag(PLRPBBUF);
    }
    if (pl_flag[PLREADCHAR - PL]) {
        fprintf(outputfp, "READCHAR\n");
        fprintf(outputfp, "\tRPUSH\n");
        fprintf(outputfp, "\tLD  \tgr5, RPBBUF\n");
        fprintf(outputfp, "\tJZE \tRC0\n");
        fprintf(outputfp, "\tST  \tgr5, 0, gr1\n");
        fprintf(outputfp, "\tST  \tgr0, RPBBUF\n");
        fprintf(outputfp, "\tJUMP\tRC3\n");
        fprintf(outputfp, "RC0\n");
        fprintf(outputfp, "\tLD  \tgr7, INP\n");
        fprintf(outputfp, "\tLD  \tgr6, IBUFSIZE\n");
        fprintf(outputfp, "\tJNZ \tRC1\n");
        fprintf(outputfp, "\tIN  \tIBUF, IBUFSIZE\n");
        fprintf(outputfp, "\tLD  \tgr7, gr0\n");
        fprintf(outputfp, "RC1\n");
        fprintf(outputfp, "\tCPA \tgr7, IBUFSIZE\n");
        fprintf(outputfp, "\tJNZ \tRC2\n");
        fprintf(outputfp, "\tLD  \tgr5, NEWLINE\n");
        fprintf(outputfp, "\tST  \tgr5, 0, gr1\n");
        fprintf(outputfp, "\tST  \tgr0, IBUFSIZE\n");
        fprintf(outputfp, "\tST  \tgr0, INP\n");
        fprintf(outputfp, "\tJUMP\tRC3\n");
        fprintf(outputfp, "RC2\n");
        fprintf(outputfp, "\tLD  \tgr5, IBUF, gr7\n");
        fprintf(outputfp, "\tADDA\tgr7, ONE\n");
        fprintf(outputfp, "\tST  \tgr5, 0, gr1\n");
        fprintf(outputfp, "\tST  \tgr7, INP\n");
        fprintf(outputfp, "RC3\n");
        fprintf(outputfp, "\tRPOP\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLRPBBUF);
        on_pl_flag(PLINP);
        on_pl_flag(PLIBUFSIZE);
        on_pl_flag(PLIBUF);
        on_pl_flag(PLNEWLINE);
        on_pl_flag(PLONE);
    }
    if (pl_flag[PLREADLINE - PL]) {
        fprintf(outputfp, "READLINE\n");
        fprintf(outputfp, "\tST  \tgr0, IBUFSIZE\n");
        fprintf(outputfp, "\tST  \tgr0, INP\n");
        fprintf(outputfp, "\tST  \tgr0, RPBBUF\n");
        fprintf(outputfp, "\tRET\n");
        on_pl_flag(PLIBUFSIZE);
        on_pl_flag(PLINP);
        on_pl_flag(PLRPBBUF);
    }
    if (pl_flag[PLONE - PL]) {
        fprintf(outputfp, "ONE         DC  1\n");
    }
    if (pl_flag[PLSIX - PL]) {
        fprintf(outputfp, "SIX         DC  6\n");
    }
    if (pl_flag[PLTEN - PL]) {
        fprintf(outputfp, "TEN         DC  10\n");
    }
    if (pl_flag[PLSPACE - PL]) {
        fprintf(outputfp, "SPACE       DC  #0020\n");
    }
    if (pl_flag[PLMINUS - PL]) {
        fprintf(outputfp, "MINUS       DC  #002D\n");
    }
    if (pl_flag[PLTAB - PL]) {
        fprintf(outputfp, "TAB         DC  #0009\n");
    }
    if (pl_flag[PLZERO - PL]) {
        fprintf(outputfp, "ZERO        DC  #0030\n");
    }
    if (pl_flag[PLNINE - PL]) {
        fprintf(outputfp, "NINE        DC  #0039\n");
    }
    if (pl_flag[PLNEWLINE - PL]) {
        fprintf(outputfp, "NEWLINE     DC  #000A\n");
    }
    if (pl_flag[PLINTBUF - PL]) {
        fprintf(outputfp, "INTBUF      DS  8\n");
    }
    if (pl_flag[PLOBUFSIZE - PL]) {
        fprintf(outputfp, "OBUFSIZE    DC  0\n");
    }
    if (pl_flag[PLIBUFSIZE - PL]) {
        fprintf(outputfp, "IBUFSIZE    DC  0\n");
    }
    if (pl_flag[PLINP - PL]) {
        fprintf(outputfp, "INP         DC  0\n");
    }
    if (pl_flag[PLOBUF - PL]) {
        fprintf(outputfp, "OBUF        DS  257\n");
    }
    if (pl_flag[PLIBUF - PL]) {
        fprintf(outputfp, "IBUF        DS  257\n");
    }
    if (pl_flag[PLRPBBUF - PL]) {
        fprintf(outputfp, "RPBBUF      DC  0\n");
    }
}

void on_pl_flag(int plnum) {
    pl_flag[plnum - PL] = 1;
}