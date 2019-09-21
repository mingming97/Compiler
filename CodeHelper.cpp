#include "CodeHelper.h"

void CodeHelper::check(const string &scope, const string &str, VALUE_TYPE val_type) {
    var_info tmp_var_info;
    table_ele tmp_ele;
    if (val_type == NOTYP_VAL || str == "") return;
    if (str.length() >= 2 && str.substr(0, 2) == "1T") {
        if (size_map.find(scope) == size_map.end())
            size_map[scope] = size_info();
        if (var_addr_map.find(str) != var_addr_map.end()) return;
        tmp_var_info.var_pos = STACK;
        tmp_var_info.base_reg = "$s7";
        tmp_var_info.offset = size_map[scope].tmp_size;
        var_addr_map[str] = tmp_var_info;

        size_map[scope].tmp_size += 4;
    }
    else if (str.find("::") != string::npos){
        vector<string> ss;
        split(str, ss, "::");
        string tmp_scope = ss[0], id = ss[1];

        if (size_map.find(tmp_scope) == size_map.end())
            size_map[tmp_scope] = size_info();
        if (var_addr_map.find(str) != var_addr_map.end()) return;

        table.find(tmp_scope, id, tmp_ele);
        tmp_var_info.var_pos = STACK;

        if (tmp_ele.id_type == PARAMETER) {
            tmp_var_info.base_reg = "$fp";
            int index = table.get_index(tmp_scope, id);
            tmp_var_info.offset = index * 4;
            size_map[tmp_scope].param_size += 4;
            if (index <= 2) {
                tmp_var_info.var_pos = REG;
                tmp_var_info.reg = "$a" + my_to_string(index + 1);
            }
        }
        else if (tmp_ele.id_type == VARIABLE) {
            tmp_var_info.base_reg = "$s6";
            tmp_var_info.offset = size_map[tmp_scope].var_size;
            if (tmp_ele.length == 0)
                size_map[tmp_scope].var_size += 4;
            else {
                size_map[tmp_scope].var_size += 4 * tmp_ele.length;
            }
        }
        if (tmp_scope == GLOBAL_SCOPE)
            tmp_var_info.base_reg = "$gp";
        var_addr_map[str] = tmp_var_info;
    }
}

void CodeHelper::gen_maps() {
    string scope;
    for (auto x: inter_codes) {
        if (x.type == FUNC_DEC) {
            scope = x.z;
            continue;
        }
        if (x.type == ASSIGN_EXP) {
            check(scope, x.z, x.z_val_type);
            check(scope, x.x, x.x_val_type);
            check(scope, x.y, x.y_val_type);
        }
    }
    map<string, vector<table_ele> >::iterator iter;
    for (iter = table.scope_map.begin(); iter != table.scope_map.end(); iter++) {
        if ((iter->first)[0] == '3') continue;
        vector<table_ele> &vec = iter->second;
        for (auto tmp_ele: vec) {
            if (tmp_ele.id_type == FUNCTION || tmp_ele.id_type == CONSTANT)
                continue;
            check(tmp_ele.scope, tmp_ele.scope + "::" + tmp_ele.id, tmp_ele.val_type);
        }
    }
}

void CodeHelper::set_global_regs() {
    unordered_map<string, string>::iterator iter;
    for (iter = var_regs.begin(); iter != var_regs.end(); iter++) {
        var_info &info = var_addr_map[iter->first];
        info.var_pos = REG;
        info.reg = iter->second;
    }
}

void CodeHelper::get_var_addr(const string &var, const string &target_reg) {
    var_info info = var_addr_map[var];
    int offset = info.base_reg == "$gp" ? info.offset : -info.offset;
    string code_str = "addiu\t" + target_reg + "\t" +
                      info.base_reg + "\t" + my_to_string(offset);
    mips_codes.push_back(code_str);
}

string CodeHelper::get_var_addr(const string &var) {
    var_info info = var_addr_map[var];
    int offset = info.base_reg == "$gp" ? info.offset : -info.offset;
    return my_to_string(offset)+"("+info.base_reg+")";
}

