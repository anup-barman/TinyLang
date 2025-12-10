
export interface DocSnippet {
  title: string;
  code: string;
}

export interface DocSection {
  id: string;
  title: string;
  content: string; // Markdown-like text 
  snippets?: DocSnippet[];
}

export const docs: DocSection[] = [
  {
    id: 'getting-started',
    title: 'Getting Started',
    content: `Welcome to TinyLang! TinyLang is a robust educational programming language designated to be simple yet powerful.

## How to Run Code
1. Type your code in the **Editor Pane** on the left.
2. Click the **Run** button (or press **Cmd/Ctrl + Enter**).
3. View the output in the **Output** tab below.
4. Check for any errors or status messages in the **Status** tab.

## Basic Structure
A typical TinyLang program consists of a \`main\` function, which serves as the entry point.

\`\`\`cpp
func main() {
  println("Hello, World!");
}
\`\`\`
`,
    snippets: [
      {
        title: 'Hello World',
        code: `func main() {
  println("Hello, TinyLang!");
}`
      }
    ]
  },
  {
    id: 'types',
    title: 'Types & Variables',
    content: `TinyLang provides a strong type system to ensure code safety.

## Primitive Types
- **int**: Represents integers (whole numbers).
  - Example: \`10\`, \`-5\`, \`0\`
- **float**: Represents floating-point numbers (decimals).
  - Example: \`3.14\`, \`-0.01\`, \`2.0\`
- **string**: Represents sequences of characters.
  - Example: \`"Hello"\`, \`"TinyLang"\`

## Variable Declaration
Variables can be declared using type inference or explicit typing.

### Type Inference (\`let\`)
Use the \`let\` keyword to declare variables where the compiler infers the type from the value.
\`\`\`cpp
let x = 10;      // Inferred as int
let name = "Hi"; // Inferred as string
\`\`\`

### Explicit Typing
You can explicitly specify the type of a variable.
\`\`\`cpp
int count = 5;
float price = 9.99;
string message = "Welcome";
\`\`\`
`,
    snippets: [
      {
        title: 'Variable Examples',
        code: `func main() {
  // Inferred
  let x = 42;
  let pi = 3.14159;
  
  // Explicit
  string greeting = "Hello User";
  
  println(greeting);
  println(x);
  println(pi);
}`
      }
    ]
  },
  {
    id: 'arrays',
    title: 'Arrays',
    content: `Arrays allow storing multiple values of the same type.

## Declaration
### Fixed Size
Declare an array with a constant size.
\`\`\`cpp
int[5] numbers; // Array of 5 integers
\`\`\`

### Dynamic Size
Declare an array with a size determined at runtime.
\`\`\`cpp
let size = 10;
int[size] dynamicArr;
\`\`\`

## Accessing Elements
Access elements using zero-based indexing.
\`\`\`cpp
numbers[0] = 100;
println(numbers[0]);
\`\`\`
`,
    snippets: [
      {
        title: 'Array Operations',
        code: `func main() {
  int[3] nums;
  nums[0] = 1;
  nums[1] = 2;
  nums[2] = 3;
  
  println("First element: ");
  println(nums[0]);
}`
      }
    ]
  },
  {
    id: 'control-flow',
    title: 'Control Flow',
    content: `Control the flow of your program with conditional statements.

## If / Else
Execute code only if a condition is true.

\`\`\`cpp
if (condition) {
  // code to run if true
} else {
  // code to run if false
}
\`\`\`
`,
    snippets: [
      {
        title: 'Check Even/Odd',
        code: `func main() {
  let num = 10;
  if (num % 2 == 0) {
    println("Even");
  } else {
    println("Odd");
  }
}`
      }
    ]
  },
  {
    id: 'loops',
    title: 'Loops',
    content: `Repeat code execution using loops.

## For Loop
The \`for\` loop is versatile for iterating over ranges or arrays.

### Syntax
\`\`\`cpp
for (init; condition; update) {
  // body
}
\`\`\`
`,
    snippets: [
      {
        title: 'Count to 5',
        code: `func main() {
  for (let i = 1; i <= 5; i = i + 1) {
    println(i);
  }
}`
      }
    ]
  },
  {
    id: 'functions',
    title: 'Functions',
    content: `Functions allow you to organize and reuse code.

## Defining a Function
Use the \`func\` keyword. You must specify parameter types and the return type (using \`->\`).

\`\`\`cpp
func add(int a, int b) -> int {
  return a + b;
}
\`\`\`

If a function does not return a value, the return type can be omitted or return \`void\` (implicit).
`,
    snippets: [
      {
        title: 'Function Call',
        code: `func square(int x) -> int {
  return x * x;
}

func main() {
  let result = square(5);
  println(result);
}`
      }
    ]
  },
  {
    id: 'io',
    title: 'Input / Output',
    content: `TinyLang provides built-in functions for interacting with the console.

## Output
### \`println(expr)\`
Prints the value of \`expr\` followed by a new line.
\`\`\`cpp
println("Hello");
println("World");
// Output:
// Hello
// World
\`\`\`

### \`print(expr)\`
Prints the value of \`expr\` **without** a new line. Useful for building output on a single line.
\`\`\`cpp
print("Hello ");
print("World");
// Output: Hello World
\`\`\`

## Input
### \`input()\`
Reads a line of text from the Standard Input (STDIN). You must type in the "STDIN" pane before running or while running.

### Type Conversion
Since \`input()\` returns a string, you often need to convert it.
- \`int(string)\`: Converts to integer.
- \`float(string)\`: Converts to float.

## String Utilities
- \`len(string)\`: Returns the length of the string.
- \`substr(string, start, length)\`: Extracts a substring.
`,
    snippets: [
      {
        title: 'User Greeting',
        code: `func main() {
  println("What is your name?");
  let name = input();
  print("Hello, ");
  println(name);
}`
      }
    ]
  },
  {
    id: 'github',
    title: 'GitHub',
    content: `TinyLang is open source!

Check out the source code, report issues, or contribute on GitHub:

[https://github.com/anup-barman/TinyLang](https://github.com/anup-barman/TinyLang)
`,
    snippets: []
  }
];
