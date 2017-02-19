int main() {
  int x = 3;
  int* y = &x;
  int** z = &y;
  int a = **z;
  return a;
}