void CodeHelper::get_var_value(const string &var, string &target_reg) {
    var_info info = var_addr_map[var];
    string code_str;
    if (info.var_pos == REG) {
        if (target_reg != "$v0" && target_reg.substr(0,2) != "$a" && target_reg.substr(0, 2) != "$s") {
            target_reg = info.reg;
            return;
        }
        else {
            code_str = "move\t" + target_reg + "\t" + info.reg;
        }
    }
    else {
        if (var[0] == '1') {
            regpool.allocate(var, cur_cnt);
            info = var_addr_map[var];
            if (info.reg != "") target_reg = info.reg;
        }
        code_str = "lw\t" + target_reg + "\t" + get_var_addr(var);
    }
    mips_codes.push_back(code_str);
}

void CodeHelper::save_var_value(const string &var, const string &source_reg) {
    regpool.allocate(var, cur_cnt);
    var_info info = var_addr_map[var];
    string code_str;
    if (info.var_pos == REG) {
        if (source_reg == info.reg) return;
        code_str = "move\t" + info.reg + "\t" + source_reg;
    }
    else
        code_str = "sw\t" + source_reg + "\t" + get_var_addr(var);
    mips_codes.push_back(code_str);
}

void CodeHelper::get_op_num_value(const string &op_num, VALUE_TYPE val_type, string &target_reg) {
    if (is_array(val_type)) return;
    if (val_type == INT_CON_VAL || val_type == CHAR_CON_VAL) {
        if (op_num == "0") {
            if (target_reg == "$v0" || target_reg.substr(0,2) == "$a" || target_reg.substr(0, 2) == "$s")
                mips_codes.push_back("move\t" + target_reg + "\t$0");
            else
                target_reg = "$0";
            return;
        }
        else {
            mips_codes.push_back("li\t" + target_reg + "\t" + op_num);
        }
    }
    else
        get_var_value(op_num, target_reg);
}

void CodeHelper::initialize_global_data() {
    mips_codes.push_back(".data");
    unsigned int str_size = 0;
    for (auto x: table.string_label) {
        table_ele tmp_ele;
        table.get_ele(x, 0, tmp_ele);
        string tmp_str = tmp_ele.id, new_str;
        vector<string> ss;
        split(tmp_str, ss, "\\");
        for (unsigned int i = 0; i < ss.size() - 1; i++)
            new_str += ss[i] + "\\\\";
        new_str += ss[ss.size() - 1];
        str_size += tmp_str.length() + 1;
        string str = "\t" + tmp_ele.scope.substr(1) + ":.asciiz \"" + new_str + "\"";
        mips_codes.push_back(str);
    }
    str_size += (4 - (str_size % 4)) % 4;
    mips_codes.push_back(".text");
    mips_codes.push_back("li\t$gp\t" + my_to_string(data_base_addr + str_size));
}

void CodeHelper::generate_mips(bool to_console, bool is_op_mode, bool is_optimize_mips) {
    mips_codes.clear();
    var_addr_map.clear();
    size_map.clear();
    regpool.set_op_mode(is_op_mode);
    regpool.clear_all();
    gen_maps();
    if (is_op_mode) set_global_regs();
    initialize_global_data();
    gen_text();
    if (is_optimize_mips) optimize_mips(3);
    if (to_console)
        for (auto x: mips_codes)
            cout << x << endl;
}

