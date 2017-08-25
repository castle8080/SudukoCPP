#include "Suduko.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace Suduko {

    //========================================================================
    // Class: Cell
    //========================================================================

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

    void Cell::unset() {
        m_value = 0;
        m_possibilities = std::set<int>{ 1,2,3,4,5,6,7,8,9 };
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

    const int Cell::id() {
        return m_row * 9 + m_col;
    }

    const int Cell::value() {
        return m_value;
    }

    void Cell::removePossibility(int _value) {
        m_possibilities.erase(_value);
    }

    void Cell::addPossibility(int _value) {
        if (!isSet()) {
            m_possibilities.insert(_value);
        }
    }

    const std::set<int>& Cell::possibilities() {
        return m_possibilities;
    }

    void Cell::clear() {
        m_value = 0;
        m_possibilities = std::set<int>{ 1,2,3,4,5,6,7,8,9 };
    }

    //========================================================================
    // Class: Board
    //========================================================================

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
        eachRelatedCell(rowNo, colNo, [value](auto & _cell) {
            _cell.removePossibility(value);
        });
        return true;
    }

    void Board::unset(int rowNo, int colNo) {
        // TODO: There seems to be some bugs in this method or one of the methods it calls.
        auto & _cell = cell(rowNo, colNo);
        if (!_cell.isSet()) {
            return;
        }
        _cell.unset();
        recomputePossibilities(rowNo, colNo);

        eachRelatedCell(rowNo, colNo, [this, rowNo, colNo](auto & relatedCell) {
            if (!relatedCell.isSet()) {
                recomputePossibilities(rowNo, colNo);
            }
        });
    }

    void Board::recomputePossibilities(int rowNo, int colNo) {
        auto & _cell = cell(rowNo, colNo);
        if (!_cell.isSet()) {
            eachRelatedCell(rowNo, colNo, [&_cell](auto & relatedCell) {
                if (relatedCell.isSet()) {
                    _cell.removePossibility(relatedCell.value());
                }
            });
        }
    }

    int Board::cellSetCount() {
        int setCount = 0;
        eachCell([&setCount](auto _cell) {
            if (_cell.isSet()) {
                setCount++;
            }
        });
        return setCount;
    }

    bool Board::isSolved() {
        return cellSetCount() == 81;
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

    //========================================================================
    // Class: Solver
    //========================================================================

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
                        pushSolutionAttempts(b, *solveCell);
                    }
                }
            }
        }
        return std::optional<std::shared_ptr<Board>>();
    }

    void Solver::pushSolutionAttempts(std::shared_ptr<Board> board, Cell & solveCell) {
        auto solveCellPtr = std::shared_ptr<Cell>(new Cell(solveCell));

        std::vector<int> setValues(solveCellPtr->possibilities().size());
        std::copy(solveCellPtr->possibilities().begin(), solveCellPtr->possibilities().end(), setValues.begin());
        std::shuffle(setValues.begin(), setValues.end(), generator);

        for (auto setValue : setValues) {
            boards.push([solveCellPtr, board, setValue]() {
                auto newBoard = std::shared_ptr<Board>(new Board(*board));
                if (newBoard->trySetValue(solveCellPtr->row(), solveCellPtr->col(), setValue)) {
                    return std::optional<std::shared_ptr<Board>>(newBoard);
                }
                else {
                    return std::optional<std::shared_ptr<Board>>();
                }
            });
        }
        if (setValues.size() > 0) {
            //std::cout << "Pushed new solution search nodes: added=" << setValues.size() << " current search size=" << boards.size() << std::endl;
        }
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
            switch (runSimplificationRules(board)) {
            case Solver::Invalid:
                return;
            case Solver::NoAction:
                return;
            case Solver::Updated:
                break;
            }
        }
    }

    Solver::RuleResult Solver::runSimplificationRules(Board & board) {
        // TODO: make this a constant.
        std::vector<Solver::Rule> rules{
            &Solver::simplificationRuleSinglePossibility,
            &Solver::simplificationRuleOnlyPossibility,
            &Solver::simplificationRuleBoxCheck,
            &Solver::simplificationRuleSharedPossibilities
        };
        for (auto rule : rules) {
            switch ((this->*rule)(board)) {
            case Solver::Invalid:
                return Solver::Invalid;
            case Solver::Updated:
                // start while loop over.
                return Solver::Updated;
            case Solver::NoAction:
                // continue rules
                break;
            }
        }
        // No updates made this round.
        return Solver::NoAction;
    }

    Solver::RuleResult Solver::simplificationRuleSinglePossibility(Board & board) {
        auto spCells = board.getCellsWithSinglePossibility();
        for (auto spCell : spCells) {
            int value = *spCell.possibilities().begin();
            if (!board.trySetValue(spCell.row(), spCell.col(), value)) {
                return Solver::Invalid;
            }
        }
        return (spCells.size() == 0) ? Solver::NoAction : Solver::Updated;
    }

    Solver::RuleResult Solver::simplificationRuleOnlyPossibility(Board & board) {
        std::map<std::tuple<Board::Region, int, int>, std::vector<std::shared_ptr<Cell>>> tracking;

        board.eachCell([&tracking](Cell & cell) {
            if (!cell.isSet()) {
                std::vector<std::tuple<Board::Region, int>> regions{ 
                    std::tuple<Board::Region, int>(Board::Row, cell.row()),
                    std::tuple<Board::Region, int>(Board::Col, cell.col()),
                    std::tuple<Board::Region, int>(Board::Box, cell.box())
                };
                for (auto setValue : cell.possibilities()) {
                    for (auto region : regions) {
                        auto k = std::make_tuple(std::get<0>(region), std::get<1>(region), setValue);
                        auto elemIter = tracking.find(k);
                        if (elemIter == tracking.end()) {
                            tracking[k] = std::vector<std::shared_ptr<Cell>>();
                        }
                        tracking[k].push_back(std::shared_ptr<Cell>(new Cell(cell)));
                    }
                }
            }
        });

        int updateCount = 0;
        for (auto & pair : tracking) {
            if (pair.second.size() == 1) {
                auto cell = pair.second[0];
                auto setValue = std::get<2>(pair.first);
                if (!board.trySetValue(cell->row(), cell->col(), setValue)) {
                    return Solver::Invalid;
                }
                updateCount++;
            }
        }

        return (updateCount == 0) ? Solver::NoAction : Solver::Updated;
    }

    Solver::RuleResult Solver::simplificationRuleSharedPossibilities(Board & board) {
        std::map<std::tuple<Board::Region, int, std::set<int>>, std::set<int>> tracking;

        board.eachCell([&tracking](Cell & cell) {
            if (!cell.isSet()) {
                std::vector<std::tuple<Board::Region, int>> regions{
                    std::tuple<Board::Region, int>(Board::Row, cell.row()),
                    std::tuple<Board::Region, int>(Board::Col, cell.col()),
                    std::tuple<Board::Region, int>(Board::Box, cell.box())
                };
                for (auto & pair : regions) {
                    auto key = std::make_tuple(
                        std::get<0>(pair),
                        std::get<1>(pair),
                        cell.possibilities());

                    auto iter = tracking.find(key);
                    if (iter == tracking.end()) {
                        tracking[key] = std::set<int>();
                    }
                    tracking[key].insert(cell.id());
                }
            }
        });

        int updateCount = 0;
        for (auto & pair : tracking) {
            auto region = std::get<0>(pair.first);
            auto regionValue = std::get<1>(pair.first);

            if (std::get<2>(pair.first).size() == pair.second.size()) {
                board.eachCellInRegion(region, regionValue, [&board, &updateCount, &pair](Cell & cell) {
                    if (!cell.isSet() && pair.second.find(cell.id()) == pair.second.end()) {
                        for (auto pValue : std::get<2>(pair.first)) {
                            if (cell.possibilities().find(pValue) != cell.possibilities().end()) {
                                cell.removePossibility(pValue);
                                updateCount++;
                            }
                        }
                    }
                });
            }
        }

        return (updateCount == 0) ? Solver::NoAction : Solver::Updated;
    }

    Solver::RuleResult Solver::simplificationRuleBoxCheck(Board & board) {
        int updateCount = 0;

        for (int boxNo = 0; boxNo < 9; boxNo++) {
            std::unordered_map<int, std::set<int>> valRows;
            std::unordered_map<int, std::set<int>> valCols;
            board.eachCellInBox(boxNo, [&valRows, &valCols](auto & cell) {
                if (!cell.isSet()) {
                    for (auto pValue : cell.possibilities()) {
                        if (valRows.find(pValue) == valRows.end()) {
                            valRows[pValue] = std::set<int>();
                        }
                        if (valCols.find(pValue) == valCols.end()) {
                            valCols[pValue] = std::set<int>();
                        }
                        valRows[pValue].insert(cell.row());
                        valCols[pValue].insert(cell.col());
                    }
                }
            });

            for (auto & pair : valRows) {
                if (pair.second.size() == 1) {
                    board.eachCellInRow(*pair.second.begin(), [&updateCount, &pair, boxNo](Cell & cell) {
                        if (!cell.isSet() && cell.box() != boxNo) {
                            if (cell.possibilities().find(pair.first) != cell.possibilities().end()) {
                                cell.removePossibility(pair.first);
                                updateCount++;
                            }
                        }
                    });
                }
            }
            for (auto & pair : valCols) {
                if (pair.second.size() == 1) {
                    board.eachCellInCol(*pair.second.begin(), [&updateCount, &pair, boxNo](Cell & cell) {
                        if (!cell.isSet() && cell.box() != boxNo) {
                            if (cell.possibilities().find(pair.first) != cell.possibilities().end()) {
                                cell.removePossibility(pair.first);
                                updateCount++;
                            }
                        }
                    });
                }
            }
        }

        return (updateCount == 0) ? Solver::NoAction : Solver::Updated;
    }

    //========================================================================
    // Class: Generator
    //========================================================================

    Generator::Generator() :
        generator(std::chrono::system_clock::now().time_since_epoch().count())
    {
        Board empty;
        Solver solver(empty);
        auto solution = solver.next();
        if (!solution.has_value()) {
            throw std::exception("Could not generate a new Suduko board.");
        }
        for (int i = 0; i < 81; i++) {
            ids.push_back(i);
        }
        std::shuffle(ids.begin(), ids.end(), generator);
        boards.push(std::make_tuple(*solution, 0));
    }

    std::optional<std::shared_ptr<Board>> Generator::generate() {
        while (!boards.empty()) {
            auto boardAndIndex = boards.top();
            boards.pop();
            auto board = std::get<0>(boardAndIndex);
            int index = std::get<1>(boardAndIndex);

            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            if (hasSingleSolution(board)) {
                std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> time_span = t2 - t1;
                if (time_span.count() >= 1.0) {
                    //std::cout << "Example board: (" << board->cellSetCount() << ")" << " check in " << time_span.count() << " ms." << std::endl;
                    //std::cout << board->display() << std::endl;
                }

                if (index < ids.size()) {
                    boards.push(std::make_tuple(board, index + 1));
                    int cellId = ids[index];
                    auto newBoard = std::shared_ptr<Board>(new Board());
                    board->eachCell([&newBoard, cellId](auto & _cell) {
                        if (_cell.isSet() && _cell.id() != cellId) {
                            newBoard->setValue(_cell.row(), _cell.col(), _cell.value());
                        }
                    });
                    boards.push(std::make_tuple(newBoard, index + 1));
                }

                return std::optional<std::shared_ptr<Board>>(board);
            }
        }
        return std::optional<std::shared_ptr<Board>>();
    }

    std::optional<std::shared_ptr<Board>> Generator::generate(int setSize) {
        while (true) {
            auto boardOpt = generate();
            if (boardOpt.has_value()) {
                auto board = *boardOpt;
                if (board->cellSetCount() == setSize) {
                    return std::optional<std::shared_ptr<Board>>(board);
                }
            }
            else {
                break;
            }
        }
        return std::optional<std::shared_ptr<Board>>();
    }

    bool Generator::hasSingleSolution(std::shared_ptr<Board> board) {
        Solver solver(*board);
        auto sol1 = solver.next();
        if (!sol1.has_value()) {
            return false;
        }
        auto sol2 = solver.next();
        return !sol2.has_value();
    }

    //========================================================================
    // Standalone Functions
    //========================================================================

    std::shared_ptr<Board> loadFromFile(const std::string & filePath) {
        std::ifstream input(filePath);
        if (!input.is_open()) {
            throw std::invalid_argument(std::string("Could not open file: ") + filePath);
        }

        auto board = std::shared_ptr<Board>(new Board());
        std::string line;
        int rowNo = 0;
        while (std::getline(input, line) && rowNo < 9) {
            // Get rid of bogus characters.
            line.erase(std::remove_if(line.begin(), line.end(), [] (auto c) {
                return c != ' ' && (c < '1' || c > '9');
            }), line.end());
            if (line.length() > 0) {
                for (int colNo = 0, len = line.length(); colNo < len && colNo < 9; colNo++) {
                    char c = line[colNo];
                    if (c >= '1' && c <= '9') {
                        int value = c - '0';
                        board->setValue(rowNo, colNo, value);
                    }
                }
                rowNo++;
            }
        }
        return board;
    }
};
