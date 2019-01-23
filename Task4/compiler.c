#include "compiler.h"

int labelnum = 0;

int pl_flag[NUMOFPL + 1];

char label_buf[MAX_OUTPUT_BUF_SIZE];

char *exit_label = NULL;

/* prototype declaration */

/* Call before output, open the file,
 * Success:0 Failed:-1 */
int init_outputfile(char *inputfilename, FILE **fp) {
    char *outputfilename;
    char outputfilename_extension[32];

    outputfilename = strtok(inputfilename, ".");
    sprintf(outputfilename_extension, "%.27s.csl", outputfilename);

    /* The file pointer is received as a pointer to the pointer */
    *fp = fopen(outputfilename_extension, "w");

    if (*fp == NULL) {
        fprintf(stderr, "\nERROR: File %s can not open.\n", outputfilename_extension);
        return -1;
    }

    init_int_array(pl_flag, NUMOFPL);

    return 0;

}

/* Close the call file after output */
void end_output(FILE *fp) {
    fprintf(fp, "\tEND\n");
    if (fclose(fp) == EOF) {
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
int create_id_label(struct ID *p, FILE *outputfp) {
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

/* Generate code at program start */
int command_start(FILE *fp, char *program_name, char **labelname) {

    /* program start */
    fprintf(fp, "$$%s\tSTART\n", program_name);
    /* Initialize gr 0 to 0 */
    fprintf(fp, "\tLAD \tgr0, 0\n");
    /* Call a label indicating compound statement of main program */
    if (create_newlabel(labelname) == ERROR) { return ERROR; }
    fprintf(fp, "\tCALL\t%s\n", *labelname);
    /* End processing */
    fprintf(fp, "\tCALL\tFLUSH\n");
    on_pl_flag(PLFLUSH);
    fprintf(fp, "\tSVC \t0\n");

    return NORMAL;
}

/* Generate code to process arguments */
int command_process_arguments(FILE *outputfp) {
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

/* Generate code of if statement */
int command_condition_statement(FILE *outputfp, char *if_labelname) {
    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
    fprintf(outputfp, "\tJZE \t%s\n", if_labelname);

};

/* Generate code for outputting character string
 * When this function is called in a call statement,
 * it reads the address to gr1 */
int command_variable(FILE *outputfp, char *name, char *procname, int is_incall) {
    struct ID *p;

    if ((p = search_idtab(name, procname, 1)) == NULL) {
        return error("%s is not defined.", current_procname);
    }
    if (is_incall == 1) {
        fprintf(outputfp, "\tLAD  \tgr1, $%s", p->name);
    } else {
        fprintf(outputfp, "\tLD  \tgr1, $%s", p->name);
    }
    if (p->procname != NULL) {
        fprintf(outputfp, "%%%s", p->procname);
    }
    fprintf(outputfp, "\n");

    return NORMAL;
}

/* Generate code to calculate expression */
int command_expression(FILE *outputfp, int opr) {
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
            //todo →いらないのか　fprintf(outputfp, "\tJZE \t%s\n", ok_labelname);
            break;
        case TGR:
            fprintf(outputfp, "\tJPL \t%s\n", ok_labelname);
            break;
        case TGREQ:
            fprintf(outputfp, "\tJMI \t%s\n", ok_labelname);
            //todo →いらないのか　fprintf(outputfp, "\tJZE \t%s\n", ok_labelname);
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
};

/* Generate code to calculate simple expression */
void command_simple_expression(FILE *outputfp, int opr) {
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
    }
    fprintf(outputfp, "\tJOV \tEOVF\n");
    if (opr == TMINUS) {
        fprintf(outputfp, "\tLD  \tgr1, gr2\n");
    }
    on_pl_flag(PLEOVF);
};

/* Generate code to calculate terms */
void command_term(FILE *outputfp, int opr) {
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

int command_factor_cast(FILE *outputfp, int cast_type, int expression_type) {
    char *labelname[2] = {NULL};

    switch (expression_type) {
        case TPINT:
            switch (cast_type) {
                case TPBOOL:
                    if (create_newlabel(&labelname[0]) == ERROR) { return ERROR; }
                    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
                    fprintf(outputfp, "\tJZE \t%s\n", labelname[0]);
                    fprintf(outputfp, "\tLAD \tgr1, 1\n");
                    fprintf(outputfp, "%s\n", labelname[0]);
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
                    // todo 文字を0,1と表示するのであってるのか確認
                    if (create_newlabel(&labelname[0]) == ERROR) { return ERROR; }
                    if (create_newlabel(&labelname[1]) == ERROR) { return ERROR; }
                    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
                    fprintf(outputfp, "\tJZE \t%s\n", labelname[0]);
                    fprintf(outputfp, "\tLAD \tgr1, 49\n");
                    fprintf(outputfp, "\tJUMP\t%s\n", labelname[1]);
                    fprintf(outputfp, "%s\n", labelname[0]);
                    fprintf(outputfp, "\tLAD \tgr1, 48\n");
                    fprintf(outputfp, "%s\n", labelname[1]);
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
                    if (create_newlabel(&labelname[0]) == ERROR) { return ERROR; }
                    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
                    fprintf(outputfp, "\tJZE \t%s\n", labelname[0]);
                    fprintf(outputfp, "\tLAD \tgr1, 1\n");
                    fprintf(outputfp, "%s\n", labelname[0]);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* Generate code indicating constant
 * Argument is the value of the constant
 * true = 1; false = 0;*/
void command_constant_num(FILE *outputfp, int num) {
    if (num == 0) {
        fprintf(outputfp, "\tLD 　\tgr1, gr0\n");
    } else {
        fprintf(outputfp, "\tLAD \tgr1, %d\n", num);
    }
}

/* Generate code for outputting character string */
void command_read_int(FILE *outputfp) {
    fprintf(outputfp, "\tCALL\tREADINT\n");
    on_pl_flag(PLREADINT);
}

/* Generate code when output specification is expression */
void command_write_expression(FILE *outputfp, int type, int length) {
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
    }
};

/* Generate code for outputting character string */
int command_write_string(FILE *outputfp, char *string) {
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

/* Add label_buf to outputfp */
void output_label_buf(FILE *outputfp) {
    fprintf(outputfp, "%s", label_buf);
}

/* Output Processing label */
void output_pl(FILE *outputfp) {
    if (pl_flag[PLE0DIV - PL]) {
        fprintf(outputfp, "E0DIV\n");
        fprintf(outputfp, "  JNZ     EOVF\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  LAD     gr1, E0DIV1\n");
        fprintf(outputfp, "  LD      gr2, gr0\n");
        fprintf(outputfp, "  CALL    WRITESTR\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  SVC     2\n");
        fprintf(outputfp, "E0DIV1  DC  '***** Run-Time Error : Zero-Divide *****'\n");
        on_pl_flag(PLEOVF);
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLEOVF - PL]) {
        fprintf(outputfp, "EOVF\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  LAD     gr1, EOVF1\n");
        fprintf(outputfp, "  LD      gr2, gr0\n");
        fprintf(outputfp, "  CALL    WRITESTR\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  SVC     1\n");
        fprintf(outputfp, "EOVF1   DC  '***** Run-Time Error : Overfrow *****'\n");
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLEROV - PL]) {
        fprintf(outputfp, "EROV\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  LAD     gr1, EROV1\n");
        fprintf(outputfp, "  LD      gr2, gr0\n");
        fprintf(outputfp, "  CALL    WRITESTR\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  SVC     3\n");
        fprintf(outputfp, "EROV1  DC  '***** Run-Time Error : Range-Over in Array Index *****'\n");
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLWRITECHAR - PL]) {
        fprintf(outputfp, "WRITECHAR\n");
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  LD      gr6, SPACE\n");
        fprintf(outputfp, "  LD      gr7, OBUFSIZE\n");
        fprintf(outputfp, "WC1\n");
        fprintf(outputfp, "  SUBA    gr2, ONE\n");
        fprintf(outputfp, "  JZE     WC2\n");
        fprintf(outputfp, "  JMI     WC2\n");
        fprintf(outputfp, "  ST      gr6, OBUF, gr7\n");
        fprintf(outputfp, "  CALL    BOVFCHECK\n");
        fprintf(outputfp, "  JUMP    WC1\n");
        fprintf(outputfp, "WC2\n");
        fprintf(outputfp, "  ST      gr1, OBUF, gr7\n");
        fprintf(outputfp, "  CALL    BOVFCHECK\n");
        fprintf(outputfp, "  ST      gr7, OBUFSIZE\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "RET\n");
        on_pl_flag(PLSPACE);
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLONE);
        on_pl_flag(PLOBUF);
        on_pl_flag(PLBOVFCHECK);
    }
    if (pl_flag[PLWRITEINT - PL]) {
        fprintf(outputfp, "WRITEINT\n");
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  LD      gr7, gr0\n");
        fprintf(outputfp, "  CPA     gr1, gr0\n");
        fprintf(outputfp, "  JPL     WI1\n");
        fprintf(outputfp, "  JZE     WI1\n");
        fprintf(outputfp, "  LD      gr4, gr0\n");
        fprintf(outputfp, "  SUBA    gr4, gr1\n");
        fprintf(outputfp, "  CPA     gr4, gr1\n");
        fprintf(outputfp, "  JZE     WI6\n");
        fprintf(outputfp, "  LD      gr1, gr4\n");
        fprintf(outputfp, "  LD      gr7, ONE\n");
        fprintf(outputfp, "WI1\n");
        fprintf(outputfp, "  LD      gr6, SIX\n");
        fprintf(outputfp, "  ST      gr0, INTBUF, gr6\n");
        fprintf(outputfp, "  SUBA    gr6, ONE\n");
        fprintf(outputfp, "  CPA     gr1, gr0\n");
        fprintf(outputfp, "  JNZ     WI2\n");
        fprintf(outputfp, "  LD      gr4, ZERO\n");
        fprintf(outputfp, "  ST      gr4, INTBUF, gr6\n");
        fprintf(outputfp, "  JUMP    W15\n");
        fprintf(outputfp, "WI2\n");
        fprintf(outputfp, "  CPA     gr1, gr0\n");
        fprintf(outputfp, "  JZE     WI3\n");
        fprintf(outputfp, "  LD      gr5, gr1\n");
        fprintf(outputfp, "  DIVA    gr1, TEN\n");
        fprintf(outputfp, "  LD      gr4, gr1\n");
        fprintf(outputfp, "  MULA    gr4, TEN\n");
        fprintf(outputfp, "  SUBA    gr5, gr4\n");
        fprintf(outputfp, "  ADDA    gr5, ZERO\n");
        fprintf(outputfp, "  ST      gr5, INTBUF, gr6\n");
        fprintf(outputfp, "  SUBA    gr6, ONE\n");
        fprintf(outputfp, "  JUMP    WI2\n");
        fprintf(outputfp, "WI3\n");
        fprintf(outputfp, "  CPA     gr7, gr0\n");
        fprintf(outputfp, "  JZE     WI4\n");
        fprintf(outputfp, "  LD      gr4, MINUS\n");
        fprintf(outputfp, "  ST      gr4, INTBUF, gr6\n");
        fprintf(outputfp, "  JUMP    WI5\n");
        fprintf(outputfp, "WI4\n");
        fprintf(outputfp, "  ADDA    gr6, ONE\n");
        fprintf(outputfp, "WI5\n");
        fprintf(outputfp, "  LAD     gr1, INTBUF, gr6\n");
        fprintf(outputfp, "  CALL    WRITESTR\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        fprintf(outputfp, "WI6\n");
        fprintf(outputfp, "  LAD     gr1, MMINT\n");
        fprintf(outputfp, "  CALL    WRITESTR\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        fprintf(outputfp, "MMINT     DC  '-32768'\n");
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
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  CPA     gr1, gr0\n");
        fprintf(outputfp, "  JZE     WB1\n");
        fprintf(outputfp, "  LAD     gr1, WBTRUE\n");
        fprintf(outputfp, "  JAMP    WB2\n");
        fprintf(outputfp, "WB1\n");
        fprintf(outputfp, "  LAD     gr1, WBFALSE\n");
        fprintf(outputfp, "WB2\n");
        fprintf(outputfp, "  CALL    WRITESTR\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        fprintf(outputfp, "WBTRUE  DC  'TRUE'\n");
        fprintf(outputfp, "WBFALSE DC  'FALSE'\n");
        on_pl_flag(PLWRITESTR);
    }
    if (pl_flag[PLWRITESTR - PL]) {
        fprintf(outputfp, "WRITESTR\n");
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  LD      gr6, gr1\n");
        fprintf(outputfp, "WS1\n");
        fprintf(outputfp, "  LD      gr4, 0, gr6\n");
        fprintf(outputfp, "  JZE     WS2\n");
        fprintf(outputfp, "  ADDA    gr6, ONE\n");
        fprintf(outputfp, "  SUBA    gr2, ONE\n");
        fprintf(outputfp, "  JUMP    WS1\n");
        fprintf(outputfp, "WS2\n");
        fprintf(outputfp, "  LD      gr7, OBUFSIZE\n");
        fprintf(outputfp, "  LD      gr5, SPACE\n");
        fprintf(outputfp, "WS3\n");
        fprintf(outputfp, "  SUBA    gr2, ONE\n");
        fprintf(outputfp, "  JMI     WS4\n");
        fprintf(outputfp, "  ST      gr5, OBUF, gr7\n");
        fprintf(outputfp, "  CALL    BOVFCHECK\n");
        fprintf(outputfp, "  JUMP    WS3\n");
        fprintf(outputfp, "WS4\n");
        fprintf(outputfp, "  LD      gr4, 0, gr1\n");
        fprintf(outputfp, "  JZE     WS5\n");
        fprintf(outputfp, "  ST      gr4, OBUF, gr7\n");
        fprintf(outputfp, "  ADDA    gr1, ONE\n");
        fprintf(outputfp, "  CALL    BOVFCHECK\n");
        fprintf(outputfp, "  JUMP    WS4\n");
        fprintf(outputfp, "WS5\n");
        fprintf(outputfp, "  ST      gr7, OBUFSIZE\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        on_pl_flag(PLONE);
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLSPACE);
        on_pl_flag(PLOBUF);
        on_pl_flag(PLBOVFCHECK);
    }
    if (pl_flag[PLBOVFCHECK - PL]) {
        fprintf(outputfp, "BOVFCHECK\n");
        fprintf(outputfp, "  ADDA    gr7, ONE\n");
        fprintf(outputfp, "  CPA     gr7, BOVFLEVEL\n");
        fprintf(outputfp, "  JMI     BOVF1\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "  LD      gr7, OBUFSIZE\n");
        fprintf(outputfp, "BOVF1\n");
        fprintf(outputfp, "  RET\n");
        fprintf(outputfp, "BOVFLEVEL\n");
        on_pl_flag(PLONE);
        on_pl_flag(PLWRITELINE);
        on_pl_flag(PLOBUFSIZE);
    }
    if (pl_flag[PLFLUSH - PL]) {
        fprintf(outputfp, "FLUSH\n");
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  LD      gr7, OBUFSIZE\n");
        fprintf(outputfp, "  JZE     FL1\n");
        fprintf(outputfp, "  CALL    WRITELINE\n");
        fprintf(outputfp, "FL1\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLWRITELINE);
    }
    if (pl_flag[PLWRITELINE - PL]) {
        fprintf(outputfp, "WRITELINE\n");
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  LD      gr7, OBUFSIZE\n");
        fprintf(outputfp, "  LD      gr6, NEWLINE\n");
        fprintf(outputfp, "  ST      gr6, OBUF, gr7\n");
        fprintf(outputfp, "  ADDA    gr7, ONE\n");
        fprintf(outputfp, "  ST      gr7, OBUFSIZE\n");
        fprintf(outputfp, "  OUT     OBUF, OBUFSIZE\n");
        fprintf(outputfp, "  ST      gr0, OBUFSIZE\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        on_pl_flag(PLOBUFSIZE);
        on_pl_flag(PLNEWLINE);
        on_pl_flag(PLOBUF);
        on_pl_flag(PLONE);
    }
    if (pl_flag[PLREADINT - PL]) {
        fprintf(outputfp, "READINT\n");
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "RI1\n");
        fprintf(outputfp, "  CALL    READCHAR\n");
        fprintf(outputfp, "  LD      gr7, 0, gr1\n");
        fprintf(outputfp, "  CPA     gr7, SPACE\n");
        fprintf(outputfp, "  JZE     RI1\n");
        fprintf(outputfp, "  CPA     gr7, TAB\n");
        fprintf(outputfp, "  JZE     RI1\n");
        fprintf(outputfp, "  CPA     gr7, NEWLINE\n");
        fprintf(outputfp, "  JZE     RI1\n");
        fprintf(outputfp, "  LD      gr5, ONE\n");
        fprintf(outputfp, "  CPA     gr7, MINUS\n");
        fprintf(outputfp, "  JNZ     RI4\n");
        fprintf(outputfp, "  LD      gr5, gr0\n");
        fprintf(outputfp, "  CALL    READCHAR\n");
        fprintf(outputfp, "  LD      gr7, 0, gr1\n");
        fprintf(outputfp, "RI4\n");
        fprintf(outputfp, "  LD      gr6, gr0\n");
        fprintf(outputfp, "RI2\n");
        fprintf(outputfp, "  CPA     gr7, ZERO\n");
        fprintf(outputfp, "  JMI     RI3\n");
        fprintf(outputfp, "  CPA     gr7, NINE\n");
        fprintf(outputfp, "  JPL     RI3\n");
        fprintf(outputfp, "  MULA    gr6, TEN\n");
        fprintf(outputfp, "  ADDA    gr6, gr7\n");
        fprintf(outputfp, "  SUBA    gr6, ZERO\n");
        fprintf(outputfp, "  CALL    READCHAR\n");
        fprintf(outputfp, "  LD      gr7, 0, gr1\n");
        fprintf(outputfp, "  JUMP    RI2\n");
        fprintf(outputfp, "RI3\n");
        fprintf(outputfp, "  ST      gr7, RPBBUF\n");
        fprintf(outputfp, "  ST      gr6, 0, gr1\n");
        fprintf(outputfp, "  CPA     gr5, gr0\n");
        fprintf(outputfp, "  JNZ     RI5\n");
        fprintf(outputfp, "  SUBA    gr5, gr6\n");
        fprintf(outputfp, "  ST      gr5, 0, gr1\n");
        fprintf(outputfp, "RI5\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
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
        fprintf(outputfp, "  RPUSH\n");
        fprintf(outputfp, "  LD      gr5, RPBBUF\n");
        fprintf(outputfp, "  JZE     RC0\n");
        fprintf(outputfp, "  ST      gr5, 0, gr1\n");
        fprintf(outputfp, "  ST      gr0, RPBBUF\n");
        fprintf(outputfp, "  JUMP    RC3\n");
        fprintf(outputfp, "RC0\n");
        fprintf(outputfp, "  LD      gr7, INP\n");
        fprintf(outputfp, "  LD      gr6, IBUFSIZE\n");
        fprintf(outputfp, "  JNZ     RC1\n");
        fprintf(outputfp, "  IN      IBUF, IBUFSIZE\n");
        fprintf(outputfp, "  LD      gr7, gr0\n");
        fprintf(outputfp, "RC1\n");
        fprintf(outputfp, "  CPA     gr7, IBUFSIZE\n");
        fprintf(outputfp, "  JNZ     RC2\n");
        fprintf(outputfp, "  LD      gr5, NEWLINE\n");
        fprintf(outputfp, "  ST      gr5, 0, gr1\n");
        fprintf(outputfp, "  ST      gr0, IBUFSIZE\n");
        fprintf(outputfp, "  ST      gr0, INP\n");
        fprintf(outputfp, "  JUMP    RC3\n");
        fprintf(outputfp, "RC2\n");
        fprintf(outputfp, "  LD      gr5, IBUF, gr7\n");
        fprintf(outputfp, "  ADDA    gr7, ONE\n");
        fprintf(outputfp, "  ST      gr5, 0, gr1\n");
        fprintf(outputfp, "  ST      gr7, INP\n");
        fprintf(outputfp, "RC3\n");
        fprintf(outputfp, "  RPOP\n");
        fprintf(outputfp, "  RET\n");
        on_pl_flag(PLRPBBUF);
        on_pl_flag(PLINP);
        on_pl_flag(PLIBUFSIZE);
        on_pl_flag(PLIBUF);
        on_pl_flag(PLNEWLINE);
        on_pl_flag(PLONE);
    }
    if (pl_flag[PLREADLINE - PL]) {
        fprintf(outputfp, "READLINE\n");
        fprintf(outputfp, "  ST      gr0, IBUFSIZE\n");
        fprintf(outputfp, "  ST      gr0, INP\n");
        fprintf(outputfp, "  ST      gr0, RPBBUF\n");
        fprintf(outputfp, "  RET\n");
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
        fprintf(outputfp, "NINE        DC  #0029\n");
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