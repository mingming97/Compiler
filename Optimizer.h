#ifndef OPTIMIZER_H_INCLUDED
#define OPTIMIZER_H_INCLUDED
#include "headers.h"
#include "table.h"
#include "Flow.h"

using namespace std;

struct Node {
    int index;
    string val;
    VALUE_TYPE val_type;
    string op;
    int l_son;
    int r_son;
    vector<int> fathers;
    bool in_q;
    bool is_mid;

    Node() {
        val = op = "";
        val_type = NOTYP_VAL;
        index = l_son = r_son = -1;
        in_q = is_mid = false;
    }
};

struct local_var_info {
    string name;
    string scope;
    int weight;
    local_var_info() {
        weight = 0;
    }
    local_var_info(const string &name_) {
        vector<string> ss;
        split(name_, ss, "::");
        scope = ss[0];
        name = name_;
        weight = 0;
    }

    bool operator <(const local_var_info &other) const {
        return weight < other.weight;
    }

    bool operator >(const local_var_info &other) const {
        return weight > other.weight;
    }
};

struct DAG {
    int node_num;
    vector<inter_code> &inter_codes;
    unsigned int &tmp_num;
    vector<Node> graph;
    unordered_map<string, int> node_table;
    unordered_map<string, int> first_pos;
    unordered_map<int, vector<string> > node_vars;
    vector<string> reserved_tmps;
    vector<int> mid_nodes;
    vector<int> mid_queue;
    DAG(vector<inter_code> &inter_codes_, unsigned int &tmp_num_):
        inter_codes(inter_codes_), tmp_num(tmp_num_){
        node_num = 0;
    }

    void cal_reserved_tmps(int end) {
        for (unsigned int i = end + 1; i < inter_codes.size(); i++) {
            inter_code reserved_code = inter_codes[i];
            if (reserved_code.type == FUNC_DEC)
                break;
            check_num(reserved_code.z);
            check_num(reserved_code.index_z);
            check_num(reserved_code.x);
            check_num(reserved_code.index_x);
            check_num(reserved_code.y);
        }
    }

    inline void check_num(const string &str) {
        if (str.size() > 1 && str.substr(0, 2) == "1T") {
            if (!is_in_reserved_tmps(str))
                reserved_tmps.push_back(str);
        }
    }

    inline string gen_tmp_var() {
        return "1T" + my_to_string(++tmp_num);
    }

    bool is_in_reserved_tmps(const string &tmp_name) {
        for (auto x: reserved_tmps)
            if (x == tmp_name)
                return true;
        return false;
    }

    inline bool is_in_table(const string &str) {
        return node_table.find(str) != node_table.end();
    }

    bool is_all_fathers_in_q(int index) {
        Node tmp = graph[index];
        for (auto x: tmp.fathers) {
            if (!graph[x].in_q)
                return false;
        }
        return true;
    }

    bool can_added_to_q(int index) {
        Node tmp = graph[index];
        return (tmp.is_mid && !tmp.in_q && is_all_fathers_in_q(index));
    }

    int find(int l_son, int r_son, const string &op) {
        for (auto x: graph) {
            if (!x.is_mid) continue;
            if (x.l_son < 0 || x.r_son < 0) continue;
            if (x.l_son == l_son && x.r_son == r_son && x.op == op)
                return x.index;
            if (op == "*" || op == "/") {
                if (x.l_son == r_son && x.r_son == l_son && x.op == op)
                    return x.index;
            }
        }
        return -1;
    }

    int check_op_num(const string &str, VALUE_TYPE val_type) {
        if (str == "" || val_type == NOTYP_VAL) return -1;
        if (is_in_table(str)) return node_table[str];
        Node tmp_node;
        tmp_node.val = str;
        tmp_node.val_type = val_type;
        tmp_node.index = node_num;
        if (str[0] != '1') {
            if (first_pos.find(str) == first_pos.end())
                first_pos[str] = tmp_node.index;
        }
        graph.push_back(tmp_node);
        node_table[str] = tmp_node.index;
        node_num++;
        return tmp_node.index;
    }

