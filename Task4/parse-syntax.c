#include "compiler.h"

/* Variable to store the token read by scan() */
int token = 0;

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

/* Structure for creating a list of dummy argument IDs */
struct PARAID paraidroot, *paraidend;

/* prototype declaration */
int parse_block(FILE *inputfp, FILE *outputfp, char *start_labelname);

int parse_variable_declaration(FILE *inputfp, FILE *outputfp);

int parse_variable_names(FILE *inputfp, FILE *outputfp);

int parse_variable_name(FILE *inputfp, FILE *outputfp);

int parse_type(FILE *inputfp, FILE *outputfp);

int parse_standard_type(FILE *inputfp, FILE *outputfp);

int parse_array_type(FILE *inputfp, FILE *outputfp);

int parse_subprogram_declaration(FILE *inputfp, FILE *outputfp);

int parse_procedure_name(FILE *inputfp, FILE *outputfp);

int parse_formal_parameters(FILE *inputfp, FILE *outputfp);

int parse_compound_statement(FILE *inputfp, FILE *outputfp, int is_insubproc);

int parse_statement(FILE *inputfp, FILE *outputfp, int is_insubproc);

int parse_condition_statement(FILE *inputfp, FILE *outputfp, int is_insubproc);

int parse_iteration_statement(FILE *inputfp, FILE *outputfp, int is_insubproc);

int parse_exit_statement(FILE *inputfp, FILE *outputfp);

int parse_call_statement(FILE *inputfp, FILE *outputfp);

int parse_expressions(FILE *inputfp, FILE *outputfp, struct TYPE *parameter_type);

int parse_return_statement(FILE *inputfp, FILE *outputfp);

int parse_assignment_statement(FILE *inputfp, FILE *outputfp, int is_insubproc);

int parse_left_part(FILE *inputfp, FILE *outputfp, struct ID **p, int is_insubproc, int is_inassign);

int parse_variable(FILE *inputfp, FILE *outputfp, struct ID **p, int is_incall, int is_insubproc, int is_inassign);

int parse_expression(FILE *inputfp, FILE *outputfp, int is_incall);

int parse_simple_expression(FILE *inputfp, FILE *outputfp, int is_incall);

int parse_term(FILE *inputfp, FILE *outputfp, int is_incall);

int parse_factor(FILE *inputfp, FILE *outputfp, int is_incall);

int parse_constant(FILE *inputfp, FILE *outputfp);

int parse_multiplicative_operator(FILE *inputfp, FILE *outputfp);

int parse_additive_operator(FILE *inputfp, FILE *outputfp);

int parse_relational_operator(FILE *inputfp, FILE *outputfp);

int parse_input_statement(FILE *inputfp, FILE *outputfp);

int parse_output_statement(FILE *inputfp, FILE *outputfp);

int parse_output_format(FILE *inputfp, FILE *outputfp);

int parse_program(FILE *inputfp, FILE *outputfp) {
    char *start_labelname = NULL;

    if (token != TPROGRAM) { return (error("Keyword 'program' is not found")); }
    token = scan(inputfp);

    if (token != TNAME) { return (error("Program name is not found")); }
    command_start(outputfp, string_attr, &start_labelname);
    token = scan(inputfp);

    if (token != TSEMI) { return (error("Semicolon is not found")); }
    token = scan(inputfp);

    if (parse_block(inputfp, outputfp, start_labelname) == ERROR) { return ERROR; }

    if (token != TDOT) { return (error("Period is not found at the end of program")); }
    token = scan(inputfp);

    return NORMAL;
}

int parse_block(FILE *inputfp, FILE *outputfp, char *start_labelname) {
    current_procname = NULL;
    while ((token == TVAR) || (token == TPROCEDURE)) {
        if (current_procname != NULL) {
            free(current_procname);
            current_procname = NULL;
        }
        switch (token) {
            case TVAR:
                if (parse_variable_declaration(inputfp, outputfp) == ERROR) { return ERROR; }
                break;
            case TPROCEDURE:
                if (parse_subprogram_declaration(inputfp, outputfp) == ERROR) { return ERROR; }
                break;
            default:
                break;
        }
    }
    if (current_procname != NULL) {
        free(current_procname);
        current_procname = NULL;
    }

    fprintf(outputfp, "%s\n", start_labelname);

    if (parse_compound_statement(inputfp, outputfp, 0) == ERROR) { return ERROR; }

    fprintf(outputfp, "\tRET\n");

    return NORMAL;
}

