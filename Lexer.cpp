#include "Lexer.h"

using namespace std;

void Lexer::nextch() {
    if (line_num == lines.size()) {
        ch = '\0';
        sy = NOTYPE;
        return;
    }

    ch = lines[line_num][ch_pos];
    last_ch_pos = ch_pos;
    last_line_num = line_num;

    ch_pos++;
    if (ch_pos == lines[line_num].length()) {
        ch_pos = 0;
        line_num++;
    }
}

void Lexer::retract() {
    ch_pos = last_ch_pos;
    line_num = last_line_num;
}

int Lexer::reserve(const string &objstr) {
    for (unsigned int i = 1; i <= KEYWORDS_NUM; i++)
        if (objstr == keywords[i])
            return i;
    return 0;
}


void Lexer::skip(char ch1, bool is_include) {
    while (ch != ch1 && ch != '\0')
        nextch();
    if (is_include) retract();
    insymbol();
}

void Lexer::skip(char ch1, char ch2, bool is_include) {
    while (ch != ch1 && ch != ch2 && ch != '\0')
        nextch();
    if (is_include) retract();
    insymbol();
}

void Lexer::skip(char ch1, char ch2, char ch3, bool is_include) {
    while (ch != ch1 && ch != ch2 && ch != ch3 && ch != '\0')
        nextch();
    if (is_include) retract();
    insymbol();
}

void Lexer::skip(SYMBOL_TYPE sy1, bool is_include) {
    while (sy != sy1) {
        insymbol();
        if (over) return;
    }
    if (!is_include) insymbol();
}

void Lexer::skip(SYMBOL_TYPE sy1, SYMBOL_TYPE sy2, bool is_include) {
    while (sy != sy1 && sy != sy2) {
        insymbol();
        if (over) return;
    }
    if (!is_include) insymbol();
}

void Lexer::skip(SYMBOL_TYPE sy1, SYMBOL_TYPE sy2, SYMBOL_TYPE sy3, bool is_include) {
    while (sy != sy1 && sy != sy2 && sy != sy3) {
        insymbol();
        if (over) return;
    }
    if (!is_include) insymbol();
}

void Lexer::skip(const vector<SYMBOL_TYPE>& syset) {
    while (true) {
        for (auto x: syset) {
            if (sy == x) {
                return;
            }
        }
        insymbol();
        if (over) return;
    }
}

void Lexer::skip(const vector<SYMBOL_TYPE>& syset1, const vector<SYMBOL_TYPE>& syset2) {
    while (true) {
        for (auto x: syset1) {
            if (sy == x) {
                return;
            }
        }
        for (auto x: syset2) {
            if (sy == x) {
                return;
            }
        }
        insymbol();
        if (over) return;
    }
}

void Lexer::skip(const vector<SYMBOL_TYPE>& syset1, const vector<SYMBOL_TYPE>& syset2,
                 const vector<SYMBOL_TYPE>& syset3) {
    while (true) {
        for (auto x: syset1) {
            if (sy == x) {
                return;
            }
        }
        for (auto x: syset2) {
            if (sy == x) {
                return;
            }
        }
        for (auto x: syset3) {
            if (sy == x) {
                return;
            }
        }
        insymbol();
        if (over) return;
    }
}

void Lexer::skip(const vector<SYMBOL_TYPE>& syset1, const vector<SYMBOL_TYPE>& syset2,
                 const vector<SYMBOL_TYPE>& syset3, const vector<SYMBOL_TYPE>& syset4) {
    while (true) {
        for (auto x: syset1) {
            if (sy == x) {
                return;
            }
        }
        for (auto x: syset2) {
            if (sy == x) {
                return;
            }
        }
        for (auto x: syset3) {
            if (sy == x) {
                return;
            }
        }
        for (auto x: syset4) {
            if (sy == x) {
                return;
            }
        }
        insymbol();
        if (over) return;
    }
}