    void check_target_num(const string &str, VALUE_TYPE val_type, int l_son, int r_son, const string &op) {
        if (r_son == -1 && op == DIRECT_ASSIGN_OP) {
            node_table[str] = l_son;
        }
        else {
            int k = find(l_son, r_son, op);
            if (k == -1) {
                Node tmp_node;
                tmp_node.val = str;
                tmp_node.val_type = val_type;
                tmp_node.op = op;
                tmp_node.l_son = l_son;
                tmp_node.r_son = r_son;
                tmp_node.index = node_num;
                graph[l_son].fathers.push_back(tmp_node.index);
                graph[r_son].fathers.push_back(tmp_node.index);
                tmp_node.is_mid = true;
                graph.push_back(tmp_node);
                mid_nodes.push_back(tmp_node.index);
                node_num++;
                node_table[str] = tmp_node.index;
            }
            else {
                node_table[str] = k;
            }
        }
    }

    void init_node_vars() {
        unordered_map<string, int>::iterator iter;
        for (auto i: mid_nodes)
            node_vars[i] = vector<string>();
        for (iter = node_table.begin(); iter != node_table.end(); iter++) {
            string name = iter->first;
            int index = iter->second;
            node_vars[index].push_back(name);
        }
    }

    void filter_node_vars() {
        init_node_vars();
        for (auto i: mid_nodes) {
            vector<string> &vars = node_vars[i];
            if (vars.size() == 0)
                vars.push_back(gen_tmp_var());
            else {
                for (unsigned int j = 0; j < vars.size(); j++) {
                    if (vars[j][0] != '1') continue;
                    if (is_in_reserved_tmps(vars[j])) continue;
                    vars.erase(vars.begin() + j);
                    j--;
                }
                if (vars.size() == 0)
                    vars.push_back(graph[i].val);
            }
            graph[i].val = vars[0];
        }
    }

};

class Optimizer {
private:
    inline string gen_tmp_var() {
        return "1T" + my_to_string(++tmp_num);
    }

    inline bool is_array(VALUE_TYPE val_type) {
        return val_type == INT_ARRAY_VAL || val_type == CHAR_ARRAY_VAL;
    }

    inline bool can_optimize(const inter_code &code) {
        return !(code.type != ASSIGN_EXP || code.x == "RET" || is_array(code.z_val_type));
    }

    void optimize_block(unsigned int, unsigned int);
    void init_leaf_vars(DAG &, vector<inter_code> &);
    void gen_inter_codes_by_dag(unsigned int, unsigned int, DAG &, vector<inter_code> &);
    bool will_be_used(int end, const string &tmp_name) {
        bool res = false;
        for (unsigned int i = end + 1; i < inter_codes.size(); i++) {
            inter_code &reserved_code = inter_codes[i];
            if (reserved_code.type == FUNC_DEC)
                break;
            res |= reserved_code.z == tmp_name ||
                  reserved_code.index_z == tmp_name ||
                  reserved_code.x == tmp_name ||
                  reserved_code.index_x == tmp_name ||
                  reserved_code.y == tmp_name;
        }
        return res;
    }
    void print_DAG(DAG &dag) {
        unordered_map<string, int>::iterator iter;
        for (iter = dag.node_table.begin(); iter != dag.node_table.end(); iter++) {
            cout << iter->first << "  " << iter->second << endl;
        }
        cout << endl;
        for (auto x: dag.graph) {
            if (x.is_mid) {
                cout << x.index << "   " << x.val << "  " << x.op <<
                "  " << x.l_son << "   " << x.r_son << endl;
            }
            else {
                cout << x.index << "   " << x.val << endl;
            }
        }
        cout << endl;
        for (auto x: dag.mid_queue) {
            cout << x << endl;
        }
        cout << endl;cout << endl;cout << endl;
    }


public:
    Table &table;
    vector<inter_code> &inter_codes;
    vector<inter_code> optimized_inter_codes;
    unsigned int tmp_num;

    unordered_map<string, unordered_map<string, local_var_info> > local_var_infos;
    unordered_map<string, string> var_regs;
    unordered_map<string, int> reg_nums;
    unordered_map<string, int> flow_idx;
    vector<Flow> flows;

    Optimizer(Table &table_, vector<inter_code> &inter_codes_, unsigned int tmp_num_):
             table(table_), inter_codes(inter_codes_) {
        tmp_num = tmp_num_;
    }

