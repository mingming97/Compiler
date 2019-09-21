#ifndef REGPOOL_H_INCLUDED
#define REGPOOL_H_INCLUDED
#include "headers.h"

using namespace std;

class RegPool {
private:
    unordered_map<string, var_info> &var_addr_map;
    vector<string> &mips_codes;
    vector<inter_code> &inter_codes;
    vector<bool> is_free; //t4 - t9
    int next_tmp;
    int bias;
    int tmp_num;
    vector<string> in_pool_vars;
    bool is_op_mode;

    string get_var_addr(const string &var) {
        var_info info = var_addr_map[var];
        int offset = info.base_reg == "$gp" ? info.offset : -info.offset;
        return my_to_string(offset)+"("+info.base_reg+")";
    }

    void save_var_value(const string &var, const string &source_reg) {
        string code_str = "sw\t" + source_reg + "\t" + get_var_addr(var);
        mips_codes.push_back(code_str);
    }

    void write_back(int index) {
        if (!is_free[index] && in_pool_vars[index] != "") {
            var_info &tmp = var_addr_map[in_pool_vars[index]];
            //mips_codes.push_back(in_pool_vars[index]);
            save_var_value(in_pool_vars[index], tmp.reg);
            tmp.var_pos = STACK;
        }
    }

    bool will_be_used(int end, const string &name) {
        for (unsigned int i = end + 1; i < inter_codes.size(); i++) {
            inter_code reserved_code = inter_codes[i];
            bool flag;
            if (reserved_code.type == FUNC_DEC)
                break;
            flag = (reserved_code.z == name) || (reserved_code.index_z == name) ||
                   (reserved_code.x == name) || (reserved_code.index_x == name) ||
                   (reserved_code.y == name);
            if (flag) return true;
        }
        return false;
    }

    bool cal_next_tmp(int cur_cnt) {
        for (int i = 0; i < tmp_num; i++) {
            int tmp = (next_tmp + i) % tmp_num;
            if (is_free[tmp] || !will_be_used(cur_cnt, in_pool_vars[tmp])) {
                next_tmp = tmp;
                return true;
            }
        }
        return false;
    }


public:
    RegPool(unordered_map<string, var_info> &var_addr_map_, vector<string> &mips_codes_,
            vector<inter_code> &inter_codes_):
            var_addr_map(var_addr_map_), mips_codes(mips_codes_), inter_codes(inter_codes_) {
        next_tmp = 0;
        bias = 4;
        tmp_num = 9 - bias + 1;
        is_free = vector<bool>(tmp_num, true);
        in_pool_vars = vector<string>(tmp_num, "");
        is_op_mode = false;
    }

    void set_op_mode(bool is_op_mode_) {
        is_op_mode = is_op_mode_;
    }


    void allocate(const string &var, int cur_cnt) {
        //cur_cnt 代表当前变量所在中间代码的位置
        //找到替换出去的临时寄存器
        //如果临时寄存器代表变量为空或在本函数中之后的中间代码不会被用到
        //则可以直接替换，无需写回内存空间
        if (!is_op_mode) return;
        if (var[0] != '1') return;
        var_info &info = var_addr_map[var];
        if (info.var_pos == REG)
            return;
        if (!cal_next_tmp(cur_cnt))
            return;
        is_free[next_tmp] = false;
        in_pool_vars[next_tmp] = var;
        info.var_pos = REG;
        info.reg = "$t" + my_to_string(next_tmp + bias);
        next_tmp = (next_tmp + 1) % tmp_num;
    }

    void clear(int cur_cnt) {
        if (!is_op_mode) return;
        for (int i = 0; i < tmp_num; i++) {
            if (!is_free[i] && will_be_used(cur_cnt, in_pool_vars[i]))
                write_back(i);
            is_free[i] = true;
            in_pool_vars[i] = "";
        }
        next_tmp = 0;
    }

    void clear_all() {
        if (!is_op_mode) return;
        for (int i = 0; i < tmp_num; i++) {
            is_free[i] = true;
            in_pool_vars[i] = "";
        }
        next_tmp = 0;
    }
};


#endif // REGPOOL_H_INCLUDED
