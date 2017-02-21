struct Point {
  int x;
  int y;
};

int main() {
  struct Point *p = malloc(sizeof(struct Point));
  p->x = 3;
  p->y = 4;
  return p->x + p->y;
}
