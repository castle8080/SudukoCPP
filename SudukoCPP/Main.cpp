#include "Suduko.h"
#include <iostream>
#include <string>

int main(int argc, char ** argv) {
	try {
		if (argc != 2) {
			std::cerr << "Error: Usage " << argv[0] << " <suduko file>" << std::endl;
			return 1;
		}

		Suduko::Board board = Suduko::loadFromFile(argv[1]);
		Suduko::Solver solver(board);

		std::cout << "Original board: " << std::endl;
		std::cout << board.display() << std::endl;

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
	catch (const std::exception & e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

