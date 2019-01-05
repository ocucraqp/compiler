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

/* prototype declaration */
int parse_block(FILE *fp);

int parse_variable_declaration(FILE *fp);

int parse_variable_names(FILE *fp);

int parse_variable_name(FILE *fp);

int parse_type(FILE *fp);

int parse_standard_type(FILE *fp);

int parse_array_type(FILE *fp);

int parse_subprogram_declaration(FILE *fp);

int parse_procedure_name(FILE *fp);

int parse_formal_parameters(FILE *fp);

int parse_compound_statement(FILE *fp);

int parse_statement(FILE *fp);

int parse_condition_statement(FILE *fp);

int parse_iteration_statement(FILE *fp);

int parse_exit_statement(FILE *fp);

int parse_call_statement(FILE *fp);

int parse_expressions(FILE *fp, struct TYPE *parameter_type);

int parse_return_statement(FILE *fp);

int parse_assignment_statement(FILE *fp);

int parse_left_part(FILE *fp);

int parse_variable(FILE *fp);

int parse_expression(FILE *fp);

int parse_simple_expression(FILE *fp);

int parse_term(FILE *fp);

int parse_factor(FILE *fp);

int parse_constant(FILE *fp);

int parse_multiplicative_operator(FILE *fp);

int parse_additive_operator(FILE *fp);

int parse_relational_operator(FILE *fp);

int parse_input_statement(FILE *fp);

int parse_output_statement(FILE *fp);

int parse_output_format(FILE *fp);

void make_paragraph();

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
        if (def_id(loop_name->name, current_procname, &temp_type) == ERROR) { return ERROR; }
    }
    init_deflinenum();
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
            if (def_id(loop_name->name, current_procname, &temp_type) == ERROR) { return ERROR; }
        }
        init_deflinenum();
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
    if (save_deflinenum() == ERROR) { return ERROR; }
    reflinenum = get_linenum();
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
    int type_holder = NORMAL;

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
                    return error("can not malloc in parse_standard_type");
                }
                init_type(next_type);
                next_type->ttype = token + 100;
                end_type->paratp = next_type;
                end_type = next_type;
            }
            printf("%s ", tokenstr[token]);
            type_holder = token + 100;
            token = scan(fp);
            return type_holder;
        default:
            return (error("Standard type is not found"));
    }
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

    if (parse_procedure_name(fp) == ERROR) { return ERROR; }

    init_type(&temp_type);
    temp_type.ttype = TPPROC;
    temp_type.paratp = NULL;
    end_type = &temp_type;
    if (token == TLPAREN) {
        if (parse_formal_parameters(fp) == ERROR) { return ERROR; }
    }

    if (def_id(current_procname, NULL, &temp_type) == ERROR) { return ERROR; }
    init_deflinenum();

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