void CodeHelper::gen_text() {
    mips_codes.push_back("jal\tmain");
    mips_codes.push_back("li\t$v0\t10");
    mips_codes.push_back("syscall");
    string op = "";
    int cnt = 0;

    string cond_target_reg1;
    string cond_target_reg2;
    for (auto code: inter_codes) {
        cnt++;
        cur_cnt = cnt - 1;
        //mips_codes.push_back(my_to_string(cnt));
        if (code.type == FUNC_DEC) {
            func_dec(code);
        }
        if (code.type == PARA_PUSH) {
            para_push(code);
        }
        if (code.type == FUNC_CALL) {
            vector<string> ss;
            split(code.z, ss, "::");
            regpool.clear(cur_cnt);
            mips_codes.push_back("jal\t" + ss[1]);
        }
        if (code.type == FUNC_RET) {
            func_ret(code);
        }
        if (code.type == ASSIGN_EXP) {
            assign_exp(code);
        }
        if (code.type == CONDITION) {
            op = code.op;
            cond_target_reg1 = "$t1";
            get_op_num_value(code.x, code.x_val_type, cond_target_reg1);
            cond_target_reg2 = "$t2";
            get_op_num_value(code.y, code.y_val_type, cond_target_reg2);
        }
        if (code.type == BNZ || code.type == BZ) {
            jump(code, op, cond_target_reg1, cond_target_reg2);
        }
        if (code.type == GOTO) {
            mips_codes.push_back("j\t" + code.label.substr(1));
        }
        if (code.type == LABEL) {
            mips_codes.push_back(code.label.substr(1) + ":");
        }
        if (code.type == READ) {
            if (code.z_val_type == CHAR_VAL)
                mips_codes.push_back("li\t$v0\t12");
            else
                mips_codes.push_back("li\t$v0\t5");
            mips_codes.push_back("syscall");
            save_var_value(code.z, "$v0");
        }
        if (code.type == PRINT) {
            if (code.z_val_type == CHAR_VAL || code.z_val_type == CHAR_CON_VAL)
                mips_codes.push_back("li\t$v0\t11");
            else
                mips_codes.push_back("li\t$v0\t1");
            string tmp_reg = "$a0";
            get_op_num_value(code.z, code.z_val_type, tmp_reg);
            mips_codes.push_back("syscall");

            //print \n
            mips_codes.push_back("li\t$v0\t11");
            mips_codes.push_back("li\t$a0\t" + my_to_string('\n'));
            mips_codes.push_back("syscall");
        }
        if (code.type == PRINTSTRING) {
            mips_codes.push_back("li\t$v0\t4");
            mips_codes.push_back("la\t$a0\t"+code.label.substr(1));
            mips_codes.push_back("syscall");

            //print \n
            mips_codes.push_back("li\t$v0\t11");
            mips_codes.push_back("li\t$a0\t" + my_to_string('\n'));
            mips_codes.push_back("syscall");
        }
    }
    mips_codes.push_back("li\t$v0\t10");
    mips_codes.push_back("syscall");
}



int CodeHelper::save_regs() {
    mips_codes.push_back("subi\t$sp\t$sp\t56");
    mips_codes.push_back("sw\t$a0\t0($sp)");
    mips_codes.push_back("sw\t$a1\t4($sp)");
    mips_codes.push_back("sw\t$a2\t8($sp)");
    mips_codes.push_back("sw\t$a3\t12($sp)");
    mips_codes.push_back("sw\t$s0\t16($sp)");
    mips_codes.push_back("sw\t$s1\t20($sp)");
    mips_codes.push_back("sw\t$s2\t24($sp)");
    mips_codes.push_back("sw\t$s3\t28($sp)");
    mips_codes.push_back("sw\t$s4\t32($sp)");
    mips_codes.push_back("sw\t$s5\t36($sp)");
    mips_codes.push_back("sw\t$s6\t40($sp)");
    mips_codes.push_back("sw\t$s7\t44($sp)");
    mips_codes.push_back("sw\t$fp\t48($sp)");
    mips_codes.push_back("sw\t$ra\t52($sp)");
    return 14;
}

int CodeHelper::save_regs(const string &func_name) {
    int para_num = (int)(size_map[func_name].param_size >> 2);
    para_num = para_num > 3 ? 3 : para_num;
    int global_reg_num = reg_nums[func_name];
    int offset = 0;
    int tot = para_num + global_reg_num + 4;
    mips_codes.push_back("subi\t$sp\t$sp\t" + my_to_string(tot << 2));
    for (int i = 0; i < para_num; i++) {
        mips_codes.push_back("sw\t$a" + my_to_string(i+1) + "\t" +
                             my_to_string(offset) + "($sp)");
        offset += 4;
    }
    for (int i = 0; i < global_reg_num; i++) {
        mips_codes.push_back("sw\t" + global_regs[i] + "\t" +
                             my_to_string(offset) + "($sp)");
        offset += 4;
    }
    mips_codes.push_back("sw\t$s6\t" + my_to_string(offset) + "($sp)");
    mips_codes.push_back("sw\t$s7\t" + my_to_string(offset+4) + "($sp)");
    mips_codes.push_back("sw\t$fp\t" + my_to_string(offset+8) + "($sp)");
    mips_codes.push_back("sw\t$ra\t" + my_to_string(offset+12) + "($sp)");
    return tot;
}

