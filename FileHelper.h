#ifndef FILEREADER_H_INCLUDED
#define FILEREADER_H_INCLUDED
#include "headers.h"
using namespace std;


class FileHelper {

public:
    static vector<string> get_lines(const string &filepath) {
        fstream fs(filepath.c_str());
        vector<string> lines;
        if (fs.is_open()) {
            string strLine;
            while (getline(fs, strLine))
                lines.push_back(strLine + "\n");
        }
        else {
            cerr << "source file doesn't exists" << endl;
            exit(0);
        }
        return lines;
    }

    static void put_lines(const string &filepath, const vector<string> &vec) {
        ofstream os(filepath.c_str());
        if (os.is_open()) {
            for (auto x: vec)
                os << x << endl;
        }
        else {
            cerr << "write to file error" << endl;
            exit(0);
        }
    }

};

#endif // FILEREADER_H_INCLUDED
