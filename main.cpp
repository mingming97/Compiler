#include "headers.h"
#include "FileHelper.h"
#include "Lexer.h"
#include "table.h"
#include "Parser.h"
#include "CodeHelper.h"
#include "Optimizer.h"

using namespace std;

string my_to_string(int val) {
    stringstream ss;
    ss << val;
    return ss.str();
}

void split(const string& s, vector<string>& v, const string& c) {
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2-pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    //if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}

void print_var_addr_map(unordered_map<string, var_info> &var_addr_map) {
    unordered_map<string, var_info>::iterator iter;
    for (iter = var_addr_map.begin(); iter != var_addr_map.end(); iter++) {
        cout << iter->first << endl;
        cout << (iter->second).base_reg << "  " << (iter->second).offset << endl;
    }
}

void print_size_map(unordered_map<string, size_info> &size_map) {
    unordered_map<string, size_info>::iterator iter;
    for (iter = size_map.begin(); iter != size_map.end(); iter++) {
        cout << iter->first << endl;
        size_info info = iter->second;
        cout << "tmp size: " << info.tmp_size << "    ";
        cout << "param size: " << info.param_size << "    ";
        cout << "var size: " << info.var_size << endl;
    }
}

int main()
{

    Error error;
    Table table(error);

    string path;
    cout << "Input source file path:" << endl;
    getline(cin, path);

    Lexer lexer(path, error);

    vector<inter_code> inter_codes;
    Parser parser(lexer, table, inter_codes, error);
    CodeHelper code_helper(table, inter_codes);

    parser.parse_init();
    if (error.has_error())
        error.print();
    else {

        code_helper.gen_file_inter_codes(false);
        code_helper.basic_optimize(3);

        FileHelper::put_lines("./inter_codes.txt", code_helper.file_inter_codes);
        cout << "Generate inter_codes.txt over." << endl;

        code_helper.generate_mips(false, false, false);
        FileHelper::put_lines("./mips_codes.asm", code_helper.mips_codes);
        cout << "Generate mips_codes.asm over." << endl;

        Optimizer optimizer(table, inter_codes, parser.tmp_num);
        optimizer.DAG_optimize(2);
        optimizer.analyse_flow();
        optimizer.allocate_global_reg();
        code_helper.set_global_regs(optimizer.var_regs, optimizer.reg_nums);

        code_helper.gen_file_inter_codes(false);
        FileHelper::put_lines("./inter_codes_op.txt", code_helper.file_inter_codes);
        cout << "Generate inter_codes_op.txt over." << endl;

        code_helper.generate_mips(false, true, true);
        FileHelper::put_lines("./mips_codes_op.asm", code_helper.mips_codes);
        cout << "Generate mips_codes_op.asm over." << endl;
    }
    return 0;
}