void CodeHelper::restore_regs() {
    mips_codes.push_back("lw\t$a0\t0($sp)");
    mips_codes.push_back("lw\t$a1\t4($sp)");
    mips_codes.push_back("lw\t$a2\t8($sp)");
    mips_codes.push_back("lw\t$a3\t12($sp)");
    mips_codes.push_back("lw\t$s0\t16($sp)");
    mips_codes.push_back("lw\t$s1\t20($sp)");
    mips_codes.push_back("lw\t$s2\t24($sp)");
    mips_codes.push_back("lw\t$s3\t28($sp)");
    mips_codes.push_back("lw\t$s4\t32($sp)");
    mips_codes.push_back("lw\t$s5\t36($sp)");
    mips_codes.push_back("lw\t$s6\t40($sp)");
    mips_codes.push_back("lw\t$s7\t44($sp)");
    mips_codes.push_back("lw\t$fp\t48($sp)");
    mips_codes.push_back("lw\t$ra\t52($sp)");
    mips_codes.push_back("addi\t$sp\t$sp\t56");
}

void CodeHelper::restore_regs(const string &func_name) {
    int para_num = (int)(size_map[func_name].param_size >> 2);
    para_num = para_num > 3 ? 3 : para_num;
    int global_reg_num = reg_nums[func_name];
    int offset = 0;
    int tot = para_num + global_reg_num + 4;
    for (int i = 0; i < para_num; i++) {
        mips_codes.push_back("lw\t$a" + my_to_string(i+1) + "\t" +
                             my_to_string(offset) + "($sp)");
        offset += 4;
    }
    for (int i = 0; i < global_reg_num; i++) {
        mips_codes.push_back("lw\t" + global_regs[i] + "\t" +
                             my_to_string(offset) + "($sp)");
        offset += 4;
    }
    mips_codes.push_back("lw\t$s6\t" + my_to_string(offset) + "($sp)");
    mips_codes.push_back("lw\t$s7\t" + my_to_string(offset+4) + "($sp)");
    mips_codes.push_back("lw\t$fp\t" + my_to_string(offset+8) + "($sp)");
    mips_codes.push_back("lw\t$ra\t" + my_to_string(offset+12) + "($sp)");
    mips_codes.push_back("addi\t$sp\t$sp\t" + my_to_string(tot << 2));
}

void CodeHelper::func_dec(const inter_code &code) {
    size_info info = size_map[code.z];
    mips_codes.push_back(code.z + ":");
    int reg_num;
    if (!is_reg_op_mode) reg_num = save_regs();
    else reg_num = save_regs(code.z);

    mips_codes.push_back("addi\t$fp\t$sp\t" + my_to_string(info.param_size + (reg_num << 2) - 4));
    mips_codes.push_back("subi\t$s6\t$sp\t4");
    if (info.var_size != 0)
        mips_codes.push_back("subi\t$sp\t$sp\t" + my_to_string(info.var_size));

    mips_codes.push_back("subi\t$s7\t$sp\t4");
    if (info.tmp_size != 0)
        mips_codes.push_back("subi\t$sp\t$sp\t" + my_to_string(info.tmp_size));
    unsigned int para_num = size_map[code.z].param_size >> 2;
    int cur_reg_num = 0;
    while (cur_reg_num < 3 && cur_reg_num < (int)para_num) {
        string code_str = "lw\t$a" + my_to_string(cur_reg_num + 1) + "\t" +
                          my_to_string(-cur_reg_num * 4) + "($fp)";
        mips_codes.push_back(code_str);
        cur_reg_num++;
    }
}

