#pragma once

#include "ast.hpp"

namespace tinylang {

class Optimizer : public ASTVisitor {
public:
  void optimize(Program &prog);

  void visit(IntLiteral &node) override;
  void visit(FloatLiteral &node) override;
  void visit(StringLiteral &node) override;
  void visit(ArrayAccess &node) override;
  void visit(TypedVarDecl &node) override;
  void visit(Variable &node) override;
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
};

} // namespace tinylang
