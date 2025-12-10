// Fibonacci
func fib(n) {
  if (n <= 1) { return n; }
  return fib(n-1) + fib(n-2);
}

func main() {
  let i = 0;
  for (i=0; i<10; i=i+1) {
    print(fib(i));
  }
}
