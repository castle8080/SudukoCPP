/*
Library for solving and creating Suduko puzzles.
*/
#ifndef SUDUKO_H
#define SUDUKO_H

#include <optional>
#include <functional>
#include <memory>
#include <random>
#include <set>
#include <stack>
#include <tuple>
#include <vector>

namespace Suduko {

    // Pre-declare types.
    class Board;

    // Alias used for the 2-d matrix of a Suduko board.
    template <typename T>
    using Matrix = std::vector<std::vector<T>>;

    // Function which generates boards.
    typedef std::function<std::optional<std::shared_ptr<Board>>()> BoardFactory;

    // Represents a Cell on a Suduko board.
    class Cell {
    private:
        // The row number [0-8] for the cell.
        int m_row;

        // The col number [0-8] for the cell.
        int m_col;

        // The value of the cell. The value 0 is used for an unset cell.
        int m_value;

        // Tracks the possible values for a cell.
        // If the value is set this should be empty.
        std::set<int> m_possibilities;

    public:

        // Create a new unset cell.
        Cell(int _row, int _col);

        // Is the cell set or not?
        const bool isSet();

        // Sets the value in a cell.
        // Checks that the value is a possible value.
        // Returns if the value could be set or not.
        bool trySet(int _value);

        void unset();

        const int value();

        const int row();

        const int col();

        // The box number of the cell [0-8].
        // The box numbers increase going left to right and then down each row.
        const int box();

        // Get the possible values for the cell.
        const std::set<int>& possibilities();

        // Remove a possible value from the cell.
        void removePossibility(int _value);

        void addPossibility(int _value);

        // Clear the cell back to unset.
        void clear();
    };

    // Represetnation of a Suduko board.
    class Board {
    private:
        /**
        * The cells for the Board.
        */
        Matrix<Cell> m_cells;

    public:

        /**
        * Construct a new board.
        */
        Board();

        void clear();

        std::string display();

        std::string debugDisplay();

        Cell& cell(int rowNo, int colNo);

        void setValue(int rowNo, int colNo, int value);

        bool trySetValue(int rowNo, int colNo, int value);

        void unset(int rowNo, int colNo);

        std::vector<Cell> getCellsWithSinglePossibility();

        int cellSetCount();

        bool isSolved();

        template <typename Func>
        void eachCell(Func f) {
            for (auto & row : m_cells) {
                for (auto & cell : row) {
                    f(cell);
                }
            }
        }

        template <typename Func>
        void eachCellInRow(int rowNo, Func f) {
            for (int colNo = 0; colNo < 9; colNo++) {
                f(cell(rowNo, colNo));
            }
        }

        template <typename Func>
        void eachCellInCol(int colNo, Func f) {
            for (int rowNo = 0; rowNo < 9; rowNo++) {
                f(cell(rowNo, colNo));
            }
        }

        template <typename Func>
        void eachCellInBox(int boxNo, Func f) {
            int brStart = (boxNo / 3) * 3;
            int brEnd = brStart + 3;
            int bcStart = (boxNo % 3) * 3;
            int bcEnd = bcStart + 3;
            for (int rowNo = brStart; rowNo < brEnd; rowNo++) {
                for (int colNo = bcStart; colNo < bcEnd; colNo++) {
                    f(cell(rowNo, colNo));
                }
            }
        }

        template <typename Func>
        void eachRelatedCell(int rowNo, int colNo, Func f) {
            auto & tCell = cell(rowNo, colNo);
            eachCellInRow(rowNo, [colNo, &f](auto & _cell) {
                if (_cell.col() != colNo) {
                    f(_cell);
                }
            });
            eachCellInCol(colNo, [rowNo, &f](auto & _cell) {
                if (_cell.row() != rowNo) {
                    f(_cell);
                }
            });
            eachCellInBox(tCell.box(), [rowNo, colNo, &f](auto & _cell) {
                if (_cell.row() != rowNo && _cell.col() != colNo) {
                    f(_cell);
                }
            });
        }
    private:
        void recomputePossibilities(int rowNo, int colNo);
    };

    // Solver for a Suduko board.
    class Solver {
    private:
        std::stack<BoardFactory> boards;
        std::default_random_engine generator;

    public:
        Solver(Board & board);

        std::optional<std::shared_ptr<Board>> next();

    private:
        std::optional<Cell> getCellToSolve(Board & board);
        void simplify(Board & board);
        void pushSolutionAttempts(std::shared_ptr<Board> board, Cell & cell);
    };

    class Generator {
    private:
        std::stack<BoardFactory> boards;
        std::default_random_engine generator;

    public:
        Generator();

        std::optional<std::shared_ptr<Board>> generate();

        std::optional<std::shared_ptr<Board>> generate(int setSize);

    private:
        void pushNextRemovals(std::shared_ptr<Board> board);

        bool Generator::hasSingleSolution(std::shared_ptr<Board> board);
    };

    // Loads a board from a file.
    std::shared_ptr<Board> loadFromFile(const std::string & filePath);
};

#endif