void CodeHelper::func_ret(const inter_code &code) {
    string ret_reg = "$v0";
    size_info info = size_map[code.x];
    if (code.z_val_type != NOTYP_VAL) {
        get_op_num_value(code.z, code.z_val_type, ret_reg);
    }
    regpool.clear_all();
    mips_codes.push_back("addi\t$sp\t$sp\t" + my_to_string(info.tmp_size + info.var_size));
    if (!is_reg_op_mode) restore_regs();
    else restore_regs(code.x);
    mips_codes.push_back("addi\t$sp\t$sp\t" + my_to_string(info.param_size));
    mips_codes.push_back("jr\t$ra");
}

void CodeHelper::para_push(const inter_code &code) {
    string target_reg = "$t3";
    if (code.z_val_type == INT_CON_VAL || code.z_val_type == CHAR_CON_VAL)
        mips_codes.push_back("li\t" + target_reg + "\t" + code.z);
    else
        get_var_value(code.z, target_reg);
    mips_codes.push_back("subi\t$sp\t$sp\t4");
    mips_codes.push_back("sw\t" + target_reg + "\t" + "0($sp)");
}


void CodeHelper::assign_exp(const inter_code &code) {
    string reg0 = "$t0";
    string reg1 = "$t1";
    string reg2 = "$t2";

    if (code.op == DIRECT_ASSIGN_OP) {
        if (is_array(code.z_val_type)) {
            if (code.index_z_val_type == INT_CON_VAL) {
                var_info info = var_addr_map[code.z];
                int offset = (atoi(code.index_z.c_str()) << 2) + info.offset;
                offset = info.base_reg == "$gp" ? offset : -offset;
                get_op_num_value(code.x, code.x_val_type, reg2);
                mips_codes.push_back("sw\t" + reg2 + "\t" + my_to_string(offset)+"("+info.base_reg+")");
                return;
            }
            get_var_addr(code.z, "$t0");
            get_op_num_value(code.index_z, code.index_z_val_type, reg1);
            mips_codes.push_back("sll\t$t1\t" + reg1 + "\t2");
            if (code.z[0] == '0')
                mips_codes.push_back("addu\t$t0\t$t0\t$t1");
            else
                mips_codes.push_back("subu\t$t0\t$t0\t$t1");
            get_op_num_value(code.x, code.x_val_type, reg2);
            mips_codes.push_back("sw\t" + reg2 + "\t0($t0)");
        }
        else if (is_array(code.x_val_type)) {
            if (code.index_x_val_type == INT_CON_VAL) {
                var_info info = var_addr_map[code.x];
                int offset = (atoi(code.index_x.c_str()) << 2) + info.offset;
                offset = info.base_reg == "$gp" ? offset : -offset;
                mips_codes.push_back("lw\t" + reg0 + "\t" + my_to_string(offset)+"("+info.base_reg+")");
                save_var_value(code.z, reg0);
                return;
            }
            get_var_addr(code.x, "$t0");
            get_op_num_value(code.index_x, code.index_x_val_type, reg1);
            mips_codes.push_back("sll\t$t1\t" + reg1 + "\t2");
            if (code.x[0] == '0')
                mips_codes.push_back("addu\t$t0\t$t0\t$t1");
            else
                mips_codes.push_back("subu\t$t0\t$t0\t$t1");
            mips_codes.push_back("lw\t$t0\t0($t0)");
            save_var_value(code.z, "$t0");
        }
        else if (code.x == "RET") {
            save_var_value(code.z, "$v0");
        }
        else {
            get_op_num_value(code.x, code.x_val_type, reg0);
            save_var_value(code.z, reg0);
        }
    }
    else {
        get_op_num_value(code.x, code.x_val_type, reg1);
        get_op_num_value(code.y, code.y_val_type, reg2);
        if (code.op == "+")
            mips_codes.push_back("addu\t$t0\t" + reg1 + "\t" + reg2);
        if (code.op == "-")
            mips_codes.push_back("subu\t$t0\t" + reg1 + "\t" + reg2);
        if (code.op == "*") {
            mips_codes.push_back("mult\t" + reg1 + "\t" + reg2);
            mips_codes.push_back("mflo\t$t0");
        }
        if (code.op == "/") {
            mips_codes.push_back("div\t" + reg1 + "\t" + reg2);
            mips_codes.push_back("mflo\t$t0");
        }
        save_var_value(code.z, "$t0");
    }
}

