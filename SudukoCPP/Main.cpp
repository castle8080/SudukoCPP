#include "Suduko.h"
#include <iostream>
#include <string>


void solve(const char * sudukoFile) {
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
            std::cout << "Board:----------------" << std::endl;
            std::cout << board->display() << std::endl;
            count++;
        }
    }
}

int main(int argc, char ** argv) {
    try {
        //if (argc != 2) {
        //    std::cerr << "Error: Usage " << argv[0] << " <suduko file>" << std::endl;
        //    return 1;
        //}
        //solve(argv[1]);
        generate(25, 100);
    }
    catch (const std::exception & e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

