#include "cross-referencer.h"

/* Variable to store the token read by scan() */
int token = 0;

/* Variable to store the magnitude of the step */
int paragraph_number = 0;

/* Variable to store the existence of emptystatement */
int existence_empty_statement = 0;

/* Variable to store whether it is inside iteration */
int whether_inside_iteration = 0;

/* string of each token */
char *tokenstr[NUMOFTOKEN + 1] = {
        "",
        "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
        "else", "procedure", "return", "call", "while", "do", "not", "or",
        "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
        "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
        ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read", "write", "break"
};

char *current_procname;

struct TYPE temp_type;
struct TYPE *end_type;

int parse_program(FILE *fp) {
    if (token != TPROGRAM) { return (error("Keyword 'program' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TNAME) { return (error("Program name is not found")); }
    printf("%s", string_attr);
    token = scan(fp);

    if (token != TSEMI) { return (error("Semicolon is not found")); }
    printf("%s\n", tokenstr[token]);
    token = scan(fp);

    if (parse_block(fp) == ERROR) { return ERROR; }

    if (token != TDOT) { return (error("Period is not found at the end of program")); }
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_block(FILE *fp) {
    current_procname = NULL;
    while ((token == TVAR) || (token == TPROCEDURE)) {
        if (current_procname != NULL) {
            free(current_procname);
            current_procname = NULL;
        }
        switch (token) {
            case TVAR:
                paragraph_number++;
                make_paragraph();
                if (parse_variable_declaration(fp) == ERROR) { return ERROR; }
                paragraph_number--;
                break;
            case TPROCEDURE:
                paragraph_number++;
                make_paragraph();
                if (parse_subprogram_declaration(fp) == ERROR) { return ERROR; }
                paragraph_number--;
                break;
            default:
                break;
        }
    }
    if (parse_compound_statement(fp) == ERROR) { return ERROR; }

    return NORMAL;
}

int parse_variable_declaration(FILE *fp) {
    struct NAME *loop_name;

    if (token != TVAR) { return (error("Keyword 'var' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    init_temp_names();
    if (parse_variable_names(fp) == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol ':' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    init_type(&temp_type);
    if (parse_type(fp) == ERROR) { return ERROR; }

    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, 0, &temp_type) == ERROR) { return ERROR; }
    }
    release_names();

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    while (token == TNAME) {
        paragraph_number++;
        make_paragraph();

        init_temp_names();
        if (parse_variable_names(fp) == ERROR) { return ERROR; }

        if (token != TCOLON) { return (error("Symbol ':' is not found")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        init_type(&temp_type);
        if (parse_type(fp) == ERROR) { return ERROR; }

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, 0, &temp_type) == ERROR) { return ERROR; }
        }
        release_names();

        if (token != TSEMI) { return (error("Symbol ';' is not found")); }
        printf("\b%s\n", tokenstr[token]);
        token = scan(fp);
        paragraph_number--;
    }

    return NORMAL;
}

int parse_variable_names(FILE *fp) {
    if (parse_variable_name(fp) == ERROR) { return ERROR; }

    while (token == TCOMMA) {
        printf("\b%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_variable_name(fp) == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_variable_name(FILE *fp) {
    if (token != TNAME) { return (error("Name is not found")); }
    temp_names(string_attr);
    printf("%s ", string_attr);
    token = scan(fp);

    return NORMAL;
}

int parse_type(FILE *fp) {
    switch (token) {
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if (parse_standard_type(fp) == ERROR) { return ERROR; }
            break;
        case TARRAY:
            if (parse_array_type(fp) == ERROR) { return ERROR; }
            break;
        default:
            return (error("Type is not found"));
    }

    return NORMAL;
}

int parse_standard_type(FILE *fp) {
    struct TYPE *next_type;

    switch (token) {
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if (temp_type.ttype == 0) {
                /* When temp_type is initial state */
                temp_type.ttype = token + 100;
                end_type = &temp_type;
            } else {
                /* When temo_type is not initial state */
                if ((next_type = (struct TYPE *) malloc((sizeof(struct TYPE)))) == NULL) {
                    return error("can not malloc in parse_procedure_name");
                }
                init_type(next_type);
                next_type->ttype = token + 100;
                end_type->paratp = next_type;
                end_type = next_type;
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Standard type is not found"));
    }

    return NORMAL;
}

int parse_array_type(FILE *fp) {
    if (token != TARRAY) { return (error("Keyword 'array' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TLSQPAREN) { return (error("Symbol '[' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TNUMBER) { return (error("Number is not found")); }
    if (num_attr < 1) {
        return error("Subscript of array is negative value");
    }
    temp_type.arraysize = num_attr;
    printf("%s ", string_attr);
    token = scan(fp);

    if (token != TRSQPAREN) { return (error("Symbol ']' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TOF) { return (error("Keyword 'of' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_standard_type(fp) == ERROR) { return ERROR; }
    temp_type.ttype += 100;

    return NORMAL;
}

int parse_subprogram_declaration(FILE *fp) {
    if (token != TPROCEDURE) { return (error("Keyword 'procedure' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_procedure_name(fp, 1) == ERROR) { return ERROR; }

    init_type(&temp_type);
    temp_type.ttype = TPPROC;
    if (token == TLPAREN) {
        if (parse_formal_parameters(fp) == ERROR) { return ERROR; }
    }

    if (def_id(current_procname, NULL, 0, &temp_type) == ERROR) { return ERROR; }

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    if (token == TVAR) {
        make_paragraph();
        if (parse_variable_declaration(fp) == ERROR) { return ERROR; }
    }

    make_paragraph();
    if (parse_compound_statement(fp) == ERROR) { return ERROR; }

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_procedure_name(FILE *fp, int def_flag) {
    if (token != TNAME) { return (error("Procedure name is not found")); }

    /* if def_flag is 1, procname is set */
    if (def_flag == 1) {
        if ((current_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
            return error("can not malloc in parse_procedure_name");
        }
        init_char_array(current_procname, MAX_IDENTIFIER_SIZE + 1);
        strncpy(current_procname, string_attr, MAX_IDENTIFIER_SIZE);
    }
    printf("%s ", string_attr);
    token = scan(fp);

    return NORMAL;
}

int parse_formal_parameters(FILE *fp) {
    struct NAME *loop_name;
    struct TYPE *ptype;

    if (token != TLPAREN) { return (error("Symbol '(' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    init_temp_names();
    if (parse_variable_names(fp) == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol ':' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_type(fp) == ERROR) { return ERROR; }
    ptype = temp_type.paratp;

    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, 0, ptype) == ERROR) { return ERROR; }
    }
    release_names();

    while (token == TSEMI) {
        printf("\b%s\t", tokenstr[token]);
        token = scan(fp);

        init_temp_names();
        if (parse_variable_names(fp) == ERROR) { return ERROR; }

        if (token != TCOLON) { return (error("Symbol ':' is not found")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_type(fp) == ERROR) { return ERROR; }
        ptype = ptype->paratp;

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, 0, ptype) == ERROR) { return ERROR; }
        }
        release_names();
    }

    if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_compound_statement(FILE *fp) {
    if (token != TBEGIN) { return (error("Keyword 'begin' is not found")); }
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    paragraph_number++;

    make_paragraph();
    if (parse_statement(fp) == ERROR) { return ERROR; }

    while (token == TSEMI) {
        if (token != TSEMI) { return (error("Symbol ';' is not found")); }
        printf("\b%s\n", tokenstr[token]);
        token = scan(fp);

        make_paragraph();
        if (parse_statement(fp) == ERROR) { return ERROR; }
    }

    if (token != TEND) { return (error("Keyword 'end' is not found")); }
    paragraph_number--;
    if (existence_empty_statement == 1) {
        existence_empty_statement = 0;
        printf("\r");
    } else {
        printf("\n");
    }
    make_paragraph();
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_statement(FILE *fp) {
    switch (token) {
        case TNAME:
            if (parse_assignment_statement(fp) == ERROR) { return ERROR; }
            break;
        case TIF:
            if (parse_condition_statement(fp) == ERROR) { return ERROR; }
            break;
        case TWHILE:
            if (parse_iteration_statement(fp) == ERROR) { return ERROR; }
            break;
        case TBREAK:
            if (parse_exit_statement(fp) == ERROR) { return ERROR; }
            break;
        case TCALL:
            if (parse_call_statement(fp) == ERROR) { return ERROR; }
            break;
        case TRETURN:
            if (parse_return_statement(fp) == ERROR) { return ERROR; }
            break;
        case TREAD:
        case TREADLN:
            if (parse_input_statement(fp) == ERROR) { return ERROR; }
            break;
        case TWRITE:
        case TWRITELN:
            if (parse_output_statement(fp) == ERROR) { return ERROR; }
            break;
        case TBEGIN:
            if (parse_compound_statement(fp) == ERROR) { return ERROR; }
            break;
        default:
            existence_empty_statement = 1;
            break;
    }

    return NORMAL;
}

int parse_condition_statement(FILE *fp) {
    if (token != TIF) { return (error("Keyword 'if' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_expression(fp) == ERROR) { return ERROR; }

    if (token != TTHEN) { return (error("Keyword 'then is not found")); }
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    paragraph_number++;

    make_paragraph();
    if (parse_statement(fp) == ERROR) { return ERROR; }
    paragraph_number--;

    if (token == TELSE) {
        if (existence_empty_statement == 1) {
            existence_empty_statement = 0;
            printf("\r");
        } else {
            printf("\n");
        }
        make_paragraph();
        printf("%s\n", tokenstr[token]);
        token = scan(fp);
        paragraph_number++;

        make_paragraph();
        if (parse_statement(fp) == ERROR) { return ERROR; }
        paragraph_number--;
    }

    return NORMAL;
}

int parse_iteration_statement(FILE *fp) {
    if (token != TWHILE) { return (error("Keyword 'while' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_expression(fp) == ERROR) { return ERROR; }

    if (token != TDO) { return (error("Keyword 'do' is not found")); }
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    paragraph_number++;
    whether_inside_iteration++;

    make_paragraph();
    if (parse_statement(fp) == ERROR) { return ERROR; }
    paragraph_number--;
    whether_inside_iteration--;

    return NORMAL;
}

int parse_exit_statement(FILE *fp) {
    if (token != TBREAK) { return (error("Keyword 'break' is not found")); }
    if (whether_inside_iteration > 0) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    } else {
        return error("Exit statement is not included in iteration statement");
    }

    return NORMAL;
}

int parse_call_statement(FILE *fp) {
    if (token != TCALL) { return (error("Keyword 'call' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_procedure_name(fp, 0) == ERROR) { return ERROR; }

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_expressions(fp) == ERROR) { return ERROR; }

        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }

    return NORMAL;
}

int parse_expressions(FILE *fp) {
    if (parse_expression(fp) == ERROR) { return ERROR; }

    while (token == TCOMMA) {
        printf("\b%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_expression(fp) == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_return_statement(FILE *fp) {
    if (token != TRETURN) { return (error("Keyword 'return' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_assignment_statement(FILE *fp) {
    if (parse_left_part(fp) == ERROR) { return ERROR; }

    if (token != TASSIGN) { return (error("Symbol ':=' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_expression(fp) == ERROR) { return ERROR; }

    return NORMAL;
}

int parse_left_part(FILE *fp) {
    if (parse_variable(fp) == ERROR) { return ERROR; }

    return NORMAL;
}

int parse_variable(FILE *fp) {
    char temp_string[MAXSTRSIZE];
    int temp_refnum = -1;

    init_char_array(temp_string, MAX_IDENTIFIER_SIZE + 1);
    strncpy(temp_string, string_attr, MAX_IDENTIFIER_SIZE);
    if (parse_variable_name(fp) == ERROR) { return ERROR; }

    if (token == TLSQPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_expression(fp) == ERROR) { return ERROR; }
        temp_refnum = 1;//todo

        if (token != TRSQPAREN) {
            return (error("Symbol ']' is not found at the end of expression"));
        }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }

    if (ref_id(temp_string, current_procname, temp_refnum) == ERROR) { return ERROR; }

    return NORMAL;
}

int parse_expression(FILE *fp) {
    if (parse_simple_expression(fp) == ERROR) { return ERROR; }

    while ((token == TEQUAL) || (token == TNOTEQ) || (token == TLE) || (token == TLEEQ) || (token == TGR) ||
           (token == TGREQ)) {
        if (parse_relational_operator(fp) == ERROR) { return ERROR; }

        if (parse_simple_expression(fp) == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_simple_expression(FILE *fp) {
    switch (token) {
        case TPLUS:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TMINUS:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            break;
    }

    if (parse_term(fp) == ERROR) { return ERROR; }

    while ((token == TPLUS) || (token == TMINUS) || (token == TOR)) {
        if (parse_additive_operator(fp) == ERROR) { return ERROR; }

        if (parse_term(fp) == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_term(FILE *fp) {
    if (parse_factor(fp) == ERROR) { return ERROR; }

    while ((token == TSTAR) || (token == TDIV) || (token == TAND)) {
        if (parse_multiplicative_operator(fp) == ERROR) { return ERROR; }

        if (parse_factor(fp) == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_factor(FILE *fp) {
    switch (token) {
        case TNAME:
            if (parse_variable(fp) == ERROR) { return ERROR; }
            break;
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if (parse_constant(fp) == ERROR) { return ERROR; }
            break;
        case TLPAREN:
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (parse_expression(fp) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TNOT:
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (parse_factor(fp) == ERROR) { return ERROR; }
            break;
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if (parse_standard_type(fp) == ERROR) { return ERROR; }

            if (token != TLPAREN) {
                return (error("Symbol '(' is not found in factor"));
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (parse_expression(fp) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Factor is not found"));
    }

    return NORMAL;
}

int parse_constant(FILE *fp) {
    switch (token) {
        case TNUMBER:
            printf("%s ", string_attr);
            token = scan(fp);
            break;
        case TFALSE:
        case TTRUE:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TSTRING:
            printf("'%s' ", string_attr);
            token = scan(fp);
            break;
        default:
            return (error("Constant is not found"));
    }

    return NORMAL;
}

int parse_multiplicative_operator(FILE *fp) {
    switch (token) {
        case TSTAR:
        case TDIV:
        case TAND:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Multiplicative operator is not found"));
    }

    return NORMAL;
}

int parse_additive_operator(FILE *fp) {
    switch (token) {
        case TPLUS:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TMINUS:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TOR:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Additive operator is not found"));
    }

    return NORMAL;
}

int parse_relational_operator(FILE *fp) {
    switch (token) {
        case TEQUAL:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TNOTEQ:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TLE:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TLEEQ:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TGR:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TGREQ:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Relational operator is not found"));
    }

    return NORMAL;
}

int parse_input_statement(FILE *fp) {
    switch (token) {
        case TREAD:
        case TREADLN:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Keyword 'read', 'readln' is not found"));
    }

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_variable(fp) == ERROR) { return ERROR; }

        while (token == TCOMMA) {
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (parse_variable(fp) == ERROR) { return ERROR; }
        }
        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }

    return NORMAL;
}

int parse_output_statement(FILE *fp) {
    switch (token) {
        case TWRITE:
        case TWRITELN:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Keyword 'write', 'writeln' is not found"));
    }

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_output_format(fp) == ERROR) { return ERROR; }

        while (token == TCOMMA) {
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (parse_output_format(fp) == ERROR) { return ERROR; }
        }
        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }

    return NORMAL;
}

int parse_output_format(FILE *fp) {
    switch (token) {
        case TSTRING:
            if (strlen(string_attr) > 1) {
                printf("'%s' ", string_attr);
                token = scan(fp);
                break;
            }
        case TPLUS:
        case TMINUS:
        case TNAME:
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TLPAREN:
        case TNOT:
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if (parse_expression(fp) == ERROR) { return ERROR; }

            if (token == TCOLON) {
                printf("%s ", tokenstr[token]);
                token = scan(fp);

                if (token != TNUMBER) { return (error("Number is not found")); }
                printf("%s ", string_attr);
                token = scan(fp);
            }
            break;
        default:
            return (error("Output format is not found"));
    }

    return NORMAL;
}

/* Stage according to paragraph_number */
void make_paragraph() {
    int i = 0;
    for (i = 0; i < paragraph_number; i++) {
        printf("    ");
    }
}