#include "Suduko.h"

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <optional>
#include <functional>
#include <memory>
#include <random>
#include <algorithm>
#include <chrono>

namespace Suduko {

    Cell::Cell(int _row, int _col) :
        m_row(_row),
        m_col(_col),
        m_value(0),
        m_possibilities{ 1,2,3,4,5,6,7,8,9 }
    {}

    bool Cell::trySet(int _value) {
        if (_value < 1 || _value > 9) {
            throw std::invalid_argument(std::string("Invalid value set: ") + std::to_string(_value) + ".");
        }
        if (m_possibilities.find(_value) == m_possibilities.end()) {
            return false;
        }

        m_value = _value;
        m_possibilities.empty();
        return true;
    }

    const bool Cell::isSet() {
        return m_value >= 1 && m_value <= 9;
    }

    const int Cell::row() {
        return m_row;
    }

    const int Cell::col() {
        return m_col;
    }

    const int Cell::box() {
        return m_row / 3 * 3 + m_col / 3;
    }

    const int Cell::value() {
        return m_value;
    }


    void Cell::removePossibility(int _value) {
        m_possibilities.erase(_value);
    }

    const std::set<int>& Cell::possibilities() {
        return m_possibilities;
    }

    void Cell::clear() {
        m_value = 0;
        m_possibilities = std::set<int>{ 1,2,3,4,5,6,7,8,9 };
    }

    Board::Board() :
        m_cells(9)
    {
        for (int rowNo = 0; rowNo < 9; rowNo++) {
            m_cells.push_back(std::vector<Cell>());
            for (int colNo = 0; colNo < 9; colNo++) {
                m_cells[rowNo].push_back(Cell(rowNo, colNo));
            }
        }
    }

    void Board::clear() {
        eachCell([](auto cell) { cell.clear(); });
    }

    std::vector<Cell> Board::getCellsWithSinglePossibility() {
        std::vector<Cell> spCells;
        eachCell([&spCells](auto & _cell) {
            if (!_cell.isSet() && _cell.possibilities().size() == 1) {
                spCells.push_back(_cell);
            }
        });
        return spCells;
    }

    std::string Board::debugDisplay() {
        std::stringstream content;

        for (int rowNo = 0; rowNo < 9; rowNo++) {
            std::stringstream lines[] = { std::stringstream(), std::stringstream(), std::stringstream() };

            if (rowNo == 3 || rowNo == 6) {
                content << "###################################" << std::endl;
            }
            else if (rowNo > 0) {
                content << "---+---+---#---+---+---#---+---+---" << std::endl;
            }

            for (int colNo = 0; colNo < 9; colNo++) {
                auto _cell = cell(rowNo, colNo);

                if (colNo == 3 || colNo == 6) {
                    lines[0] << "#";
                    lines[1] << "#";
                    lines[2] << "#";
                }
                else if (colNo > 0) {
                    lines[0] << "|";
                    lines[1] << "|";
                    lines[2] << "|";
                }

                if (_cell.isSet()) {
                    lines[0] << " v ";
                    lines[1] << ">" << _cell.value() << "<";
                    lines[2] << " ^ ";
                }
                else {
                    for (int _pVal = 1; _pVal <= 9; _pVal++) {
                        int lineNo = (_pVal - 1) / 3;
                        if (_cell.possibilities().find(_pVal) != _cell.possibilities().end()) {
                            lines[lineNo] << _pVal;
                        }
                        else {
                            lines[lineNo] << " ";
                        }
                    }
                }

            }

            for (int lineNo = 0; lineNo < 3; lineNo++) {
                content << lines[lineNo].str() << std::endl;
            }
        }

        return content.str();
    }

    std::string Board::display() {
        std::stringstream str;

        for (int rowNo = 0; rowNo < 9; rowNo++) {
            if (rowNo == 3 || rowNo == 6) {
                str << "---+---+---" << std::endl;
            }
            for (int colNo = 0; colNo < 9; colNo++) {
                if (colNo == 3 || colNo == 6) {
                    str << "|";
                }
                auto _cell = cell(rowNo, colNo);
                if (!_cell.isSet()) {
                    str << " ";
                }
                else {
                    str << _cell.value();
                }
            }
            str << std::endl;
        }

        return str.str();
    }

    Cell& Board::cell(int rowNo, int colNo) {
        return m_cells[rowNo][colNo];
    }

