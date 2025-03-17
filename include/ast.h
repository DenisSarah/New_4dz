#pragma once

#include <string>
#include <memory>
#include <vector>
using namespace std;

enum TokenType { IDENTIFIER, NUMBER, SYMBOL, KEYWORD, END };

struct Token {
    TokenType type;
    string value;
};

vector<Token> tokenize(const string &input);

struct Expr {
    virtual string toString(int parentPrec) const = 0;
    virtual string toString() const { return toString(-1); }
    virtual ~Expr() {}
};

struct VarExpr : public Expr {
    string name;
    VarExpr(const string &n);
    string toString(int parentPrec) const override;
};

struct ConstExpr : public Expr {
    string value;
    ConstExpr(const string &val);
    string toString(int parentPrec) const override;
};

struct BinOpExpr : public Expr {
    shared_ptr<Expr> left, right;
    string op;
    BinOpExpr(shared_ptr<Expr> l, const string &o, shared_ptr<Expr> r);
    int precedence() const;
    string toString(int parentPrec) const override;
};

struct NotExpr : public Expr {
    shared_ptr<Expr> expr;
    NotExpr(shared_ptr<Expr> e);
    int precedence() const;
    string toString(int parentPrec) const override;
};

struct NegExpr : public Expr {
    shared_ptr<Expr> expr;
    NegExpr(shared_ptr<Expr> e);
    int precedence() const;
    string toString(int parentPrec) const override;
};

inline bool isKeyword(const string &s) {
    return s == "if" || s == "else" || s == "return" ||
           s == "int" || s == "bool" || s == "true" || s == "false";
}
