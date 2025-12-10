#include "codegen.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

using namespace tinylang;

void printJson(bool success, const std::string &stdout_str,
               const std::string &stderr_str, int exit_code, long time_ms,
               const std::string &error_phase = "",
               const std::string &error_msg = "", int line = 0, int col = 0) {
  std::cout << "{\n";
  std::cout << "  \"success\": " << (success ? "true" : "false") << ",\n";
  if (!success) {
    std::cout << "  \"compile_errors\": [ { \"phase\": \"" << error_phase
              << "\", \"message\": \"" << error_msg << "\", \"line\": " << line
              << ", \"col\": " << col << " } ],\n";
  } else {
    std::cout << "  \"compile_errors\": [],\n";
  }
  // Escape JSON strings manually or use minimal escaping
  auto escape = [](const std::string &s) {
    std::string res;
    for (char c : s) {
      if (c == '"')
        res += "\\\"";
      else if (c == '\\')
        res += "\\\\";
      else if (c == '\n')
        res += "\\n";
      else if (c == '\r')
        res += "\\r";
      else if (c == '\t')
        res += "\\t";
      else
        res += c;
    }
    return res;
  };

  std::cout << "  \"stdout\": \"" << escape(stdout_str) << "\",\n";
  std::cout << "  \"stderr\": \"" << escape(stderr_str) << "\",\n";
  std::cout << "  \"exit_code\": " << exit_code << ",\n";
  std::cout << "  \"time_ms\": " << time_ms << "\n";
  std::cout << "}" << std::endl;
}

std::string exec(const char *cmd, std::string input = "") {
  // This is raw exec, but for the assignment we specifically need to capture
  // output. Using simple popen is easier for reading output. Writing input to
  // popen is harder (bidirectional). Let's use temporary files for stdin/stdout
  // to keep it robust and simple in C++.
  return ""; // Unused helper
}

int main(int argc, char **argv) {
  std::string filePath;
  std::string stdinContent;
  bool run = false;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--run")
      run = true;
    else if (std::string(argv[i]) == "--file" && i + 1 < argc)
      filePath = argv[++i];
    else if (std::string(argv[i]) == "--stdin" && i + 1 < argc)
      stdinContent = argv[++i];
  }

  if (filePath.empty()) {
    std::cerr
        << "Usage: tinylang-compiler --run --file <path> [--stdin <input>]"
        << std::endl;
    return 1;
  }

  // Read source
  std::ifstream f(filePath);
  if (!f) {
    printJson(false, "", "", 1, 0, "file", "Could not open file: " + filePath);
    return 0;
  }
  std::stringstream buffer;
  buffer << f.rdbuf();
  std::string source = buffer.str();

  auto start = std::chrono::high_resolution_clock::now();

  try {
    // 1. Lexer
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Check for lexer errors
    for (const auto &t : tokens) {
      if (t.type == TokenType::Error) {
        printJson(false, "", "", 1, 0, "lexer",
                  "Unexpected character: " + t.text, t.line, t.col);
        return 0;
      }
    }

    // 2. Parser
    Parser parser(std::move(tokens));
    auto prog = parser.parse();

    // 3. Semantic
    SemanticAnalyzer semantic;
    semantic.analyze(*prog);

    // 4. Optimizer
    Optimizer optimizer;
    optimizer.optimize(*prog);

    // 5. Codegen
    Codegen codegen;
    std::string cppCode = codegen.generate(*prog);

    // Write to tmp
    std::string tmpCpp = "/tmp/tinylang_gen.cpp";
    std::ofstream out(tmpCpp);
    out << cppCode;
    out.close();

    // Compile with g++
    std::string exePath = "/tmp/tinylang_run";
    std::string compileCmd = "g++ -O2 -std=c++20 -o " + exePath + " " + tmpCpp +
                             " 2>&1"; // capture gcc stderr

    // Execute compilation
    FILE *pipe = popen(compileCmd.c_str(), "r");
    if (!pipe)
      throw std::runtime_error("popen failed");
    char buf[128];
    std::string compileOutput;
    while (fgets(buf, 128, pipe) != NULL)
      compileOutput += buf;
    int ret = pclose(pipe);

    if (ret != 0) {
      // Compilation failed (C++ error, likely codegen bug or unhandled case)
      printJson(false, "", compileOutput, ret, 0, "codegen",
                "C++ Compilation failed: " + compileOutput);
      return 0;
    }

    if (run) {
      // Execute the binary
      // Write stdin to tmp file to pipe it safely
      std::string tmpStdin = "/tmp/tinylang_stdin.txt";
      std::ofstream val(tmpStdin);
      val << stdinContent;
      val.close();

      std::string cmd = exePath + " < " + tmpStdin;
      // Use popen again?
      // To capture both stdout and stderr, we use shell redirection
      cmd += " > /tmp/tinylang_out.txt 2> /tmp/tinylang_err.txt";

      auto runStart = std::chrono::high_resolution_clock::now();
      int runRet = system(cmd.c_str());
      auto runEnd = std::chrono::high_resolution_clock::now();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(runEnd -
                                                                      runStart)
                    .count();

      // Read back output
      std::ifstream info("/tmp/tinylang_out.txt");
      std::stringstream oss;
      oss << info.rdbuf();
      std::ifstream errf("/tmp/tinylang_err.txt");
      std::stringstream ess;
      ess << errf.rdbuf();

      int exitCode = WEXITSTATUS(runRet);
      std::string errStr = ess.str();

      if (exitCode != 0) {
        // Runtime error
        std::string msg = errStr.empty() ? "Program exited with code " +
                                               std::to_string(exitCode)
                                         : errStr;
        printJson(false, oss.str(), errStr, exitCode, ms, "runtime", msg);
      } else {
        printJson(true, oss.str(), errStr, 0, ms);
      }

    } else {
      // Just compiled
      printJson(true, "", "", 0, 0);
    }

  } catch (const ParseError &e) {
    printJson(false, "", "", 1, 0, "parser", e.what(), e.line, e.col);
  } catch (const SemanticError &e) {
    printJson(false, "", "", 1, 0, "semantic", e.what(), e.line, e.col);
  } catch (const std::exception &e) {
    printJson(false, "", "", 1, 0, "unknown", e.what());
  }

  return 0;
}