#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace tinylang {

Lexer::Lexer(std::string source) : source(std::move(source)) {}

char Lexer::peek(int offset) const {
  if (pos + offset >= source.length())
    return '\0';
  return source[pos + offset];
}

char Lexer::advance() {
  char c = source[pos++];
  if (c == '\n') {
    line++;
    col = 1;
  } else {
    col++;
  }
  return c;
}

bool Lexer::match(char expected) {
  if (peek() == expected) {
    advance();
    return true;
  }
  return false;
}

void Lexer::skipWhitespace() {
  while (true) {
    char c = peek();
    if (isspace(c)) {
      advance();
    } else {
      break;
    }
  }
}

Token Lexer::makeToken(TokenType type, std::string text) {
  // Correct tracking: token position is start, but we advanced.
  // Simplified: we will just capture current line/col at start of token
  // For now, let's fixup slightly in tokenize loop
  return {type, text, line, col};
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (pos < source.length()) {
    skipWhitespace();
    if (pos >= source.length())
      break;

    int startLine = line;
    int startCol = col;

    char c = advance();

    // Lambda to easily push token with correct start pos
    auto push = [&](TokenType type, std::string text) {
      tokens.push_back({type, text, startLine, startCol});
    };

    if (isdigit(c)) {
      std::string numStr;
      numStr += c;
      bool isFloat = false;
      while (isdigit(peek()))
        numStr += advance();
      if (peek() == '.') {
        isFloat = true;
        numStr += advance(); // consume dot
        while (isdigit(peek()))
          numStr += advance();
      }
      if (isFloat)
        push(TokenType::Float, numStr);
      else
        push(TokenType::Number, numStr);
      continue;
    }

    if (c == '"') {
      // String literal
      std::string val;
      while (peek() != '"' && peek() != '\0') {
        val += advance();
      }
      if (peek() == '"')
        advance(); // closing quote
      push(TokenType::StringLiteral, val);
      continue;
    }

    if (isalpha(c) || c == '_') {
      std::string idStr;
      idStr += c;
      while (isalnum(peek()) || peek() == '_')
        idStr += advance();

      static const std::unordered_map<std::string, TokenType> keywords = {
          {"func", TokenType::Func},   {"let", TokenType::Let},
          {"print", TokenType::Print}, {"println", TokenType::Println},
          {"for", TokenType::For},     {"if", TokenType::If},
          {"else", TokenType::Else},   {"return", TokenType::Return}};

      if (keywords.count(idStr)) {
        push(keywords.at(idStr), idStr);
      } else {
        push(TokenType::Identifier, idStr);
      }
      continue;
    }

    switch (c) {
    case '(':
      push(TokenType::LParen, "(");
      break;
    case ')':
      push(TokenType::RParen, ")");
      break;
    case '{':
      push(TokenType::LBrace, "{");
      break;
    case '}':
      push(TokenType::RBrace, "}");
      break;
    case '[':
      push(TokenType::LBracket, "[");
      break;
    case ']':
      push(TokenType::RBracket, "]");
      break;
    case ';':
      push(TokenType::Semicolon, ";");
      break;
    case ',':
      push(TokenType::Comma, ",");
      break;
    case '+':
      push(TokenType::Plus, "+");
      break;
    case '-':
      if (match('>'))
        push(TokenType::Arrow, "->");
      else
        push(TokenType::Minus, "-");
      break;
    case '*':
      push(TokenType::Star, "*");
      break;
    case '/':
      if (match('/')) {
        // Comment: skip until end of line
        while (peek() != '\n' && peek() != '\0')
          advance();
      } else {
        push(TokenType::Slash, "/");
      }
      break;
    case '%':
      push(TokenType::Mod, "%");
      break;
    case '=':
      if (match('='))
        push(TokenType::Equals, "==");
      else
        push(TokenType::Assign, "=");
      break;
    case '!':
      if (match('='))
        push(TokenType::NotEquals, "!=");
      else
        push(TokenType::Not, "!");
      break;
    case '<':
      if (match('='))
        push(TokenType::LessEq, "<=");
      else
        push(TokenType::Less, "<");
      break;
    case '>':
      if (match('='))
        push(TokenType::GreaterEq, ">=");
      else
        push(TokenType::Greater, ">");
      break;
    default:
      // Unknown character
      push(TokenType::Error, std::string(1, c));
      break;
    }
  }

  tokens.push_back({TokenType::EndOfFile, "", line, col});
  return tokens;
}

std::string tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::Func:
    return "func";
  case TokenType::Let:
    return "let";
  // ... include others if needed for debugging
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::Number:
    return "Number";
  case TokenType::EndOfFile:
    return "EOF";
  default:
    return "Token";
  }
}

} // namespace tinylang
