#pragma once

#include "ast.h"
#include <memory>
using namespace std;

shared_ptr<Expr> simplify(shared_ptr<Expr> expr);
