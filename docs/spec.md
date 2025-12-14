# TinyLang Specification

## Overview
TinyLang is a simple, educational programming language designed to demonstrate the core phases of a compiler. It supports basic arithmetic, variables, control flow (`for` loops), functions, and I/O (`print` vs `println`).

## Grammar (EBNF)

```ebnf
program        ::= { declaration | stmt } ;

declaration    ::= function_decl 
                 | typed_var_decl ;

function_decl  ::= "func" identifier "(" [ typed_param_list ] ")" block ;
typed_param_list ::= typed_param { "," typed_param } ;
typed_param    ::= type identifier ;

stmt           ::= typed_var_decl
                 | var_decl
                 | assignment
                 | print_stmt
                 | for_stmt
                 | if_stmt
                 | return_stmt
                 | expr_stmt ;

typed_var_decl ::= type identifier [ "=" expr ] ";" ;
var_decl       ::= "let" identifier "=" expr ";" ;

type           ::= base_type [ "[" [ expr ] "]" ] ;
base_type      ::= "int" | "float" | "string" ;

assignment     ::= identifier [ "[" expr "]" ] "=" expr ";" ;
if_stmt        ::= "if" "(" expr ")" block [ "else" block ] ;
for_stmt       ::= "for" "(" ( var_decl | assignment | ";" ) [ expr ] ";" [ assignment | expr ] ")" block ;
return_stmt    ::= "return" [ expr ] ";" ;
print_stmt     ::= ( "print" | "println" ) "(" expr ")" ";" ;
expr_stmt      ::= expr ";" ;
block          ::= "{" { stmt } "}" ;

expr           ::= term { ( "==" | "!=" | "<" | "<=" | ">" | ">=" ) term } ;
term           ::= factor { ( "+" | "-" ) factor } ;
factor         ::= unary { ( "*" | "/" | "%" ) unary } ;
unary          ::= ( "!" | "-" ) unary | primary ;
primary        ::= number | float_literal | string_literal
                 | identifier [ "[" expr "]" ] [ "(" [ arg_list ] ")" ]
                 | "(" expr ")" ;

arg_list       ::= expr { "," expr } ;
```

## Types
- **Primitves**: `int`, `float`, `string`.
- **Arrays**: `int[]`, `float[10]`, `string[n]`. Zero-indexed.

## Safety
- **Uninitialized Reads**: Undefined behavior by default. Use `--check-undef-reads` to catch at runtime.
- **Bounds Checking**: On by default. Disable with `--no-bounds-check`.

## Built-in Functions
- `input()`: Reads a token from stdin (returns String). Reads one whitespace-delimited word at a time.
- `print(expr)`: Prints expression to stdout (no newline).
- `println(expr)`: Prints expression to stdout with newline.
- `len(string)`: Returns length of string (Int).
- `substr(string, start, len)`: Returns substring (String).

## Compiler Phases
The specific implementation is split into 6 distinct files in `compiler/src/`:

1.  **Lexer (`lexer.cpp`)**: Converts raw source text into a stream of tokens. Handles line/column tracking.
2.  **Parser (`parser.cpp`)**: Consumes tokens to build an Abstract Syntax Tree (AST).
3.  **AST (`ast.cpp`)**: Defines the node structures (Expressions, Statements, Declarations).
4.  **Semantic (`semantic.cpp`)**: Traverses the AST to check for undefined variables and ensure basic type safety (integers).
5.  **Optimizer (`optimizer.cpp`)**: per-forms simple optimizations like constant folding (e.g., transforming `3 + 4` into `7`).
6.  **Codegen (`codegen.cpp`)**: Transpiles the AST into valid C++ code. `print()` maps to `std::cout`.

## Build & Run

### Prerequisites
- C++ Compiler (g++ with C++20/23 support)
- CMake 3.10+
- Python 3.8+
- Node.js 16+

### Compiler
```bash
cd compiler
mkdir build && cd build
cmake ..
make
# Run
./tinylang-compiler --run --file ../examples/hello.tl
```

### Server
```bash
cd server
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn app.main:app --reload
```

### Frontend
```bash
cd frontend
npm install
npm run dev
```

## Security Note
This project runs a local C++ compiler and executes the resulting binary on your machine. **Do not expose this server to the public internet** without proper sandboxing (e.g., Docker, gVisor, Firejail).
