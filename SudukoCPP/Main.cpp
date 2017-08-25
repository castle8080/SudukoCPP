#include "Suduko.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>

void help(const char * exeName) {
    // TODO: show help
}

void solve(std::string sudukoFile) {
    auto board = Suduko::loadFromFile(sudukoFile);
    Suduko::Solver solver(*board);

    std::cout << "Original board: " << std::endl;
    std::cout << board->display() << std::endl;

    while (true) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        auto solved = solver.next();
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> time_span = t2 - t1;
        if (solved.has_value()) {
            std::cout << "Solved in " << time_span.count() << " ms." << std::endl;
            std::cout << (*solved)->display() << std::endl;
        }
        else {
            std::cout << "No further sollutions: " << time_span.count() << " ms." << std::endl;
            break;
        }
    }
}

void generate(int setSize, int puzzleCount, int boardMaxTries) {
    int count = 0;
    while (count < puzzleCount) {
        Suduko::Generator generator;
        for (int i = 0; i < boardMaxTries; i++) {
            auto boardOpt = generator.generate();
            if (boardOpt.has_value()) {
                auto board = *boardOpt;
                if (board->cellSetCount() <= setSize) {
                    std::cout << board->display() << std::endl;
                    count++;
                    break;
                }
            }
        }
    }
}

int main(int argc, char ** argv) {
    try {
        std::string action = "help";
        int count = 1;
        int cellSet = 25;
        int boardMaxTries = 1000;
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
            generate(cellSet, count, boardMaxTries);
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

