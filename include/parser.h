#pragma once

#include "ast.h"
#include <vector>
#include <string>
#include <memory>
using namespace std;

struct Statement;

struct Function {
    string name;
    vector<pair<string, string>> parameters;
    string retType;
    vector<shared_ptr<Statement>> statements;
    shared_ptr<Expr> retExpr;
};

struct Parser {
    vector<Token> tokens;
    size_t pos;
    Parser(const vector<Token>& tokens);
    Token currentToken();
    void advance();
    bool accept(TokenType type, const string &val = "");
    void expect(TokenType type, const string &val = "");
    Function parseFunction();
    vector<pair<string, string>> parseParameters();
    vector<shared_ptr<Statement>> parseStatements(bool stopAtReturn = false);
    shared_ptr<Statement> parseStatement();
    shared_ptr<Statement> parseIfStmt();
    shared_ptr<Statement> parseAssignStmt();
    shared_ptr<Expr> parseExpression();
    shared_ptr<Expr> parseLogicalExpr();
    shared_ptr<Expr> parseRelationalExpr();
    shared_ptr<Expr> parseAdditiveExpr();
    shared_ptr<Expr> parseMultiplicativeExpr();
    shared_ptr<Expr> parseUnaryExpr();
    shared_ptr<Expr> parsePrimary();
};

struct Statement {
    virtual ~Statement() {}
};

struct AssignStmt : public Statement {
    string var;
    shared_ptr<Expr> expr;
    AssignStmt(const string &v, shared_ptr<Expr> e);
};

struct IfStmt : public Statement {
    shared_ptr<Expr> cond;
    vector<shared_ptr<Statement>> thenStmts;
    vector<shared_ptr<Statement>> elseStmts;
    IfStmt(shared_ptr<Expr> c, const vector<shared_ptr<Statement>> &t, const vector<shared_ptr<Statement>> &el);
};

struct ReturnStmt : public Statement {
    shared_ptr<Expr> expr;
    ReturnStmt(shared_ptr<Expr> e);
};
