struct Point {
  char x;
  char y;
};

struct Line {
  struct Point p1;
  struct Point p2;
};

int main() {
  struct Point p1;
  p1.x = 2;
  p1.y = 3;

  struct Point p2;
  p2.x = 5;
  p2.y = 6;

  struct Line l;
  l.p1 = p1;
  l.p2 = p2;
}
