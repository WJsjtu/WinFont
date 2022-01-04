#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "USAGE: " << argv[0]
                  << " {sym} {rsrc}\n\n"
                     "  Creates {sym}.c from the contents of {rsrc}\n";
        return EXIT_FAILURE;
    }

    std::ifstream inFile(argv[2], std::ios::binary);
    if (!inFile.is_open()) {
        return EXIT_FAILURE;
    }
    std::string data_string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    const char* data = data_string.c_str();
    inFile.close();

    std::string outFilePath = argv[1];
    outFilePath += ".c";
    std::ofstream outFile(outFilePath);
    if (!outFile.is_open()) {
        return EXIT_FAILURE;
    }

    outFile << "#include <stdlib.h>\n"
            << "const char " << argv[1] << "[] = {\n";
    size_t linecount = 0;
    for (size_t i = 0; i < data_string.size(); i++) {
        outFile << (int)data[i] << ", ";
        if (++linecount == 10) {
            outFile << std::endl;
            linecount = 0;
        }
    }

    if (linecount > 0) {
        outFile << std::endl;
    }
    outFile << "};" << std::endl;
    outFile << "const size_t " << argv[1] << "_len = sizeof(" << argv[1] << ");" << std::endl;

    outFile.close();

    return EXIT_SUCCESS;
}