#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED
#include "headers.h"
#include "FileHelper.h"
class Error;

class Lexer {
private:
    vector<string> lines;
    char ch;
    void nextch();
    void retract();
    int reserve(const string &);
    bool real_read;
    Error &error;
public:
    enum SYMBOL_TYPE sy;
    string id;
    string err_id;
    int int_con;
    char char_con;
    unsigned int last_ch_pos;
    unsigned int last_line_num;
    unsigned int ch_pos;
    unsigned int line_num;
    bool over;
    Lexer(const string& filename, Error &error_):error(error_) {
        ch = '\0';
        ch_pos = 0;
        line_num = 0;
        over = false;
        last_ch_pos = 0;
        last_line_num = 0;
        lines = FileHelper::get_lines(filename);
        real_read = true;
    }
    bool insymbol();
    void skip(char, bool);
    void skip(char, char, bool);
    void skip(char, char, char, bool);
    void skip(SYMBOL_TYPE, bool);
    void skip(SYMBOL_TYPE, SYMBOL_TYPE, bool);
    void skip(SYMBOL_TYPE, SYMBOL_TYPE, SYMBOL_TYPE, bool);
    void skip(const vector<SYMBOL_TYPE>&);
    void skip(const vector<SYMBOL_TYPE>&, const vector<SYMBOL_TYPE>&);
    void skip(const vector<SYMBOL_TYPE>&, const vector<SYMBOL_TYPE>&, const vector<SYMBOL_TYPE>&);
    void skip(const vector<SYMBOL_TYPE>&, const vector<SYMBOL_TYPE>&, const vector<SYMBOL_TYPE>&,
              const vector<SYMBOL_TYPE>&);

    pair<unsigned int, unsigned int> get_state() {
        return make_pair(line_num, ch_pos);
    }

    void set_state(unsigned int line_num_, unsigned int ch_pos_) {
        line_num = line_num_;
        ch_pos = ch_pos_;
    }

    void no_read_next() {
        real_read = false;
    }

    void print(SYMBOL_TYPE, const string &, int, char);

};


#endif // LEXER_H_INCLUDED
