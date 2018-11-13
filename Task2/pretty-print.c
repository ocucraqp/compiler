#include "pretty-print.h"

int token = 0;

/* string of each token */
char *tokenstr[NUMOFTOKEN + 1] = {
        "",
        "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
        "else", "procedure", "return", "call", "while", "do", "not", "or",
        "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
        "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
        ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read", "write", "break"
};

/* */
int parse_program(FILE *fp) {
    if (token != TPROGRAM) { return (error("Keyword 'program' is not found")); }
    printf("%s", tokenstr[token]);
    token = scan(fp);
    if (token != TNAME) { return (error("Program name is not fount")); }
    printf(" %s", string_attr);
    token = scan(fp);
    if (token != TSEMI) { return (error("Semicolon is not found")); }
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    if (parse_block(fp) == ERROR) { return ERROR; }
    if (token != TDOT) { return (error("Period is not found at the end of program")); }
    printf("%s", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_block(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_variable_declaration(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_variable_names(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_variable_name(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_type(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_standard_type(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_array_type(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_subprogram_declaration(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_procedure_name(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_formal_parameters(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_compound_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_condition_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_iteration_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_exit_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_call_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_expressions(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_return_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_assignment_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_left_part(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_variabble(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_experssion(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_simple_expression(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_term(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_factor(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_constant(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_multiplicative_operator(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_additive_operator(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_relational_operator(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_input_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_output_statement(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_output_format(FILE *fp) {
    // todo
    return NORMAL;
}

int parse_empty_statement(FILE *fp) {
    // todo
    return NORMAL;
}