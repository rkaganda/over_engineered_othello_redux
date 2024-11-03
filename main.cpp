#include <iostream>
#include <map>
#include <utility> 

class BoardSquare {
private:
    // 0 for empty, 1 for player, 2 for player 2
    int value; 

public:
    // Constructor to initialize a square as empty
    BoardSquare() {
        value = 0;
    }

    // set the value
    void setValue(int player) {
        value = player;
    }

    // get the value
    int getValue() const {
        return value;
    }

    // check if the square is empty
    bool isEmpty() const {
        return value == 0;
    }
};


class Board {
private:
    // map holds the BoardSquares
    std::map<std::pair<int, int>, BoardSquare> board; 

public:
    // init the board
    Board() {
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                // create an empty BoardSquare
                board[{row, col}] = BoardSquare(); 
            }
        }
    }

    // places a piece 
    void placePiece(int row, int col, int player) {
        board.at({row, col}).setValue(player); // Set the value to 1 or 2 based on the player
    }
    
    int getBoardPlaceValue(int row, int col) const {
        return board.at({row, col}).getValue();
    }

    // Print the board for debugging or display (ASCII representation)
    void printBoard() const {
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                int value = getBoardPlaceValue(row, col);
                if (value == 0)
                    std::cout << ". ";
                else if (value == 1)
                    std::cout << "X ";
                else if (value == 2)
                    std::cout << "O ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Board theBoard = Board();
    theBoard.printBoard();
    return 0;
}