int parse_variable_declaration(FILE *inputfp, FILE *outputfp) {
    struct NAME *loop_name;

    if (token != TVAR) { return (error("Keyword 'var' is not found")); }
    token = scan(inputfp);

    init_temp_names();
    if (parse_variable_names(inputfp, outputfp) == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol ':' is not found")); }
    token = scan(inputfp);

    init_type(&temp_type);
    if (parse_type(inputfp, outputfp) == ERROR) { return ERROR; }

    /* Define id as many as name */
    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, &temp_type, 0, outputfp) == ERROR) { return ERROR; }
    }
    release_vallinenum();
    release_names();

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    token = scan(inputfp);

    while (token == TNAME) {
        /* Repeat */
        init_temp_names();
        if (parse_variable_names(inputfp, outputfp) == ERROR) { return ERROR; }

        if (token != TCOLON) { return (error("Symbol ':' is not found")); }
        token = scan(inputfp);

        init_type(&temp_type);
        if (parse_type(inputfp, outputfp) == ERROR) { return ERROR; }

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, &temp_type, 0, outputfp) == ERROR) { return ERROR; }
        }
        release_vallinenum();
        release_names();

        if (token != TSEMI) { return (error("Symbol ';' is not found")); }
        token = scan(inputfp);
    }

    return NORMAL;
}

