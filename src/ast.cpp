#include "ast.h"
#include <cctype>

vector<Token> tokenize(const string &input) {
    vector<Token> tokens;
    size_t i = 0;
    while (i < input.size()) {
        if (isspace(input[i])) {
            i++;
            continue;
        }

        if (isdigit(input[i])) {
            size_t start = i;
            while (i < input.size() && isdigit(input[i]))
                i++;
            tokens.push_back({NUMBER, input.substr(start, i - start)});
            continue;
        }

        if (isalpha(input[i])) {
            size_t start = i;
            while (i < input.size() && (isalnum(input[i]) || input[i] == '_'))
                i++;
            string word = input.substr(start, i - start);

            if (isKeyword(word))
                tokens.push_back({KEYWORD, word});
            else
                tokens.push_back({IDENTIFIER, word});
            continue;
        }

        string sym(1, input[i]);
        tokens.push_back({SYMBOL, sym});
        i++;
    }
    tokens.push_back({END, ""});
    return tokens;
}


VarExpr::VarExpr(const string &n) : name(n) {}
string VarExpr::toString(int) const {
    return "'" + name + "'";
}

ConstExpr::ConstExpr(const string &val) : value(val) {}
string ConstExpr::toString(int) const {
    return value;
}

BinOpExpr::BinOpExpr(shared_ptr<Expr> l, const string &o, shared_ptr<Expr> r)
    : left(l), op(o), right(r) {}
int BinOpExpr::precedence() const {
    if(op == "*" || op == "/") return 3;
    if(op == "+" || op == "-") return 2;
    if(op == "<" || op == ">" || op == "<=" || op == ">=") return 1;
    if(op == "&" || op == "|") return 0;
    return -1;
}

string BinOpExpr::toString(int parentPrec) const {
    int prec = precedence();
    string leftStr = left->toString(prec);
    string rightStr = right->toString(prec+1);
    string s = leftStr + " " + op + " " + rightStr;
    if(prec < parentPrec) return "(" + s + ")";
    return s;
}

NotExpr::NotExpr(shared_ptr<Expr> e) : expr(e) {}
int NotExpr::precedence() const { return 4; }
string NotExpr::toString(int parentPrec) const {
    string s = "!" + expr->toString(precedence());
    if(precedence() < parentPrec) return "(" + s + ")";
    return s;
}

NegExpr::NegExpr(shared_ptr<Expr> e) : expr(e) {}
int NegExpr::precedence() const { return 4; }
string NegExpr::toString(int parentPrec) const {
    string s = "-" + expr->toString(precedence());
    if(precedence() < parentPrec) return "(" + s + ")";
    return s;
}
