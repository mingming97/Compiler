#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED
#include "headers.h"

using namespace std;

class Error {
private:
    vector<string> errors;

    inline string err_pos(unsigned int line_num, unsigned int ch_pos) {
        return " at line: " + my_to_string(line_num+1) + " char: " + my_to_string(ch_pos) + " ";
    }

public:

    inline bool has_error() {
        return errors.size() != 0;
    }

    void unknown_error(const string &err, unsigned int line_num, unsigned int ch_pos) {
        unsigned int err_len = err.length();
        if (err[err_len - 1] == '\n') err_len -= 1;
        string str;
        str = "unknown identifier " + err.substr(0, err_len) +
              " at line: " + my_to_string(line_num + 1) + " char: " + my_to_string(ch_pos);
        errors.push_back(str);
    }
    inline void programme_not_finish() {
        errors.push_back("Programme has extra contents at the end of the file!");
    }
    void finish_early() {
        errors.push_back("Programme finishes too early");
        print();
        exit(0);
    }
    inline void unknown_ret_type(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Unknown return type \"" + id +
                       "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void lack_main() {
        errors.push_back("No main function");
    }

    inline void lack_lparent(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \'(\'" + err_pos(line_num+1, ch_pos);
        errors.push_back(str);
    }
    inline void lack_rparent(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \')\'" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void lack_lbrace(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \'{\'" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void lack_rbrace(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \'}\'" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void lack_lbrack(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \'[\'" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void lack_rbrack(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \']\'" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void lack_semicolon(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \';\'" + err_pos(line_num - 1, ch_pos);
        errors.push_back(str);
    }
    inline void lack_colon(unsigned int line_num, unsigned int ch_pos) {
        string str = "Lack \':\'" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void wrong_const_dec(unsigned int line_num, unsigned int ch_pos) {
        string str = "Wrong constant declaration" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void expect_identifier(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expected an identifier" + err_pos(line_num, ch_pos) +
                     ", but got \"" + id + "\" here.";
        errors.push_back(str);
    }
    inline void expect_becomes(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expected \"=\"" + err_pos(line_num, ch_pos) +
                     ", but got \"" + id + "\" here.";
        errors.push_back(str);
    }
    inline void expect_char(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expected an char" + err_pos(line_num, ch_pos) +
                     ", but got \"" + id + "\" here.";
        errors.push_back(str);
    }
    inline void expect_legal_type(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expected an legal type: int or char" + err_pos(line_num, ch_pos) +
                     ", but got \"" + id + "\" here.";
        errors.push_back(str);
    }
    inline void expect_integer(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expected an integer" + err_pos(line_num, ch_pos) +
                     ", but got \"" + id + "\" here.";
        errors.push_back(str);
    }
    inline void illegal_integer(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Illegal number \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void array_len_zero(unsigned int line_num, unsigned int ch_pos) {
        string str = "Array length shouldn't be zero" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void multiple_def(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Multiple definition for identifier \"" +
                      id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void undefined_id(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Undefined identifier \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void expect_array(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expect an array," + err_pos(line_num, ch_pos) +
                     "\"" + id + "\" is not a array type";
        errors.push_back(str);
    }
    inline void expect_function(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expect a function," + err_pos(line_num, ch_pos) +
                     "\"" + id + "\" is not a function";
        errors.push_back(str);
    }
    inline void expect_nonvoid(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expect non void function," + err_pos(line_num, ch_pos) +
                     "\"" + id + "\" is a void function";
        errors.push_back(str);
    }
    inline void unknown_id(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Unknown identifier " + id + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void val_type_wrong(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Wrong value type for identifier \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void wrong_sy_after_id(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Wrong symbol after identifier \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void wrong_rvalue(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "\"" + id + "\"" + err_pos(line_num, ch_pos) + " cannot be assigned";
        errors.push_back(str);
    }
    inline void type_mismatch(unsigned int line_num, unsigned int ch_pos) {
        string str = "Assignment mismatch" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void index_type_error(unsigned int line_num, unsigned int ch_pos) {
        string str = "Index type error" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void overlength_array(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Index out of range for array \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void condition_wrong(unsigned int line_num, unsigned int ch_pos) {
        string str = "Condition wrong" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void expect_integer_char(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Expect an integer or char," + err_pos(line_num, ch_pos) +
                     ", but got \"" + id + "\" here.";
        errors.push_back(str);
    }
    inline void param_mismatch(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Parameter mismatch for calling function \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void param_num_mismatch(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Parameter number mismatch for calling function \"" + id + "\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void void_return_nonvoid(unsigned int line_num, unsigned int ch_pos) {
        string str = "Void function has contents after \"return\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void nonvoid_return_void(unsigned int line_num, unsigned int ch_pos) {
        string str = "Non-void function has no contents after \"return\"" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void return_type_mismatch(unsigned int line_num, unsigned int ch_pos) {
        string str = "Return value type is not match with its definition" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void compare_type_mismatch(unsigned int line_num, unsigned int ch_pos) {
        string str = "Compare value type mismatch" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void no_return_value(const string &id) {
        string str = "No return value for non-void function \"" + id + "\"";
        errors.push_back(str);
    }
    inline void case_repeat(const string &id, unsigned int line_num, unsigned int ch_pos) {
        string str = "Same case " + id + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }
    inline void compare_not_int(unsigned int line_num, unsigned int ch_pos) {
        string str = "Expect compare between integers" + err_pos(line_num, ch_pos);
        errors.push_back(str);
    }

    void print() {
        for (auto x: errors)
            cout << x << endl;
    }
};

#endif // ERROR_H_INCLUDED