    void Board::setValue(int rowNo, int colNo, int value) {
        if (!trySetValue(rowNo, colNo, value)) {
            throw std::invalid_argument("Could not set value for cell.");
        }
    }


    bool Board::trySetValue(int rowNo, int colNo, int value) {
        auto & tCell = cell(rowNo, colNo);
        if (!tCell.trySet(value)) {
            return false;
        }
        eachCellInRow(rowNo, [colNo, value](auto & _cell) {
            if (_cell.col() != colNo) {
                _cell.removePossibility(value);
            }
        });
        eachCellInCol(colNo, [rowNo, value](auto & _cell) {
            if (_cell.row() != rowNo) {
                _cell.removePossibility(value);
            }
        });
        eachCellInBox(tCell.box(), [rowNo, colNo, value](auto & _cell) {
            if (_cell.row() != rowNo && _cell.col() != colNo) {
                _cell.removePossibility(value);
            }
        });
        return true;
    }

    bool Board::isSolved() {
        int setCount = 0;
        eachCell([&setCount](auto _cell) {
            if (_cell.isSet()) {
                setCount++;
            }
        });
        return setCount == 81;
    }

    Solver::Solver(Board & board) :
        generator(std::chrono::system_clock::now().time_since_epoch().count())
    {
        auto _board = std::shared_ptr<Board>(new Board(board));
        boards.push([_board]() { return std::optional<std::shared_ptr<Board>>(_board); });
    }

    std::optional<std::shared_ptr<Board>> Solver::next() {
        while (!boards.empty()) {
            auto optBoard = boards.top()();
            boards.pop();
            if (optBoard.has_value()) {
                auto b = *optBoard;
                simplify(*b);
                if (b->isSolved()) {
                    return std::optional<std::shared_ptr<Board>>(b);
                }
                else {
                    auto solveCell = getCellToSolve(*b);
                    if (solveCell.has_value()) {
                        auto _solveCell = std::shared_ptr<Cell>(new Cell(*solveCell));

                        std::vector<int> setValues(solveCell->possibilities().size());
                        std::copy(solveCell->possibilities().begin(), solveCell->possibilities().end(), setValues.begin());
                        std::shuffle(setValues.begin(), setValues.end(), generator);

                        for (auto setValue : setValues) {
                            boards.push([_solveCell, b, setValue]() {
                                auto newBoard = std::shared_ptr<Board>(new Board(*b));
                                if (newBoard->trySetValue(_solveCell->row(), _solveCell->col(), setValue)) {
                                    return std::optional<std::shared_ptr<Board>>(newBoard);
                                }
                                else {
                                    return std::optional<std::shared_ptr<Board>>();
                                }
                            });
                        }
                    }
                }
            }
        }
        return std::optional<std::shared_ptr<Board>>();
    }

    std::optional<Cell> Solver::getCellToSolve(Board & board) {
        std::optional<Cell> solveCell;
        board.eachCell([&solveCell](auto & _cell) {
            if (!_cell.isSet()) {
                if (!solveCell.has_value() || _cell.possibilities().size() < solveCell->possibilities().size()) {
                    solveCell = _cell;
                }
            }
        });
        return solveCell;
    }

    void Solver::simplify(Board & board) {
        while (true) {
            auto spCells = board.getCellsWithSinglePossibility();
            for (auto spCell : spCells) {
                int value = *spCell.possibilities().begin();

                if (!board.trySetValue(spCell.row(), spCell.col(), value)) {
                    // Invalid board!
                    return;
                }
            }
            if (spCells.size() == 0) {
                break;
            }
        }
    }

    // Helper

    Board loadFromFile(const std::string & filePath) {
        std::ifstream input(filePath);
        if (!input.is_open()) {
            throw std::invalid_argument(std::string("Could not open file: ") + filePath);
        }

        Board board;
        std::string line;
        int rowNo = 0;
        while (std::getline(input, line) && rowNo < 9) {
            if (line.length() > 0) {
                for (int colNo = 0, len = line.length(); colNo < len && colNo < 9; colNo++) {
                    char c = line[colNo];
                    if (c >= '1' && c <= '9') {
                        int value = c - '0';
                        board.setValue(rowNo, colNo, value);
                    }
                }
                rowNo++;
            }
        }

        return board;
    }

};