int parse_procedure_name(FILE *fp) {
    if (token != TNAME) { return (error("Procedure name is not found")); }

    if ((current_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
        return error("can not malloc in parse_procedure_name");
    }
    init_char_array(current_procname, MAX_IDENTIFIER_SIZE + 1);
    strncpy(current_procname, string_attr, MAX_IDENTIFIER_SIZE);

    printf("%s ", string_attr);
    if (save_deflinenum() == ERROR) { return ERROR; };
    reflinenum = get_linenum();
    token = scan(fp);

    return NORMAL;
}

int parse_formal_parameters(FILE *fp) {
    struct NAME *loop_name;
    struct TYPE *ptype, *next_type;

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
    if (check_standard_type_to_pointer(ptype) == ERROR) { return ERROR; }

    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, ptype) == ERROR) { return ERROR; }
        if (loop_name->nextnamep != NULL) {
            if ((next_type = (struct TYPE *) malloc((sizeof(struct TYPE)))) == NULL) {
                return error("can not malloc in parse_formal_parameters");
            }
            init_type(next_type);
            next_type->ttype = ptype->ttype;
            ptype->paratp = next_type;
            ptype = next_type;
        }
    }
    init_deflinenum();
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
        if (check_standard_type_to_pointer(ptype) == ERROR) { return ERROR; }

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, ptype) == ERROR) { return ERROR; }
        }
        init_deflinenum();
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
    int expression_type_holder = NORMAL;

    if (token != TIF) { return (error("Keyword 'if' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if ((expression_type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
    if (expression_type_holder != TPBOOL) {
        return error("The type of the expression in the condition statement must be boolean");
    }

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
    int expression_type_holder = NORMAL;

    if (token != TWHILE) { return (error("Keyword 'while' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if ((expression_type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
    if (expression_type_holder != TPBOOL) {
        return error("The type of the expression in the iteration statement must be boolean");
    }

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
    struct TYPE *parameter_type;
    char *temp_procname;
    temp_procname == NULL;

    if (current_procname != NULL) {
        if ((temp_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
            return error("can not malloc-1 in parse_call_statement");
        }
        init_char_array(temp_procname, MAX_IDENTIFIER_SIZE + 1);
        strncpy(temp_procname, current_procname, MAX_IDENTIFIER_SIZE);
    }

    if (token != TCALL) { return (error("Keyword 'call' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (parse_procedure_name(fp) == ERROR) { return ERROR; }

    if (current_procname != NULL) {
        if ((parameter_type = (struct TYPE *) malloc(sizeof(struct TYPE))) == NULL) {
            return error("can not malloc-2 in parse_call_statement");
        }
        init_type(parameter_type);
        if (ref_id(current_procname, NULL, -1, &parameter_type) == ERROR) { return ERROR; }
        if (temp_procname == NULL) {
            current_procname = NULL;
        } else {
            init_char_array(current_procname, MAX_IDENTIFIER_SIZE + 1);
            strncpy(current_procname, temp_procname, MAX_IDENTIFIER_SIZE);
            free(temp_procname);
            temp_procname = NULL;
        }
    }

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (parse_expressions(fp, parameter_type) == ERROR) { return ERROR; }

        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }

    return NORMAL;
}

int parse_expressions(FILE *fp, struct TYPE *parameter_type) {
    int type_holder = NORMAL, i = 0;
    struct TYPE **temp_type;

    if (parameter_type->paratp == NULL) {
        return error("There are unnecessary arguments.");
    } else {
        temp_type = &(parameter_type->paratp);
    }

    if ((type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
    i++;
    if (type_holder != (*temp_type)->ttype) {
        return error("The type of the 1st argument is incorrect.");
    }

    while (token == TCOMMA) {
        printf("\b%s ", tokenstr[token]);
        token = scan(fp);

        if ((type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
        i++;
        temp_type = &((*temp_type)->paratp);
        if (type_holder != (*temp_type)->ttype) {
            switch (i % 10) {
                case 1:
                    return error("The type of the %dst argument is incorrect.", i);
                case 2:
                    return error("The type of the %dnd argument is incorrect.", i);
                case 3:
                    return error("The type of the %drd argument is incorrect.", i);
                default:
                    return error("The type of the %dth argument is incorrect.", i);
            }
        }
    }

    if ((*temp_type)->paratp != NULL) {
        return error("Argument shortage");
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
    int type_holder = NORMAL, expression_type_holder = NORMAL;

    if ((type_holder = parse_left_part(fp)) == ERROR) { return ERROR; }

    if (token != TASSIGN) { return (error("Symbol ':=' is not found")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if ((expression_type_holder = parse_expression(fp)) == ERROR) { return ERROR; }

    if ((type_holder % 100) != (expression_type_holder % 100)) {
        return error("The type of the expression differs from the left part");
    }

    return NORMAL;
}

int parse_left_part(FILE *fp) {
    int type_holder = NORMAL;

    if ((type_holder = parse_variable(fp)) == ERROR) { return ERROR; }

    return type_holder;
}

int parse_variable(FILE *fp) {
    int temp_refnum = -1, type_holder = NORMAL, expression_type_holder = NORMAL;
    struct TYPE *parameter_type;

    init_temp_names();
    if (parse_variable_name(fp) == ERROR) { return ERROR; }

    if (token == TLSQPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (token == TNUMBER) {
            temp_refnum = num_attr;
            printf("%s ", string_attr);
            token = scan(fp);
        } else {
            if ((expression_type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
            if (expression_type_holder != TPINT) {
                return error(
                        "When referring to an array type variable,\n"
                        "it is necessary to attach an expression of type integer");
            }
            temp_refnum = 0;
        }

        if (token != TRSQPAREN) {
            return (error("Symbol ']' is not found at the end of expression"));
        }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }

    if ((parameter_type = (struct TYPE *) malloc(sizeof(struct TYPE))) == NULL) {
        return error("can not malloc in parse_variable");
    }
    init_type(parameter_type);
    if ((type_holder = ref_id(temp_name_root->name, current_procname, temp_refnum, &parameter_type)) ==
        ERROR) { return ERROR; }

    release_names();
    return type_holder;
}

int parse_expression(FILE *fp) {
    int type_holder = NORMAL, operand1_type = NORMAL, operand2_type = NORMAL;

    if ((operand1_type = parse_simple_expression(fp)) == ERROR) { return ERROR; }
    type_holder = operand1_type;

    while ((token == TEQUAL) || (token == TNOTEQ) || (token == TLE) || (token == TLEEQ) || (token == TGR) ||
           (token == TGREQ)) {
        if (parse_relational_operator(fp) == ERROR) { return ERROR; }

        if ((operand2_type = parse_simple_expression(fp)) == ERROR) { return ERROR; }
        if (operand1_type != operand2_type) {
            return error("The operands of relational operators have different types");
        }
        type_holder = TPBOOL;
    }

    return type_holder;
}

int parse_simple_expression(FILE *fp) {
    int type_holder = NORMAL, pm_flag = 0, operand1_type = NORMAL, operand2_type = NORMAL;

    switch (token) {
        case TPLUS:
        case TMINUS:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            pm_flag = 1;
            break;
        default:
            pm_flag = 0;
            break;
    }

    if ((operand1_type = parse_term(fp)) == ERROR) { return ERROR; }
    if (pm_flag == 1 && operand1_type != TPINT) {
        return error("The operands of + and - must be of type integer");
    }

    while ((token == TPLUS) || (token == TMINUS) || (token == TOR)) {
        switch (token) {
            case TPLUS:
            case TMINUS:
                pm_flag = 1;
                break;
            default:
                pm_flag = 0;
                break;
        }
        if (parse_additive_operator(fp) == ERROR) { return ERROR; }

        if ((operand2_type = parse_term(fp)) == ERROR) { return ERROR; }
        if (pm_flag == 1 && operand1_type != TPINT && operand1_type == operand2_type) {
            return error("The operands of '+' and '-' must be of type integer");
        }
        if (pm_flag == 0 && operand1_type != TPBOOL && operand1_type == operand2_type) {
            return error("The operands of 'or' must be of type boolean");
        }
    }
    type_holder = operand1_type;

    return type_holder;
}

int parse_term(FILE *fp) {
    int type_holder = NORMAL, md_flag = 0, operand1_type = NORMAL, operand2_type = NORMAL;

    if ((operand1_type = parse_factor(fp)) == ERROR) { return ERROR; }

    while ((token == TSTAR) || (token == TDIV) || (token == TAND)) {
        switch (token) {
            case TSTAR:
            case TDIV:
                md_flag = 1;
                break;
            default:
                md_flag = 0;
                break;
        }
        if (parse_multiplicative_operator(fp) == ERROR) { return ERROR; }

        if ((operand2_type = parse_factor(fp)) == ERROR) { return ERROR; }
        if (md_flag == 1 && operand1_type != TPINT && operand1_type == operand2_type) {
            return error("The operands of '*' and 'div' must be of type integer");
        }
        if (md_flag == 0 && operand1_type != TPBOOL && operand1_type == operand2_type) {
            return error("The operands of 'and' must be of type boolean");
        }
    }
    type_holder = operand1_type;

    return type_holder;
}

int parse_factor(FILE *fp) {
    int type_holder = NORMAL, expression_type_holder = NORMAL;

    switch (token) {
        case TNAME:
            if ((type_holder = parse_variable(fp)) == ERROR) { return ERROR; }
            break;
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if ((type_holder = parse_constant(fp)) == ERROR) { return ERROR; }
            break;
        case TLPAREN:
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if ((type_holder = parse_expression(fp)) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        case TNOT:
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if ((type_holder = parse_factor(fp)) == ERROR) { return ERROR; }
            if (type_holder != TPBOOL) {
                return error("The operand of 'not' must be of type boolean.");
            }
            break;
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if ((type_holder = parse_standard_type(fp)) == ERROR) { return ERROR; }

            if (token != TLPAREN) {
                return (error("Symbol '(' is not found in factor"));
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if ((expression_type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
            if (check_standard_type(expression_type_holder) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return (error("Factor is not found"));
    }

    return type_holder;
}

int parse_constant(FILE *fp) {
    int type_holder = NORMAL;

    switch (token) {
        case TNUMBER:
            printf("%s ", string_attr);
            token = scan(fp);
            type_holder = TPINT;
            break;
        case TFALSE:
        case TTRUE:
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            type_holder = TPBOOL;
            break;
        case TSTRING:
            if ((int) strlen(string_attr) != 1) {
                return error("Constant string length must be 1");
            }
            printf("'%s' ", string_attr);
            token = scan(fp);
            type_holder = TPCHAR;
            break;
        default:
            return (error("Constant is not found"));
    }

    return type_holder;
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
    int type_holder = NORMAL;
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

        if ((type_holder = parse_variable(fp)) == ERROR) { return ERROR; }
        if (type_holder != TPINT && type_holder != TPCHAR) {
            return error("The variable in the input statement is not integer type or char type");
        }

        while (token == TCOMMA) {
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if ((type_holder = parse_variable(fp)) == ERROR) { return ERROR; }
            if (type_holder != TPINT && type_holder != TPCHAR) {
                return error("The variable in the input statement is not integer type or char type");
            }
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
    int str_length = 0, type_holder = NORMAL;

    switch (token) {
        case TSTRING:
            str_length = (int) strlen(string_attr);
            if (str_length == 0 || str_length > 2) {
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
            if ((type_holder = parse_expression(fp)) == ERROR) { return ERROR; }
            if (check_standard_type(type_holder) == ERROR) { return ERROR; }

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