#include "parser.hpp"

namespace tinylang {

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

Token Parser::peek() const { return tokens[current]; }

Token Parser::previous() const { return tokens[current - 1]; }

Token Parser::advance() {
  if (peek().type != TokenType::EndOfFile)
    current++;
  return previous();
}

bool Parser::check(TokenType type) const {
  if (peek().type == TokenType::EndOfFile)
    return false;
  return peek().type == type;
}

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

Token Parser::consume(TokenType type, const std::string &message) {
  if (check(type))
    return advance();
  throw ParseError(message, peek().line, peek().col);
}

std::unique_ptr<Program> Parser::parse() {
  auto program = std::make_unique<Program>();
  while (peek().type != TokenType::EndOfFile) {
    if (check(TokenType::Func)) {
      program->declarations.push_back(functionDecl());
    } else {
      // Global statements (optional in spec but good to have)
      try {
        program->declarations.push_back(statement());
      } catch (const ParseError &e) {
        // Synchronize? For now just rethrow
        throw;
      }
    }
  }
  return program;
}

std::unique_ptr<FuncDecl> Parser::functionDecl() {
  consume(TokenType::Func, "Expect 'func'");
  Token name = consume(TokenType::Identifier, "Expect function name");
  consume(TokenType::LParen, "Expect '(' after function name");

  std::vector<std::pair<std::string, std::string>> params;
  if (!check(TokenType::RParen)) {
    do {
      std::string type = "";
      if (check(TokenType::Identifier)) {
        std::string txt = peek().text;
        if (txt == "int" || txt == "float" || txt == "string" ||
            txt == "void") {
          type = txt;
          advance();
        }
      }
      Token param = consume(TokenType::Identifier, "Expect parameter name");
      params.push_back({type, param.text});
    } while (match(TokenType::Comma));
  }
  consume(TokenType::RParen, "Expect ')' after paremeters");

  std::string returnType = "";
  if (match(TokenType::Arrow)) {
    if (check(TokenType::Identifier)) {
      returnType = advance().text;
    }
  }

  auto body = block();
  auto decl = std::make_unique<FuncDecl>(name.text, params, returnType,
                                         std::move(body));
  decl->line = name.line;
  decl->col = name.col;
  return decl;
}

std::unique_ptr<Block> Parser::block() {
  consume(TokenType::LBrace, "Expect '{'");
  auto node = std::make_unique<Block>();
  while (!check(TokenType::RBrace) && !check(TokenType::EndOfFile)) {
    node->statements.push_back(statement());
  }
  consume(TokenType::RBrace, "Expect '}'");
  return node;
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match(TokenType::Let))
    return varDecl();
  if (match(TokenType::For))
    return forStmt();
  if (match(TokenType::If))
    return ifStmt();
  if (match(TokenType::Print))
    return printStmt(false);
  if (match(TokenType::Println))
    return printStmt(true);
  if (match(TokenType::Return))
    return returnStmt();
  if (match(TokenType::LBrace))
    return block();

  // Check for Typed Declaration: type identifier ...
  // Peek to see if it looks like a type.
  if (check(TokenType::Identifier)) {
    std::string txt = peek().text;
    if (txt == "int" || txt == "float" || txt == "string") {
      return typedVarDecl();
    }
  }

  return expressionStmt();
}

Parser::ParsedType Parser::parseType() {
  std::string base = advance().text; // int, float, string
  bool isArray = false;
  std::unique_ptr<Expr> size = nullptr;

  if (match(TokenType::LBracket)) {
    isArray = true;
    if (!check(TokenType::RBracket)) {
      size = expression();
    }
    consume(TokenType::RBracket, "Expected ']' after array size.");
  }
  return {base, isArray, std::move(size)};
}

std::unique_ptr<Stmt> Parser::typedVarDecl() {
  auto type = parseType();
  std::string name =
      consume(TokenType::Identifier, "Expected variable name.").text;

  std::unique_ptr<Expr> init = nullptr;
  if (match(TokenType::Assign)) {
    init = expression();
  }
  consume(TokenType::Semicolon, "Expected ';' after declaration.");
  return std::make_unique<TypedVarDecl>(name, type.name, type.isArray,
                                        std::move(type.size), std::move(init));
}

std::unique_ptr<Stmt> Parser::varDecl() {
  Token name = consume(TokenType::Identifier, "Expect variable name");
  consume(TokenType::Assign, "Expect '='");
  auto init = expression();
  consume(TokenType::Semicolon, "Expect ';'");
  return std::make_unique<VarDecl>(name.text, std::move(init));
}

std::unique_ptr<Stmt> Parser::forStmt() {
  consume(TokenType::LParen, "Expect '(' after 'for'");

  std::unique_ptr<Stmt> init = nullptr;
  if (!match(TokenType::Semicolon)) {
    if (match(TokenType::Let)) {
      init = varDecl();
    } else if (check(TokenType::Identifier) &&
               (peek().text == "int" || peek().text == "float" ||
                peek().text == "string")) {
      init = typedVarDecl();
    } else {
      Token id =
          consume(TokenType::Identifier, "Expect identifier in for-init");
      consume(TokenType::Assign, "Expect '='");
      auto val = expression();
      init = std::make_unique<AssignStmt>(id.text, std::move(val));
      consume(TokenType::Semicolon, "Expect ';'");
    }
  }

  std::unique_ptr<Expr> cond = nullptr;
  if (!check(TokenType::Semicolon)) {
    cond = expression();
  }
  consume(TokenType::Semicolon, "Expect ';'");

  std::unique_ptr<Stmt> update = nullptr;
  if (!check(TokenType::RParen)) {
    Token id =
        consume(TokenType::Identifier, "Expect identifier in for-update");
    consume(TokenType::Assign, "Expect '='");
    auto val = expression();
    update = std::make_unique<AssignStmt>(id.text, std::move(val));
  }
  consume(TokenType::RParen, "Expect ')'");

  auto body = block();
  return std::make_unique<ForStmt>(std::move(init), std::move(cond),
                                   std::move(update), std::move(body));
}

