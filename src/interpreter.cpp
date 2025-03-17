#include "interpreter.h"
#include "parser.h"
#include "ast.h"
#include <iostream>
#include <cstdlib>
using namespace std;

shared_ptr<Expr> eval_expr(shared_ptr<Expr> expr, const State &state) {
    if (auto var = dynamic_pointer_cast<VarExpr>(expr)) {
        auto it = state.memory.find(var->name);
        if (it != state.memory.end())
            return it->second;
        return expr;
    } else if (dynamic_pointer_cast<ConstExpr>(expr)) {
        return expr;
    } else if (auto bin = dynamic_pointer_cast<BinOpExpr>(expr)) {
        auto left = eval_expr(bin->left, state);
        auto right = eval_expr(bin->right, state);
        return make_shared<BinOpExpr>(left, bin->op, right);
    } else if (auto notE = dynamic_pointer_cast<NotExpr>(expr)) {
        auto inner = eval_expr(notE->expr, state);
        return make_shared<NotExpr>(inner);
    } else if (auto negE = dynamic_pointer_cast<NegExpr>(expr)) {
        auto inner = eval_expr(negE->expr, state);
        return make_shared<NegExpr>(inner);
    }
    return expr;
}

vector<State> executeStatement(shared_ptr<Statement> stmt, const State &state) {
    vector<State> states;
    if (auto assign = dynamic_pointer_cast<AssignStmt>(stmt)) {
        State newState = state;
        newState.memory[assign->var] = eval_expr(assign->expr, state);
        states.push_back(newState);
    } else if (auto ifStmt = dynamic_pointer_cast<IfStmt>(stmt)) {
        State thenState = state;
        thenState.pathCondition.push_back(eval_expr(ifStmt->cond, state));
        vector<State> thenStates = executeBlock(ifStmt->thenStmts, thenState);
        State elseState = state;
        elseState.pathCondition.push_back(make_shared<NotExpr>(eval_expr(ifStmt->cond, state)));
        vector<State> elseStates = executeBlock(ifStmt->elseStmts, elseState);
        states.insert(states.end(), thenStates.begin(), thenStates.end());
        states.insert(states.end(), elseStates.begin(), elseStates.end());
    } else if (auto retStmt = dynamic_pointer_cast<ReturnStmt>(stmt)) {
        State newState = state;
        newState.result = eval_expr(retStmt->expr, state);
        states.push_back(newState);
    }
    return states;
}

vector<State> executeBlock(const vector<shared_ptr<Statement>> &stmts, const State &initialState) {
    vector<State> states;
    states.push_back(initialState);
    for (auto stmt : stmts) {
        vector<State> newStates;
        for (auto st : states) {
            vector<State> stmtStates = executeStatement(stmt, st);
            newStates.insert(newStates.end(), stmtStates.begin(), stmtStates.end());
        }
        states = newStates;
    }
    return states;
}

vector<State> symbolic_execution(const Function &func) {
    State initState;
    for (auto &param : func.parameters) {
        initState.memory[param.second] = make_shared<VarExpr>(param.second);
    }
    vector<State> states = executeBlock(func.statements, initState);
    for (auto &st : states) {
        st.result = eval_expr(func.retExpr, st);
    }
    return states;
}
