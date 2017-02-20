int main() {
  char a[10];
  char* c = a; //c[0]
  c++; //c[1]
  c++; //c[2]
  c++; //c[3]
  c--; //c[2]
  *c = 5;
}
