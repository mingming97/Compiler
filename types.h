#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

using namespace std;

enum SYMBOL_TYPE {
    IDENT, CONSTSY, INTSY, CHARSY, VOIDSY,
    MAINSY, IFSY, WHILESY,
    SWITCHSY, CASESY, DEFAULTSY,
    SCANFSY, PRINTFSY, RETURNSY,
    INTCON, CHARCON, STRINGCON,
    EQL, NEQ, GTR, GEQ, LSS, LEQ,
    PLUS, MINUS, TIMES, DIV,
    LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE,
    COMMA, COLON, SEMICOLON,
    BECOMES,
    NOTYPE
};

enum IDENT_TYPE {
    CONSTANT,
    VARIABLE,
    FUNCTION,
    PARAMETER
};

enum VALUE_TYPE {
    INT_VAL,
    CHAR_VAL,
    INT_ARRAY_VAL,
    CHAR_ARRAY_VAL,
    INT_CON_VAL,
    CHAR_CON_VAL,
    NOTYP_VAL
};

enum RETURN_TYPE {
    VOID_RET,
    INT_RET,
    CHAR_RET,
};

enum INTER_CODE_TYPE {
    FUNC_DEC,       // z, ret_type
    PARA_DEC,       // z, val_type
    PARA_PUSH,      // z
    FUNC_CALL,      // z, x
    FUNC_RET,       // z, x
    ASSIGN_EXP,     // z, op, x, y
    CONDITION,      // x, op, y
    GOTO,           // label
    BNZ,            // label
    BZ,             // label
    LABEL,          // label
    READ,           // z, z_val_type
    PRINTSTRING,    // label
    PRINT,          // z, z_val_type
};

enum VAR_POS {
    STACK,
    REG,
    UNCERTAIN
};

struct inter_code {
    // z = x op y
    INTER_CODE_TYPE type;
    string x;
    string y;
    string z;
    string op;
    VALUE_TYPE x_val_type;
    VALUE_TYPE y_val_type;
    VALUE_TYPE z_val_type;

    string index_x;
    string index_z;
    VALUE_TYPE index_x_val_type;
    VALUE_TYPE index_z_val_type;
    string label;
    RETURN_TYPE ret_type;

    inter_code() {
        x = y = z = op = index_x = index_z = label = "";
        x_val_type = y_val_type = z_val_type =
        index_x_val_type = index_z_val_type = NOTYP_VAL;
        ret_type = VOID_RET;
    }
};

struct exp_ret_info {
    string tmp_name;        // 如果没计算出值，中间变量的名字(也可能为全局变量)
    int int_val;
    bool is_valid;          // int_val是否是确定的(expression是否能计算出有效值)
    VALUE_TYPE val_type;
    exp_ret_info() {
        is_valid = false;
        int_val = 0;
        tmp_name = "";
        val_type = NOTYP_VAL;
    }
    void clear() {
        is_valid = false;
        int_val = 0;
        tmp_name = "";
        val_type = NOTYP_VAL;
    }
};

struct case_table_ele {
    string label_name;
    int int_val;
    VALUE_TYPE val_type;
};

struct var_info {
    VAR_POS var_pos;

    string reg;

    string base_reg;
    int offset;

    var_info() {
        var_pos = UNCERTAIN;
        offset = 0;
    }

};

struct size_info {
    unsigned int param_size;
    unsigned int var_size;
    unsigned int tmp_size;
    size_info() {
        param_size = var_size = tmp_size = 0;
    }
};

struct bitvec {
    bool* bits;
    unsigned int len;
    bitvec() {len = 0;}
    bitvec(unsigned int len_) {
        bits = new bool[len_]();
        len = len_;
    }
    bitvec(const bitvec &other) {
        bits = new bool[other.len];
        len = other.len;
        for (unsigned int i = 0; i < len; i++)
            bits[i] = other.bits[i];
    }
    ~bitvec() {if (len != 0) delete []bits;}

    void print() {
        for (unsigned int i = 0; i < len; i++)
            cout << bits[i];
        cout << endl;
    }

    unsigned int count() {
        unsigned int cnt = 0;
        for (unsigned int i = 0; i < len; i++)
            if (bits[i] == 1) cnt++;
        return cnt;
    }

    bitvec& operator=(const bitvec &other) {
        if (this == &other) return *this;
        if (this->len != 0) delete []bits;
        this->bits = new bool[other.len];
        this->len = other.len;
        for (unsigned int i = 0; i < other.len; i++)
            this->bits[i] = other.bits[i];
        return *this;
    }
    bitvec operator~() const{
        bitvec tmp(len);
        for (unsigned int i = 0; i < len; i++)
            tmp.bits[i] = !bits[i];
        return tmp;
    }
    bitvec operator&(const bitvec &other) {
        bitvec tmp(len);
        for (unsigned int i = 0; i < len; i++)
            tmp.bits[i] = bits[i] & other.bits[i];
        return tmp;
    }
    bitvec operator|(const bitvec &other) {
        bitvec tmp(len);
        for (unsigned int i = 0; i < len; i++)
            tmp.bits[i] = bits[i] | other.bits[i];
        return tmp;
    }
    bitvec& operator&=(const bitvec &other) {
        for (unsigned int i = 0; i < this->len; i++)
            this->bits[i] &= other.bits[i];
        return *this;
    }
    bitvec& operator|=(const bitvec &other) {
        for (unsigned int i = 0; i < this->len; i++)
            this->bits[i] |= other.bits[i];
        return *this;
    }
    bitvec operator-(const bitvec &other) {
        return (*this & ~other);
    }
    bool operator==(const bitvec &other) {
        for (unsigned int i = 0; i < len; i++)
            if (bits[i] != other.bits[i])
                return false;
        return true;
    }
    bool& operator[](const int i) {
        return bits[len - i];
    }
};


#endif // TYPES_H_INCLUDED
