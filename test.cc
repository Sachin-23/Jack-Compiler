#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char *argv[]) {

  std::ifstream inFile;

  inFile.open(argv[1], std::ios::in);

  std::stringstream s;

  s << inFile.get(); 

  std::cout << s << std::endl;

  return 0;
}

