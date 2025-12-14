# Getting Started with TinyLang

Welcome to TinyLang! This guide will tackle the basics of writing programs in TinyLang.

## 1. Variables & Types
TinyLang supports `int`, `float`, and `string`.

```tinylang
// Inferred types
let x = 10;
let message = "Hello";

// Explicit types
int a = 5;
float pi = 3.14;
string name; // Uninitialized
```

## 2. Input / Output
Use `print()` to write to the console (without newline) and `println()` to write with a newline. Use `input()` to read text.

```tinylang
print("What is your name? ");
string user = input();
println("Hello " + user);
```

> **Note:** `input()` reads whitespace-separated tokens (words), not entire lines. If you type "John Doe", a single `input()` call will only read "John".


## 3. Arrays
Arrays can help store lists of data.

```tinylang
// Fixed size
int[5] numbers;
numbers[0] = 100;

// Dynamic size
let n = 10;
int[n] dynamicList;
```

## 4. Control Flow
Use `if/else` to make decisions.

```tinylang
let age = 18;
if (age >= 18) {
  print("Adult");
} else {
  print("Minor");
}
```

## 5. Loops
Use `for` loops to iterate.

```tinylang
for (let i=0; i<5; i=i+1) {
  print(i);
}
```
