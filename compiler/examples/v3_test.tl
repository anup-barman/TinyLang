func main() {
  int[5] arr;
  arr[0] = 10;
  arr[1] = 20;

  if (arr[0] < arr[1]) {
    print("Ordered");
  } else {
    print("Unordered");
  }

  int n = 3;
  string[n] names;
  names[0] = "A";
  names[1] = "B";
  names[2] = "C";
  
  for (let i=0; i<n; i=i+1) {
    print(names[i]);
  }
}
