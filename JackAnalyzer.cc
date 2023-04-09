#include <iostream>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Process each file
void processFile(std::string file) {
  std::string fileName = file;
  // Only files ending .jack extension
  if (fileName.substr(fileName.find_last_of(".")) == ".jack") {
    std::cout << fileName << std::endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: JackAnalyzer [file or directory]" << std::endl;
    exit(1);
  }
  /* 
    check if the file is directory or a single file 
    if single file open do xyz things
    if a directory then for each file do xyz things
  */
  struct stat pathStat; 
  stat(argv[1], &pathStat);
  std::string path = argv[1];
   
  // Regular file
  if (S_ISREG(pathStat.st_mode)) {
    // std::cout << "Processing a single file: " << argv[1] << std::endl;
    processFile(argv[1]);
  }
  // Directory of Jack files
  else if (S_ISDIR(pathStat.st_mode)) {
    DIR *dir;
    dir = opendir(argv[1]);

    if (dir == NULL) {
      std::cerr << "Failed to open directory: " << argv[1] << std::endl;
      exit(1);
    }

    struct dirent *entry; 
    // for every file
    while ((entry=readdir(dir))) {
      // Extra '/' won't make any difference
      processFile(path + "/" + entry -> d_name);
    }
     
    closedir(dir); 
  }
  else {
    std::cerr << "File is invalid" << std::endl;
    exit(1);
  }
    
  return 0;
}

