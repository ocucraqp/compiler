#include "compiler.h"

int labelnum = 0;

/* prototype declaration */
int create_newlabel(char **labelname);

int init_outputfile(char *inputfilename, FILE **fp) {
    char *outputfilename;
    char outputfilename_extension[32];

    outputfilename = strtok(inputfilename, ".");
    sprintf(outputfilename_extension, "%.27s.csl", outputfilename);
    printf("%s\n", outputfilename_extension);

    /* The file pointer is received as a pointer to the pointer */
    *fp = fopen(outputfilename_extension, "w");

    if (*fp == NULL) {
        fprintf(stderr, "\nERROR: File %s can not open.\n", outputfilename_extension);
        return -1;
    }

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
    char newlabel[LABELSIZE];

    labelnum++;
    if (labelnum > 9999) {
        return error("Too many labels");
    }
    snprintf(newlabel, LABELSIZE, "L%04d", labelnum);
    *labelname = newlabel;
    return NORMAL;
}

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
                    fprintf(outputfp, "    DC      0");
                    break;
                case TPARRAYINT:
                case TPARRAYCHAR:
                case TPARRAYBOOL:
                    fprintf(outputfp, "    DS      %d", p->itp->arraysize);
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
    char *labelname = "L0000";

    /* program start */
    fprintf(fp, "$$%s    START\n", program_name);
    /* Initialize gr 0 to 0 */
    fprintf(fp, "        LAD     gr0,0\n");
    /* Call a label indicating compound statement of main program */
    if (create_newlabel(&labelname) == ERROR) { return ERROR; }
    fprintf(fp, "        CALL    %s\n", labelname);
    /* End processing */
    fprintf(fp, "        CALL    FLUSH\n");
    fprintf(fp, "        SVC     0\n");

    return NORMAL;
}