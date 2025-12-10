# TinyLang

A complete, local-only coding platform for TinyLang, a simple educational programming language.
Includes a C++ compiler, Python server, and React IDE.

## Quick Start

### Prerequisites
- Linux or macOS (Windows supported via WSL)
- C++ Compiler (g++ 10+)
- Python 3.8+
- Node.js & npm (for frontend)

#### System Package Installation (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install -y build-essential cmake python3 python3-pip python3-venv nodejs npm
```

### Build & Run
1. **Build Everything**:
   ```bash
   chmod +x scripts/build_all.sh
   ./scripts/build_all.sh
   ```

2. **Run Locally**:
   ```bash
   chmod +x scripts/run_local.sh
   ./scripts/run_local.sh
   ```
   - Frontend: http://localhost:5173
   - Backend: http://localhost:8000

## Project Structure
- `compiler/`: C++ Compiler (Lexer, Parser, AST, Semantic, Optimizer, Codegen).
- `server/`: Python FastAPI server to execute compiler locally.
- `frontend/`: React + TypeScript + Monaco IDE.
- `docs/`: Language Specification (`spec.md`).

## Features
- **Language**: `let`, `func`, `if/else`, `for`, `print`, `println`, `input()`.
- **Compiler**: 6-phase pipelined compiler producing native C++ -> Binary.
- **Error Reporting**: Detailed file, line, and column tracking for syntax and semantic errors.
- **IDE**: Modern dark theme, 3-pane layout (Code, Input, Output), Syntax Highlighting.

## License
MIT
