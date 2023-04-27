#include <iostream>
#include <map>

class Table {
  private:
     std::map<char,int> mymap;

  public:
     void define(char c, int i) {
       mymap[c] = i;
     }

     bool count(char c) {
        return mymap.count(c) > 0;
     }
};

int main () {
  Table m1, m2;

  m1.define('a', 500);
  m1.define('c', 401);
  m1.define('f', 205);

  m2.define('b', 500);
  m2.define('d', 401);
  m2.define('e', 205);

  if (m1.count('b')) {
    std::cout << "Yay" << std::endl;
  }

  if (m1.count('b')) {
    std::cout << "Yay" << std::endl;
  }

  return 0;
}

