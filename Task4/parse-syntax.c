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

/* Variable to be determined in each syntax */
int is_insubproc = 0, is_incall = 0, is_inassign = 0, is_inleft_part = 0, is_ininput = 0;

/* prototype declaration */
int parse_block(char *start_labelname);

int parse_variable_declaration();

int parse_variable_names();

int parse_variable_name();

int parse_type();

int parse_standard_type();

int parse_array_type();

int parse_subprogram_declaration();

int parse_procedure_name();

int parse_formal_parameters();

int parse_compound_statement();

int parse_statement();

int parse_condition_statement();

int parse_iteration_statement();

int parse_exit_statement();

int parse_call_statement();

int parse_expressions(struct TYPE *parameter_type);

int parse_return_statement();

int parse_assignment_statement();

int parse_left_part(struct ID **p);

int parse_variable(struct ID **p);

int parse_expression(int *is_computed);

int parse_simple_expression(struct ID **p, int *is_computed);

int parse_term(struct ID **p, int *is_computed);

int parse_factor(struct ID **p, int *is_computed);

int parse_constant();

int parse_multiplicative_operator();

int parse_additive_operator();

int parse_relational_operator();

int parse_input_statement();

int parse_output_statement();

int parse_output_format();

int parse_program() {
    char *start_labelname = NULL;

    if (token != TPROGRAM) { return (error("Keyword 'program' is not found")); }
    token = scan();

    if (token != TNAME) { return (error("Program name is not found")); }
    command_start(string_attr, &start_labelname);
    token = scan();

    if (token != TSEMI) { return (error("Semicolon is not found")); }
    token = scan();

    if (parse_block(start_labelname) == ERROR) { return ERROR; }

    if (token != TDOT) { return (error("Period is not found at the end of program")); }
    token = scan();

    return NORMAL;
}

int parse_block(char *start_labelname) {
    current_procname = NULL;

    /* Variable declaration and subprogram processing */
    while ((token == TVAR) || (token == TPROCEDURE)) {
        if (current_procname != NULL) {
            free(current_procname);
            current_procname = NULL;
        }
        switch (token) {
            case TVAR:
                if (parse_variable_declaration() == ERROR) { return ERROR; }
                break;
            case TPROCEDURE:
                if (parse_subprogram_declaration() == ERROR) { return ERROR; }
                break;
            default:
                break;
        }
    }
    if (current_procname != NULL) {
        free(current_procname);
        current_procname = NULL;
    }

    command_label(start_labelname);

    if (parse_compound_statement() == ERROR) { return ERROR; }

    command_return();

    return NORMAL;
}

int parse_variable_declaration() {
    struct NAME *loop_name;

    if (token != TVAR) { return (error("Keyword 'var' is not found")); }
    token = scan();

    init_temp_names();
    if (parse_variable_names() == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol ':' is not found")); }
    token = scan();

    init_type(&temp_type);
    if (parse_type() == ERROR) { return ERROR; }

    /* Define id as many as name */
    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, &temp_type, 0) == ERROR) { return ERROR; }
    }
    release_vallinenum();
    release_names();

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    token = scan();

    while (token == TNAME) {
        /* Repeat */
        init_temp_names();
        if (parse_variable_names() == ERROR) { return ERROR; }

        if (token != TCOLON) { return (error("Symbol ':' is not found")); }
        token = scan();

        init_type(&temp_type);
        if (parse_type() == ERROR) { return ERROR; }

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, &temp_type, 0) == ERROR) { return ERROR; }
        }
        release_vallinenum();
        release_names();

        if (token != TSEMI) { return (error("Symbol ';' is not found")); }
        token = scan();
    }

    return NORMAL;
}

int parse_variable_names() {
    if (parse_variable_name() == ERROR) { return ERROR; }

    while (token == TCOMMA) {
        token = scan();
        if (parse_variable_name() == ERROR) { return ERROR; }
    }

    return NORMAL;
}

int parse_variable_name() {
    if (token != TNAME) { return (error("Name is not found")); }
    if (temp_names(string_attr) == ERROR) { return ERROR; }
    if (save_vallinenum() == ERROR) { return ERROR; }
    token = scan();

    return NORMAL;
}

int parse_type() {
    switch (token) {
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if (parse_standard_type() == ERROR) { return ERROR; }
            break;
        case TARRAY:
            if (parse_array_type() == ERROR) { return ERROR; }
            break;
        default:
            return (error("Type is not found"));
    }

    return NORMAL;
}

