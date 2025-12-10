#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace tinylang {

// Forward declarations
struct ASTVisitor;

struct Node {
  virtual ~Node() = default;
  virtual void accept(ASTVisitor &visitor) = 0;
  int line = 0;
  int col = 0;
};

struct Expr : Node {};

struct Stmt : Node {};

struct IntLiteral : Expr {
  int value;
  IntLiteral(int v) : value(v) {}
  void accept(ASTVisitor &v) override;
};
// Alias Number to IntLiteral for backward compat if needed or just replace
// usage
using Number = IntLiteral;

struct FloatLiteral : Expr {
  double value;
  FloatLiteral(double v) : value(v) {}
  void accept(ASTVisitor &v) override;
};

struct StringLiteral : Expr {
  std::string value;
  StringLiteral(std::string v) : value(std::move(v)) {}
  void accept(ASTVisitor &v) override;
};

struct Variable : Expr {
  std::string name;
  Variable(std::string n) : name(std::move(n)) {}
  void accept(ASTVisitor &v) override;
};

struct BinaryExpr : Expr {
  std::string op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  BinaryExpr(std::string o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
      : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
  void accept(ASTVisitor &v) override;
};

struct CallExpr : Expr {
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;
  CallExpr(std::string c, std::vector<std::unique_ptr<Expr>> a)
      : callee(std::move(c)), args(std::move(a)) {}
  void accept(ASTVisitor &v) override;
};

struct VarDecl : Stmt {
  std::string name;
  std::unique_ptr<Expr> initializer;
  VarDecl(std::string n, std::unique_ptr<Expr> i)
      : name(std::move(n)), initializer(std::move(i)) {}
  void accept(ASTVisitor &v) override;
};

struct PrintStmt : Stmt {
  std::unique_ptr<Expr> expr;
  bool newLine;
  PrintStmt(std::unique_ptr<Expr> e, bool nl = true)
      : expr(std::move(e)), newLine(nl) {}
  void accept(ASTVisitor &v) override;
};

struct Block : Stmt {
  std::vector<std::unique_ptr<Stmt>> statements;
  void accept(ASTVisitor &v) override;
};

struct ForStmt : Stmt {
  std::unique_ptr<Stmt> init; // usually AssignStmt or nullptr if we allowed
                              // null (but grammar says assignment)
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> update;
  std::unique_ptr<Block> body;
  ForStmt(std::unique_ptr<Stmt> i, std::unique_ptr<Expr> c,
          std::unique_ptr<Stmt> u, std::unique_ptr<Block> b)
      : init(std::move(i)), condition(std::move(c)), update(std::move(u)),
        body(std::move(b)) {}
  void accept(ASTVisitor &v) override;
};

struct FuncDecl : Node {
  std::string name;
  // stored as {type, name}. if type is empty, it's inferred (auto)
  std::vector<std::pair<std::string, std::string>> params;
  std::string returnType; // e.g. "int", "void", or empty for auto
  std::unique_ptr<Block> body;

  FuncDecl(std::string n, std::vector<std::pair<std::string, std::string>> p,
           std::string rt, std::unique_ptr<Block> b)
      : name(std::move(n)), params(std::move(p)), returnType(std::move(rt)),
        body(std::move(b)) {}
  void accept(ASTVisitor &v) override;
};

// Replaces AssignStmt (sort of, or updates it)
// But we have `a[i] = x;`. AssignStmt currently has `std::string name`.
// We should update AssignStmt to support array index, or use a general LHS?
// Existing: struct AssignStmt : Stmt { std::string name; std::unique_ptr<Expr>
// value; ... } Let's modify AssignStmt to handle optional index.
struct AssignStmt : Stmt {
  std::string name;
  std::unique_ptr<Expr> index; // Optional, for array assignment
  std::unique_ptr<Expr> value;
  AssignStmt(std::string n, std::unique_ptr<Expr> v,
             std::unique_ptr<Expr> idx = nullptr)
      : name(n), index(std::move(idx)), value(std::move(v)) {}
  void accept(ASTVisitor &v) override;
};

struct IfStmt : Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> thenBranch;
  std::unique_ptr<Stmt> elseBranch; // Optional
  IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t,
         std::unique_ptr<Stmt> e = nullptr)
      : condition(std::move(c)), thenBranch(std::move(t)),
        elseBranch(std::move(e)) {}
  void accept(ASTVisitor &v) override;
};

struct ArrayAccess : Expr {
  std::string name;
  std::unique_ptr<Expr> index;
  ArrayAccess(std::string n, std::unique_ptr<Expr> idx)
      : name(n), index(std::move(idx)) {}
  void accept(ASTVisitor &v) override;
};

struct TypedVarDecl : Stmt {
  std::string name;
  std::string type; // "int", "float", "string"
  bool isArray;
  std::unique_ptr<Expr> arraySize;   // Optional, for arrays
  std::unique_ptr<Expr> initializer; // Optional
  TypedVarDecl(std::string n, std::string t, bool isArr,
               std::unique_ptr<Expr> size, std::unique_ptr<Expr> init)
      : name(n), type(t), isArray(isArr), arraySize(std::move(size)),
        initializer(std::move(init)) {}
  void accept(ASTVisitor &v) override;
};

struct ReturnStmt : Stmt {
  std::unique_ptr<Expr> value;
  ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
  void accept(ASTVisitor &v) override;
};

struct Program : Node {
  std::vector<std::unique_ptr<Node>> declarations; // Funcs or Stmts
  void accept(ASTVisitor &v) override;
};

struct UnaryExpr : Expr {
  std::string op;
  std::unique_ptr<Expr> operand;
  UnaryExpr(std::string o, std::unique_ptr<Expr> expr)
      : op(std::move(o)), operand(std::move(expr)) {}
  void accept(ASTVisitor &v) override;
};

struct ExprStmt : Stmt {
  std::unique_ptr<Expr> expr;
  ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
  void accept(ASTVisitor &v) override;
};

struct ASTVisitor {
  virtual void visit(IntLiteral &node) = 0;
  virtual void visit(FloatLiteral &node) = 0;
  virtual void visit(StringLiteral &node) = 0;
  virtual void visit(Variable &node) = 0;
  virtual void visit(BinaryExpr &node) = 0;
  virtual void visit(UnaryExpr &node) = 0;
  virtual void visit(CallExpr &node) = 0;
  virtual void visit(VarDecl &node) = 0;
  virtual void visit(AssignStmt &node) = 0;
  virtual void visit(PrintStmt &node) = 0;
  virtual void visit(ExprStmt &node) = 0;
  virtual void visit(Block &node) = 0;
  virtual void visit(IfStmt &node) = 0;
  virtual void visit(ForStmt &node) = 0;
  virtual void visit(FuncDecl &node) = 0;
  virtual void visit(ReturnStmt &node) = 0;
  virtual void visit(Program &node) = 0;
  virtual void visit(ArrayAccess &node) = 0;
  virtual void visit(TypedVarDecl &node) = 0;
};

} // namespace tinylang
