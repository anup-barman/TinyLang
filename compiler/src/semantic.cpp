#include "semantic.hpp"
#include <iostream>

namespace tinylang {

void SemanticAnalyzer::analyze(Program &prog) {
  // Global scope
  enterScope();
  // Built-ins?
  // functions["print"] = 1; // Actually print is a statement in our grammar,
  // but let's handle it if it were a function But print is a Stmt, so it won't
  // appear in CallExpr unless we parse it as such, but we parse it as
  // PrintStmt.

  prog.accept(*this);
  exitScope();
}

void SemanticAnalyzer::enterScope() { scopes.push_back({}); }

void SemanticAnalyzer::exitScope() { scopes.pop_back(); }

void SemanticAnalyzer::declare(const std::string &name, Type type) {
  if (scopes.empty())
    return;
  auto &scope = scopes.back();
  if (scope.find(name) != scope.end()) {
    throw SemanticError("Variable '" + name +
                        "' already declared in this scope.");
  }
  scope[name] = {false, type};
}

void SemanticAnalyzer::define(const std::string &name) {
  if (scopes.empty())
    return;
  auto &scope = scopes.back();
  scope[name].isDefined = true;
}

SymbolInfo *SemanticAnalyzer::resolve(const std::string &name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if (it->find(name) != it->end()) {
      return &(*it)[name];
    }
  }
  return nullptr;
}

void SemanticAnalyzer::visit(IntLiteral &node) { lastType = Type::Int; }
void SemanticAnalyzer::visit(FloatLiteral &node) { lastType = Type::Float; }
void SemanticAnalyzer::visit(StringLiteral &node) { lastType = Type::String; }

void SemanticAnalyzer::visit(Variable &node) {
  auto info = resolve(node.name);
  if (!info) {
    throw SemanticError("Undefined variable '" + node.name + "'", node.line,
                        node.col);
  }
  lastType = info->type;
}

void SemanticAnalyzer::visit(BinaryExpr &node) {
  node.left->accept(*this);
  Type leftType = lastType;
  node.right->accept(*this);
  Type rightType = lastType;

  // Type checking and inference
  if (leftType == Type::String && rightType == Type::String && node.op == "+") {
    lastType = Type::String;
    return;
  }
  if ((leftType == Type::Int || leftType == Type::Float) &&
      (rightType == Type::Int || rightType == Type::Float)) {
    if (leftType == Type::Float || rightType == Type::Float)
      lastType = Type::Float;
    else
      lastType = Type::Int;

    // Coercion check? We rely on C++ auto/codegen for runtime, but semantic
    // should be strict? Let's allow mixed.
    return;
  }

  // Comparison operators return Int (boolean)
  if (node.op == "==" || node.op == "!=" || node.op == "<" || node.op == ">" ||
      node.op == "<=" || node.op == ">=") {
    lastType = Type::Int;
    return; // Valid for all types if equal?Strings? Yes.
  }

  throw SemanticError("Type mismatch in binary operation", node.line, node.col);
}

void SemanticAnalyzer::visit(UnaryExpr &node) {
  node.operand->accept(*this);
  // lastType remains same? ! is int. - is same type.
  if (node.op == "!")
    lastType = Type::Int;
}

void SemanticAnalyzer::visit(CallExpr &node) {
  // Built-in functions checks
  if (node.callee == "input") {
    lastType = Type::String;
    return;
  }
  if (node.callee == "len") {
    if (node.args.size() != 1)
      throw SemanticError("len() expects 1 argument", node.line, node.col);
    node.args[0]->accept(*this);
    if (lastType != Type::String)
      throw SemanticError("len() expects string", node.line, node.col);
    lastType = Type::Int;
    return;
  }

  if (node.callee == "int") {
    if (node.args.size() != 1)
      throw SemanticError("int() expects 1 argument", node.line, node.col);
    node.args[0]->accept(*this);
    lastType = Type::Int;
    return;
  }

  if (node.callee == "float") {
    if (node.args.size() != 1)
      throw SemanticError("float() expects 1 argument", node.line, node.col);
    node.args[0]->accept(*this);
    lastType = Type::Float;
    return;
  }

  if (node.callee == "substr") {
    if (node.args.size() != 3)
      throw SemanticError("substr() expects 3 arguments", node.line, node.col);
    for (auto &arg : node.args)
      arg->accept(*this);
    lastType = Type::String;
    return;
  }

  for (auto &arg : node.args) {
    arg->accept(*this);
  }

  if (functions.find(node.callee) == functions.end()) {
    throw SemanticError("Undefined function '" + node.callee + "'", node.line,
                        node.col);
  }
  // We don't track function return types in AST yet (spec didn't add return
  // types to syntax). So we assume generic int? Or void? or 'auto'? For
  // specific requirement "seamless integration", we should assume functions
  // return something. Let's assume Unknown or Int for now to avoid breaking
  // existing `factorial` which returns Int.
  lastType = functions[node.callee].returnType;

  // Default user functions return Int/unknown for now?
  if (lastType == Type::Unknown)
    lastType = Type::Int;
}

void SemanticAnalyzer::visit(VarDecl &node) {
  // Analyze init to find type
  if (node.initializer) {
    node.initializer->accept(*this);
  } else {
    lastType = Type::Int; // Default?
  }

  declare(node.name, lastType);
  define(node.name);
}

