#include "parser.h"
#include <iostream>
#include <cstdlib>

Parser::Parser(const vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token Parser::currentToken() {
    if(pos < tokens.size())
        return tokens[pos];
    return {END, ""};
}
void Parser::advance() { if(pos < tokens.size()) pos++; }
bool Parser::accept(TokenType type, const string &val) {
    if(currentToken().type == type && (val=="" || currentToken().value==val)) {
        advance();
        return true;
    }
    return false;
}
void Parser::expect(TokenType type, const string &val) {
    if(!accept(type,val)) {
        std::cerr << "Expected token: " << val << " at position " << pos << std::endl;
        exit(1);
    }
}

Function Parser::parseFunction() {
    Function func;
    Token tk = currentToken();
    if(tk.type != IDENTIFIER) { std::cerr << "Expected function name" << std::endl; exit(1); }
    func.name = tk.value;
    advance();
    expect(SYMBOL, "(");
    func.parameters = parseParameters();
    expect(SYMBOL, ")");
    expect(SYMBOL, ":");
    tk = currentToken();
    if(tk.type != KEYWORD) { std::cerr << "Expected return type" << std::endl; exit(1); }
    func.retType = tk.value;
    advance();
    expect(SYMBOL, "{");
    func.statements = parseStatements(true);
    expect(KEYWORD, "return");
    func.retExpr = parseExpression();
    expect(SYMBOL, "}");
    return func;
}

vector<pair<string,string>> Parser::parseParameters() {
    vector<pair<string,string>> params;
    if(currentToken().type==KEYWORD && (currentToken().value=="int" || currentToken().value=="bool")) {
        while(true) {
            string type = currentToken().value;
            advance();
            string var = currentToken().value;
            expect(IDENTIFIER);
            params.push_back({type, var});
            if(!accept(SYMBOL, ",")) break;
        }
    }
    return params;
}

vector<shared_ptr<Statement>> Parser::parseStatements(bool stopAtReturn) {
    vector<shared_ptr<Statement>> stmts;
    while(true) {
        Token tk = currentToken();
        if(tk.type==END) break;
        if(stopAtReturn && tk.type==KEYWORD && tk.value=="return") break;
        if(tk.type==SYMBOL && (tk.value=="}" || tk.value=="else")) break;
        stmts.push_back(parseStatement());
    }
    return stmts;
}

shared_ptr<Statement> Parser::parseStatement() {
    Token tk = currentToken();
    if(tk.type==KEYWORD && tk.value=="if")
        return parseIfStmt();
    else
        return parseAssignStmt();
}

shared_ptr<Statement> Parser::parseIfStmt() {
    expect(KEYWORD, "if");
    expect(SYMBOL, "(");
    shared_ptr<Expr> cond = parseExpression();
    expect(SYMBOL, ")");
    expect(SYMBOL, "{");
    vector<shared_ptr<Statement>> thenStmts = parseStatements();
    expect(SYMBOL, "}");
    expect(KEYWORD, "else");
    expect(SYMBOL, "{");
    vector<shared_ptr<Statement>> elseStmts = parseStatements();
    expect(SYMBOL, "}");
    return make_shared<IfStmt>(cond, thenStmts, elseStmts);
}

shared_ptr<Statement> Parser::parseAssignStmt() {
    string var = currentToken().value;
    expect(IDENTIFIER);
    expect(SYMBOL, "=");
    shared_ptr<Expr> expr = parseExpression();
    return make_shared<AssignStmt>(var, expr);
}

shared_ptr<Expr> Parser::parseExpression() {
    return parseLogicalExpr();
}

shared_ptr<Expr> Parser::parseLogicalExpr() {
    shared_ptr<Expr> left = parseRelationalExpr();
    while(true) {
        Token tk = currentToken();
        if(tk.type==SYMBOL && (tk.value=="&" || tk.value=="|")) {
            string op = tk.value;
            advance();
            shared_ptr<Expr> right = parseRelationalExpr();
            left = make_shared<BinOpExpr>(left, op, right);
        } else break;
    }
    return left;
}

shared_ptr<Expr> Parser::parseRelationalExpr() {
    shared_ptr<Expr> left = parseAdditiveExpr();
    while(true) {
        Token tk = currentToken();
        if(tk.type==SYMBOL && (tk.value=="<" || tk.value==">")) {
            string op = tk.value;
            advance();
            shared_ptr<Expr> right = parseAdditiveExpr();
            left = make_shared<BinOpExpr>(left, op, right);
        } else break;
    }
    return left;
}

shared_ptr<Expr> Parser::parseAdditiveExpr() {
    shared_ptr<Expr> left = parseMultiplicativeExpr();
    while(true) {
        Token tk = currentToken();
        if(tk.type==SYMBOL && (tk.value=="+" || tk.value=="-")) {
            string op = tk.value;
            advance();
            shared_ptr<Expr> right = parseMultiplicativeExpr();
            left = make_shared<BinOpExpr>(left, op, right);
        } else break;
    }
    return left;
}

shared_ptr<Expr> Parser::parseMultiplicativeExpr() {
    shared_ptr<Expr> left = parseUnaryExpr();
    while(true) {
        Token tk = currentToken();
        if(tk.type==SYMBOL && (tk.value=="*" || tk.value=="/")) {
            string op = tk.value;
            advance();
            shared_ptr<Expr> right = parseUnaryExpr();
            left = make_shared<BinOpExpr>(left, op, right);
        } else break;
    }
    return left;
}

shared_ptr<Expr> Parser::parseUnaryExpr() {
    Token tk = currentToken();
    if(tk.type==SYMBOL && tk.value=="!") {
        advance();
        shared_ptr<Expr> inner = parseUnaryExpr();
        return make_shared<NotExpr>(inner);
    } else if(tk.type==SYMBOL && tk.value=="-") {
        advance();
        shared_ptr<Expr> inner = parseUnaryExpr();
        return make_shared<NegExpr>(inner);
    } else {
        return parsePrimary();
    }
}

shared_ptr<Expr> Parser::parsePrimary() {
    Token tk = currentToken();
    if(tk.type==NUMBER) {
        advance();
        return make_shared<ConstExpr>(tk.value);
    } else if(tk.type==KEYWORD && (tk.value=="true" || tk.value=="false")) {
        advance();
        return make_shared<ConstExpr>(tk.value);
    } else if(tk.type==IDENTIFIER) {
        string name = tk.value;
        advance();
        return make_shared<VarExpr>(name);
    } else if(tk.type==SYMBOL && tk.value=="(") {
        advance();
        shared_ptr<Expr> expr = parseExpression();
        expect(SYMBOL, ")");
        return expr;
    } else {
        std::cerr << "Unexpected token: " << tk.value << " at position " << pos << std::endl;
        exit(1);
    }
}


// Реализация конструктора для AssignStmt
AssignStmt::AssignStmt(const string &v, shared_ptr<Expr> e)
    : var(v), expr(e) { }

// Реализация конструктора для IfStmt
IfStmt::IfStmt(shared_ptr<Expr> c, const vector<shared_ptr<Statement>> &t, const vector<shared_ptr<Statement>> &el)
    : cond(c), thenStmts(t), elseStmts(el) { }

// Реализация конструктора для ReturnStmt
ReturnStmt::ReturnStmt(shared_ptr<Expr> e)
    : expr(e) { }