int parse_variable_names(FILE *inputfp, FILE *outputfp) {
    if (parse_variable_name(inputfp, outputfp) == ERROR) { return ERROR; }

    while (token == TCOMMA) {
        token = scan(inputfp);
        if (parse_variable_name(inputfp, outputfp) == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_variable_name(FILE *inputfp, FILE *outputfp) {
    if (token != TNAME) { return (error("Name is not found")); }
    if (temp_names(string_attr) == ERROR) { return ERROR; }
    if (save_vallinenum() == ERROR) { return ERROR; }
    token = scan(inputfp);

    return NORMAL;
}

int parse_type(FILE *inputfp, FILE *outputfp) {
    switch (token) {
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if (parse_standard_type(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        case TARRAY:
            if (parse_array_type(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        default:
            return (error("Type is not found"));
    }

    return NORMAL;
}

int parse_standard_type(FILE *inputfp, FILE *outputfp) {
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
            type_holder = token + 100;
            token = scan(inputfp);
            return type_holder;
        default:
            return (error("Standard type is not found"));
    }
}

int parse_array_type(FILE *inputfp, FILE *outputfp) {
    if (token != TARRAY) { return (error("Keyword 'array' is not found")); }
    token = scan(inputfp);

    if (token != TLSQPAREN) { return (error("Symbol '[' is not found")); }
    token = scan(inputfp);

    if (token != TNUMBER) { return (error("Number is not found")); }
    if (num_attr < 1) {
        return error("Subscript of array is negative value");
    }
    temp_type.arraysize = num_attr;
    token = scan(inputfp);

    if (token != TRSQPAREN) { return (error("Symbol ']' is not found")); }
    token = scan(inputfp);

    if (token != TOF) { return (error("Keyword 'of' is not found")); }
    token = scan(inputfp);

    if (parse_standard_type(inputfp, outputfp) == ERROR) { return ERROR; }
    temp_type.ttype += 100;

    return NORMAL;
}

int parse_subprogram_declaration(FILE *inputfp, FILE *outputfp) {
    struct ID *p;

    if (token != TPROCEDURE) { return (error("Keyword 'procedure' is not found")); }
    token = scan(inputfp);

    if (parse_procedure_name(inputfp, outputfp) == ERROR) { return ERROR; }

    init_type(&temp_type);
    temp_type.ttype = TPPROC;
    temp_type.paratp = NULL;
    end_type = &temp_type;
    paraidroot.paraidp = NULL;
    paraidroot.nextparaidp = NULL;
    paraidend = &paraidroot;
    if (token == TLPAREN) {
        if (parse_formal_parameters(inputfp, outputfp) == ERROR) { return ERROR; }
    }

    if (def_id(current_procname, NULL, &temp_type, 0, outputfp) == ERROR) { return ERROR; }
    release_vallinenum();

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    token = scan(inputfp);

    if (token == TVAR) {
        if (parse_variable_declaration(inputfp, outputfp) == ERROR) { return ERROR; }
    }

    /* output label of sub proc id */
    if ((p = search_idtab(current_procname, NULL, 1)) == NULL) {
        return error("%s is not defined.", current_procname);
    } else {
        create_id_label(p, outputfp);
        command_process_arguments(outputfp);
    }

    if (parse_compound_statement(inputfp, outputfp, 1) == ERROR) { return ERROR; }

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    token = scan(inputfp);

    fprintf(outputfp, "\tRET\n");

    return NORMAL;
}

int parse_procedure_name(FILE *inputfp, FILE *outputfp) {
    if (token != TNAME) { return (error("Procedure name is not found")); }

    if ((current_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
        return error("can not malloc in parse_procedure_name");
    }
    init_char_array(current_procname, MAX_IDENTIFIER_SIZE + 1);
    strncpy(current_procname, string_attr, MAX_IDENTIFIER_SIZE);

    if (save_vallinenum() == ERROR) { return ERROR; };
    token = scan(inputfp);

    return NORMAL;
}

int parse_formal_parameters(FILE *inputfp, FILE *outputfp) {
    struct NAME *loop_name;
    struct TYPE *ptype, *next_type;
    struct ID *p;
    struct PARAID *paraidp;

    if (token != TLPAREN) { return (error("Symbol '(' is not found")); }
    token = scan(inputfp);

    init_temp_names();
    if (parse_variable_names(inputfp, outputfp) == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol ':' is not found")); }
    token = scan(inputfp);

    if (parse_type(inputfp, outputfp) == ERROR) { return ERROR; }
    ptype = temp_type.paratp;
    if (check_standard_type_to_pointer(ptype) == ERROR) { return ERROR; }

    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, ptype, 1, outputfp) == ERROR) { return ERROR; }

        if ((p = search_idtab(loop_name->name, current_procname, 1)) == NULL) {
            return error("%s is not defined.", current_procname);
        } else {
            if ((paraidp = (struct PARAID *) malloc(sizeof(struct PARAID))) == NULL) {
                return error("can not malloc-1 in parse_formal_parameters");
            }
            paraidp->paraidp = p;
            paraidp->nextparaidp = NULL;
            paraidend->nextparaidp = paraidp;
            paraidend = paraidp;
        }

        if (loop_name->nextnamep != NULL) {
            if ((next_type = (struct TYPE *) malloc((sizeof(struct TYPE)))) == NULL) {
                return error("can not malloc-2 in parse_formal_parameters");
            }
            init_type(next_type);
            next_type->ttype = ptype->ttype;
            ptype->paratp = next_type;
            ptype = next_type;
        }
    }
    release_vallinenum();
    release_names();

    while (token == TSEMI) {
        token = scan(inputfp);

        init_temp_names();
        if (parse_variable_names(inputfp, outputfp) == ERROR) { return ERROR; }

        if (token != TCOLON) { return (error("Symbol ':' is not found")); }
        token = scan(inputfp);

        if (parse_type(inputfp, outputfp) == ERROR) { return ERROR; }
        ptype = ptype->paratp;
        if (check_standard_type_to_pointer(ptype) == ERROR) { return ERROR; }

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, ptype, 1, outputfp) == ERROR) { return ERROR; }
        }
        release_vallinenum();
        release_names();
    }

    if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
    token = scan(inputfp);

    return NORMAL;
}

int parse_compound_statement(FILE *inputfp, FILE *outputfp, int is_insubproc) {
    if (token != TBEGIN) { return (error("Keyword 'begin' is not found")); }
    token = scan(inputfp);

    if (parse_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }

    while (token == TSEMI) {
        if (token != TSEMI) { return (error("Symbol ';' is not found")); }
        token = scan(inputfp);

        if (parse_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
    }

    if (token != TEND) { return (error("Keyword 'end' is not found")); }
    token = scan(inputfp);

    return NORMAL;
}

int parse_statement(FILE *inputfp, FILE *outputfp, int is_insubproc) {
    switch (token) {
        case TNAME:
            if (parse_assignment_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
            break;
        case TIF:
            if (parse_condition_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
            break;
        case TWHILE:
            if (parse_iteration_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
            break;
        case TBREAK:
            if (parse_exit_statement(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        case TCALL:
            if (parse_call_statement(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        case TRETURN:
            if (parse_return_statement(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        case TREAD:
        case TREADLN:
            if (parse_input_statement(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        case TWRITE:
        case TWRITELN:
            if (parse_output_statement(inputfp, outputfp) == ERROR) { return ERROR; }
            break;
        case TBEGIN:
            if (parse_compound_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
            break;
        default:
            break;
    }

    return NORMAL;
}

int parse_condition_statement(FILE *inputfp, FILE *outputfp, int is_insubproc) {
    int expression_type_holder = NORMAL;
    char *if_labelname = NULL, *else_labelname = NULL;

    if (token != TIF) { return (error("Keyword 'if' is not found")); }
    token = scan(inputfp);

    if ((expression_type_holder = parse_expression(inputfp, outputfp, 0)) == ERROR) { return ERROR; }
    if (expression_type_holder != TPBOOL) {
        return error("The type of the expression in the condition statement must be boolean");
    }
    command_condition_statement(outputfp, &if_labelname);

    if (token != TTHEN) { return (error("Keyword 'then is not found")); }
    token = scan(inputfp);
    if (parse_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }

    if (token == TELSE) {
        if (create_newlabel(&else_labelname) == ERROR) { return ERROR; }
        fprintf(outputfp, "\tJUMP\t%s\n", else_labelname);
        fprintf(outputfp, "%s\n", if_labelname);

        token = scan(inputfp);

        if (parse_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
        fprintf(outputfp, "%s\n", else_labelname);
    } else {
        fprintf(outputfp, "%s\n", if_labelname);
    }

    return NORMAL;
}

int parse_iteration_statement(FILE *inputfp, FILE *outputfp, int is_insubproc) {
    int expression_type_holder = NORMAL;
    char *while_labelname[4] = {NULL};

    if (token != TWHILE) { return (error("Keyword 'while' is not found")); }
    token = scan(inputfp);

    if (create_newlabel(&(while_labelname[0])) == ERROR) { return ERROR; }
    fprintf(outputfp, "%s\n", while_labelname[0]);

    if ((expression_type_holder = parse_expression(inputfp, outputfp, 0)) == ERROR) { return ERROR; }
    if (expression_type_holder != TPBOOL) {
        return error("The type of the expression in the iteration statement must be boolean");
    }

    if (token != TDO) { return (error("Keyword 'do' is not found")); }
    token = scan(inputfp);
    whether_inside_iteration++;

    if (create_newlabel(&(while_labelname[1])) == ERROR) { return ERROR; }
    if (create_newlabel(&(while_labelname[2])) == ERROR) { return ERROR; }
    fprintf(outputfp, "\tJPL \t%s\n", while_labelname[2]);
    fprintf(outputfp, "\tLD  \tgr1, gr0\n");
    if (create_newlabel(&(while_labelname[3])) == ERROR) { return ERROR; }
    fprintf(outputfp, "\tJUMP\t%s\n", while_labelname[3]);
    fprintf(outputfp, "%s\n", while_labelname[2]);
    fprintf(outputfp, "\tLAD \tgr1, 1\n");
    fprintf(outputfp, "%s\n", while_labelname[3]);
    fprintf(outputfp, "\tCPA \tgr1, gr0\n");
    fprintf(outputfp, "\tJZE \t%s\n", while_labelname[1]);

    if (parse_statement(inputfp, outputfp, is_insubproc) == ERROR) { return ERROR; }
    whether_inside_iteration--;

    fprintf(outputfp, "\tJUMP\t%s\n", while_labelname[0]);
    fprintf(outputfp, "%s\n", while_labelname[1]);

    return NORMAL;
}

int parse_exit_statement(FILE *inputfp, FILE *outputfp) {
    if (token != TBREAK) { return (error("Keyword 'break' is not found")); }
    if (whether_inside_iteration > 0) {
        token = scan(inputfp);
    } else {
        return error("Exit statement is not included in iteration statement");
    }

    return NORMAL;
}

int parse_call_statement(FILE *inputfp, FILE *outputfp) {
    struct TYPE *parameter_type;
    char *call_procname;
    char temp_procname[MAX_IDENTIFIER_SIZE + 1];
    call_procname == NULL;

    if (current_procname != NULL) {
        if ((call_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
            return error("can not malloc-1 in parse_call_statement");
        }
        init_char_array(call_procname, MAX_IDENTIFIER_SIZE + 1);
        strncpy(call_procname, current_procname, MAX_IDENTIFIER_SIZE);
    }

    if (token != TCALL) { return (error("Keyword 'call' is not found")); }
    token = scan(inputfp);

    if (parse_procedure_name(inputfp, outputfp) == ERROR) { return ERROR; }

    if (current_procname != NULL) {
        /* If current_procname and temp_procname are the same, it is judged as a recursive call */
        if (call_procname != NULL) {
            if (strncmp(current_procname, call_procname, MAX_IDENTIFIER_SIZE) == 0) {
                return error("Recursive calls are not allowed");
            }
        }
        if ((parameter_type = (struct TYPE *) malloc(sizeof(struct TYPE))) == NULL) {
            return error("can not malloc-2 in parse_call_statement");
        }
        init_type(parameter_type);
        if (ref_id(current_procname, NULL, -1, &parameter_type) == ERROR) { return ERROR; }
        release_vallinenum();
        strncpy(temp_procname, current_procname, MAX_IDENTIFIER_SIZE + 1);
        if (call_procname == NULL) {
            free(current_procname);
            current_procname = NULL;
        } else {
            init_char_array(current_procname, MAX_IDENTIFIER_SIZE + 1);
            strncpy(current_procname, call_procname, MAX_IDENTIFIER_SIZE);
            free(call_procname);
            call_procname = NULL;
        }
    } else {
        return error("Procedure name is not found");
    }

    if (token == TLPAREN) {
        token = scan(inputfp);

        if (parse_expressions(inputfp, outputfp, parameter_type) == ERROR) { return ERROR; }

        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        token = scan(inputfp);
    }

    fprintf(outputfp, "\tCALL\t$%s\n", temp_procname);

    return NORMAL;
}

int parse_expressions(FILE *inputfp, FILE *outputfp, struct TYPE *parameter_type) {
    int type_holder = NORMAL, i = 0;
    struct TYPE **temp_type;

    if (parameter_type->paratp == NULL) {
        return error("There are unnecessary arguments.");
    } else {
        temp_type = &(parameter_type->paratp);
    }

    if ((type_holder = parse_expression(inputfp, outputfp, 1)) == ERROR) { return ERROR; }
    i++;
    if (type_holder != (*temp_type)->ttype) {
        return error("The type of the 1st argument is incorrect.");
    }

    while (token == TCOMMA) {
        token = scan(inputfp);

        if ((type_holder = parse_expression(inputfp, outputfp, 1)) == ERROR) { return ERROR; }
        i++;
        temp_type = &((*temp_type)->paratp);
        if ((*temp_type) != NULL) {
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
        } else {
            return error("Too many arguments.");
        }
    }

    fprintf(outputfp, "\tPUSH\t0, gr1\n");

    if ((*temp_type)->paratp != NULL) {
        return error("Argument shortage");
    }

    return NORMAL;
}

int parse_return_statement(FILE *inputfp, FILE *outputfp) {
    if (token != TRETURN) { return (error("Keyword 'return' is not found")); }
    token = scan(inputfp);

    return NORMAL;
}

int parse_assignment_statement(FILE *inputfp, FILE *outputfp, int is_insubproc) {
    int type_holder = NORMAL, expression_type_holder = NORMAL;
    struct ID *p;

    if ((type_holder = parse_left_part(inputfp, outputfp, &p, is_insubproc, 1)) == ERROR) { return ERROR; }

    if (is_insubproc) {
        fprintf(outputfp, "\tPUSH\t0, gr1\n");
    }

    if (token != TASSIGN) { return (error("Symbol ':=' is not found")); }
    token = scan(inputfp);

    if ((expression_type_holder = parse_expression(inputfp, outputfp, 0)) == ERROR) { return ERROR; }

    if ((type_holder % 100) != (expression_type_holder % 100)) {
        return error("The type of the expression differs from the left part");
    }

    if (is_insubproc) {
        fprintf(outputfp, "\tPOP \tgr2\n");
        fprintf(outputfp, "\tST  \tgr1, 0, gr2\n");
    } else {
        fprintf(outputfp, "\tST  \tgr1, $%s", p->name);
        if (p->procname != NULL) {
            fprintf(outputfp, "%%%s", p->procname);
        }
        fprintf(outputfp, "\n");
    }

    return NORMAL;
}

int parse_left_part(FILE *inputfp, FILE *outputfp, struct ID **p, int is_insubproc, int is_inassign) {
    int type_holder = NORMAL;

    if ((type_holder = parse_variable(inputfp, outputfp, p, 0, is_insubproc, is_inassign)) == ERROR) { return ERROR; }

    return type_holder;
}

int parse_variable(FILE *inputfp, FILE *outputfp, struct ID **p, int is_incall, int is_insubproc, int is_inassign) {
    int temp_refnum = -1, type_holder = NORMAL, expression_type_holder = NORMAL;
    struct TYPE *parameter_type;
    struct NAME *temp_valname;

    init_temp_names();
    if (parse_variable_name(inputfp, outputfp) == ERROR) { return ERROR; }
    temp_valname = temp_name_root;

    if (token == TLSQPAREN) {
        token = scan(inputfp);

        if (token == TNUMBER) {
            temp_refnum = num_attr;
            expression_type_holder = TPINT;
            token = scan(inputfp);
        } else {
            if ((expression_type_holder = parse_expression(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
            temp_refnum = 0;
        }

        if (token != TRSQPAREN) {
            return (error("Symbol ']' is not found at the end of expression"));
        }
        token = scan(inputfp);
    }

    if ((parameter_type = (struct TYPE *) malloc(sizeof(struct TYPE))) == NULL) {
        return error("can not malloc in parse_variable");
    }
    init_type(parameter_type);
    if ((type_holder = ref_id(temp_valname->name, current_procname, temp_refnum, &parameter_type)) ==
        ERROR) { return ERROR; }
    if (!is_insubproc && is_inassign) {
        *p = search_idtab(temp_valname->name, current_procname, 1);
    } else {
        if (command_variable(outputfp, temp_valname->name, current_procname, is_incall) == ERROR) {
            return ERROR;
        }
    }
    release_vallinenum();
    release_names();

    if (expression_type_holder != NORMAL) {
        if (expression_type_holder == TPINT) {
            if (type_holder > 200) {
                type_holder -= 100;
            } else {
                return error("When an expression is attached, the type of variable name must be array type");
            }
        } else {
            return error("When referring to an array type variable,\n"
                         "it is necessary to attach an expression of type integer");
        }
    }

    return type_holder;
}

int parse_expression(FILE *inputfp, FILE *outputfp, int is_incall) {
    int type_holder = NORMAL, operand1_type = NORMAL, operand2_type = NORMAL;

    if ((operand1_type = parse_simple_expression(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
    type_holder = operand1_type;

    while ((token == TEQUAL) || (token == TNOTEQ) || (token == TLE) || (token == TLEEQ) || (token == TGR) ||
           (token == TGREQ)) {
        if (parse_relational_operator(inputfp, outputfp) == ERROR) { return ERROR; }

        fprintf(outputfp, "\tLD  \tgr1, 0, gr1\n");
        fprintf(outputfp, "\tPUSH\t0, gr1\n");

        if ((operand2_type = parse_simple_expression(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
        if ((operand1_type % 100) != (operand2_type % 100)) {
            return error("The operands of relational operators have different types");
        }
        command_expression(outputfp);
        type_holder = TPBOOL;
    }

    return type_holder;
}

int parse_simple_expression(FILE *inputfp, FILE *outputfp, int is_incall) {
    int type_holder = NORMAL, pm_flag = 0, operand1_type = NORMAL, operand2_type = NORMAL;

    switch (token) {
        case TPLUS:
        case TMINUS:
            token = scan(inputfp);
            pm_flag = 1;
            break;
        default:
            pm_flag = 0;
            break;
    }

    if ((operand1_type = parse_term(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
    if (pm_flag == 1 && operand1_type != TPINT) {
        return error("The operands of + and - must be of type integer");
    }

    while ((token == TPLUS) || (token == TMINUS) || (token == TOR)) {
        pm_flag = token;
        if (parse_additive_operator(inputfp, outputfp) == ERROR) { return ERROR; }

        fprintf(outputfp, "\tLD  \tgr1, 0, gr1\n");
        fprintf(outputfp, "\tPUSH\t0, gr1\n");

        if ((operand2_type = parse_term(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
        if (pm_flag != TOR && (operand1_type != TPINT || operand2_type != TPINT)) {
            return error("The operands of '+' and '-' must be of type integer");
        }
        if (pm_flag == TOR && (operand1_type != TPBOOL || operand2_type != TPBOOL)) {
            return error("The operands of 'or' must be of type boolean");
        }
        command_simple_expression(outputfp, pm_flag);
    }
    type_holder = operand1_type;

    return type_holder;
}

int parse_term(FILE *inputfp, FILE *outputfp, int is_incall) {
    char *labelname = NULL;
    char output_buf_add[MAX_OUTPUT_BUF_SIZE];

    int type_holder = NORMAL, md_flag = 0, operand1_type = NORMAL, operand2_type = NORMAL;

    if ((operand1_type = parse_factor(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }

    while ((token == TSTAR) || (token == TDIV) || (token == TAND)) {
        md_flag = token;
        if (parse_multiplicative_operator(inputfp, outputfp) == ERROR) { return ERROR; }

        fprintf(outputfp, "\tLD  \tgr1, 0, gr1\n");
        fprintf(outputfp, "\tPUSH\t0, gr1\n");

        if ((operand2_type = parse_factor(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
        if (md_flag != TAND && (operand1_type != TPINT || operand2_type != TPINT)) {
            return error("The operands of '*' and 'div' must be of type integer");
        }
        if (md_flag == TAND && (operand1_type != TPBOOL || operand2_type != TPBOOL)) {
            return error("The operands of 'and' must be of type boolean");
        }
        command_term(outputfp, md_flag);

        if (is_incall == 1) {
            create_newlabel(&labelname);
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
        }
    }
    type_holder = operand1_type;

    return type_holder;
}

int parse_factor(FILE *inputfp, FILE *outputfp, int is_incall) {
    int type_holder = NORMAL, expression_type_holder = NORMAL;
    struct ID *p;

    switch (token) {
        case TNAME:
            //todo parse_variable(inputfp, outputfp, &p, is_call, 0, 0) 5つめの引数
            if ((type_holder = parse_variable(inputfp, outputfp, &p, is_incall, 0, 0)) == ERROR) { return ERROR; }
            break;
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if ((type_holder = parse_constant(inputfp, outputfp)) == ERROR) { return ERROR; }
            break;
        case TLPAREN:
            token = scan(inputfp);

            if ((type_holder = parse_expression(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            token = scan(inputfp);
            break;
        case TNOT:
            token = scan(inputfp);

            if ((type_holder = parse_factor(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
            if (type_holder != TPBOOL) {
                return error("The operand of 'not' must be of type boolean.");
            }
            break;
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if ((type_holder = parse_standard_type(inputfp, outputfp)) == ERROR) { return ERROR; }

            if (token != TLPAREN) {
                return (error("Symbol '(' is not found in factor"));
            }
            token = scan(inputfp);

            if ((expression_type_holder = parse_expression(inputfp, outputfp, is_incall)) == ERROR) { return ERROR; }
            if (check_standard_type(expression_type_holder) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            token = scan(inputfp);
            break;
        default:
            return (error("Factor is not found"));
    }

    return type_holder;
}

int parse_constant(FILE *inputfp, FILE *outputfp) {
    int type_holder = NORMAL;

    switch (token) {
        case TNUMBER:
            command_constant_num(outputfp, num_attr);
            type_holder = TPINT;
            token = scan(inputfp);
            break;
        case TFALSE:
            command_constant_num(outputfp, 0);
            type_holder = TPBOOL;
            token = scan(inputfp);
            break;
        case TTRUE:
            command_constant_num(outputfp, 1);
            type_holder = TPBOOL;
            token = scan(inputfp);
            break;
        case TSTRING:
            if ((int) strlen(string_attr) != 1) {
                return error("Constant string length must be 1");
            }
            //todo
            token = scan(inputfp);
            type_holder = TPCHAR;
            break;
        default:
            return (error("Constant is not found"));
    }

    return type_holder;
}

int parse_multiplicative_operator(FILE *inputfp, FILE *outputfp) {
    switch (token) {
        case TSTAR:
        case TDIV:
        case TAND:
            token = scan(inputfp);
            break;
        default:
            return (error("Multiplicative operator is not found"));
    }

    return NORMAL;
}

int parse_additive_operator(FILE *inputfp, FILE *outputfp) {
    switch (token) {
        case TPLUS:
        case TMINUS:
        case TOR:
            token = scan(inputfp);
            break;
        default:
            return (error("Additive operator is not found"));
    }

    return NORMAL;
}

int parse_relational_operator(FILE *inputfp, FILE *outputfp) {
    switch (token) {
        case TEQUAL:
        case TNOTEQ:
        case TLE:
        case TLEEQ:
        case TGR:
        case TGREQ:
            token = scan(inputfp);
            break;
        default:
            return (error("Relational operator is not found"));
    }

    return NORMAL;
}

int parse_input_statement(FILE *inputfp, FILE *outputfp) {
    int is_ln = token;
    struct ID *p;

    int type_holder = NORMAL;
    switch (token) {
        case TREAD:
        case TREADLN:
            token = scan(inputfp);
            break;
        default:
            return (error("Keyword 'read', 'readln' is not found"));
    }

    if (token == TLPAREN) {
        token = scan(inputfp);

        //todo parse_variable(inputfp, outputfp, &p, 0, 0, 0) 5つめの引数
        if ((type_holder = parse_variable(inputfp, outputfp, &p, 0, 0, 0)) == ERROR) { return ERROR; }
        if (type_holder != TPINT && type_holder != TPCHAR) {
            return error("The variable in the input statement is not integer type or char type");
        } else if (type_holder == TPINT) {
            command_read_int(outputfp);
        } else if (type_holder == TPCHAR) {
            //todo
        }

        while (token == TCOMMA) {
            token = scan(inputfp);

            //todo parse_variable(inputfp, outputfp, &p, 0, 0, 0) 5つめの引数
            if ((type_holder = parse_variable(inputfp, outputfp, &p, 0, 0, 0)) == ERROR) { return ERROR; }
            if (type_holder != TPINT && type_holder != TPCHAR) {
                return error("The variable in the input statement is not integer type or char type");
            } else if (type_holder == TPINT) {
                command_read_int(outputfp);
            } else if (type_holder == TPCHAR) {
                //todo
            }
        }
        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        token = scan(inputfp);
    }

    if (is_ln == TREADLN) {
        fprintf(outputfp, "\tCALL\tREADLINE\n");
        on_pl_flag(PLREADLINE);
    }

    return NORMAL;
}

int parse_output_statement(FILE *inputfp, FILE *outputfp) {
    int is_ln = token;

    switch (token) {
        case TWRITE:
        case TWRITELN:
            token = scan(inputfp);
            break;
        default:
            return (error("Keyword 'write', 'writeln' is not found"));
    }

    if (token == TLPAREN) {
        token = scan(inputfp);

        if (parse_output_format(inputfp, outputfp) == ERROR) { return ERROR; }

        while (token == TCOMMA) {
            token = scan(inputfp);

            if (parse_output_format(inputfp, outputfp) == ERROR) { return ERROR; }
        }
        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        token = scan(inputfp);
    }

    if (is_ln == TWRITELN) {
        fprintf(outputfp, "\tCALL\tWRITELINE\n");
        on_pl_flag(PLWRITELINE);
    }

    return NORMAL;
}

int parse_output_format(FILE *inputfp, FILE *outputfp) {
    int str_length = 0, type_holder = NORMAL;

    switch (token) {
        case TSTRING:
            str_length = (int) strlen(string_attr);
            if (str_length == 0 || str_length >= 2) {
                command_write_string(outputfp, string_attr);
                token = scan(inputfp);
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
            if ((type_holder = parse_expression(inputfp, outputfp, 0)) == ERROR) { return ERROR; }
            if (check_standard_type(type_holder) == ERROR) { return ERROR; }

            if (token == TCOLON) {
                token = scan(inputfp);

                if (token != TNUMBER) { return (error("Number is not found")); }
                command_write_expression(outputfp, type_holder, num_attr);
                token = scan(inputfp);
            } else {
                command_write_expression(outputfp, type_holder, 0);
            }
            break;
        default:
            return (error("Output format is not found"));
    }

    return NORMAL;
}