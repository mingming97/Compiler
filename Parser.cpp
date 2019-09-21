#include "Parser.h"

void Parser::parse_init() {
    if (insymbol_with_error(0)) return;
    if (!parse_programme())
        return;
    if (!lexer.over)
        //error
        error.programme_not_finish();
}

bool Parser::parse_programme() {
    parse_const_dec(GLOBAL_SCOPE);
    parse_var_dec(GLOBAL_SCOPE);
    if (lexer.sy == INTSY || lexer.sy == CHARSY)
        parse_nonvoid_func_def(GLOBAL_SCOPE);
    else if (lexer.sy == VOIDSY)
        parse_void_func_def(GLOBAL_SCOPE);
    else {
        //error
        error.unknown_ret_type(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.sy = CHARSY;
        parse_nonvoid_func_def(GLOBAL_SCOPE);
    }
    while (lexer.sy != MAINSY) {
        if (lexer.sy == INTSY || lexer.sy == CHARSY)
            parse_nonvoid_func_def(GLOBAL_SCOPE);
        else if (lexer.sy == VOIDSY)
            parse_void_func_def(GLOBAL_SCOPE);
        else {
            //error
            error.unknown_ret_type(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.sy = CHARSY;
            parse_nonvoid_func_def(GLOBAL_SCOPE);
        }
    }
    if (over_void_main && lexer.sy == MAINSY) {
        insymbol_with_error(0);
        if (lexer.sy != LPARENT) {
            // ERROR
            error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(rparent_lbrace_sy, state_begin_sy);
        }
        insymbol_with_error(0);
        if (lexer.sy != RPARENT) {
            error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            lexer.no_read_next();
        }

        table.push(GLOBAL_SCOPE, MAIN_SCOPE, VOID_RET, lexer.last_line_num, lexer.last_ch_pos);

        inter_code inter_code_tmp;
        inter_code_tmp.type = FUNC_DEC;
        inter_code_tmp.z = MAIN_SCOPE;
        inter_code_tmp.ret_type = VOID_RET;
        push_back(inter_code_tmp);

        insymbol_with_error(0);
        if (lexer.sy != LBRACE) {
            error.lack_lbrace(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            lexer.no_read_next();
        }
        insymbol_with_error(0);
        parse_compound_statement(MAIN_SCOPE);
        if (lexer.sy != RBRACE) {
            error.lack_rbrace(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(func_begin_sy);
            lexer.no_read_next();
        }
        lexer.insymbol();
        return true;
    }
    else {
        error.lack_main();
        return false;
    }
    return true;
}

bool Parser::parse_const_dec(const string &scope) {
    if (scope != GLOBAL_SCOPE && lexer.sy != CONSTSY) return true;
    bool is_first_wrong = false;
    if (lexer.sy != CONSTSY) {
        if (lexer.sy == INTSY || lexer.sy == VOIDSY || lexer.sy == CHARSY)
            return true;
        error.wrong_const_dec(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
        is_first_wrong = true;
    }
    if (!is_first_wrong) {
        insymbol_with_error(0);
        if (parse_const_def(scope)) {
            if (lexer.sy != SEMICOLON) {
                //error
                error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
                lexer.no_read_next();
            }
            lexer.insymbol();
        }
    }
    while (true) {
        if (scope != GLOBAL_SCOPE && lexer.sy != CONSTSY) break;
        if (lexer.sy != CONSTSY) {
            if (lexer.sy == INTSY || lexer.sy == VOIDSY || lexer.sy == CHARSY)
                break;
            error.wrong_const_dec(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            continue;
        }
        insymbol_with_error(0);
        if (!parse_const_def(scope)) continue;
        if (lexer.sy != SEMICOLON) {
            error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            lexer.no_read_next();
        }
        insymbol_with_error(0);
    }
    return true;
}

bool Parser::parse_const_def(const string &scope) {
    string id_tmp;
    int result;
    if (lexer.sy == INTSY) {
        insymbol_with_error(0);
        if (lexer.sy != IDENT) {
            //ERROR PROCESS
            error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            if (scope == GLOBAL_SCOPE)
                lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            else
                lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
            return false;
        }
        id_tmp = lexer.id;
        insymbol_with_error(0);
        if (lexer.sy != BECOMES) {
            //error process
            error.expect_becomes(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            if (scope == GLOBAL_SCOPE)
                lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            else
                lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
            return false;
        }
        insymbol_with_error(0);
        if (!parse_integer(result)) return false;
        table.push(scope, id_tmp, result, lexer.last_line_num, lexer.last_ch_pos);
        while (lexer.sy == COMMA) {
            insymbol_with_error(0);
            if (lexer.sy != IDENT) {
                error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
                if (scope == GLOBAL_SCOPE)
                    lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
                else
                    lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
                return false;
            }
            id_tmp = lexer.id;
            insymbol_with_error(0);
            if (lexer.sy != BECOMES) {
                error.expect_becomes(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
                if (scope == GLOBAL_SCOPE)
                    lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
                else
                    lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
                return false;
            }
            insymbol_with_error(0);
            if (!parse_integer(result)) return false;
            table.push(scope, id_tmp, result, lexer.last_line_num, lexer.last_ch_pos);
        }
    }
    else if (lexer.sy == CHARSY) {
        insymbol_with_error(0);
        if (lexer.sy != IDENT) {
            //ERROR PROCESS
            error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            if (scope == GLOBAL_SCOPE)
                lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            else
                lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
            return false;
        }
        id_tmp = lexer.id;
        insymbol_with_error(0);
        if (lexer.sy != BECOMES) {
            //error process
            error.expect_becomes(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            if (scope == GLOBAL_SCOPE)
                lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            else
                lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
            return false;
        }
        insymbol_with_error(0);
        if (lexer.sy != CHARCON) {
            //ERROR PROCESS
            error.expect_char(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            if (scope == GLOBAL_SCOPE)
                lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
            else
                lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
            return false;
        }
        table.push(scope, id_tmp, lexer.char_con, lexer.last_line_num, lexer.last_ch_pos);
        insymbol_with_error(0);
        while (lexer.sy == COMMA) {
            insymbol_with_error(0);
            if (lexer.sy != IDENT) {
                //ERROR PROCESS
                error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
                if (scope == GLOBAL_SCOPE)
                    lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
                else
                    lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
                return false;
            }
            id_tmp = lexer.id;
            insymbol_with_error(0);
            if (lexer.sy != BECOMES) {
                //error
                error.expect_becomes(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
                if (scope == GLOBAL_SCOPE)
                    lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
                else
                    lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
                return false;
            }
            insymbol_with_error(0);
            if (lexer.sy != CHARCON) {
                //ERROR PROCESS
                error.expect_char(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
                if (scope == GLOBAL_SCOPE)
                    lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
                else
                    lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
                return false;
            }
            table.push(scope, id_tmp, lexer.char_con, lexer.last_line_num, lexer.last_ch_pos);
            insymbol_with_error(0);
        }
    }
    else {
        //error process
        error.expect_legal_type(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        if (scope == GLOBAL_SCOPE)
            lexer.skip(const_begin_sy, var_begin_sy, func_begin_sy);
        else
            lexer.skip(const_begin_sy, var_begin_sy, state_begin_sy);
        return false;
    }
    return true;
}

bool Parser::parse_integer(int &result) {
    int sign = 1;
    if (lexer.sy == MINUS) {
        sign = -1;
        insymbol_with_error(0);
    }
    if (lexer.sy == PLUS)
        insymbol_with_error(0);
    if (!parse_unsigned_integer(result)) return false;
    result *= sign;
    return true;
}

bool Parser::parse_unsigned_integer(int &result) {
    if (lexer.sy != INTCON) {
        //error process
        error.expect_integer(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy, rparent_lbrace_sy, var_begin_sy);
        return false;
    }
    if (lexer.int_con != 0 && lexer.id[0] == '0') {
        //error process
        error.illegal_integer(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy, rparent_lbrace_sy, var_begin_sy);
        return false;
    }
    if (lexer.int_con == 0 && lexer.id.length() > 1) {
        //error process
        error.illegal_integer(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy, rparent_lbrace_sy, var_begin_sy);
        return false;
    }
    result = lexer.int_con;
    insymbol_with_error(0);
    return true;
}

bool Parser::parse_var_dec(const string &scope) {
    if (lexer.sy == VOIDSY) return true;
    if (lexer.sy != INTSY && lexer.sy != CHARSY) return true;

    if (scope == GLOBAL_SCOPE && is_func_def()) return true;
    // is_next_func 用于判断是否是函数定义 是的话返回true

    parse_var_def(scope);
    //parse_var_def也会进行检查函数定义 但是如果一开始就没有
    //定义的话直接返回true，这里面相当于是后续的检查，用于之后
    //的while
    if (lexer.sy != SEMICOLON) {
        //ERROR
        error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    while (parse_var_def(scope)) {
        if (lexer.sy != SEMICOLON) {
            // error
            error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
            lexer.no_read_next();
        }
        insymbol_with_error(0);
    }
    return true;
}

bool Parser::parse_var_def(const string &scope) {
    if (scope == GLOBAL_SCOPE && is_func_def()) return false;
    if (lexer.sy != INTSY && lexer.sy != CHARSY) return false;
    SYMBOL_TYPE type = lexer.sy;
    string tmp_id;
    int length;
    insymbol_with_error(0);
    if (lexer.sy != IDENT) {
        //error
        error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
        return false;
    }
    tmp_id = lexer.id;
    insymbol_with_error(0);
    if (lexer.sy == LBRACK) {
        insymbol_with_error(0);
        if (!parse_unsigned_integer(length)) return false;
        if (length == 0) {
            //error
            error.array_len_zero(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
            return false;
        }
        if (lexer.sy != RBRACK) {
            //ERROR
            error.lack_rbrack(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            //lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
            return false;
        }
        if (type == INTSY) table.push(scope, tmp_id, INT_ARRAY_VAL, length, lexer.last_line_num, lexer.last_ch_pos);
        if (type == CHARSY) table.push(scope, tmp_id, CHAR_ARRAY_VAL, length, lexer.last_line_num, lexer.last_ch_pos);
        insymbol_with_error(0);
    }
    else {
        if (type == INTSY) table.push(scope, tmp_id, VARIABLE, INT_VAL, lexer.last_line_num, lexer.last_ch_pos);
        if (type == CHARSY) table.push(scope, tmp_id, VARIABLE, CHAR_VAL, lexer.last_line_num, lexer.last_ch_pos);
    }
    while (lexer.sy == COMMA) {
        if (insymbol_with_error(0)) return false;
        if (lexer.sy != IDENT) {
            //error
            error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            //lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
            return false;
        }
        tmp_id = lexer.id;
        insymbol_with_error(0);
        if (lexer.sy == LBRACK) {
            insymbol_with_error(0);
            if (!parse_unsigned_integer(length)) return false;
            if (length == 0) {
                //error
                error.array_len_zero(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                //lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
                return false;
            }
            if (lexer.sy != RBRACK) {
                //ERROR
                error.lack_rbrack(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                //lexer.skip(var_begin_sy, state_begin_sy, func_begin_sy);
                return false;
            }
            if (type == INTSY) table.push(scope, tmp_id, INT_ARRAY_VAL, length, lexer.last_line_num, lexer.last_ch_pos);
            if (type == CHARSY) table.push(scope, tmp_id, CHAR_ARRAY_VAL, length, lexer.last_line_num, lexer.last_ch_pos);
            lexer.insymbol();
        }
        else {
            if (type == INTSY) table.push(scope, tmp_id, VARIABLE, INT_VAL, lexer.last_line_num, lexer.last_ch_pos);
            if (type == CHARSY) table.push(scope, tmp_id, VARIABLE, CHAR_VAL, lexer.last_line_num, lexer.last_ch_pos);
        }
    }
    return true;
}

bool Parser::parse_dec_head(const string &scope, string &name) {
    if (lexer.sy != INTSY && lexer.sy != CHARSY) return false;
    RETURN_TYPE ret_type = lexer.sy == INTSY ? INT_RET : CHAR_RET;
    insymbol_with_error(0);
    if (lexer.sy != IDENT) {
        //error
        error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.id = gen_lab();
    }
    name = lexer.id;
    table.push(scope, name, ret_type, lexer.last_line_num, lexer.last_ch_pos);
    inter_code tmp;
    tmp.type = FUNC_DEC;
    tmp.z = name;
    tmp.ret_type = ret_type;
    push_back(tmp);
    insymbol_with_error(0);
    return true;
}

bool Parser::parse_nonvoid_func_def(const string &scope) {
    string name;
    is_return = false;
    parse_dec_head(scope, name);
    if (lexer.sy != LPARENT) {
        //error
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(rparent_lbrace_sy, state_begin_sy, const_begin_sy, var_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_paramlist(name);
    if (lexer.sy != RPARENT) {
        //ERROR
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy, const_begin_sy, var_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    if (lexer.sy != LBRACE) {
        error.lack_lbrace(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy, const_begin_sy, var_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_compound_statement(name);
    if (lexer.sy != RBRACE) {
        //error
        error.lack_rbrace(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(func_begin_sy);
        lexer.no_read_next();
    }
    if (!is_return) {
        error.no_return_value(name);
    }
    lexer.insymbol();
    return true;
}

bool Parser::parse_void_func_def(const string &scope) {
    string name;
    if (lexer.sy != VOIDSY) return false;
    insymbol_with_error(0);
    if (lexer.sy != IDENT) {
        if (lexer.sy == MAINSY) {
            over_void_main = true;
            return true;
        }
        //error
        error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.id = gen_lab();
    }
    name = lexer.id;
    table.push(scope, name, VOID_RET, lexer.last_line_num, lexer.last_ch_pos);
    inter_code tmp;
    tmp.type = FUNC_DEC;
    tmp.z = name;
    tmp.ret_type = VOID_RET;
    push_back(tmp);
    insymbol_with_error(0);
    if (lexer.sy != LPARENT) {
        // ERROR
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(rparent_lbrace_sy, state_begin_sy, const_begin_sy, var_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_paramlist(name);
    if (lexer.sy != RPARENT) {
        //ERROR
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    if (lexer.sy != LBRACE) {
        //ERROR
        error.lack_lbrace(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy, const_begin_sy, var_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_compound_statement(name);
    if (lexer.sy != RBRACE) {
        //error
        error.lack_rbrace(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(func_begin_sy);
        lexer.no_read_next();
    }
    tmp = inter_codes[inter_codes.size() - 1];
    if (tmp.type != FUNC_RET) {
        tmp.type = FUNC_RET;
        tmp.z = "";
        tmp.z_val_type = NOTYP_VAL;
        tmp.x = name;
        push_back(tmp);
    }
    lexer.insymbol();
    return true;
}

bool Parser::parse_compound_statement(const string &scope) {
    parse_const_dec(scope);
    parse_var_dec(scope);
    while (parse_statement(scope));
    return true;
}

bool Parser::parse_parameter(const string &scope) {
    if (lexer.sy != INTSY && lexer.sy != CHARSY) {
        //ERROR
        error.expect_legal_type(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(rparent_lbrace_sy, const_begin_sy, var_begin_sy, state_begin_sy);
        return false;
    }
    VALUE_TYPE val_type = lexer.sy == INTSY ? INT_VAL : CHAR_VAL;
    insymbol_with_error(0);
    if (lexer.sy != IDENT) {
        error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(rparent_lbrace_sy, const_begin_sy, var_begin_sy, state_begin_sy);
        return false;
    }
    table.push(scope, lexer.id, PARAMETER, val_type, lexer.last_line_num, lexer.last_ch_pos);
    inter_code tmp;
    tmp.type = PARA_DEC;
    tmp.z = lexer.id;
    tmp.z_val_type = val_type;
    push_back(tmp);

    insymbol_with_error(0);
    return true;
}

bool Parser::parse_paramlist(const string &scope) {
    if (lexer.sy == RPARENT || lexer.sy == LBRACE) return true;
    if (!parse_parameter(scope)) return false;
    while (lexer.sy == COMMA) {
        lexer.insymbol();
        if (!parse_parameter(scope)) return false;
    }
    return true;
}

bool Parser::parse_expression(const string &scope, exp_ret_info &info) {
    bool is_first_minus = false;
    bool has_first_sign = false;
    exp_ret_info res_info, tmp_info;
    res_info.tmp_name = gen_tmp_var();
    inter_code tmp_inter_code;
    if (lexer.sy == PLUS || lexer.sy == MINUS) {
        has_first_sign = true;
        if (lexer.sy == MINUS)
            is_first_minus = true;
        insymbol_with_error(0);
    }
    if (!parse_term(scope, tmp_info)) return false;
    if (tmp_info.is_valid) {
        res_info.is_valid = true;
        res_info.val_type = tmp_info.val_type;
        if (is_first_minus) res_info.int_val = -tmp_info.int_val;
        else res_info.int_val = tmp_info.int_val;
        if (has_first_sign) res_info.val_type = INT_CON_VAL;
    }
    else {
        res_info.is_valid = false;
        res_info.val_type = tmp_info.val_type;
        if (has_first_sign) res_info.val_type = INT_VAL;
        tmp_inter_code.type = ASSIGN_EXP;
        tmp_inter_code.z = res_info.tmp_name;
        tmp_inter_code.z_val_type = res_info.val_type;
        tmp_inter_code.x = "0";
        tmp_inter_code.x_val_type = INT_CON_VAL;
        tmp_inter_code.op = is_first_minus ? "-" : "+";
        tmp_inter_code.y = tmp_info.tmp_name;
        tmp_inter_code.y_val_type = tmp_info.val_type;
        push_back(tmp_inter_code);
    }
    while (lexer.sy == PLUS || lexer.sy == MINUS) {
        string op = lexer.sy == PLUS ? "+" : "-";
        int sign = op == "+" ? 1 : -1;
        tmp_info.clear();
        if (insymbol_with_error(0)) return false;
        if (!parse_term(scope, tmp_info)) return false;
        if (res_info.is_valid) {
            if (tmp_info.is_valid) {
                res_info.is_valid = true;
                res_info.val_type = INT_CON_VAL;
                res_info.int_val = res_info.int_val + sign * tmp_info.int_val;
            }
            else {
                res_info.is_valid = false;
                res_info.val_type = INT_VAL;
                tmp_inter_code.type = ASSIGN_EXP;
                tmp_inter_code.z = res_info.tmp_name;
                tmp_inter_code.z_val_type = INT_VAL;
                tmp_inter_code.x = my_to_string(res_info.int_val);
                tmp_inter_code.x_val_type = INT_CON_VAL;
                tmp_inter_code.op = op;
                tmp_inter_code.y = tmp_info.tmp_name;
                tmp_inter_code.y_val_type = tmp_info.val_type;
                push_back(tmp_inter_code);
            }
        }
        else {
            res_info.is_valid = false;
            res_info.val_type = INT_VAL;
            tmp_inter_code.type = ASSIGN_EXP;
            tmp_inter_code.z = res_info.tmp_name;
            tmp_inter_code.z_val_type = INT_VAL;
            tmp_inter_code.x = res_info.tmp_name;
            tmp_inter_code.x_val_type = INT_VAL;
            tmp_inter_code.op = op;
            if (tmp_info.is_valid) {
                tmp_inter_code.y = my_to_string(tmp_info.int_val);
                tmp_inter_code.y_val_type = INT_CON_VAL;
            }
            else {
                tmp_inter_code.y = tmp_info.tmp_name;
                tmp_inter_code.y_val_type = tmp_info.val_type;
            }
            push_back(tmp_inter_code);
        }
    }
    info = res_info;
    return true;
}

bool Parser::parse_term(const string &scope, exp_ret_info &info) {
    exp_ret_info tmp_info, res_info;
    inter_code tmp_inter_code;
    res_info.tmp_name = gen_tmp_var();
    if (!parse_factor(scope, tmp_info)) return false;
    if (tmp_info.is_valid) {
        res_info.is_valid = true;
        res_info.val_type = tmp_info.val_type;
        res_info.int_val = tmp_info.int_val;
    }
    else {
        res_info.is_valid = false;
        res_info.val_type = tmp_info.val_type;
        res_info.tmp_name = tmp_info.tmp_name;
        /*tmp_inter_code.type = ASSIGN_EXP;
        tmp_inter_code.z = res_info.tmp_name;
        tmp_inter_code.z_val_type = NOTYP_VAL;
        tmp_inter_code.x = tmp_info.tmp_name;
        tmp_inter_code.x_val_type = tmp_info.val_type;
        tmp_inter_code.op = DIRECT_ASSIGN_OP;
        push_back(tmp_inter_code);*/

    }
    while (lexer.sy == TIMES || lexer.sy == DIV) {
        string op = (lexer.sy == TIMES) ? "*" : "/";
        tmp_info.clear();
        insymbol_with_error(0);
        if (!parse_factor(scope, tmp_info)) return false;
        if (res_info.is_valid) {
            if (tmp_info.is_valid) {
                res_info.is_valid = true;
                res_info.val_type = INT_CON_VAL;
                res_info.int_val = op == "*" ? res_info.int_val * tmp_info.int_val :
                                               res_info.int_val / tmp_info.int_val;
            }
            else {
                res_info.is_valid = false;
                res_info.val_type = INT_VAL;
                tmp_inter_code.type = ASSIGN_EXP;
                tmp_inter_code.z = res_info.tmp_name;
                tmp_inter_code.z_val_type = INT_VAL;
                tmp_inter_code.x = my_to_string(res_info.int_val);
                tmp_inter_code.x_val_type = INT_CON_VAL;
                tmp_inter_code.op = op;
                tmp_inter_code.y = tmp_info.tmp_name;
                tmp_inter_code.y_val_type = tmp_info.val_type;
                push_back(tmp_inter_code);
            }
        }
        else {
            res_info.is_valid = false;
            res_info.val_type = INT_VAL;
            tmp_inter_code.type = ASSIGN_EXP;
            tmp_inter_code.z = res_info.tmp_name;
            tmp_inter_code.z_val_type = INT_VAL;
            tmp_inter_code.x = res_info.tmp_name;
            tmp_inter_code.x_val_type = INT_VAL;
            tmp_inter_code.op = op;
            if (tmp_info.is_valid) {
                tmp_inter_code.y = my_to_string(tmp_info.int_val);
                tmp_inter_code.y_val_type = INT_CON_VAL;
            }
            else {
                tmp_inter_code.y = tmp_info.tmp_name;
                tmp_inter_code.y_val_type = tmp_info.val_type;
            }
            push_back(tmp_inter_code);
        }
    }
    info = res_info;
    return true;
}

bool Parser::parse_factor(const string &scope, exp_ret_info &info) {
    exp_ret_info res_info, tmp_info;
    res_info.tmp_name = gen_tmp_var();
    table_ele ele;
    inter_code tmp_inter_code;
    if (lexer.sy == IDENT){
        if (!table.find(scope, lexer.id, ele)) {
            //error
            error.undefined_id(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy, rparent_lbrace_sy);
            return false;
        }
        insymbol_with_error(0);

        if (lexer.sy == LBRACK) {
            if (ele.val_type != INT_ARRAY_VAL && ele.val_type != CHAR_ARRAY_VAL) {
                //ERROR
                error.expect_array(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy, rparent_lbrace_sy);
                return false;
            }
            insymbol_with_error(0);
            if (!parse_expression(scope, tmp_info)) return false;
            if (tmp_info.val_type == CHAR_VAL || tmp_info.val_type == CHAR_CON_VAL) {
                error.index_type_error(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy, rparent_lbrace_sy);
                return false;
            }

            if (lexer.sy != RBRACK) {
                error.lack_rbrack(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy, rparent_lbrace_sy);
                return false;
            }
            res_info.is_valid = false;
            res_info.val_type = ele.val_type == INT_ARRAY_VAL ? INT_VAL : CHAR_VAL;
            tmp_inter_code.type = ASSIGN_EXP;
            tmp_inter_code.z = res_info.tmp_name;
            tmp_inter_code.z_val_type = res_info.val_type;
            tmp_inter_code.x = code_name(ele.scope, ele.id);
            tmp_inter_code.x_val_type = ele.val_type;
            if (tmp_info.is_valid) {
                if (tmp_info.int_val >= ele.length || tmp_info.int_val < 0) {
                //error
                    error.overlength_array(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                    lexer.skip(';', '\n', true);
                    lexer.skip(state_begin_sy);
                    return false;
                }
                tmp_inter_code.index_x = my_to_string(tmp_info.int_val);
                tmp_inter_code.index_x_val_type = INT_CON_VAL;
            }
            else {
                tmp_inter_code.index_x = tmp_info.tmp_name;
                tmp_inter_code.index_x_val_type = tmp_info.val_type;
            }
            tmp_inter_code.op = DIRECT_ASSIGN_OP;
            push_back(tmp_inter_code);
        }
        else if (lexer.sy == LPARENT) {
            if (ele.id_type != FUNCTION) {
                //error
                error.expect_function(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(state_begin_sy, rparent_lbrace_sy);
                return false;
            }
            if (ele.id_type == FUNCTION && ele.ret_type == VOID_RET) {
                //error
                error.expect_nonvoid(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(state_begin_sy, rparent_lbrace_sy);
                return false;
            }
            if (!parse_nonvoid_func_call(scope, ele)) return false;
            lexer.no_read_next();

            res_info.is_valid = false;
            res_info.val_type = ele.ret_type == INT_RET ? INT_VAL : CHAR_VAL;
            tmp_inter_code.type = ASSIGN_EXP;
            tmp_inter_code.z = res_info.tmp_name;
            tmp_inter_code.z_val_type = res_info.val_type;
            tmp_inter_code.x = "RET";
            tmp_inter_code.x_val_type = NOTYP_VAL;
            tmp_inter_code.op = DIRECT_ASSIGN_OP;
            push_back(tmp_inter_code);
        }
        else {
            if (ele.val_type != INT_VAL && ele.val_type != CHAR_VAL &&
                ele.val_type != INT_CON_VAL && ele.val_type != CHAR_CON_VAL) {
                //ERROR
                error.val_type_wrong(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(state_begin_sy, rparent_lbrace_sy);
                return false;
            }
            lexer.no_read_next();

            if (ele.id_type == CONSTANT) {
                res_info.is_valid = true;
                res_info.val_type = ele.val_type;
                res_info.int_val = ele.val_type == INT_CON_VAL ? ele.int_const : ele.char_const;
            }
            else {
                res_info.is_valid = false;
                res_info.val_type = ele.val_type;
                tmp_inter_code.type = ASSIGN_EXP;
                tmp_inter_code.z = res_info.tmp_name;
                tmp_inter_code.z_val_type = res_info.val_type;
                tmp_inter_code.x = code_name(ele.scope, ele.id);
                tmp_inter_code.x_val_type = ele.val_type;
                tmp_inter_code.op = DIRECT_ASSIGN_OP;
                push_back(tmp_inter_code);
            }
        }
    }
    else if (lexer.sy == LPARENT) {
        if (insymbol_with_error(0)) return false;
        if (!parse_expression(scope, tmp_info)) return false;
        if (lexer.sy != RPARENT) {
            //error
            error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy, rparent_lbrace_sy);
            return false;
        }
        if (tmp_info.is_valid) {
            res_info.is_valid = true;
            res_info.val_type = INT_CON_VAL;
            res_info.int_val = tmp_info.int_val;
        }
        else {
            res_info.is_valid = false;
            res_info.val_type = INT_VAL;
            tmp_inter_code.type = ASSIGN_EXP;
            tmp_inter_code.z = res_info.tmp_name;
            tmp_inter_code.z_val_type = res_info.val_type;
            tmp_inter_code.x = tmp_info.tmp_name;
            tmp_inter_code.x_val_type = tmp_info.val_type;
            tmp_inter_code.op = DIRECT_ASSIGN_OP;
            push_back(tmp_inter_code);
        }
    }
    else if (lexer.sy == CHARCON) {
        res_info.is_valid = true;
        res_info.int_val = lexer.char_con;
        res_info.val_type = CHAR_CON_VAL;
    }
    else {
        int res;
        if (!parse_integer(res)) return false;
        res_info.is_valid = true;
        res_info.int_val = res;
        res_info.val_type = INT_CON_VAL;
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    info = res_info;
    return true;
}

bool Parser::parse_statement(const string &scope) {
    if (lexer.sy == IFSY) {
        parse_if(scope);
        lexer.no_read_next();
    }
    else if (lexer.sy == WHILESY) {
        parse_while(scope);
        lexer.no_read_next();
    }
    else if (lexer.sy == LBRACE) {
        insymbol_with_error(0);
        while (parse_statement(scope));
        if (lexer.sy != RBRACE) {
            //error
            error.lack_rbrace(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return true;
        }
    }
    else if (lexer.sy == IDENT) {
        string id = lexer.id;
        table_ele ele;
        if (!table.find(scope, id, ele)) {
            //ERROR
            error.undefined_id(id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return true;
        }
        insymbol_with_error(0);
        if (lexer.sy == LPARENT) {
            if (ele.id_type != FUNCTION) {
                //error
                error.expect_function(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(state_begin_sy);
                return true;
            }
            else if (ele.ret_type == VOID_RET) {
                if (!parse_void_func_call(scope, ele)) return true;
            }
            else {
                if (!parse_nonvoid_func_call(scope, ele)) return true;
            }
            if (lexer.sy != SEMICOLON) {
                //ERROR
                error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(state_begin_sy);
                return true;
            }
        }
        else if (lexer.sy == BECOMES || lexer.sy == LBRACK) {
            if (!parse_assign(scope, ele)) {
                return true;
            }
            if (lexer.sy != SEMICOLON) {
                //error
                error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(state_begin_sy);
                return true;
            }
        }
        else {
            error.wrong_sy_after_id(id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            return true;
        }
    }
    else if (lexer.sy == SCANFSY) {
        if (!parse_read(scope)) return true;
        if (lexer.sy != SEMICOLON) {
            //ERROR
            error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            return true;
        }
    }
    else if (lexer.sy == PRINTFSY) {
        if (!parse_write(scope)) return true;
        if (lexer.sy != SEMICOLON) {
            //ERROR
            error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            return true;
        }
    }
    else if (lexer.sy == SWITCHSY) {
        if (!parse_switch(scope)) return true;
        lexer.no_read_next();
    }
    else if (lexer.sy == RETURNSY) {
        if (!parse_return(scope)) return true;
        if (lexer.sy != SEMICOLON) {
            //error
            error.lack_semicolon(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            return true;
        }
    }
    else if (lexer.sy == SEMICOLON) {
        insymbol_with_error(0);
        return true;
    }
    else {
        return false;
    }
    lexer.insymbol();
    return true;
}

bool Parser::parse_assign(const string &scope, const table_ele &ele) {
    exp_ret_info tmp_info;
    inter_code tmp_inter_code;
    tmp_inter_code.type = ASSIGN_EXP;
    if (ele.id_type == CONSTANT || ele.id_type == FUNCTION) {
        //ERROR
        error.wrong_rvalue(ele.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }

    if (lexer.sy == BECOMES) {
        if (ele.val_type != INT_VAL && ele.val_type != CHAR_VAL) {
            //error
            error.wrong_rvalue(ele.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        tmp_inter_code.z = code_name(ele.scope, ele.id);
        tmp_inter_code.z_val_type = ele.val_type;
        tmp_inter_code.op = DIRECT_ASSIGN_OP;
        insymbol_with_error(0);
        if (!parse_expression(scope, tmp_info)) return false;
        if (ele.val_type == CHAR_VAL) {
            if (tmp_info.val_type != CHAR_VAL && tmp_info.val_type != CHAR_CON_VAL) {
                //error
                error.type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy);
                return false;
            }
        }
        if (ele.val_type == INT_VAL) {
            if (tmp_info.val_type != INT_VAL && tmp_info.val_type != INT_CON_VAL) {
                //error
                error.type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy);
                return false;
            }
        }
        tmp_inter_code.x = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
        tmp_inter_code.x_val_type = tmp_info.val_type;
        push_back(tmp_inter_code);
    }
    else if (lexer.sy == LBRACK) {
        if (ele.length == 0) {
            //error
            error.expect_array(ele.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(state_begin_sy);
            return false;
        }
        tmp_inter_code.z = code_name(ele.scope, ele.id);
        tmp_inter_code.z_val_type = ele.val_type;
        tmp_inter_code.op = DIRECT_ASSIGN_OP;
        insymbol_with_error(0);
        if (!parse_expression(scope, tmp_info)) return false;
        if (tmp_info.val_type == CHAR_VAL || tmp_info.val_type == CHAR_CON_VAL) {
            error.index_type_error(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        if (tmp_info.is_valid) {
            if (tmp_info.int_val >= ele.length || tmp_info.int_val < 0) {
                //error
                error.overlength_array(ele.id, lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy);
                return false;
            }
            tmp_inter_code.index_z = my_to_string(tmp_info.int_val);
            tmp_inter_code.index_z_val_type = tmp_info.val_type;
        }
        else {
            tmp_inter_code.index_z = tmp_info.tmp_name;
            tmp_inter_code.index_z_val_type = tmp_info.val_type;
        }
        if (lexer.sy != RBRACK) {
            //error
            error.lack_rbrack(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        insymbol_with_error(0);
        if (lexer.sy != BECOMES) {
            //error
            error.expect_becomes(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        insymbol_with_error(0);
        tmp_info.clear();
        if (!parse_expression(scope, tmp_info)) return false;
        if (ele.val_type == CHAR_ARRAY_VAL) {
            if (tmp_info.val_type != CHAR_VAL && tmp_info.val_type != CHAR_CON_VAL) {
                //error
                error.type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
                lexer.skip(';', '\n', true);
                lexer.skip(state_begin_sy);
                return false;
            }
        }
        tmp_inter_code.x = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
        tmp_inter_code.x_val_type = tmp_info.val_type;
        push_back(tmp_inter_code);
    }
    return true;
}

bool Parser::parse_if(const string &scope) {
    string lab;
    inter_code tmp_inter_code;
    if (lexer.sy != IFSY) {
        //ERROR
        return false;
    }
    insymbol_with_error(0);
    if (lexer.sy != LPARENT) {
        //ERROR
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_condition(scope, lab);
    if (lexer.sy != RPARENT) {
        //error
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_statement(scope);
    tmp_inter_code.type = LABEL;
    tmp_inter_code.label = lab;
    push_back(tmp_inter_code);
    return true;
}

bool Parser::parse_condition(const string &scope, string &lab) {
    exp_ret_info tmp_info1, tmp_info2;
    inter_code tmp_inter_code1, tmp_inter_code2;
    lab = gen_lab();
    tmp_inter_code1.type = BNZ;
    tmp_inter_code1.label = lab;
    if (!parse_expression(scope, tmp_info1)) return false;
    if (lexer.sy == EQL || lexer.sy == NEQ || lexer.sy == GTR ||
        lexer.sy == GEQ || lexer.sy == LSS || lexer.sy == LEQ) {
        string cmp = lexer.id;
        insymbol_with_error(0);
        if (!parse_expression(scope, tmp_info2)) return false;
        if ((tmp_info1.val_type != INT_VAL && tmp_info1.val_type != INT_CON_VAL) ||
            (tmp_info2.val_type != INT_VAL && tmp_info2.val_type != INT_CON_VAL)) {
            error.compare_not_int(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(rparent_lbrace_sy, state_begin_sy);
            return false;
        }
        if (!check_type(tmp_info1.val_type, tmp_info2.val_type)) {
            //error
            error.compare_type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(rparent_lbrace_sy, state_begin_sy);
            return false;
        }
        tmp_inter_code2.type = CONDITION;
        tmp_inter_code2.x = tmp_info1.is_valid ? my_to_string(tmp_info1.int_val) : tmp_info1.tmp_name;
        tmp_inter_code2.x_val_type = tmp_info1.val_type;
        tmp_inter_code2.y = tmp_info2.is_valid ? my_to_string(tmp_info2.int_val) : tmp_info2.tmp_name;
        tmp_inter_code2.y_val_type = tmp_info2.val_type;
        tmp_inter_code2.op = cmp;
    }
    else if (lexer.sy == RPARENT){
        if (!check_type(tmp_info1.val_type, INT_CON_VAL)) {
            //error
            error.compare_type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(rparent_lbrace_sy, state_begin_sy);
            return false;
        }
        tmp_inter_code2.type = CONDITION;
        tmp_inter_code2.x = tmp_info1.is_valid ? my_to_string(tmp_info1.int_val) : tmp_info1.tmp_name;
        tmp_inter_code2.x_val_type = tmp_info1.val_type;
        tmp_inter_code2.y = "0";
        tmp_inter_code2.y_val_type = INT_CON_VAL;
        tmp_inter_code2.op = "!=";
    }
    else {
        error.condition_wrong(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(rparent_lbrace_sy, state_begin_sy);
        return false;
    }
    push_back(tmp_inter_code2);
    push_back(tmp_inter_code1);
    return true;
}

bool Parser::parse_while(const string &scope) {
    string labA, labB;
    inter_code tmp_inter_code;
    labA = gen_lab();
    tmp_inter_code.type = LABEL;
    tmp_inter_code.label = labA;
    push_back(tmp_inter_code);
    if (lexer.sy != WHILESY) {
        //error
        return false;
    }
    insymbol_with_error(0);
    if (lexer.sy != LPARENT) {
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_condition(scope, labB);
    if (lexer.sy != RPARENT) {
        //error
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_statement(scope);
    tmp_inter_code.type = GOTO;
    tmp_inter_code.label = labA;
    push_back(tmp_inter_code);
    tmp_inter_code.type = LABEL;
    tmp_inter_code.label = labB;
    push_back(tmp_inter_code);
    return true;
}

bool Parser::parse_switch(const string &scope) {
    if (case_depth == 0) switch_codes.clear();
    exp_ret_info tmp_info;
    string end_label = gen_lab();
    vector<case_table_ele> cases;
    vector<inter_code> local_switch_codes;
    inter_code tmp_inter_code;
    if (lexer.sy != SWITCHSY) {
        //error
        return false;
    }
    insymbol_with_error(0);
    if (lexer.sy != LPARENT) {
        //error
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_expression(scope, tmp_info);
    if (lexer.sy != RPARENT) {
        //ERROR
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    if (lexer.sy != LBRACE) {
        //ERROR
        error.lack_lbrace(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_cond_table(scope, cases, end_label, tmp_info.val_type, local_switch_codes);
    for (auto x : cases) {
        tmp_inter_code.type = CONDITION;
        tmp_inter_code.x = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
        tmp_inter_code.x_val_type = tmp_info.val_type;
        tmp_inter_code.y = my_to_string(x.int_val);
        tmp_inter_code.y_val_type = x.val_type;
        tmp_inter_code.op = "==";
        push_back(tmp_inter_code);
        tmp_inter_code.type = BZ;
        tmp_inter_code.label = x.label_name;
        push_back(tmp_inter_code);
    }
    parse_default(scope, end_label);
    for (auto x: local_switch_codes)
        push_back(x);
    tmp_inter_code.type = LABEL;
    tmp_inter_code.label = end_label;
    push_back(tmp_inter_code);
    if (lexer.sy != RBRACE) {
        //ERROR
        error.lack_rbrace(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    return true;
}

bool Parser::parse_cond_table(const string &scope, vector<case_table_ele> &cases,
                              const string &end_label, VALUE_TYPE val_type, vector<inter_code> &local_switch_codes) {
    case_depth++;
    unsigned int start = switch_codes.size();
    while (parse_case(scope, cases, end_label, val_type));
    local_switch_codes.assign(switch_codes.begin() + start, switch_codes.end());
    switch_codes.erase(switch_codes.begin() + start, switch_codes.end());
    case_depth--;
    return true;
}

bool Parser::parse_case(const string &scope, vector<case_table_ele> &cases,
                        const string &end_label, VALUE_TYPE val_type) {
    if (lexer.sy != CASESY)
        return false;
    case_table_ele ele;
    inter_code tmp_inter_code;
    int res;
    ele.label_name = gen_lab();
    insymbol_with_error(0);
    if (lexer.sy == CHARCON) {
        ele.int_val = (int)lexer.char_con;
        ele.val_type = CHAR_CON_VAL;
    }
    else if (parse_integer(res)) {
        ele.int_val = res;
        ele.val_type = INT_CON_VAL;
        lexer.no_read_next();
    }
    else {
        //error
        error.expect_integer_char(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
    }
    if (!check_type(val_type, ele.val_type)) {
        //error
        error.compare_type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
    }

    bool is_find = false;
    for (auto x: cases) {
        if (x.int_val == ele.int_val) {
            error.case_repeat(my_to_string(x.int_val), lexer.last_line_num, lexer.last_ch_pos);
            is_find = true;
            break;
        }
    }
    if (!is_find) cases.push_back(ele);

    insymbol_with_error(0);
    if (lexer.sy != COLON) {
        //ERROR
        error.lack_colon(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    tmp_inter_code.type = LABEL;
    tmp_inter_code.label = ele.label_name;
    push_back(tmp_inter_code);
    parse_statement(scope);
    tmp_inter_code.type = GOTO;
    tmp_inter_code.label = end_label;
    push_back(tmp_inter_code);

    return true;
}

bool Parser::parse_default(const string &scope, const string &end_label) {
    inter_code tmp_inter_code;
    if (lexer.sy != DEFAULTSY) {
        tmp_inter_code.type = GOTO;
        tmp_inter_code.label = end_label;
        push_back(tmp_inter_code);
        return true;
    }
    insymbol_with_error(0);
    if (lexer.sy != COLON) {
        //error
        error.lack_colon(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(rparent_lbrace_sy, state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    parse_statement(scope);
    tmp_inter_code.type = GOTO;
    tmp_inter_code.label = end_label;
    push_back(tmp_inter_code);
    return true;
}

bool Parser::parse_nonvoid_func_call(const string &scope, const table_ele &ele) {
    inter_code tmp_inter_code;
    if (ele.ret_type == VOID_RET) {
        //error
        return false;
    }
    if (lexer.sy != LPARENT){
        //error
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    if (!parse_value_paramlist(scope, ele)) return false;
    tmp_inter_code.type = FUNC_CALL;
    tmp_inter_code.z = code_name(ele.scope, ele.id);
    tmp_inter_code.x = scope;
    push_back(tmp_inter_code);
    if (lexer.sy != RPARENT) {
        //ERROR
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    lexer.insymbol();
    return true;
}

bool Parser::parse_void_func_call(const string &scope, const table_ele &ele) {
    inter_code tmp_inter_code;
    if (ele.ret_type != VOID_RET) {
        //error
        return false;
    }
    if (lexer.sy != LPARENT) {
        //error
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    if (!parse_value_paramlist(scope, ele)) return false;
    tmp_inter_code.type = FUNC_CALL;
    tmp_inter_code.z = code_name(ele.scope, ele.id);
    tmp_inter_code.x = scope;
    push_back(tmp_inter_code);
    if (lexer.sy != RPARENT) {
        //ERROR
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    lexer.insymbol();
    return true;
}

bool Parser::parse_value_paramlist(const string &scope, const table_ele &ele) {
    string tar_func_name = ele.id;
    exp_ret_info tmp_info;
    table_ele tmp_ele;
    inter_code tmp_inter_code;
    int pos = table.get_first_para_pos(tar_func_name);
    if (lexer.sy == RPARENT && pos == -1) return true;
    if (lexer.sy != RPARENT && pos == -1) {
        //error
        error.param_num_mismatch(tar_func_name, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    if (!parse_expression(scope, tmp_info)) return false;
    if (!table.get_ele(tar_func_name, pos, tmp_ele)) {
        //error
        error.param_mismatch(tar_func_name, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    if (!check_type(tmp_info.val_type, tmp_ele.val_type)) {
        //error
        error.param_mismatch(tar_func_name, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    tmp_inter_code.type = PARA_PUSH;
    tmp_inter_code.z = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
    tmp_inter_code.z_val_type = tmp_info.val_type;
    push_back(tmp_inter_code);
    pos++;
    while (lexer.sy == COMMA) {
        tmp_info.clear();
        insymbol_with_error(0);
        if (!parse_expression(scope, tmp_info)) return false;
        if (!table.get_ele(tar_func_name, pos, tmp_ele)) {
            //error
            error.param_num_mismatch(ele.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        if (tmp_ele.id_type != PARAMETER) {
            error.param_num_mismatch(tar_func_name, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        if (!check_type(tmp_info.val_type, tmp_ele.val_type)) {
            //error
            error.param_mismatch(tar_func_name, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        tmp_inter_code.type = PARA_PUSH;
        tmp_inter_code.z = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
        tmp_inter_code.z_val_type = tmp_info.val_type;
        push_back(tmp_inter_code);
        pos++;
    }
    if (table.get_ele(tar_func_name, pos, tmp_ele)) {
        if (tmp_ele.id_type == PARAMETER) {
            //error
            error.param_num_mismatch(tar_func_name, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
    }
    return true;
}


bool Parser::parse_read(const string &scope) {
    table_ele ele;
    inter_code tmp_inter_code;
    if (lexer.sy != SCANFSY) {
        return false;
    }
    insymbol_with_error(0);
    if (lexer.sy != LPARENT) {
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    if (lexer.sy != IDENT) {
        error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    if (!table.find(scope, lexer.id, ele)) {
        error.unknown_id(ele.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    if (ele.id_type == CONSTANT || ele.id_type == FUNCTION) {
        error.wrong_rvalue(ele.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    tmp_inter_code.type = READ;
    tmp_inter_code.z = code_name(ele.scope, ele.id);
    tmp_inter_code.z_val_type = ele.val_type;
    push_back(tmp_inter_code);
    insymbol_with_error(0);
    while (lexer.sy == COMMA) {
        insymbol_with_error(0);
        if (lexer.sy != IDENT) {
            error.expect_identifier(lexer.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        if (!table.find(scope, lexer.id, ele)) {
            error.unknown_id(ele.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        if (ele.id_type == CONSTANT || ele.id_type == FUNCTION) {
            error.wrong_rvalue(ele.id, lexer.last_line_num, lexer.last_ch_pos);
            lexer.skip(';', '\n', true);
            lexer.skip(state_begin_sy);
            return false;
        }
        tmp_inter_code.type = READ;
        tmp_inter_code.z = code_name(ele.scope, ele.id);
        tmp_inter_code.z_val_type = ele.val_type;
        push_back(tmp_inter_code);
        insymbol_with_error(0);
    }
    if (lexer.sy != RPARENT) {
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    return true;
}

bool Parser::parse_write(const string &scope) {
    inter_code tmp_inter_code, tmp_inter_code2;
    exp_ret_info tmp_info;
    if (lexer.sy != PRINTFSY) {
        return false;
    }
    insymbol_with_error(0);
    if (lexer.sy != LPARENT) {
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    bool has_string = false, has_exp = false;
    if (lexer.sy == STRINGCON) {
        has_string = true;
        string str_lab = gen_str_lab();
        table.push(str_lab, lexer.id);
        tmp_inter_code.type = PRINTSTRING;
        tmp_inter_code.label = str_lab;
    }
    else {
        has_exp = true;
        if (!parse_expression(scope, tmp_info)) return false;
        lexer.no_read_next();
        tmp_inter_code.type = PRINT;
        tmp_inter_code.z = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
        tmp_inter_code.z_val_type = tmp_info.val_type;
    }
    insymbol_with_error(0);
    if (has_string && lexer.sy == COMMA) {
        has_exp = true;
        insymbol_with_error(0);
        if (!parse_expression(scope, tmp_info)) return false;
        tmp_inter_code2.type = PRINT;
        tmp_inter_code2.z = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
        tmp_inter_code2.z_val_type = tmp_info.val_type;
    }
    if (has_string && has_exp) {
        push_back(tmp_inter_code);
        push_back(tmp_inter_code2);
    }
    else {
        push_back(tmp_inter_code);
    }
    if (lexer.sy != RPARENT) {
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    insymbol_with_error(0);
    return true;
}

bool Parser::parse_return(const string &scope) {
    exp_ret_info tmp_info;
    table_ele tmp_ele;
    inter_code tmp_inter_code;
    tmp_inter_code.x = scope;
    if (lexer.sy != RETURNSY) {
        //error
        return false;
    }
    if (!table.find(GLOBAL_SCOPE, scope, tmp_ele)) {
        //error
        error.undefined_id(tmp_ele.id, lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        return false;
    }
    insymbol_with_error(0);
    if (lexer.sy != LPARENT && tmp_ele.ret_type == VOID_RET) {
        tmp_inter_code.type = FUNC_RET;
        tmp_inter_code.z = "";
        tmp_inter_code.z_val_type = NOTYP_VAL;
        push_back(tmp_inter_code);
        return true;
    }

    if (lexer.sy == SEMICOLON && tmp_ele.ret_type != VOID_RET) {
        //ERROR
        error.nonvoid_return_void(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    if (lexer.sy != SEMICOLON && lexer.sy != RBRACE && tmp_ele.ret_type == VOID_RET) {
        //ERROR
        error.void_return_nonvoid(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    if (lexer.sy != LPARENT && tmp_ele.ret_type != VOID_RET) {
        error.lack_lparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(';', '\n', true);
        lexer.skip(state_begin_sy);
        return false;
    }
    insymbol_with_error(0);
    if (!parse_expression(scope, tmp_info)) return false;
    if (!check_type(tmp_info.val_type, tmp_ele.ret_type)) {
        //error
        error.return_type_mismatch(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        return false;
    }
    tmp_inter_code.type = FUNC_RET;
    tmp_inter_code.z = tmp_info.is_valid ? my_to_string(tmp_info.int_val) : tmp_info.tmp_name;
    tmp_inter_code.z_val_type = tmp_info.val_type;
    push_back(tmp_inter_code);
    if (lexer.sy != RPARENT) {
        //error
        error.lack_rparent(lexer.last_line_num, lexer.last_ch_pos);
        lexer.skip(state_begin_sy);
        lexer.no_read_next();
    }
    is_return = true;
    lexer.insymbol();
    return true;
}
