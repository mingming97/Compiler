#ifndef CODEHELPER_H_INCLUDED
#define CODEHELPER_H_INCLUDED
#include "headers.h"
#include "table.h"
#include "RegPool.h"
#include "Flow.h"

using namespace std;

class CodeHelper {
private:
    Table &table;
    vector<inter_code> &inter_codes;
    unordered_map<string, string> var_regs;
    unordered_map<string, int> reg_nums;

    int cur_cnt;
    unsigned int data_base_addr;
    unsigned int global_var_base_addr;
    unsigned int tmp_base_addr;
    bool is_reg_op_mode;


    string trans_ret_type(RETURN_TYPE ret_type) {
        if (ret_type == INT_RET)
            return "int";
        if (ret_type == CHAR_RET)
            return "char";
        return "void";
    }

    string trans_val_type(VALUE_TYPE val_type) {
        if (val_type == INT_VAL || val_type == INT_ARRAY_VAL)
            return "int";
        if (val_type == CHAR_VAL || val_type == CHAR_ARRAY_VAL)
            return "char";
        return "";
    }

    inline bool is_array(VALUE_TYPE val_type) {
        return val_type == INT_ARRAY_VAL || val_type == CHAR_ARRAY_VAL;
    }

    void check(const string &, const string &, VALUE_TYPE);
    void gen_maps();

    void initialize_global_data();

    void gen_text();

    void func_dec(const inter_code &);

    int save_regs();
    int save_regs(const string &);
    void restore_regs();
    void restore_regs(const string &);

    void para_push(const inter_code &);

    void func_ret(const inter_code &);
    void func_call(const inter_code &);

    void assign_exp(const inter_code &);
    void get_op_num_value(const string &, VALUE_TYPE, string &);

    void jump(const inter_code &, const string &, const string &, const string &);

    void get_var_addr(const string &, const string &);
    string get_var_addr(const string &);

    void get_var_value(const string &, string &);
    void save_var_value(const string &, const string &);

    void set_global_regs();

public:
    vector<string> file_inter_codes;
    vector<string> mips_codes;

    unordered_map<string, size_info> size_map;
    unordered_map<string, var_info> var_addr_map;

    RegPool regpool;

    CodeHelper(Table &table_, vector<inter_code> &inter_codes_):
        table(table_), inter_codes(inter_codes_), regpool(var_addr_map, mips_codes, inter_codes_) {
        data_base_addr = 0x10010000;
        cur_cnt = 0;
        is_reg_op_mode = false;
    }

    void gen_file_inter_codes(bool);
    void generate_mips(bool, bool, bool);
    void basic_optimize(int);
    void set_global_regs(unordered_map<string, string> &var_regs_,
                         unordered_map<string, int> &reg_nums_) {
        var_regs = var_regs_;
        reg_nums = reg_nums_;
        is_reg_op_mode = true;
    }
    void optimize_mips(int);
};

#endif // CODEHELPER_H_INCLUDED
