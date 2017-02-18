int add(int* x, int* y) {
  *y = 5;
  return *x + *y;
}

int main() {
  int a = 3;
  int b = 4;
  int c = add(&a, &b);
}
