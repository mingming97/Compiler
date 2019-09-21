#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED
#include "headers.h"


struct table_ele {
    string id;
    enum IDENT_TYPE id_type;
    enum VALUE_TYPE val_type;
    enum RETURN_TYPE ret_type;
    char char_const;
    int int_const;
    int length;
    string scope;
};

class Table {
private:
    Error &error;

    bool check_multiple_def(const string &id, const vector<table_ele> &table) {
        for (auto x: table)
            if (x.id == id) {
                return false;
            }
        for (auto x: scope_map[GLOBAL_SCOPE]) {
            if (x.id == id && x.id_type == FUNCTION) {
                return false;
            }
        }
        return true;
    }

    void initialize_scope(const string &scope) {
        if (scope_map.find(scope) != scope_map.end())
            return;
        scope_map[scope] = vector<table_ele>();
    }

public:
    map<string, vector<table_ele> > scope_map;
    vector<string> string_label;
    Table(Error &error_):error(error_){}
    void printtable() {
        map<string, vector<table_ele> >::iterator iter;
        for (iter = scope_map.begin(); iter != scope_map.end(); iter++) {
            string scope;
            scope = iter->first;
            vector<table_ele> &vec = iter->second;
            for (auto x : vec) {
                // cout << x.id << " " << x.int_const  << " " << x.char_const <<endl;
                //cout << x.length << " " << x.id_type <<" " << x.val_type << endl;
                cout << x.scope <<"  " << x.id << endl;
            }
        }
    }

    bool find(const string &scope, const string &id, table_ele &ele) {
        table_ele tmp;
        ele = tmp;
        if (scope_map.find(scope) != scope_map.end()) {
            for (auto x : scope_map[scope])
                if (x.id == id) {
                    ele = x;
                    return true;
                }
        }
        if (scope_map.find(GLOBAL_SCOPE) != scope_map.end()) {
            for (auto x : scope_map[GLOBAL_SCOPE])
                if (x.id == id) {
                    ele = x;
                    return true;
                }
        }
        return false;
    }

    int get_first_para_pos(const string &func_name) {
        if (scope_map.find(func_name) != scope_map.end()) {
            vector<table_ele> &eles = scope_map[func_name];
            for (unsigned int i = 0;i < eles.size(); i++)
                if (eles[i].id_type == PARAMETER)
                    return (int)i;
        }
        return -1;
    }

    bool get_ele(const string &scope, unsigned int pos, table_ele &ele) {
        if (scope_map.find(scope) != scope_map.end()) {
            if (pos < scope_map[scope].size()) {
                ele = scope_map[scope][pos];
                return true;
            }
        }
        return false;
    }

    void push(const string &scope, const string &id, RETURN_TYPE ret_type,
              unsigned int line_num, unsigned int ch_pos) {
        initialize_scope(scope);
        if (!check_multiple_def(id, scope_map[scope])) {
            error.multiple_def(id, line_num, ch_pos);
            return;
        }
        table_ele tmp;
        tmp.length = 0;
        tmp.id = id;
        tmp.id_type = FUNCTION;
        tmp.ret_type = ret_type;
        tmp.scope = scope;
        scope_map[scope].push_back(tmp);
    }

    void push(const string &scope, const string &id, IDENT_TYPE id_type, VALUE_TYPE val_type,
              unsigned int line_num, unsigned int ch_pos) {
        initialize_scope(scope);
        if (!check_multiple_def(id, scope_map[scope])) {
            error.multiple_def(id, line_num, ch_pos);
            return;
        }
        if (id_type != VARIABLE && id_type != PARAMETER)
            return;
        table_ele tmp;
        tmp.length = 0;
        tmp.id = id;
        tmp.id_type = id_type;
        tmp.val_type = val_type;
        tmp.scope = scope;
        scope_map[scope].push_back(tmp);
    }

    void push(const string &scope, const string &id, char c_,
              unsigned int line_num, unsigned int ch_pos) {
        initialize_scope(scope);
        if (!check_multiple_def(id, scope_map[scope])) {
            error.multiple_def(id, line_num, ch_pos);
            return;
        }
        table_ele tmp;
        tmp.length = 0;
        tmp.id = id;
        tmp.char_const = c_;
        tmp.id_type = CONSTANT;
        tmp.val_type = CHAR_CON_VAL;
        tmp.scope = scope;
        scope_map[scope].push_back(tmp);
    }

    void push(const string &scope, const string &id, int i_,
              unsigned int line_num, unsigned int ch_pos) {
        initialize_scope(scope);
        if (!check_multiple_def(id, scope_map[scope])) {
            error.multiple_def(id, line_num, ch_pos);
            return;
        }
        table_ele tmp;
        tmp.length = 0;
        tmp.id = id;
        tmp.int_const = i_;
        tmp.id_type = CONSTANT;
        tmp.val_type = INT_CON_VAL;
        tmp.scope = scope;
        scope_map[scope].push_back(tmp);
    }

    void push(const string &scope, const string &id, VALUE_TYPE val_type, int length,
              unsigned int line_num, unsigned int ch_pos) {
        initialize_scope(scope);
        if (!check_multiple_def(id, scope_map[scope])) {
            error.multiple_def(id, line_num, ch_pos);
            return;
        }
        if (val_type != INT_ARRAY_VAL && val_type != CHAR_ARRAY_VAL)
            return;
        table_ele tmp;
        tmp.length = length;
        tmp.id = id;
        tmp.id_type = VARIABLE;
        tmp.val_type = val_type;
        tmp.scope = scope;
        scope_map[scope].push_back(tmp);
    }

    void push(string &scope, const string &id) {
        for (auto x: string_label) {
            if (scope_map[x][0].id == id) {
                scope = x;
                return;
            }
        }
        initialize_scope(scope);
        table_ele tmp;
        tmp.id = id;
        tmp.scope = scope;
        string_label.push_back(scope);
        scope_map[scope].push_back(tmp);
    }

    IDENT_TYPE get_id_type(const string &scope, const string &id) {
        table_ele tmp_ele;
        find(scope, id, tmp_ele);
        return tmp_ele.id_type;
    }

    unsigned int get_index(const string &scope, const string &id) {
        IDENT_TYPE id_type = get_id_type(scope, id);
        vector<table_ele> &vec = scope_map[scope];
        unsigned int i, index = 0;
        for (i = 0; i < vec.size(); i++)
            if (vec[i].id_type == id_type)
                break;
        for (; i < vec.size(); i++) {
            if (vec[i].id_type != id_type)
                break;
            if (vec[i].id == id)
                return index;
            if (vec[i].length == 0) index += 1;
            else index += vec[i].length;
        }
        return -1;
    }

    /*int cal_size(const string &scope, IDENT_TYPE id_type) {
        vector<table_ele> &vec = scope_map[scope];
        unsigned int i, size = 0;
        for (i = 0; i < vec.size(); i++)
            if (vec[i].id_type == id_type)
                break;
        for (; i < vec.size(); i++) {
            if (vec[i].id_type != id_type)
                break;
            if (vec[i].length == 0) size += 4;
            else size += vec[i].length * 4;
        }
        return size;
    }*/

};


#endif // TABLE_H_INCLUDED
