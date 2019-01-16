#include "compiler.h"

int labelnum = 0;

int pl_flag[NUMOFPL + 1];

char label_buf[MAX_OUTPUT_BUF_SIZE];

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
int command_start(FILE *fp, char *program_name) {
    char *labelname = NULL;

    /* program start */
    fprintf(fp, "$$%s\tSTART\n", program_name);
    /* Initialize gr 0 to 0 */
    fprintf(fp, "\tLAD \tgr0, 0\n");
    /* Call a label indicating compound statement of main program */
    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
    fprintf(fp, "\tCALL\t%s\n", labelname);
    /* End processing */
    fprintf(fp, "\tCALL\tFLUSH\n");
    on_pl_flag(PLFLUSH);
    fprintf(fp, "\tSVC \t0\n");

    free(labelname);
    labelname = NULL;

    return NORMAL;
}

int command_process_arguments(FILE *outputfp, struct ID *p) {
    //todo
}

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