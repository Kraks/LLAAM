int main() {
  int i, j;
  int sum = 0;
  for (i = 0, j = 0; i < 10 && j < 10; i++, j++) {
    sum += (i + j);
  }
  return sum;
}