void CodeHelper::jump(const inter_code &code, const string &op, const string &reg1, const string &reg2) {
    bool is_positive = (code.type == BZ);
    string ins;
    if (op == "==")
        ins = is_positive ? "beq" : "bne";
    if (op == "!=")
        ins = is_positive ? "bne" : "beq";
    if (op == "<")
        ins = is_positive ? "blt" : "bge";
    if (op == "<=")
        ins = is_positive ? "ble" : "bgt";
    if (op == ">")
        ins = is_positive ? "bgt" : "ble";
    if (op == ">=")
        ins = is_positive ? "bge" : "blt";
    mips_codes.push_back(ins + "\t" + reg1 + "\t" + reg2 + "\t" +
                         code.label.substr(1));
}

void CodeHelper::basic_optimize(int times) {
    while (times--) {
        for (unsigned int i = 0; i < inter_codes.size() - 1; i++) {
            if (inter_codes[i].type != ASSIGN_EXP)
                continue;
            if (inter_codes[i].z[0] != '1')
                continue;
            if (inter_codes[i].x != "RET") {
                if (inter_codes[i].op == DIRECT_ASSIGN_OP) {
                    if (inter_codes[i].x[0] != '1' && is_array(inter_codes[i].x_val_type))
                        continue;
                    if (inter_codes[i + 1].type == ASSIGN_EXP) {
                        bool is_erase = false;
                        if (inter_codes[i + 1].x == inter_codes[i].z) {
                            inter_codes[i + 1].x = inter_codes[i].x;
                            inter_codes[i + 1].x_val_type = inter_codes[i].x_val_type;
                            is_erase = true;
                        }
                        if (inter_codes[i + 1].y == inter_codes[i].z) {
                            inter_codes[i + 1].y = inter_codes[i].x;
                            inter_codes[i + 1].y_val_type = inter_codes[i].x_val_type;
                            is_erase = true;
                        }
                        if (is_erase) {
                            inter_codes.erase(inter_codes.begin() + i);
                            i--;
                        }
                    }
                }
                else {
                    if (inter_codes[i + 1].type != ASSIGN_EXP || inter_codes[i + 1].op != DIRECT_ASSIGN_OP)
                        continue;
                    if (inter_codes[i + 1].x[0] != '1')
                        continue;
                    if (inter_codes[i + 1].z[0] != '1' && is_array(inter_codes[i + 1].z_val_type))
                        continue;
                    if (inter_codes[i + 1].x == inter_codes[i].z) {
                        inter_code tmp = inter_codes[i];
                        tmp.z = inter_codes[i + 1].z;
                        tmp.z_val_type = inter_codes[i + 1].z_val_type;
                        inter_codes[i + 1] = tmp;
                        inter_codes.erase(inter_codes.begin() + i);
                        i--;
                    }
                }
            }
            else {
                if (inter_codes[i].x == "RET" && inter_codes[i + 1].op == DIRECT_ASSIGN_OP &&
                    inter_codes[i].z == inter_codes[i + 1].x && inter_codes[i + 1].z[0] == '1') {
                    inter_codes[i + 1].x = "RET";
                    inter_codes.erase(inter_codes.begin() + i);
                    i--;
                }
            }
        }
    }
}

