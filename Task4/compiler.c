#include "compiler.h"

int init_outputfile(char *inputfilename, FILE **fp) {
    char *outputfilename;
    char outputfilename_extension[32];

    outputfilename = strtok(inputfilename, ".");
    sprintf(outputfilename_extension, "%.27s.csl", outputfilename);
    printf("%s is outputed.", outputfilename_extension);

    /* The file pointer is received as a pointer to the pointer */
    *fp = fopen(outputfilename_extension, "w");

    if (*fp == NULL) {
        fprintf(stderr, "\nERROR: File %s can not open.\n", outputfilename_extension);
        return -1;
    }

    return 0;

}

void command_start(FILE *fp, char *program_name){
    fprintf(fp, "$$%s    START\n", program_name);
}

/* Close the call file after output */
void end_output(FILE *fp) {
    if (fclose(fp) == EOF) {
        fprintf(stderr, "\nERROR: Output file can not close.\n");
    };
}