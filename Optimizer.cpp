#include "Optimizer.h"

void Optimizer::init_leaf_vars(DAG &dag, vector<inter_code> &tmp_inter_codes) {
    unordered_map<string, int>::iterator iter;
    for (iter = dag.node_table.begin(); iter != dag.node_table.end(); iter++) {
        string name = iter->first;
        int index = iter->second;
        if (dag.first_pos.find(name) == dag.first_pos.end()) continue;
        if (name[0] != '1' && index != dag.first_pos[name]) {
            inter_code tmp_code;
            tmp_code.type = ASSIGN_EXP;
            tmp_code.op = DIRECT_ASSIGN_OP;
            tmp_code.z = gen_tmp_var();
            tmp_code.z_val_type = dag.graph[dag.first_pos[name]].val_type;
            tmp_code.x = name;
            tmp_code.x_val_type = dag.graph[dag.first_pos[name]].val_type;
            dag.graph[dag.first_pos[name]].val = tmp_code.z;
            tmp_inter_codes.push_back(tmp_code);
        }
    }
}


void Optimizer::gen_inter_codes_by_dag(unsigned int start, unsigned int end, DAG &dag,
                            vector<inter_code> &tmp_inter_codes) {
    unsigned int sz = dag.mid_nodes.size();
    if (sz == 0) {
        for (unsigned int i = start; i <= end; i++)
            optimized_inter_codes.push_back(inter_codes[i]);
        return;
    }
    while (sz != dag.mid_queue.size()) {
        for (auto x: dag.mid_nodes) {
            if (!dag.can_added_to_q(x)) continue;
            dag.graph[x].in_q = true;
            dag.mid_queue.push_back(x);
            int l_son = dag.graph[x].l_son;
            while (dag.can_added_to_q(l_son)) {
                dag.graph[l_son].in_q = true;
                dag.mid_queue.push_back(l_son);
                l_son = dag.graph[l_son].l_son;
            }
        }
    }
    reverse(dag.mid_queue.begin(), dag.mid_queue.end());
    for (auto index: dag.mid_queue) {
        inter_code tmp_code;
        tmp_code.type = ASSIGN_EXP;
        Node fa_node = dag.graph[index];
        Node l_son_node = dag.graph[fa_node.l_son];
        Node r_son_node = dag.graph[fa_node.r_son];
        vector<string> &vars = dag.node_vars[index];
        if (l_son_node.val == "0" && fa_node.op == "+") {
            tmp_code.op = DIRECT_ASSIGN_OP;
            tmp_code.x = r_son_node.val;
            tmp_code.x_val_type = r_son_node.val_type;
            tmp_code.z_val_type = tmp_code.x_val_type;
        }
        else if (fa_node.op == ARRAY_ASSIGN_OP){
            tmp_code.op = DIRECT_ASSIGN_OP;
            tmp_code.x = l_son_node.val;
            tmp_code.x_val_type = l_son_node.val_type;
            tmp_code.index_x = r_son_node.val;
            tmp_code.index_x_val_type = r_son_node.val_type;
            tmp_code.z_val_type = tmp_code.x_val_type == INT_ARRAY_VAL ? INT_VAL : CHAR_VAL;
        }
        else {
            tmp_code.op = fa_node.op;
            tmp_code.x = l_son_node.val;
            tmp_code.x_val_type = l_son_node.val_type;
            tmp_code.y = r_son_node.val;
            tmp_code.y_val_type = r_son_node.val_type;
            tmp_code.z_val_type  = INT_VAL;
        }
        for (auto var: vars) {
            tmp_code.z = var;
            tmp_inter_codes.push_back(tmp_code);
        }
    }
    if (tmp_inter_codes.size() >= (end - start + 1)) {
        for (unsigned int i = start; i <= end; i++)
            optimized_inter_codes.push_back(inter_codes[i]);
    }
    else {
        for (auto x: tmp_inter_codes)
            optimized_inter_codes.push_back(x);
    }
}

void Optimizer::optimize_block(unsigned int start, unsigned int end) {
    DAG dag(inter_codes, tmp_num);
    dag.cal_reserved_tmps(end);
    vector<inter_code> tmp_inter_codes;
    for (unsigned int i = start; i <= end; i++) {
        inter_code code = inter_codes[i];
        string op = code.op;
        int l_son = -1, r_son = -1;
        if (is_array(code.x_val_type)) {
            l_son = dag.check_op_num(code.x, code.x_val_type);
            r_son = dag.check_op_num(code.index_x, code.index_x_val_type);
            op = ARRAY_ASSIGN_OP;
        }
        else if (op == DIRECT_ASSIGN_OP) {
            if (code.z[0] != '1' || will_be_used(i, code.z)) {
                op = "+";
                l_son = dag.check_op_num("0", INT_CON_VAL);
                r_son = dag.check_op_num(code.x, code.x_val_type);
            }
        }
        else {
            l_son = dag.check_op_num(code.x, code.x_val_type);
            r_son = dag.check_op_num(code.y, code.y_val_type);
        }
        dag.check_target_num(code.z, code.z_val_type, l_son, r_son, op);
    }
    //print_DAG(dag);
    init_leaf_vars(dag, tmp_inter_codes);
    dag.filter_node_vars();
    gen_inter_codes_by_dag(start, end, dag, tmp_inter_codes);
}
