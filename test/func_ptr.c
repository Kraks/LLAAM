int add(int x, int y) {
  return x + y;
}

int main() {
  int (*add_ptr)(int, int);
  add_ptr = &add;
  int a = add_ptr(3, 4);
}