bool Lexer::insymbol() {
    if (!real_read) {
        real_read = true;
        return true;
    }
    id = "";
    err_id = "";
    nextch();
    if (ch == '\0') {
        over = true;
        return false;
    }
    while (isspace(ch))
        nextch();
    if (ch == '\0') {
        over = true;
        return false;
    }
    err_id += ch;
    if (isdigit(ch)) {
        while (isdigit(ch)) {
            id += ch;
            nextch();
            err_id += ch;
        }
        retract();
        int_con = atoi(id.c_str());
        sy = INTCON;
    }
    else if (isalpha(ch) || ch == '_') {
        while (isalpha(ch) || ch == '_' || isdigit(ch)) {
            id += ch;
            nextch();
            err_id += ch;
        }
        retract();
        sy = (SYMBOL_TYPE)reserve(id);
    }
    else if (ch == '\'') {
        id += ch;
        nextch();
        err_id += ch;
        if (isalpha(ch) || isdigit(ch) || ch == '_' ||
            ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            char_con = ch;
            id += ch;
            sy = CHARCON;
            nextch();
            id += ch;
            if (ch != '\'') {
                sy = NOTYPE;
                error.unknown_error(err_id, last_line_num, last_ch_pos);
                retract();
                return false;
            }
        }
        else {
            sy = NOTYPE;
            error.unknown_error(err_id, last_line_num, last_ch_pos);
            return false;
        }
    }
    else if (ch == '"') {
        nextch();
        err_id += ch;
        while (ch == 32 || ch == 33 || (ch >= 35 && ch <= 126)) {
            id += ch;
            err_id += ch;
            nextch();
        }
        if (ch != '"') {
            sy = NOTYPE;
            error.unknown_error(err_id, last_line_num, last_ch_pos);
            retract();
            return false;
        }
        else {
            sy = STRINGCON;
        }
    }
    else if (ch == '=') {
        nextch();
        if (ch == '=') {
            id = "==";
            sy = EQL;
        }
        else {
            id = "=";
            sy = BECOMES;
            retract();
        }
    }
    else if (ch == '!') {
        nextch();
        if (ch == '=') {
            id = "!=";
            sy = NEQ;
        }
        else {
            sy = NOTYPE;
            error.unknown_error(err_id, last_line_num, last_ch_pos);
            retract();
            return false;
        }
    }
    else if (ch == '>') {
        nextch();
        if (ch == '=') {
            id = ">=";
            sy = GEQ;
        }
        else {
            id = ">";
            sy = GTR;
            retract();
        }
    }
    else if (ch == '<') {
        nextch();
        if (ch == '=') {
            id = "<=";
            sy = LEQ;
        }
        else {
            id = "<";
            sy = LSS;
            retract();
        }
    }
    else if (ch == '+') {
        id = "+";
        sy = PLUS;
    }
    else if (ch == '-') {
        id = "-";
        sy = MINUS;
    }
    else if (ch == '*') {
        id = "*";
        sy = TIMES;
    }
    else if (ch == '/') {
        id = "/";
        sy = DIV;
    }
    else if (ch == '(') {
        id = "(";
        sy = LPARENT;
    }
    else if (ch == ')') {
        id = ")";
        sy = RPARENT;
    }
    else if (ch == '[') {
        id = "[";
        sy = LBRACK;
    }
    else if (ch == ']') {
        id = "]";
        sy = RBRACK;
    }
    else if (ch == '{') {
        id = "{";
        sy = LBRACE;
    }
    else if (ch == '}') {
        id = "}";
        sy = RBRACE;
    }
    else if (ch == ',') {
        id = ",";
        sy = COMMA;
    }
    else if (ch == ':') {
        id = ":";
        sy = COLON;
    }
    else if (ch == ';') {
        id = ";";
        sy = SEMICOLON;
    }
    else {
        // error process
        sy = NOTYPE;
        error.unknown_error(err_id, line_num, ch_pos);
        nextch();
        return false;
    }
    return true;
}

void Lexer::print(SYMBOL_TYPE sy, const string &id, int int_con, char char_con) {
    switch (sy) {
        case IDENT:     cout << "IDENT";    break;
        case CONSTSY:   cout << "CONSTSY";  break;
        case INTSY:     cout << "INTSY";    break;
        case CHARSY:    cout << "CHARSY";   break;
        case VOIDSY:    cout << "VOIDSY";   break;
        case MAINSY:    cout << "MAINSY";   break;
        case IFSY:      cout << "IFSY";     break;
        case WHILESY:   cout << "WHILESY";  break;
        case SWITCHSY:  cout << "SWITCHSY"; break;
        case CASESY:    cout << "CASESY";   break;
        case DEFAULTSY: cout << "DEFAULTSY";break;
        case SCANFSY:   cout << "SCANFSY";  break;
        case PRINTFSY:  cout << "PRINTFSY"; break;
        case RETURNSY:  cout << "RETURNSY"; break;
        case INTCON:    cout << "INTCON";   break;
        case CHARCON:   cout << "CHARCON";  break;
        case STRINGCON: cout << "STRINGCON";break;
        case EQL:       cout << "EQL";      break;
        case NEQ:       cout << "NEQ";      break;
        case GTR:       cout << "GTR";      break;
        case GEQ:       cout << "GEQ";      break;
        case LSS:       cout << "LSS";      break;
        case LEQ:       cout << "LEQ";      break;
        case PLUS:      cout << "PLUS";     break;
        case MINUS:     cout << "MINUS";    break;
        case TIMES:     cout << "TIMES";    break;
        case DIV:       cout << "DIV";      break;
        case LPARENT:   cout << "LPARENT";  break;
        case RPARENT:   cout << "RPARENT";  break;
        case LBRACK:    cout << "LBRACK";   break;
        case RBRACK:    cout << "RBRACK";   break;
        case LBRACE:    cout << "LBRACE";   break;
        case RBRACE:    cout << "RBRACE";   break;
        case COMMA:     cout << "COMMA";    break;
        case COLON:     cout << "COLON";    break;
        case SEMICOLON: cout << "SEMICOLON";break;
        case BECOMES:   cout << "BECOMES";  break;
        default:        cout << "UNKNOWN";  break;
    }
    if (sy == INTCON)
        cout << " " << int_con << endl;
    else if (sy == CHARCON)
        cout << " " << char_con << endl;
    else
        cout << " " << id << endl;
}
