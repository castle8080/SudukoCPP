#include "Suduko.h"
#include <iostream>
#include <string>
#include <cstring>

void help(const char * exeName) {
    // TODO: show help
}

void solve(std::string sudukoFile) {
    auto board = Suduko::loadFromFile(sudukoFile);
    Suduko::Solver solver(*board);

    std::cout << "Original board: " << std::endl;
    std::cout << board->display() << std::endl;

    while (true) {
        auto solved = solver.next();
        if (solved.has_value()) {
            std::cout << "Solved: " << std::endl << (*solved)->display() << std::endl;
        }
        else {
            break;
        }
    }
}

void generate(int setSize, int puzzleCount) {
    int count = 0;
    while (count < puzzleCount) {
        Suduko::Generator generator;
        auto boardOpt = generator.generate(setSize);
        if (boardOpt.has_value()) {
            auto board = *boardOpt;
            std::cout << board->display() << std::endl;
            count++;
        }
    }
}

int main(int argc, char ** argv) {
    try {
        std::string action = "help";
        int count = 1;
        int cellSet = 25;
        std::string solveFile = "";

        for (int i = 1; i < argc; i ++) {
            if (strcmp(argv[i], "--generate") == 0) {
                action = "generate";
            }
            else if ((strcmp(argv[i], "--solve") == 0) && i < (argc - 1)) {
                action = "solve";
                solveFile = argv[i + 1];
                i++;
            }
            else if (strcmp(argv[i], "--help") == 0) {
                action = "hep";
            }
            else if ((strcmp(argv[i], "--count") == 0) && i < (argc - 1)) {
                count = atoi(argv[i+1]);
                i++;
            }
            else if ((strcmp(argv[i], "--cellSet") == 0) && i < (argc - 1)) {
                cellSet = atoi(argv[i + 1]);
                i++;
            }
            else {
                help(argv[0]);
                return 1;
            }
        }

        if (action == "help") {
            help(argv[0]);
        }
        else if (action == "generate") {
            generate(cellSet, count);
        }
        else if (action == "solve") {
            solve(solveFile);
        }
    }
    catch (const std::exception & e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