std::unique_ptr<Stmt> Parser::ifStmt() {
  consume(TokenType::LParen, "Expect '('");
  auto cond = expression();
  consume(TokenType::RParen, "Expect ')'");
  auto thenBranch = block();
  std::unique_ptr<Block> elseBranch = nullptr;
  if (match(TokenType::Else)) {
    elseBranch = block();
  }
  return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch),
                                  std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::returnStmt() {
  std::unique_ptr<Expr> val = nullptr;
  if (!check(TokenType::Semicolon)) {
    val = expression();
  }
  consume(TokenType::Semicolon, "Expect ';'");
  return std::make_unique<ReturnStmt>(std::move(val));
}

std::unique_ptr<Stmt> Parser::expressionStmt() {
  auto expr = expression();

  if (match(TokenType::Assign)) {
    if (auto varNode = dynamic_cast<Variable *>(expr.get())) {
      auto val = expression();
      consume(TokenType::Semicolon, "Expect ';'");
      return std::make_unique<AssignStmt>(varNode->name, std::move(val));
    }
    if (auto arrNode = dynamic_cast<ArrayAccess *>(expr.get())) {
      auto val = expression();
      consume(TokenType::Semicolon, "Expect ';'");
      return std::make_unique<AssignStmt>(arrNode->name, std::move(val),
                                          std::move(arrNode->index));
    }
    throw ParseError("Invalid assignment target.", peek().line, peek().col);
  }

  consume(TokenType::Semicolon, "Expect ';'");
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::primary() {
  if (match(TokenType::Number)) {
    return std::make_unique<IntLiteral>(std::stoi(previous().text));
  }
  if (match(TokenType::Float)) {
    return std::make_unique<FloatLiteral>(std::stod(previous().text));
  }
  if (match(TokenType::StringLiteral)) {
    return std::make_unique<StringLiteral>(previous().text);
  }
  if (match(TokenType::Identifier)) {
    Token idToken = previous();
    std::string name = idToken.text;

    if (match(TokenType::LBracket)) {
      auto index = expression();
      consume(TokenType::RBracket, "Expect ']'");
      auto arr = std::make_unique<ArrayAccess>(name, std::move(index));
      arr->line = idToken.line;
      arr->col = idToken.col;
      return arr;
    }

    if (match(TokenType::LParen)) {
      std::vector<std::unique_ptr<Expr>> args;
      if (!check(TokenType::RParen)) {
        do {
          args.push_back(expression());
        } while (match(TokenType::Comma));
      }
      consume(TokenType::RParen, "Expect ')'");
      auto call = std::make_unique<CallExpr>(name, std::move(args));
      call->line = idToken.line;
      call->col = idToken.col;
      return call;
    }
    auto var = std::make_unique<Variable>(name);
    var->line = idToken.line;
    var->col = idToken.col;
    return var;
  }
  if (match(TokenType::LParen)) {
    auto expr = expression();
    consume(TokenType::RParen, "Expect ')'");
    return expr;
  }

  throw ParseError("Expect expression", peek().line, peek().col);
}

std::unique_ptr<Stmt> Parser::printStmt(bool newLine) {
  consume(TokenType::LParen, "Expect '('");
  auto expr = expression();
  consume(TokenType::RParen, "Expect ')'");
  consume(TokenType::Semicolon, "Expect ';'");
  return std::make_unique<PrintStmt>(std::move(expr), newLine);
}

std::unique_ptr<Expr> Parser::expression() { return equality(); }

std::unique_ptr<Expr> Parser::equality() {
  auto expr = comparison();
  while (match(TokenType::Equals) || match(TokenType::NotEquals)) {
    std::string op = previous().text;
    auto right = comparison();
    expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  auto expr = term();
  while (match(TokenType::Less) || match(TokenType::LessEq) ||
         match(TokenType::Greater) || match(TokenType::GreaterEq)) {
    std::string op = previous().text;
    auto right = term();
    expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  auto expr = factor();
  while (match(TokenType::Plus) || match(TokenType::Minus)) {
    std::string op = previous().text;
    auto right = factor();
    expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  auto expr = unary();
  while (match(TokenType::Star) || match(TokenType::Slash) ||
         match(TokenType::Mod)) {
    std::string op = previous().text;
    auto right = unary();
    expr = std::make_unique<BinaryExpr>(op, std::move(expr), std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match(TokenType::Not) || match(TokenType::Minus)) {
    std::string op = previous().text;
    auto right = unary();
    return std::make_unique<UnaryExpr>(op, std::move(right));
  }
  return primary();
}

} // namespace tinylang
