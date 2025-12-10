// Factorial Example
func factorial(n) {
  if (n < 2) {
    return 1;
  }
  return n * factorial(n - 1);
}

func main() {
  let n = 5;
  print(factorial(n));
}
