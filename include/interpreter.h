#pragma once

#include "ast.h"
#include "parser.h"
#include <map>
#include <vector>
using namespace std;

struct State {
    map<string, shared_ptr<Expr>> memory;
    vector<shared_ptr<Expr>> pathCondition;
    shared_ptr<Expr> result;
};

vector<State> symbolic_execution(const Function &func);
vector<State> executeBlock(const vector<shared_ptr<Statement>> &stmts, const State &initialState);
shared_ptr<Expr> eval_expr(shared_ptr<Expr> expr, const State &state);

