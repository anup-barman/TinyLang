#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include <memory>
#include <stdexcept>
#include <vector>

namespace tinylang {

class ParseError : public std::runtime_error {
public:
  int line;
  int col;
  ParseError(const std::string &msg, int l, int c)
      : std::runtime_error(msg), line(l), col(c) {}
};

class Parser {
public:
  explicit Parser(std::vector<Token> tokens);
  std::unique_ptr<Program> parse();

private:
  std::vector<Token> tokens;
  size_t current = 0;

  Token peek() const;
  Token previous() const;
  Token advance();
  bool check(TokenType type) const;
  bool match(TokenType type);
  Token consume(TokenType type, const std::string &message);

  std::unique_ptr<FuncDecl> functionDecl();
  std::unique_ptr<Stmt> statement();
  std::unique_ptr<Stmt> varDecl();
  std::unique_ptr<Stmt> typedVarDecl();
  std::unique_ptr<Stmt> ifStmt();
  std::unique_ptr<Stmt> forStmt();
  std::unique_ptr<Stmt> printStmt(bool newLine = false);
  std::unique_ptr<Stmt> returnStmt();
  std::unique_ptr<Block> block();
  std::unique_ptr<Stmt> expressionStmt();

  // Type parsing
  struct ParsedType {
    std::string name;
    bool isArray;
    std::unique_ptr<Expr> size;
  };
  ParsedType parseType();

  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> primary();
};

} // namespace tinylang
