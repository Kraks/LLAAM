int main() {
  char* c = malloc(4);
  *c = 1;
  free(c);
}
