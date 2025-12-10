#pragma once

#include "ast.hpp"
#include <map>
#include <string>
#include <vector>

namespace tinylang {

class SemanticError : public std::runtime_error {
public:
  int line;
  int col;
  SemanticError(const std::string &msg, int l = 0, int c = 0)
      : std::runtime_error(msg), line(l), col(c) {}
};

enum class Type { Int, Float, String, Void, Unknown };

struct SymbolInfo {
  bool isDefined;
  Type type;
};

class SemanticAnalyzer : public ASTVisitor {
public:
  void analyze(Program &prog);

  void visit(IntLiteral &node) override;
  void visit(FloatLiteral &node) override;
  void visit(StringLiteral &node) override;
  void visit(ArrayAccess &node) override;
  void visit(TypedVarDecl &node) override;
  void visit(Variable &node) override;
  // ... others same ...
  void visit(BinaryExpr &node) override;
  void visit(UnaryExpr &node) override;
  void visit(CallExpr &node) override;
  void visit(VarDecl &node) override;
  void visit(AssignStmt &node) override;
  void visit(PrintStmt &node) override;
  void visit(ExprStmt &node) override;
  void visit(Block &node) override;
  void visit(IfStmt &node) override;
  void visit(ForStmt &node) override;
  void visit(FuncDecl &node) override;
  void visit(ReturnStmt &node) override;
  void visit(Program &node) override;

private:
  std::vector<std::map<std::string, SymbolInfo>> scopes;
  struct FuncInfo {
    int argCount;
    Type returnType;
  };
  std::map<std::string, FuncInfo> functions;

  // Helper to store last expression type for type checking
  Type lastType = Type::Unknown;

  void enterScope();
  void exitScope();
  void declare(const std::string &name, Type type);
  void define(const std::string &name);
  SymbolInfo *resolve(const std::string &name);
};

} // namespace tinylang
