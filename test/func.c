int add(int x, int y) {
  return x + y;
}

int sub(int x, int y) {
  return x - y;
}

int main() {
  int x = 3;
  int y = 4;
  int a = add(x, y);
  int b = sub(y, x);
  int c = a + b;
  return c;
}
