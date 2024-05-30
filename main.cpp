#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "lexer.hpp"
#include "parser.hpp"
#include "generation.hpp"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Error: correct syntax: " << argv[0] << " <filename.unn> <output_filename>\n";
        return EXIT_FAILURE;
    }

    std::string output_filename = std::string(argv[2]) + ".asm";

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "ERROR: File not found\n";
        return EXIT_FAILURE;
    }

    Token *tokens = lexer(file);

    Node *root = parser(tokens);

    generate_code(root, "generated.asm");

    std::ifstream assembly_file("generated.asm");
    if (!assembly_file.is_open()) {
        std::cerr << "ERROR: Unable to open generated.asm\n";
        return EXIT_FAILURE;
    }

    std::string nasm_command = "nasm -f elf64 generated.asm -o generated.o";
    std::string gcc_command = "gcc generated.o -o " + std::string(argv[2]) + " -lc -no-pie";

    if (system(nasm_command.c_str()) != 0) {
        std::cerr << "ERROR: NASM command failed\n";
        return EXIT_FAILURE;
    }

    if (system(gcc_command.c_str()) != 0) {
        std::cerr << "ERROR: GCC command failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "FINISHED\n";
    return EXIT_SUCCESS;
}
