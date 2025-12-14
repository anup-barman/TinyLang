# TinyLang Compiler Manual

This guide describes how to manually build, run, and interact with the TinyLang compiler (`tinylang-compiler`).

## 1. Building the Compiler

The compiler is written in C++ and uses CMake.

**Prerequisites:**
- C++ Compiler (g++ 10+ recommended)
- CMake 3.10+

**Build Steps:**
```bash
cd compiler
mkdir -p build
cd build
cmake ..
make
```

This will produce the executable `compiler/build/tinylang-compiler`.

## 2. CLI Usage

The compiler supports several flags for different modes of operation.

```bash
./tinylang-compiler [FLAGS]
```

### Flags

| Flag | Description |
| :--- | :--- |
| `--run` | Compiles the source **and executes** it immediately. Output is returned as JSON. |
| `--file <path>` | Path to the TinyLang source file (`.tl`) to accept. |
| `--stdin <text>` | String input to be fed to the program's `input()` function (Script Mode only). |

### Example Uses

**Compile and Run (Batch Mode):**
```bash
./tinylang-compiler --run --file ../examples/factorial.tl
```

**Compile and Run with Input:**
```bash
# Pass argument for input() calls
./tinylang-compiler --run --file ../examples/simple_input.tl --stdin "Alice"
```

---

## 3. Output Format (Batch Mode)

When using `--run`, the compiler outputs a JSON object containing the results of both the compilation and the execution. This is designed for the TinyLang IDE/Server integration.

**Example Output:**
```json
{
  "success": true,
  "compile_errors": [],
  "stdout": "Program Output Here",
  "stderr": "",
  "exit_code": 0,
  "time_ms": 5
}
```

- **`success`**: `true` if compilation and execution were successful.
- **`compile_errors`**: List of errors if compilation failed.
- **`stdout`**: Standard output from the TinyLang program.
- **`stderr`**: Standard error or runtime crash details.
- **`exit_code`**: System exit code of the compiled binary.
- **`time_ms`**: Execution time in milliseconds.

---

## 4. Interactive Mode

By default, the compiler acts as a transpiler. It converts TinyLang code to C++, compiles that C++ code into a machine binary, and places it at `/tmp/tinylang_run`.

To run a program **interactively** (e.g., to type inputs manually in the terminal):

1. **Compile Only:**
   Run the compiler **without** the `--run` flag.
   ```bash
   ./tinylang-compiler --file ../examples/two_inputs.tl
   ```
   This generates the executable at `/tmp/tinylang_run` and prints a JSON status confirming compilation success.

2. **Execute the Binary:**
   Run the generated binary directly.
   ```bash
   /tmp/tinylang_run
   ```
   You can now interact with the program naturally in your terminal.

---

## 5. Input Handling Behavior

The TinyLang `input()` function is implemented using C++ `std::cin`.

- It reads **whitespace-delimited** words (tokens), not entire lines.
- **Space-separated inputs**: If you provide `"John Doe"` as input to a program with two `input()` calls:
  - Call 1 gets `"John"`
  - Call 2 gets `"Doe"`
- **Newline-separated inputs**: Behave identically to space-separated inputs.
