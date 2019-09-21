#ifndef CONSTS_H_INCLUDED
#define CONSTS_H_INCLUDED
#include <string>
#include <vector>
#include "types.h"
using namespace std;

const vector<string> keywords = {
    "", "const", "int", "char", "void", "main",
    "if", "while", "switch", "case", "default",
    "scanf", "printf", "return"
};

const unsigned int KEYWORDS_NUM = 13;
const string GLOBAL_SCOPE = "0G";
const string MAIN_SCOPE = "main";
const string DIRECT_ASSIGN_OP = "<-";
const string ARRAY_ASSIGN_OP = "=[]";



const vector<SYMBOL_TYPE> state_begin_sy = {
    IDENT, IFSY, WHILESY, SWITCHSY, SCANFSY,
    PRINTFSY, RETURNSY, LBRACE, RBRACE
};

const vector<SYMBOL_TYPE> rparent_lbrace_sy {
    RPARENT, LBRACE
};


const vector<SYMBOL_TYPE> const_begin_sy = {
    CONSTSY
};



const vector<SYMBOL_TYPE> var_begin_sy = {
    INTSY, CHARSY
};

const vector<SYMBOL_TYPE> func_begin_sy = {
    INTSY, CHARSY, VOIDSY
};

const vector<string> global_regs = {
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5"
};



#endif // CONSTS_H_INCLUDED