int parse_standard_type() {
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
            token = scan();
            return type_holder;
        default:
            return (error("Standard type is not found"));
    }
}

int parse_array_type() {
    if (token != TARRAY) { return (error("Keyword 'array' is not found")); }
    token = scan();

    if (token != TLSQPAREN) { return (error("Symbol '[' is not found")); }
    token = scan();

    if (token != TNUMBER) { return (error("Number is not found")); }
    if (num_attr < 1) {
        return error("Subscript of array is negative value");
    }
    temp_type.arraysize = num_attr;
    token = scan();

    if (token != TRSQPAREN) { return (error("Symbol ']' is not found")); }
    token = scan();

    if (token != TOF) { return (error("Keyword 'of' is not found")); }
    token = scan();

    if (parse_standard_type() == ERROR) { return ERROR; }
    temp_type.ttype += 100;

    return NORMAL;
}

int parse_subprogram_declaration() {
    struct ID *p;

    is_insubproc = 1;

    if (token != TPROCEDURE) { return (error("Keyword 'procedure' is not found")); }
    token = scan();

    if (parse_procedure_name() == ERROR) { return ERROR; }

    init_type(&temp_type);
    temp_type.ttype = TPPROC;
    temp_type.paratp = NULL;
    end_type = &temp_type;
    paraidroot.paraidp = NULL;
    paraidroot.nextparaidp = NULL;
    paraidend = &paraidroot;
    if (token == TLPAREN) {
        if (parse_formal_parameters() == ERROR) { return ERROR; }
    }

    if (def_id(current_procname, NULL, &temp_type, 0) == ERROR) { return ERROR; }
    release_vallinenum();

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    token = scan();

    if (token == TVAR) {
        if (parse_variable_declaration() == ERROR) { return ERROR; }
    }

    /* output label of sub proc id */
    if ((p = search_idtab(current_procname, NULL, 1)) == NULL) {
        return error("%s is not defined.", current_procname);
    } else {
        create_id_label(p);
        command_process_arguments();
    }

    if (parse_compound_statement() == ERROR) { return ERROR; }

    if (token != TSEMI) { return (error("Symbol ';' is not found")); }
    token = scan();

    command_return();

    is_insubproc = 0;

    return NORMAL;
}

