#include "parser.h"
#include "interpreter.h"
#include "simplify.h"
#include "ast.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

int main(int argc, char* argv[]) {
    if(argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];
    ifstream ifs(inputFile);
    if(!ifs) {
        cerr << "Failed to open input file " << inputFile << endl;
        return 1;
    }
    stringstream buffer;
    buffer << ifs.rdbuf();
    string programText = buffer.str();
    vector<Token> tokens = tokenize(programText);
    Parser parser(tokens);
    Function func = parser.parseFunction();
    vector<State> finalStates = symbolic_execution(func);
    
    ofstream ofs(outputFile);
    if(!ofs) {
        cerr << "Failed to open output file " << outputFile << endl;
        return 1;
    }
    ofs << "{\n";
    for(const auto &st : finalStates) {
        ofs << "\t{\n";
        for(auto &p : st.memory) {
            ofs << "\t\t" << p.first << " = " << simplify(p.second)->toString() << "\n";
        }
        ofs << "\t\tpc = ";
        if(st.pathCondition.empty())
            ofs << "true";
        else {
            for(size_t i = 0; i < st.pathCondition.size(); i++) {
                ofs << simplify(st.pathCondition[i])->toString();
                if(i + 1 < st.pathCondition.size())
                    ofs << " & ";
            }
        }
        ofs << "\n";
        ofs << "\t\tresult = " << (st.result ? simplify(st.result)->toString() : "undefined") << "\n";
        ofs << "\t}\n";
    }
    ofs << "}\n";
    return 0;
}
