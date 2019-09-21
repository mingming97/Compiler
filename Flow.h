#ifndef FLOW_H_INCLUDED
#define FLOW_H_INCLUDED
#include "headers.h"

using namespace std;

struct flow_info {
    vector<bitvec> vec_set;
    vector<bool> has_gen;
    vector<vector<unsigned int> > vecs;
};

struct link {
    vector<unsigned int> points;
};

class Flow {
private:
    vector<inter_code> &inter_codes;
    unsigned int start, end;
    flow_info gen;
    flow_info kill;
    flow_info in;
    flow_info out;
    vector<vector<unsigned int> > prev;
    unsigned int bit_len;
    unsigned int exit_pos;
    unordered_map<string, vector<link> > links;
    inline bool is_array(VALUE_TYPE val_type) {
        return val_type == INT_ARRAY_VAL || val_type == CHAR_ARRAY_VAL;
    }
    inline unsigned int bit_id(unsigned int i) {
        return i - start + 1;
    }
    inline unsigned int vec_id(unsigned int i) {
        return i - start;
    }
    void cal_gen_kill() {
        for (unsigned int i = start; i <= end; i++) {
            inter_code &code = inter_codes[i];
            gen.vec_set.push_back(bitvec(bit_len));
            kill.vec_set.push_back(bitvec(bit_len));
            in.vec_set.push_back(bitvec(bit_len));
            out.vec_set.push_back(bitvec(bit_len));
            gen.has_gen.push_back(false);
            if (code.type == ASSIGN_EXP) {
                if (code.z[0] != '0' && code.z[0] != '1' && !is_array(code.z_val_type)) {
                    gen.vec_set[vec_id(i)][bit_id(i)] = 1;
                    gen.has_gen[vec_id(i)] = true;
                    for (unsigned int j = start; j <= end; j++) {
                        inter_code &tmp_code = inter_codes[j];
                        if (j == i) continue;
                        if (tmp_code.type == ASSIGN_EXP && tmp_code.z == code.z) {
                            kill.vec_set[vec_id(i)][bit_id(j)] = 1;
                        }
                    }
                }
            }
        }
        in.vec_set.push_back(bitvec(bit_len));
    }
    void cal_prev() {
        for (unsigned int i = start; i <= end; i++) {
            inter_code &code = inter_codes[i];
            if (code.type == FUNC_RET) {
                prev[vec_id(exit_pos)].push_back(i);
            }
            else if (code.type == GOTO) {
                for (unsigned int j = start; j <= end; j++) {
                    if (inter_codes[j].type == LABEL && inter_codes[j].label == code.label)
                        prev[vec_id(j)].push_back(i);
                }
            }
            else if (code.type == BNZ || code.type == BZ) {
                for (unsigned int j = start; j <= end; j++) {
                    if (inter_codes[j].type == LABEL && inter_codes[j].label == code.label)
                        prev[vec_id(j)].push_back(i);
                }
                if (i + 1 <= end)
                    prev[vec_id(i + 1)].push_back(i);
            }
            else {
                prev[vec_id(i + 1)].push_back(i);
            }
        }
    }
    void cal_in_out() {
        bitvec tmp_in, tmp_out;
        bool is_change = true;
        while (is_change) {
            is_change = false;
            for (unsigned int i = start; i <= end; i++) {
                unsigned int idx = vec_id(i);
                tmp_in = in.vec_set[idx];
                tmp_out = out.vec_set[idx];
                for (auto x: prev[idx])
                    in.vec_set[idx] |= out.vec_set[vec_id(x)];
                out.vec_set[idx] = gen.vec_set[idx] | (in.vec_set[idx] - kill.vec_set[idx]);
                if (!(tmp_in == in.vec_set[idx] && tmp_out == out.vec_set[idx]))
                    is_change = true;
            }
            unsigned int exit_id = vec_id(exit_pos);
            tmp_in = in.vec_set[exit_id];
            for (auto x: prev[exit_id])
                in.vec_set[exit_id] |= out.vec_set[vec_id(x)];
            if (!(tmp_in == in.vec_set[exit_id]))
                is_change = true;
        }
        in.vecs = vector<vector<unsigned int> >(bit_len, vector<unsigned int>());
        out.vecs = vector<vector<unsigned int> >(bit_len, vector<unsigned int>());
        for (unsigned int i = start; i <= end; i++) {
            for (unsigned int j = 1; j <= bit_len; j++) {
                if (in.vec_set[vec_id(i)][j] == 1) in.vecs[vec_id(i)].push_back(j - 1 + start);
                if (out.vec_set[vec_id(i)][j] == 1) out.vecs[vec_id(i)].push_back(j - 1 + start);
            }
        }
    }

    bool in_vec(unsigned int tar, const vector<unsigned int> &vec) {
        for (auto x: vec)
            if (tar == x)
                return true;
        return false;
    }

    inline bool check(const inter_code &code, const string &name) {
        return code.x == name || code.index_x == name || code.y == name || code.index_z == name;
    }

    void cal_links() {
        for (unsigned int i = start; i <= end; i++) {
            if (!gen.has_gen[vec_id(i)]) continue;
            string var_name = inter_codes[i].z;
            link tmp_link;
            tmp_link.points.push_back(i);
            for (unsigned int j = start; j <= end; j++) {
                if (in_vec(i, in.vecs[vec_id(j)]) && check(inter_codes[j], var_name))
                    tmp_link.points.push_back(j);
            }
            if (tmp_link.points.size() > 1) {
                if (links.find(var_name) == links.end())
                    links[var_name] = vector<link>();
                links[var_name].push_back(tmp_link);
            }
        }
    }

public:
    Flow(unsigned int start_, unsigned int end_, vector<inter_code> &inter_codes_):
        inter_codes(inter_codes_) {
        start = start_;
        end = end_;
        bit_len = end - start + 1;
        exit_pos = end_ + 1;
        prev = vector<vector<unsigned int> >(end - start + 2, vector<unsigned int>());
        cal_gen_kill();
        cal_prev();
        cal_in_out();
        cal_links();
    }

    void print() {
        /*for (auto x: in.vec_set) x.print();
        cout << endl;
        for (auto x: gen.vec_set) x.print();*/
        unordered_map<string, vector<link> >::iterator iter;
        for (iter = links.begin(); iter != links.end(); iter++) {
            cout << iter->first << ": " << endl;
            vector<link> &tmp = iter->second;
            for (auto x: tmp) {
                for (auto y: x.points) {
                    cout << y << "  ";
                }
                cout << endl;
            }
        }
        /*for (unsigned int i = 0; i <= bit_len; i++) {
            cout << i << ":    ";
            for (auto x: prev[i])
                cout << x << " ";
            cout << endl;
        }*/
    }

};

#endif // FLOW_H_INCLUDED
