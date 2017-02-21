int main() {
  int* i = malloc(sizeof(int)*2);
  *i = 2;
  *(i+1) = 3;
  return (*i + *(i+1));
}
