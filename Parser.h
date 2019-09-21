#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED
#include "headers.h"
#include "Lexer.h"
#include "table.h"

using namespace std;


class Parser {
private:
    Lexer &lexer;
    Table &table;
    vector<inter_code> &inter_codes;
    vector<inter_code> switch_codes;
    int case_depth;
    Error &error;
    bool over_void_main;
    bool is_return;
    unsigned int lab_num;
    unsigned int str_num;
    bool insymbol_with_error(int error_num) {
        if (!lexer.insymbol()) {
            if (lexer.over)
                error.finish_early();
            while (!lexer.insymbol()) {
                if (lexer.over)
                    error.finish_early();
            }
            return true;
        }
        return false;
    }
    inline string code_name(const string &scope, const string &id) {
        return scope + "::" + id;
    }

    bool check_type(VALUE_TYPE val_type1, VALUE_TYPE val_type2) {
        if (val_type1 == INT_VAL || val_type1 == INT_CON_VAL) {
            if (val_type2 == INT_VAL || val_type2 == INT_CON_VAL)
                return true;
        }
        if (val_type1 == CHAR_VAL || val_type1 == CHAR_CON_VAL) {
            if (val_type2 == CHAR_VAL || val_type2 == CHAR_CON_VAL)
                return true;
        }
        return false;
    }

    bool check_type(VALUE_TYPE val_type, RETURN_TYPE ret_type) {
        if (ret_type == INT_RET) {
            if (val_type == INT_VAL || val_type == INT_CON_VAL)
                return true;
        }
        if (ret_type == CHAR_RET) {
            if (val_type == CHAR_VAL || val_type == CHAR_CON_VAL)
                return true;
        }
        return false;
    }

    bool is_func_def() {
        pair<unsigned int, unsigned int> ini_pos = lexer.get_state();
        unsigned int line_num = ini_pos.first;
        unsigned int ch_pos = ini_pos.second;
        string id = lexer.id;
        enum SYMBOL_TYPE sy = lexer.sy;
        int int_con = lexer.int_con;
        char char_con = lexer.char_con;
        bool res = false;
        if (lexer.sy == INTSY || lexer.sy == CHARSY) {
            lexer.insymbol();
            if (lexer.sy == IDENT) {
                lexer.insymbol();
                if (lexer.sy == LPARENT)
                    res = true;
            }
        }
        lexer.set_state(line_num, ch_pos);
        lexer.id = id;
        lexer.sy = sy;
        lexer.int_con = int_con;
        lexer.char_con = char_con;
        return res;
    }

    inline string gen_tmp_var() {
        return "1T" + my_to_string(++tmp_num);
    }

    inline string gen_lab() {
        return "2LABEL" + my_to_string(++lab_num);
    }

    inline string gen_str_lab() {
        return "3STRING" + my_to_string(++str_num);
    }

    void push_back(inter_code tmp_inter_code) {
        if (case_depth > 0)
            switch_codes.push_back(tmp_inter_code);
        else
            inter_codes.push_back(tmp_inter_code);
    }


public:
    unsigned int tmp_num;
    Parser(Lexer &lexer_, Table &table_, vector<inter_code> &inter_codes_, Error &error_):
        lexer(lexer_), table(table_), inter_codes(inter_codes_), error(error_) {
        over_void_main = false;
        tmp_num = 0;
        lab_num = 0;
        str_num = 0;
        switch_codes = vector<inter_code>();
        case_depth = 0;
    };
    void parse_init();
    bool parse_programme();
    bool parse_const_dec(const string &);
    bool parse_const_def(const string &);
    bool parse_unsigned_integer(int &);
    bool parse_integer(int &);
    bool parse_dec_head(const string &, string &);
    bool parse_var_dec(const string &);
    bool parse_var_def(const string &);
    bool parse_nonvoid_func_def(const string &);
    bool parse_void_func_def(const string &);
    bool parse_compound_statement(const string &);
    bool parse_paramlist(const string &);
    bool parse_parameter(const string &);
    bool parse_expression(const string &, exp_ret_info &);
    bool parse_term(const string &, exp_ret_info &);
    bool parse_factor(const string &, exp_ret_info &);
    bool parse_statement(const string &);
    bool parse_assign(const string &, const table_ele &);
    bool parse_if(const string &);
    bool parse_condition(const string &, string &);
    bool parse_while(const string &);
    bool parse_switch(const string &);
    bool parse_cond_table(const string &, vector<case_table_ele> &, const string &, VALUE_TYPE,
                          vector<inter_code> &);
    bool parse_case(const string &, vector<case_table_ele> &, const string &, VALUE_TYPE);
    bool parse_default(const string &, const string &);
    bool parse_nonvoid_func_call(const string &, const table_ele &);
    bool parse_void_func_call(const string &, const table_ele &);
    bool parse_value_paramlist(const string &, const table_ele &);
    bool parse_read(const string &);
    bool parse_write(const string &);
    bool parse_return(const string &);
};


#endif // PARSER_H_INCLUDED
