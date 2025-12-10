#include "ast.hpp"

namespace tinylang {

void Variable::accept(ASTVisitor &v) { v.visit(*this); }
void BinaryExpr::accept(ASTVisitor &v) { v.visit(*this); }
void CallExpr::accept(ASTVisitor &v) { v.visit(*this); }
void VarDecl::accept(ASTVisitor &v) { v.visit(*this); }
void AssignStmt::accept(ASTVisitor &v) { v.visit(*this); }
void PrintStmt::accept(ASTVisitor &v) { v.visit(*this); }
void Block::accept(ASTVisitor &v) { v.visit(*this); }
void IfStmt::accept(ASTVisitor &v) { v.visit(*this); }
void ForStmt::accept(ASTVisitor &v) { v.visit(*this); }
void FuncDecl::accept(ASTVisitor &v) { v.visit(*this); }
void ReturnStmt::accept(ASTVisitor &v) { v.visit(*this); }
void Program::accept(ASTVisitor &v) { v.visit(*this); }
void UnaryExpr::accept(ASTVisitor &v) { v.visit(*this); }
void ExprStmt::accept(ASTVisitor &v) { v.visit(*this); }
void ArrayAccess::accept(ASTVisitor &v) { v.visit(*this); }
void TypedVarDecl::accept(ASTVisitor &v) { v.visit(*this); }

void IntLiteral::accept(ASTVisitor &v) { v.visit(*this); }
void FloatLiteral::accept(ASTVisitor &v) { v.visit(*this); }
void StringLiteral::accept(ASTVisitor &v) { v.visit(*this); }

} // namespace tinylang
