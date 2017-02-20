int add(int* x) {
  //return x[0] + x[1];
  return *x + *(x+1);
}

int main() {
  int a[2];
  a[0] = 3;
  a[1] = 4;
  return add(a);
}
