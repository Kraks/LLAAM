int add(int* x) {
  int n = x[0] + x[1];
  int m = *x + *(x+1);
  return n + m;
}

int main() {
  int a[2];
  a[0] = 3;
  a[1] = 4;
  return add(a);
}