int parse_procedure_name() {
    if (token != TNAME) { return (error("Procedure name is not found")); }

    if ((current_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
        return error("can not malloc in parse_procedure_name");
    }
    init_char_array(current_procname, MAX_IDENTIFIER_SIZE + 1);
    strncpy(current_procname, string_attr, MAX_IDENTIFIER_SIZE);

    if (save_vallinenum() == ERROR) { return ERROR; };
    token = scan();

    return NORMAL;
}

int parse_formal_parameters() {
    struct NAME *loop_name;
    struct TYPE *ptype, *next_type;
    struct ID *p;
    struct PARAID *paraidp;

    if (token != TLPAREN) { return (error("Symbol '(' is not found")); }
    token = scan();

    init_temp_names();
    if (parse_variable_names() == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol ':' is not found")); }
    token = scan();

    if (parse_type() == ERROR) { return ERROR; }
    ptype = temp_type.paratp;
    if (check_standard_type_to_pointer(ptype) == ERROR) { return ERROR; }

    for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
        if (def_id(loop_name->name, current_procname, ptype, 1) == ERROR) { return ERROR; }

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
        token = scan();

        init_temp_names();
        if (parse_variable_names() == ERROR) { return ERROR; }

        if (token != TCOLON) { return (error("Symbol ':' is not found")); }
        token = scan();

        if (parse_type() == ERROR) { return ERROR; }
        ptype = ptype->paratp;
        if (check_standard_type_to_pointer(ptype) == ERROR) { return ERROR; }

        for (loop_name = temp_name_root; loop_name != NULL; loop_name = loop_name->nextnamep) {
            if (def_id(loop_name->name, current_procname, ptype, 1) == ERROR) { return ERROR; }
        }
        release_vallinenum();
        release_names();
    }

    if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
    token = scan();

    return NORMAL;
}

int parse_compound_statement() {
    if (token != TBEGIN) { return (error("Keyword 'begin' is not found")); }
    token = scan();

    if (parse_statement() == ERROR) { return ERROR; }

    while (token == TSEMI) {
        if (token != TSEMI) { return (error("Symbol ';' is not found")); }
        token = scan();

        if (parse_statement() == ERROR) { return ERROR; }
    }

    if (token != TEND) { return (error("Keyword 'end' is not found")); }
    token = scan();

    return NORMAL;
}

int parse_statement() {
    switch (token) {
        case TNAME:
            if (parse_assignment_statement() == ERROR) { return ERROR; }
            break;
        case TIF:
            if (parse_condition_statement() == ERROR) { return ERROR; }
            break;
        case TWHILE:
            if (parse_iteration_statement() == ERROR) { return ERROR; }
            break;
        case TBREAK:
            if (parse_exit_statement() == ERROR) { return ERROR; }
            break;
        case TCALL:
            if (parse_call_statement() == ERROR) { return ERROR; }
            break;
        case TRETURN:
            if (parse_return_statement() == ERROR) { return ERROR; }
            break;
        case TREAD:
        case TREADLN:
            if (parse_input_statement() == ERROR) { return ERROR; }
            break;
        case TWRITE:
        case TWRITELN:
            if (parse_output_statement() == ERROR) { return ERROR; }
            break;
        case TBEGIN:
            if (parse_compound_statement() == ERROR) { return ERROR; }
            break;
        default:
            break;
    }

    return NORMAL;
}

int parse_condition_statement() {
    int expression_type_holder = NORMAL, is_computed = 0;
    char *if_labelname = NULL, *else_labelname = NULL;

    if (create_newlabel(&if_labelname) == ERROR) { return ERROR; }
    if (create_newlabel(&else_labelname) == ERROR) { return ERROR; }

    if (token != TIF) { return (error("Keyword 'if' is not found")); }
    token = scan();

    if ((expression_type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }
    if (expression_type_holder != TPBOOL) {
        return error("The type of the expression in the condition statement must be boolean");
    }
    command_condition_statement(if_labelname);

    if (token != TTHEN) { return (error("Keyword 'then is not found")); }
    token = scan();
    if (parse_statement() == ERROR) { return ERROR; }

    if (token == TELSE) {
        command_jump(else_labelname);
        command_label(if_labelname);

        token = scan();

        if (parse_statement() == ERROR) { return ERROR; }
        command_label(else_labelname);
    } else {
        command_label(if_labelname);
    }

    return NORMAL;
}

int parse_iteration_statement() {
    int expression_type_holder = NORMAL, is_computed = 0;
    char *while_labelname[2] = {NULL};
    char *temp_exit_label = NULL;

    if (create_newlabel(&(while_labelname[0])) == ERROR) { return ERROR; }
    if (create_newlabel(&(while_labelname[1])) == ERROR) { return ERROR; }
    temp_exit_label = exit_label;

    if (token != TWHILE) { return (error("Keyword 'while' is not found")); }
    token = scan();

    command_label(while_labelname[0]);

    if ((expression_type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }
    if (expression_type_holder != TPBOOL) {
        return error("The type of the expression in the iteration statement must be boolean");
    }

    if (token != TDO) { return (error("Keyword 'do' is not found")); }
    token = scan();
    whether_inside_iteration++;

    command_condition_statement(while_labelname[1]);
    exit_label = while_labelname[1];

    if (parse_statement() == ERROR) { return ERROR; }
    whether_inside_iteration--;

    exit_label = temp_exit_label;
    command_jump(while_labelname[0]);
    command_label(while_labelname[1]);

    return NORMAL;
}

int parse_exit_statement() {
    if (token != TBREAK) { return (error("Keyword 'break' is not found")); }
    if (whether_inside_iteration > 0) {
        token = scan();
    } else {
        return error("Exit statement is not included in iteration statement");
    }
    command_jump(exit_label);

    return NORMAL;
}

int parse_call_statement() {
    struct TYPE *parameter_type;
    char *call_procname;
    char temp_procname[MAX_IDENTIFIER_SIZE + 1];
    call_procname = NULL;

    is_incall = 1;

    if (current_procname != NULL) {
        if ((call_procname = (char *) malloc((MAX_IDENTIFIER_SIZE * sizeof(char)) + 1)) == NULL) {
            return error("can not malloc-1 in parse_call_statement");
        }
        init_char_array(call_procname, MAX_IDENTIFIER_SIZE + 1);
        strncpy(call_procname, current_procname, MAX_IDENTIFIER_SIZE);
    }

    if (token != TCALL) { return (error("Keyword 'call' is not found")); }
    token = scan();

    if (parse_procedure_name() == ERROR) { return ERROR; }

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
        if (ref_id(current_procname, NULL, &parameter_type) == ERROR) { return ERROR; }
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
        token = scan();

        if (parse_expressions(parameter_type) == ERROR) { return ERROR; }

        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        token = scan();
    }

    command_expressions(temp_procname);

    is_incall = 0;

    return NORMAL;
}

int parse_expressions(struct TYPE *parameter_type) {
    int type_holder = NORMAL, i = 0, is_computed = 0;
    struct TYPE **temp_type;

    if (parameter_type->paratp == NULL) {
        return error("There are unnecessary arguments.");
    } else {
        temp_type = &(parameter_type->paratp);
    }

    if ((type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }
    i++;
    if (type_holder != (*temp_type)->ttype) {
        return error("The type of the 1st argument is incorrect.");
    }

    while (token == TCOMMA) {
        token = scan();
        if (!is_computed) {
            command_push_gr1();
        }
        is_computed = 0;

        if ((type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }
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

    command_push_gr1();

    if ((*temp_type)->paratp != NULL) {
        return error("Argument shortage");
    }

    return NORMAL;
}

int parse_return_statement() {
    if (token != TRETURN) { return (error("Keyword 'return' is not found")); }
    token = scan();
    command_return();

    return NORMAL;
}

int parse_assignment_statement() {
    int type_holder = NORMAL, expression_type_holder = NORMAL, is_computed = 0;
    struct ID *p;

    is_inassign = 1;

    if ((type_holder = parse_left_part(&p)) == ERROR) { return ERROR; }

    if (is_insubproc && p->ispara) {
        command_push_gr1();
    }

    if (token != TASSIGN) { return (error("Symbol ':=' is not found")); }
    token = scan();

    if ((expression_type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }

    if ((type_holder % 100) != (expression_type_holder % 100)) {
        return error("The type of the expression differs from the left part");
    }
    command_assign(is_insubproc, p);

    is_inassign = 0;

    return NORMAL;
}

int parse_left_part(struct ID **p) {
    int type_holder = NORMAL;

    is_inleft_part = 1;

    if ((type_holder = parse_variable(p)) == ERROR) { return ERROR; }

    is_inleft_part = 0;

    return type_holder;
}

int parse_variable(struct ID **p) {
    int type_holder = NORMAL, expression_type_holder = NORMAL, is_index = 0, is_computed = 0;
    struct TYPE *parameter_type;
    struct NAME *temp_valname;

    /* Get variable name */
    init_temp_names();
    if (parse_variable_name() == ERROR) { return ERROR; }
    temp_valname = temp_name_root;

    if (token == TLSQPAREN) {
        /* Handling expressions for arrays */
        token = scan();

        if ((expression_type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }

        if (token != TRSQPAREN) {
            return (error("Symbol ']' is not found at the end of expression"));
        }
        token = scan();

        is_index = 1;
    }

    if ((parameter_type = (struct TYPE *) malloc(sizeof(struct TYPE))) == NULL) {
        return error("can not malloc in parse_variable");
    }
    init_type(parameter_type);
    if ((type_holder = ref_id(temp_valname->name, current_procname, &parameter_type)) == ERROR) { return ERROR; }
    if ((*p = search_idtab(temp_valname->name, current_procname, 1)) == NULL) {
        return error("%s is not defined", current_procname);
    }

    if (is_index) {
        command_judge_index((*p)->itp->arraysize);
    }

    if ((is_insubproc || !(is_inassign && is_inleft_part))
        && !(is_inleft_part && !(*p)->ispara)) {
        if (command_variable(*p, is_incall, is_insubproc, is_ininput, is_index) == ERROR) { return ERROR; }
    } else if (is_index) {
        /* PUSH array index */
        command_push_gr1();
    }

    if ((is_incall && (token != TRPAREN && token != TCOMMA))
        || (is_insubproc && !is_incall && !is_inleft_part && !is_ininput && (*p)->ispara)) {
        command_ld_gr1_0_gr1();
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

int parse_expression(int *is_computed) {
    int type_holder = NORMAL, operand1_type = NORMAL, operand2_type = NORMAL, opr = 0;
    struct ID *p = NULL;

    if ((operand1_type = parse_simple_expression(&p, is_computed)) == ERROR) { return ERROR; }
    type_holder = operand1_type;

    while ((token == TEQUAL) || (token == TNOTEQ) || (token == TLE) || (token == TLEEQ) || (token == TGR) ||
           (token == TGREQ)) {
        opr = token;
        if (parse_relational_operator() == ERROR) { return ERROR; }

        command_push_gr1();

        if ((operand2_type = parse_simple_expression(&p, is_computed)) == ERROR) { return ERROR; }
        if ((operand1_type % 100) != (operand2_type % 100)) {
            return error("The operands of relational operators have different types");
        }
        type_holder = TPBOOL;

        if (command_expression(opr) == ERROR) { return ERROR; }

        if (is_insubproc || is_incall) { (*is_computed) = 1; };
    }

    if (is_incall && (*is_computed)) {
        command_expression_by_call();
    }

    return type_holder;
}

int parse_simple_expression(struct ID **p, int *is_computed) {
    int type_holder = NORMAL, pm_flag = 0, operand1_type = NORMAL, operand2_type = NORMAL;

    pm_flag = token;
    if (pm_flag == TPLUS || pm_flag == TMINUS) {
        token = scan();
    }

    if ((operand1_type = parse_term(p, is_computed)) == ERROR) { return ERROR; }
    if ((pm_flag == TPLUS || pm_flag == TMINUS) && operand1_type != TPINT) {
        return error("The operands of + and - must be of type integer");
    }
    if (pm_flag == TMINUS) {
        command_minus(is_incall);
        *is_computed = 1;
    }

    while ((token == TPLUS) || (token == TMINUS) || (token == TOR)) {
        pm_flag = token;
        if (parse_additive_operator() == ERROR) { return ERROR; }

        command_push_gr1();

        if ((operand2_type = parse_term(p, is_computed)) == ERROR) { return ERROR; }
        if (pm_flag != TOR && (operand1_type != TPINT || operand2_type != TPINT)) {
            return error("The operands of '+' and '-' must be of type integer");
        }
        if (pm_flag == TOR && (operand1_type != TPBOOL || operand2_type != TPBOOL)) {
            return error("The operands of 'or' must be of type boolean");
        }
        command_simple_expression(pm_flag);

        if (is_insubproc || is_incall) { *is_computed = 1; };
    }
    type_holder = operand1_type;

    return type_holder;
}

int parse_term(struct ID **p, int *is_computed) {

    int type_holder = NORMAL, md_flag = 0, operand1_type = NORMAL, operand2_type = NORMAL;

    if ((operand1_type = parse_factor(p, is_computed)) == ERROR) { return ERROR; }

    while ((token == TSTAR) || (token == TDIV) || (token == TAND)) {
        md_flag = token;
        if (parse_multiplicative_operator() == ERROR) { return ERROR; }

        command_push_gr1();

        if ((operand2_type = parse_factor(p, is_computed)) == ERROR) { return ERROR; }
        if (md_flag != TAND && (operand1_type != TPINT || operand2_type != TPINT)) {
            return error("The operands of '*' and 'div' must be of type integer");
        }
        if (md_flag == TAND && (operand1_type != TPBOOL || operand2_type != TPBOOL)) {
            return error("The operands of 'and' must be of type boolean");
        }
        command_term(md_flag);

        if (is_insubproc || is_incall) { *is_computed = 1; };
    }
    type_holder = operand1_type;

    return type_holder;
}

int parse_factor(struct ID **p, int *is_computed) {
    int type_holder = NORMAL, expression_type_holder = NORMAL;

    switch (token) {
        case TNAME:
            if ((type_holder = parse_variable(p)) == ERROR) { return ERROR; }
            break;
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if ((type_holder = parse_constant()) == ERROR) { return ERROR; }
            break;
        case TLPAREN:
            token = scan();

            if ((type_holder = parse_expression(is_computed)) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }
            token = scan();
            break;
        case TNOT:
            token = scan();
            if ((type_holder = parse_factor(p, is_computed)) == ERROR) { return ERROR; }
            if (type_holder != TPBOOL) {
                return error("The operand of 'not' must be of type boolean.");
            }
            command_factor_not_factor();
            break;
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if ((type_holder = parse_standard_type()) == ERROR) { return ERROR; }

            if (token != TLPAREN) {
                return (error("Symbol '(' is not found in factor"));
            }
            token = scan();

            if ((expression_type_holder = parse_expression(is_computed)) == ERROR) { return ERROR; }

            if (check_standard_type(expression_type_holder) == ERROR) { return ERROR; }

            if (token != TRPAREN) {
                return (error("Symbol ')' is not found at the end of factor"));
            }

            command_factor_cast(type_holder, expression_type_holder);

            token = scan();
            break;
        default:
            return (error("Factor is not found"));
    }

    return type_holder;
}

int parse_constant() {
    int type_holder = NORMAL;

    switch (token) {
        case TNUMBER:
            command_constant_num(num_attr);
            type_holder = TPINT;
            token = scan();
            break;
        case TFALSE:
            command_constant_num(0);
            type_holder = TPBOOL;
            token = scan();
            break;
        case TTRUE:
            command_constant_num(1);
            type_holder = TPBOOL;
            token = scan();
            break;
        case TSTRING:
            if ((int) strlen(string_attr) != 1) {
                return error("Constant string length must be 1");
            }
            command_constant_num(string_attr[0]);
            token = scan();
            type_holder = TPCHAR;
            break;
        default:
            return (error("Constant is not found"));
    }

    return type_holder;
}

int parse_multiplicative_operator() {
    switch (token) {
        case TSTAR:
        case TDIV:
        case TAND:
            token = scan();
            break;
        default:
            return (error("Multiplicative operator is not found"));
    }

    return NORMAL;
}

int parse_additive_operator() {
    switch (token) {
        case TPLUS:
        case TMINUS:
        case TOR:
            token = scan();
            break;
        default:
            return (error("Additive operator is not found"));
    }

    return NORMAL;
}

int parse_relational_operator() {
    switch (token) {
        case TEQUAL:
        case TNOTEQ:
        case TLE:
        case TLEEQ:
        case TGR:
        case TGREQ:
            token = scan();
            break;
        default:
            return (error("Relational operator is not found"));
    }

    return NORMAL;
}

int parse_input_statement() {
    int is_ln = token;
    struct ID *p;

    is_ininput = 1;

    int type_holder = NORMAL;
    switch (token) {
        case TREAD:
        case TREADLN:
            token = scan();
            break;
        default:
            return (error("Keyword 'read', 'readln' is not found"));
    }

    if (token == TLPAREN) {
        token = scan();

        if ((type_holder = parse_variable(&p)) == ERROR) { return ERROR; }
        if (type_holder == TPINT) {
            command_read_int();
        } else if (type_holder == TPCHAR) {
            command_read_char();
        } else {
            return error("The variable in the input statement is not integer type or char type");
        }

        while (token == TCOMMA) {
            token = scan();

            if ((type_holder = parse_variable(&p)) == ERROR) { return ERROR; }
            if (type_holder == TPINT) {
                command_read_int();
            } else if (type_holder == TPCHAR) {
                command_read_char();
            } else {
                return error("The variable in the input statement is not integer type or char type");
            }
        }
        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        token = scan();
    }

    if (is_ln == TREADLN) {
        command_read_line();
    }

    is_ininput = 0;

    return NORMAL;
}

int parse_output_statement() {
    int is_ln = token;

    switch (token) {
        case TWRITE:
        case TWRITELN:
            token = scan();
            break;
        default:
            return (error("Keyword 'write', 'writeln' is not found"));
    }

    if (token == TLPAREN) {
        token = scan();

        if (parse_output_format() == ERROR) { return ERROR; }

        while (token == TCOMMA) {
            token = scan();

            if (parse_output_format() == ERROR) { return ERROR; }
        }
        if (token != TRPAREN) { return (error("Symbol ')' is not found")); }
        token = scan();
    }

    if (is_ln == TWRITELN) {
        command_write_line();
    }

    return NORMAL;
}

int parse_output_format() {
    int str_length = 0, type_holder = NORMAL, is_computed = 0;

    switch (token) {
        case TSTRING:
            str_length = (int) strlen(string_attr);
            if (str_length == 0 || str_length >= 2) {
                command_write_string(string_attr);
                token = scan();
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
            if ((type_holder = parse_expression(&is_computed)) == ERROR) { return ERROR; }
            if (check_standard_type(type_holder) == ERROR) { return ERROR; }

            if (token == TCOLON) {
                token = scan();

                if (token != TNUMBER) { return (error("Number is not found")); }
                command_write_expression(type_holder, num_attr);
                token = scan();
            } else {
                command_write_expression(type_holder, 0);
            }
            break;
        default:
            return (error("Output format is not found"));
    }

    return NORMAL;
}