void SemanticAnalyzer::visit(TypedVarDecl &node) {
  if (node.initializer) {
    node.initializer->accept(*this);
    // Type check logic...
  }
  // Track type
  Type t = Type::Int;
  if (node.type == "float")
    t = Type::Float;
  else if (node.type == "string")
    t = Type::String;
  // Map Array?
  // We need a better type system in semantic, but for now just map base types.
  // We can't represent Array<Type> easily without extending Type enum or class.
  // Let's assume Type::Int/Float/String covers it for now, codegen handles
  // vector.

  declare(node.name, t);
  define(node.name);
  // Is Defined = true if initialized or not?
  // "Uninitialized reading is undefined behavior".
  // Init flag: if no initializer, it is uninitialized.
  if (!node.initializer && !node.isArray &&
      !node.arraySize) { // ArrayDecl without init might count as uninit or
                         // empty?
    // Let's mark as not fully defined but "declared".
    // The symbol table `bool isDefined` currently tracks if it's usable.
    // Let's use it to mean "Initialized".
    auto &scope = scopes.back();
    scope[node.name].isDefined = false;
  }

  if (node.arraySize) {
    node.arraySize->accept(*this);
    if (lastType != Type::Int)
      throw SemanticError("Array size must be integer.", node.line, node.col);
  }
}

void SemanticAnalyzer::visit(ArrayAccess &node) {
  auto info = resolve(node.name);
  if (!info)
    throw SemanticError("Undefined array '" + node.name + "'", node.line,
                        node.col);

  if (!info->isDefined) {
    // Warning? Or Error? User asked for "Undefined behavior" or warning.
    // We can print warning to stderr?
    std::cerr << "Warning: Possible read of uninitialized variable '"
              << node.name << "'\n";
  }

  node.index->accept(*this);
  if (lastType != Type::Int)
    throw SemanticError("Array index must be integer.", node.line, node.col);

  lastType = info->type;
}

void SemanticAnalyzer::visit(AssignStmt &node) {
  auto info = resolve(node.name);
  if (!info) {
    throw SemanticError("Assignment to undefined variable '" + node.name + "'",
                        node.line, node.col);
  }
  node.value->accept(*this);
  // Check assignable? info->type vs lastType?
  // TinyLang is dynamic or static with inference?
  // "let x = ..." implies inference on declaration.
  // Re-assignment: usually must match type.
  if (info->type != lastType &&
      !(info->type == Type::Float && lastType == Type::Int)) {
    // Allow Int -> Float promotion
    throw SemanticError("Type mismatch in assignment", node.line, node.col);
  }
  // Check index if array assign
  if (node.index) {
    node.index->accept(*this);
    if (lastType != Type::Int)
      throw SemanticError("Array index must be integer.", node.line, node.col);
  }

  node.value->accept(*this);
  // Logic for type matching...

  define(node.name); // Mark initialized
}

void SemanticAnalyzer::visit(PrintStmt &node) { node.expr->accept(*this); }

void SemanticAnalyzer::visit(ExprStmt &node) { node.expr->accept(*this); }

void SemanticAnalyzer::visit(Block &node) {
  enterScope();
  for (auto &stmt : node.statements) {
    stmt->accept(*this);
  }
  exitScope();
}

void SemanticAnalyzer::visit(IfStmt &node) {
  node.condition->accept(*this);
  node.thenBranch->accept(*this);
  if (node.elseBranch) {
    node.elseBranch->accept(*this);
  }
}

void SemanticAnalyzer::visit(ForStmt &node) {
  enterScope(); // For loop creates a scope for init variable
  if (node.init)
    node.init->accept(*this);
  if (node.condition)
    node.condition->accept(*this);
  if (node.update)
    node.update->accept(*this);
  node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzer::visit(FuncDecl &node) {
  // Struct FuncInfo { int argCount; Type returnType; }
  // functions map updated in Program pass

  enterScope();
  for (const auto &param : node.params) {
    Type pType = Type::Int; // Default
    if (param.first == "float")
      pType = Type::Float;
    if (param.first == "string")
      pType = Type::String;

    declare(param.second, pType);
    define(param.second);
  }
  node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzer::visit(ReturnStmt &node) {
  if (node.value) {
    node.value->accept(*this);
  }
}

void SemanticAnalyzer::visit(Program &node) {
  // First pass for function headers?
  // C++ doesn't require it if ordered, but flexible languages do.
  // Let's do a pre-pass or just allow recursive calls if defined before?
  // Spec doesn't strictly say. Recursive descent is top-down.
  // If we want to support forward reference, we scan declarations first.
  // For simplicity, we just visit. If A calls B, B must be in `functions` map.
  // To support mutual recursion, we'd need two passes.
  // Let's do two passes over top-level: 1. Collect functions. 2. Analyze
  // bodies.

  // Pass 1: Collect function signatures
  for (const auto &decl : node.declarations) {
    if (auto func = dynamic_cast<FuncDecl *>(decl.get())) {
      if (functions.find(func->name) != functions.end()) {
        throw SemanticError("Function '" + func->name + "' redefined.",
                            func->line, func->col);
      }
      functions[func->name] = {(int)func->params.size(),
                               Type::Int}; // Assume Int return
    }
  }

  // Pass 2: Analyze bodies (and global stmts)
  for (const auto &decl : node.declarations) {
    decl->accept(*this);
  }
}

} // namespace tinylang