void CodeHelper::optimize_mips(int times) {
    while (times--) {
        for (unsigned int i = 0; i < mips_codes.size() - 1; i++) {
            vector<string> ss1, ss2;
            string code1 = mips_codes[i], code2 = mips_codes[i + 1];
            split(code1, ss1, "\t");
            split(code2, ss2, "\t");
            if (ss1[0] == "move" && ss1[1] == ss1[2]) {
                mips_codes.erase(mips_codes.begin() + i);
                i--;
                continue;
            }
            if ((ss1[0] == "addu" || ss1[0] == "subu" || ss1[0] == "mflo" || ss1[0] == "li" || ss1[0] == "lw") && ss2[0] == "move") {
                if (ss1[1] == ss2[2] && ss1[1].substr(0, 2) == "$t") {
                    string new_code;
                    if (ss1[0] == "mflo")
                        new_code = ss1[0] + "\t" + ss2[1];
                    else if (ss1[0] == "li" || ss1[0] == "lw")
                        new_code = ss1[0] + "\t" + ss2[1] + "\t" + ss1[2];
                    else
                        new_code = ss1[0] + "\t" + ss2[1] + "\t" + ss1[2] + "\t" + ss1[3];
                    mips_codes[i] = new_code;
                    mips_codes.erase(mips_codes.begin() + i + 1);
                    i--;
                    continue;
                }
            }
        }
    }
}

void CodeHelper::gen_file_inter_codes(bool to_console) {
    int cnt = 0;
    file_inter_codes.clear();
    for (auto &code: inter_codes) {
        string tmp = "";
        cnt++;
        if (code.type == FUNC_DEC) {
            tmp += trans_ret_type(code.ret_type) + " ";
            tmp += code.z;
            tmp += "()";
        }
        if (code.type == PARA_DEC) {
            tmp += "para ";
            tmp += trans_val_type(code.z_val_type) + " ";
            tmp += code.z;
        }
        if (code.type == PARA_PUSH) {
            tmp += "push ";
            tmp += code.z;
        }
        if (code.type == FUNC_CALL) {
            tmp += "call ";
            tmp += code.z;
            //tmp += "       " + code.x;
        }
        if (code.type == FUNC_RET) {
            tmp += "ret ";
            if (code.z_val_type == CHAR_CON_VAL) {
                tmp += '\'';
                tmp += (char)atoi(code.z.c_str());
                tmp += '\'';
            }
            else
                tmp += code.z;
            //tmp += "    "+code.x;
        }
        if (code.type == ASSIGN_EXP) {
            if (is_array(code.z_val_type)) {
                tmp += code.z;
                tmp += "[" + code.index_z + "] = ";
                tmp += code.x;
            }
            else if (is_array(code.x_val_type)) {
                tmp += code.z;
                tmp += " = ";
                tmp += code.x;
                tmp += "[" + code.index_x + "]";
            }
            else if (code.op == DIRECT_ASSIGN_OP) {
                tmp += code.z + " = " + code.x;
            }
            else {
                tmp += code.z;
                tmp += " = ";
                if (code.x == "0" && code.op == "+") {
                    code.x = code.y;
                    code.x_val_type = code.y_val_type;
                    code.y = "";
                    code.y_val_type = NOTYP_VAL;
                    code.op = DIRECT_ASSIGN_OP;
                    tmp += code.x;
                }
                else
                    tmp += code.x + code.op + code.y;
            }
        }
        if (code.type == CONDITION) {
            tmp += code.x + " " + code.op + " " + code.y;
        }
        if (code.type == GOTO) {
            tmp += "GOTO " + code.label;
        }
        if (code.type == BNZ) {
            tmp += "BNZ " + code.label;
        }
        if (code.type == BZ) {
            tmp += "BZ " + code.label;
        }
        if (code.type == LABEL) {
            tmp += code.label + ":";
        }
        if (code.type == READ) {
            tmp += "READ " + code.z;
        }
        if (code.type == PRINT) {
            tmp += "PRINT " + code.z;
        }
        if (code.type == PRINTSTRING) {
            table_ele tmp_ele;
            table.get_ele(code.label, 0, tmp_ele);
            tmp += "PRINT \"" + tmp_ele.id + "\"";
        }
        file_inter_codes.push_back(tmp);
        if (to_console) {
            cout << left << setw(5) << cnt - 1;
            cout <<  tmp << endl;
        }
    }
}
