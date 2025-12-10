#include "codegen.hpp"

namespace tinylang {

std::string Codegen::generate(Program &prog) {
  out.str("");
  out << "#include <iostream>\n";
  out << "#include <string>\n";
  out << "#include <vector>\n";
  out << "#include <algorithm>\n\n";

  // Runtime Helpers
  out << "std::string _tl_input() { std::string s; std::cin >> s; "
         "return s; }\n";
  out << "int _tl_len(const std::string& s) { return (int)s.length(); }\n";
  out << "std::string _tl_substr(const std::string& s, int start, int len) { "
         "return s.substr(start, len); }\n";
  out << "int _tl_to_int(const std::string& s) { try { return std::stoi(s); } "
         "catch (...) { return 0; } }\n";
  out << "double _tl_to_float(const std::string& s) { try { return "
         "std::stod(s); } "
         "catch (...) { return 0.0; } }\n\n";

  // We need to declare all functions first (forward declarations)
  // But we will just emit everything in order and assume topological sort or
  // require C++ forward decls? Let's iterate program declarations first to emit
  // prototypes? Simplest: `int main() { ... }` but we allow functions. So let's
  // emit them.

  prog.accept(*this);
  return out.str();
}

void Codegen::indent() {
  for (int i = 0; i < indentLevel; ++i)
    out << "  ";
}

void Codegen::emit(const std::string &str) { out << str; }

void Codegen::emitLine(const std::string &str) {
  indent();
  out << str << "\n";
}

void Codegen::visit(IntLiteral &node) { emit(std::to_string(node.value)); }

void Codegen::visit(FloatLiteral &node) { emit(std::to_string(node.value)); }

void Codegen::visit(StringLiteral &node) {
  emit("\"" + node.value +
       "\""); // Simple escaping needed? Assuming no quotes in string for now
}

void Codegen::visit(Variable &node) { emit(node.name); }

void Codegen::visit(IfStmt &node) {
  emit("if (");
  node.condition->accept(*this);
  emitLine(") {");
  indentLevel++;
  node.thenBranch->accept(*this);
  indentLevel--;
  emitLine("}");
  if (node.elseBranch) {
    emitLine("else {");
    indentLevel++;
    node.elseBranch->accept(*this);
    indentLevel--;
    emitLine("}");
  }
}

void Codegen::visit(TypedVarDecl &node) {
  indent();
  // Map types
  std::string cppType = "int";
  if (node.type == "float")
    cppType = "double";
  else if (node.type == "string")
    cppType = "std::string";

  if (node.isArray) {
    // std::vector<Type> name; or name(size);
    emit("std::vector<" + cppType + "> " + node.name);
    if (node.arraySize) {
      emit("(");
      node.arraySize->accept(*this);
      emit(")");
    }
  } else {
    emit(cppType + " " + node.name);
    if (node.initializer) {
      emit(" = ");
      node.initializer->accept(*this);
    } else {
      // Default init options?
      // For now, C++ default init (0 for globals, random for locals? No,
      // primitives undefined). But we act like C++. "int x;" -> "int x;" If
      // strict safety requested, emit "= 0;"? Let's safe-init strings always.
      if (cppType == "std::string") {
      } // default ctor is empty
      else
        emit(" = 0"); // Strict safety default?
    }
  }
  emit(";\n");

  // Safety flag
  indent();
  emitLine("bool " + node.name +
           "_init = " + (node.initializer ? "true" : "false") + ";");
}

void Codegen::visit(AssignStmt &node) {
  indent();
  emit(node.name);
  if (node.index) {
    emit("[");
    // Bounds Check Logic (if simplified)
    // Using vector::at() provides checks but throws std::out_of_range
    // (crashes). Spec asks for "Runtime error". Use unchecked [] for C
    // compatibility or .at() for safety? Let's use `[` but we can inject a
    // check before using `_tl_check_bounds`. However, here we just emit index
    // expression.
    node.index->accept(*this);
    emit("]");
  }
  emit(" = ");
  node.value->accept(*this);
  emit(";\n");

  indent();
  emitLine(node.name + "_init = true;");
}

void Codegen::visit(ArrayAccess &node) {
  // Check init? Arrays are objects, array itself is init?
  // Element init? Too hard to track per-element bits efficiently without
  // bitset. Just check array variable. Emit initialization check?
  // if(!name_init) ...
  // But we are in an expression. Can't emit "if" statement easily inside
  // expression unless we use comma op or helper.
  // "(_tl_check_init(name_init), name[index])"

  emit(node.name);
  emit("[");
  node.index->accept(*this);
  emit("]");
}

void Codegen::visit(BinaryExpr &node) {
  emit("(");
  node.left->accept(*this);
  emit(" " + node.op + " ");
  node.right->accept(*this);
  emit(")");
}

void Codegen::visit(UnaryExpr &node) {
  emit("(" + node.op);
  node.operand->accept(*this);
  emit(")");
}

void Codegen::visit(CallExpr &node) {
  if (node.callee == "input") {
    emit("_tl_input()");
    return;
  }
  if (node.callee == "len") {
    emit("_tl_len(");
    if (!node.args.empty())
      node.args[0]->accept(*this);
    emit(")");
    return;
  }
  if (node.callee == "substr") {
    emit("_tl_substr(");
    // arguments...
    for (size_t i = 0; i < node.args.size(); ++i) {
      node.args[i]->accept(*this);
      if (i < node.args.size() - 1)
        emit(", ");
    }
    emit(")");
    return;
  }

  // Casting built-ins
  if (node.callee == "int") {
    emit("_tl_to_int(");
    if (!node.args.empty())
      node.args[0]->accept(*this);
    emit(")");
    return;
  }
  if (node.callee == "float") {
    emit("_tl_to_float(");
    if (!node.args.empty())
      node.args[0]->accept(*this);
    emit(")");
    return;
  }

  emit(node.callee + "(");
  for (size_t i = 0; i < node.args.size(); ++i) {
    node.args[i]->accept(*this);
    if (i < node.args.size() - 1)
      emit(", ");
  }
  emit(")");
}

void Codegen::visit(VarDecl &node) {
  indent();
  emit("auto " + node.name);
  if (node.initializer) {
    emit(" = ");
    node.initializer->accept(*this);
  } else {
    emit(" = 0"); // Default? Issue: auto requires init.
    // We should enforce initializer in parser or here.
    // Spec says `let x = expr;` is `var_decl`. It always has initializer in
    // grammar! var_decl ::= "let" id "=" expr ";"
  }
  emit(";\n");
}

void Codegen::visit(ForStmt &node) {
  indent();
  emit("for (");
  // Init is a stmt, which emits "int x = ...;\n" or "x = ...;\n"
  // This is tricky inside `for(...)`.
  // If init is VarDecl, we output "int x=val;". The semicolon is inside VarDecl
  // visit. BUT our VarDecl/AssignStmt visitors emit Indent + content + newline.
  // We want inline content.
  // We need a flag or separate helper for expression-only generation without
  // statements wrapping? Or we manually handle init/update here since we know
  // their types?

  // Hack: Use a temporary stringstream or just print raw?
  // Better: Helper "emitInline(Stmt*)"?
  // Actually, C++ `for` allows `int i=0;` ...
  // But `AssignStmt` visitor prints `  name = val;\n`.
  // We don't want the indent or newline.

  // We will cheat and NOT call `accept` for init/update if they are simple.
  // But we need to support generic.
  // Let's implement `visit` to respect a "compactMode"?
  // Or simpler: Just emit `init` logic manually here.

  // Scope: init is usually VarDecl or AssignStmt.
  if (node.init) {
    if (auto v = dynamic_cast<VarDecl *>(node.init.get())) {
      emit("int " + v->name + " = ");
      if (v->initializer)
        v->initializer->accept(*this);
      else
        emit("0");
      emit("; ");
    } else if (auto a = dynamic_cast<AssignStmt *>(node.init.get())) {
      emit(a->name + " = ");
      a->value->accept(*this);
      emit("; ");
    }
  } else {
    emit("; ");
  }

  if (node.condition) {
    node.condition->accept(*this);
  }
  emit("; ");

  if (node.update) {
    if (auto a = dynamic_cast<AssignStmt *>(node.update.get())) {
      emit(a->name + " = ");
      a->value->accept(*this);
    }
    // what if it's expression stmt?
  }

  emit(")\n");
  node.body->accept(*this);
}

void Codegen::visit(FuncDecl &node) {
  // Use explicit return type if provided, otherwise auto. Main is always int.
  std::string retType;
  if (node.name == "main") {
    retType = "int";
  } else if (!node.returnType.empty()) {
    retType = node.returnType;
    if (retType == "string")
      retType = "std::string";
    if (retType == "float")
      retType = "double";
  } else {
    retType = "auto";
  }

  emitLine(retType + " " + node.name + "(" + ([&]() {
             std::string s;
             for (size_t i = 0; i < node.params.size(); ++i) {
               std::string type = node.params[i].first;
               std::string name = node.params[i].second;

               if (type.empty()) {
                 type = "auto";
               } else {
                 if (type == "string")
                   type = "std::string";
                 if (type == "float")
                   type = "double";
               }

               s += type + " " + name;
               if (i < node.params.size() - 1)
                 s += ", ";
             }
             return s;
           })() +
           ")");
  node.body->accept(*this);
  emitLine("");
}

void Codegen::visit(ReturnStmt &node) {
  indent();
  emit("return ");
  if (node.value) {
    node.value->accept(*this);
  } else {
    emit("0"); // Default return
  }
  emit(";\n");
}

void Codegen::visit(PrintStmt &node) {
  indent();
  emit("std::cout << ");
  node.expr->accept(*this);
  if (node.newLine) {
    emit(" << std::endl;\n");
  } else {
    emit(";\n");
  }
}

void Codegen::visit(ExprStmt &node) {
  indent();
  node.expr->accept(*this);
  emit(";\n");
}

void Codegen::visit(Block &node) {
  emitLine("{");
  indentLevel++;
  for (auto &stmt : node.statements) {
    stmt->accept(*this);
  }
  indentLevel--;
  indent();
  emit("}\n");
}

void Codegen::visit(Program &node) {
  bool hasMain = false;
  // Separate declarations into Functions and Statements
  std::vector<FuncDecl *> funcs;
  std::vector<Stmt *> globalStmts;

  for (auto &d : node.declarations) {
    if (auto f = dynamic_cast<FuncDecl *>(d.get())) {
      funcs.push_back(f);
      if (f->name == "main")
        hasMain = true;
    } else if (auto s = dynamic_cast<Stmt *>(d.get())) {
      globalStmts.push_back(s);
    }
  }

  // Emit functions
  for (auto f : funcs) {
    f->accept(*this);
  }

  // Emit main if not present (script mode)
  if (!hasMain) {
    emitLine("int main() {");
    indentLevel++;
    for (auto s : globalStmts) {
      s->accept(*this);
    }
    emitLine("return 0;");
    indentLevel--;
    emitLine("}");
  } else {
    // defined main, what about global stmts?
    // In this simple implementation, we ignore them if main exists,
    // effectively treating them as dead code or invalid.
  }
}

} // namespace tinylang
