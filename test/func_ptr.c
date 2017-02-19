int add(int x, int y) {
  return x + y;
}

int main() {
  int (*add_ptr)(int, int);
  add_ptr = &add;
  int (*another_add_ptr)(int, int);
  another_add_ptr = add_ptr;
  int a = another_add_ptr(3, 4);
  return a;
}
