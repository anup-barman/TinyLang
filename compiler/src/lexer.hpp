#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace tinylang {

enum class TokenType {
  // Keywords
  Func,
  Let,
  Print,
  Println, // Added Println
  For,
  If,
  Else,
  Return,
  // Identifiers & Literals
  Identifier,
  Number,
  Float,
  StringLiteral,
  // Operators
  Plus,
  Minus,
  Star,
  Slash,
  Mod,
  Equals,
  NotEquals,
  Less,
  LessEq,
  Greater,
  GreaterEq,
  Assign,
  Not,
  // Punctuation
  LParen,
  RParen,
  LBrace,
  RBrace,
  LBracket,
  RBracket,
  Comma,
  Semicolon,
  Arrow,
  // Special
  EndOfFile,
  Error
};

struct Token {
  TokenType type;
  std::string text;
  int line;
  int col;
};

class Lexer {
public:
  explicit Lexer(std::string source);
  std::vector<Token> tokenize();

private:
  std::string source;
  size_t pos = 0;
  int line = 1;
  int col = 1;

  char peek(int offset = 0) const;
  char advance();
  bool match(char expected);
  void skipWhitespace();

  Token makeToken(TokenType type, std::string text);
  Token number();
  Token identifier();
};

std::string tokenTypeToString(TokenType type);

} // namespace tinylang
