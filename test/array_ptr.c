int add(int* x) {
  int n = x[0] + x[1]; //3+4
  int m = *x + *(x+1); //3+4
  return n + m; //7+7
}

int main() {
  int a[2];
  a[0] = 3;
  a[1] = 4;
  return add(a); //14
}