    void DAG_optimize(int times) {
        while (times--) {
            unsigned int sz = inter_codes.size();
            unsigned int start, end;
            for (unsigned int i = 0; i < sz; i++) {
                if (!can_optimize(inter_codes[i])) {
                    optimized_inter_codes.push_back(inter_codes[i]);
                    continue;
                }
                start = i;
                while (i < sz && can_optimize(inter_codes[i]))
                    i++;
                i--;
                end = i;
                optimize_block(start, end);
            }
            inter_codes.clear();
            for (auto x: optimized_inter_codes)
                inter_codes.push_back(x);
            optimized_inter_codes.clear();
        }
    }

    void allocate_global_reg() {
        unsigned int sz = inter_codes.size();
        unsigned int start, end;
        for (unsigned int i = 0; i < sz; i++) {
            inter_code &tmp = inter_codes[i];
            if (tmp.type == FUNC_DEC) {
                reg_nums[tmp.z] = 0;
                local_var_infos[tmp.z] = unordered_map<string, local_var_info>();
                start = i;
                i++;
                while (i < sz && inter_codes[i].type != FUNC_DEC) i++;
                i--;
                end = i;
                allocate_func(start, end, tmp.z);
            }
        }
    }

    void analyse_flow() {
        unsigned int sz = inter_codes.size();
        unsigned int start, end;
        for (unsigned int i = 0; i < sz; i++) {
            inter_code &tmp = inter_codes[i];
            if (tmp.type == FUNC_DEC) {
                string func_name = tmp.z;
                start = i;
                i++;
                while (i < sz && inter_codes[i].type != FUNC_DEC) i++;
                i--;
                end = i;
                Flow flow(start, end, inter_codes);
                //flow.print();
                flows.push_back(flow);
                flow_idx[func_name] = flows.size() - 1;
            }
        }
    }

    void add_weight(const string &var, int ini_weight) {
        if (var == "" || (var[0] >= '0' && var[0] <= '3')) return;
        if (var.find("::") == string::npos) return;
        vector<string> ss;
        split(var, ss, "::");
        string func_name = ss[0];
        table_ele tmp_ele;
        table.find(func_name, ss[1], tmp_ele);
        if (tmp_ele.id_type == PARAMETER) return;
        if (tmp_ele.val_type == INT_ARRAY_VAL || tmp_ele.val_type == CHAR_ARRAY_VAL) return;
        unordered_map<string, local_var_info> &tmp_map = local_var_infos[func_name];
        if (tmp_map.find(var) == tmp_map.end())
            tmp_map[var] = local_var_info(var);
        tmp_map[var].weight += ini_weight;
    }

    void allocate_func(unsigned int start, unsigned int end, const string &func_name) {
        unordered_map<string, unsigned int> label_pos;
        unordered_map<string, bool> is_go_back;
        vector<local_var_info> tmp_vec;
        int ini_weight = 1;
        for (unsigned int i = start; i <= end; i++) {
            if (inter_codes[i].type == LABEL) {
                label_pos[inter_codes[i].label] = i;
                is_go_back[inter_codes[i].label] = false;
            }
        }
        for (unsigned int i = start; i <= end; i++) {
            inter_code &code = inter_codes[i];
            if (code.type == GOTO) {
                unsigned int pos = label_pos[code.label];
                if (pos > i) continue;
                if (!is_go_back[code.label]) {
                    is_go_back[code.label] = true;
                    ini_weight = 10;
                    i = pos;
                }
                else {
                    is_go_back[code.label] = false;
                    ini_weight = 1;
                }
            }
            else {
                add_weight(code.z, ini_weight);
                add_weight(code.index_z, ini_weight);
                add_weight(code.x, ini_weight);
                add_weight(code.index_x, ini_weight);
                add_weight(code.y, ini_weight);
            }
        }
        unordered_map<string, local_var_info> &tmp_map = local_var_infos[func_name];
        unordered_map<string, local_var_info>::iterator iter;
        for (iter = tmp_map.begin(); iter != tmp_map.end(); iter++) {
            tmp_vec.push_back(iter->second);
        }
        sort(tmp_vec.begin(), tmp_vec.end(), greater<local_var_info>());
        unsigned int reg_num = 0;
        for (auto x: tmp_vec) {
            var_regs[x.name] = global_regs[reg_num];
            //cout << x.name << "   " << x.weight << "   " << var_regs[x.name] << endl;
            reg_num++;
            if (reg_num == global_regs.size() - 1) break;
        }
        reg_nums[func_name] = (int)reg_num;
    }
};



#endif // OPTIMIZER_H_INCLUDED
