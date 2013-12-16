#include <fstream>
#include <iostream>
#include <cstdlib> // EXIT_FAILURE

#define LINE_WIDTH 80
#define STRINGIFY_1(V) #V
#define STRINGIFY(V) STRINGIFY_1(V)

int main(int argc, char ** argv)
{
  if ((argc - 1) % 2 != 0) {
    std::cerr << "Usage: "
              << argv[0]
              << " <name>"
              << " <file>"
              << " <name>"
              << " <file>"
              << " etc.."
              << std::endl;
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i += 2) {
    char buffer[LINE_WIDTH + 1];

    std::ifstream file(argv[i+1]);
    std::cout << "std::string " << argv[i] << " =" << std::endl;

    for (std::string line; std::getline(file, line); ) {

      if (line.length() < LINE_WIDTH) {
        std::sprintf(buffer, "%-" STRINGIFY(LINE_WIDTH) "s",
                     line.c_str());

      } else {
        std::sprintf(buffer, "%-" STRINGIFY(LINE_WIDTH) "s",
                     line.substr(0, LINE_WIDTH).c_str());
        std::cout << "\"" << buffer << "\"" << std::endl;
        std::sprintf(buffer, "%-" STRINGIFY(LINE_WIDTH) "s",
                     line.substr(LINE_WIDTH, line.length()).c_str());
      }

      std::cout << "\"" << buffer << "\\n\"" << std::endl;
    }

    std::cout << ";" << std::endl;
    file.close();
  }

  return 0;
}
