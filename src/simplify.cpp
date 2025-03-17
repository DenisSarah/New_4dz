#include "simplify.h"
#include "ast.h"
#include <stdexcept>
#include <cstdlib>
#include <memory>
using namespace std;

shared_ptr<Expr> simplify(shared_ptr<Expr> expr) {
    if (!expr)
        return expr;
    if (auto bin = dynamic_pointer_cast<BinOpExpr>(expr)) {
        auto left = simplify(bin->left);
        auto right = simplify(bin->right);
        auto leftConst = dynamic_pointer_cast<ConstExpr>(left);
        auto rightConst = dynamic_pointer_cast<ConstExpr>(right);
        
        if (leftConst && rightConst) {
            if (bin->op == "&" || bin->op == "|") {
                bool a, b;
                if (leftConst->value == "true")
                    a = true;
                else if (leftConst->value == "false")
                    a = false;
                else return make_shared<BinOpExpr>(left, bin->op, right);
                
                if (rightConst->value == "true")
                    b = true;
                else if (rightConst->value == "false")
                    b = false;
                else return make_shared<BinOpExpr>(left, bin->op, right);
                
                bool res = (bin->op == "&") ? (a && b) : (a || b);
                return make_shared<ConstExpr>(res ? "true" : "false");
            }
            try {
                int a = stoi(leftConst->value);
                int b = stoi(rightConst->value);
                int res;
                if (bin->op == "+") res = a + b;
                else if (bin->op == "-") res = a - b;
                else if (bin->op == "*") res = a * b;
                else if (bin->op == "/") res = a / b;
                else if (bin->op == "<") res = (a < b) ? 1 : 0;
                else if (bin->op == ">") res = (a > b) ? 1 : 0;
                else if (bin->op == "<=") res = (a <= b) ? 1 : 0;
                else if (bin->op == ">=") res = (a >= b) ? 1 : 0;
                else return make_shared<BinOpExpr>(left, bin->op, right);
                return make_shared<ConstExpr>(to_string(res));
            } catch (const std::invalid_argument&) {
                return make_shared<BinOpExpr>(left, bin->op, right);
            }
        } else {
            if (bin->op == "|" || bin->op == "&") {
                if (leftConst) {
                    if (bin->op == "|" && leftConst->value == "false")
                        return right;
                    if (bin->op == "|" && leftConst->value == "true")
                        return make_shared<ConstExpr>("true");
                    if (bin->op == "&" && leftConst->value == "false")
                        return make_shared<ConstExpr>("false");
                    if (bin->op == "&" && leftConst->value == "true")
                        return right;
                }
                if (rightConst) {
                    if (bin->op == "|" && rightConst->value == "false")
                        return left;
                    if (bin->op == "|" && rightConst->value == "true")
                        return make_shared<ConstExpr>("true");
                    if (bin->op == "&" && rightConst->value == "false")
                        return make_shared<ConstExpr>("false");
                    if (bin->op == "&" && rightConst->value == "true")
                        return left;
                }
            }
            if (bin->op == "*" || bin->op == "/") {
                if (auto leftBin = dynamic_pointer_cast<BinOpExpr>(left)) {
                    if (leftBin->op == "+" || leftBin->op == "-") {
                        auto distributed = make_shared<BinOpExpr>(
                            make_shared<BinOpExpr>(leftBin->left, bin->op, right),
                            leftBin->op,
                            make_shared<BinOpExpr>(leftBin->right, bin->op, right)
                        );
                        return simplify(distributed);
                    }
                }
                if (auto rightBin = dynamic_pointer_cast<BinOpExpr>(right)) {
                    if (rightBin->op == "+" || rightBin->op == "-") {
                        auto distributed = make_shared<BinOpExpr>(
                            make_shared<BinOpExpr>(left, bin->op, rightBin->left),
                            rightBin->op,
                            make_shared<BinOpExpr>(left, bin->op, rightBin->right)
                        );
                        return simplify(distributed);
                    }
                }
            }
            return make_shared<BinOpExpr>(left, bin->op, right);
        }
    } else if (auto notExpr = dynamic_pointer_cast<NotExpr>(expr)) {
        auto inner = simplify(notExpr->expr);
        if (auto bin = dynamic_pointer_cast<BinOpExpr>(inner)) {
            if (bin->op == ">")
                return simplify(make_shared<BinOpExpr>(bin->left, "<=", bin->right));
            else if (bin->op == "<")
                return simplify(make_shared<BinOpExpr>(bin->left, ">=", bin->right));
            else if (bin->op == ">=")
                return simplify(make_shared<BinOpExpr>(bin->left, "<", bin->right));
            else if (bin->op == "<=")
                return simplify(make_shared<BinOpExpr>(bin->left, ">", bin->right));
        }
        auto innerConst = dynamic_pointer_cast<ConstExpr>(inner);
        if (innerConst) {
            if (innerConst->value == "true")
                return make_shared<ConstExpr>("false");
            else if (innerConst->value == "false")
                return make_shared<ConstExpr>("true");
        }
        return make_shared<NotExpr>(inner);
    } else if (auto negExpr = dynamic_pointer_cast<NegExpr>(expr)) {
        auto inner = simplify(negExpr->expr);
        auto innerConst = dynamic_pointer_cast<ConstExpr>(inner);
        if (innerConst) {
            try {
                int a = stoi(innerConst->value);
                return make_shared<ConstExpr>(to_string(-a));
            } catch (const std::invalid_argument&) {
                return make_shared<NegExpr>(inner);
            }
        }
        return make_shared<NegExpr>(inner);
    }
    return expr;
}
