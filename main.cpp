#include <iostream>
#include <map>
#include <utility>
#include <stack>
#include <limits>

struct PlayerMove {
    int thePlayer;
    std::pair<int,int> theLocation;
    
    PlayerMove(int player, std::pair<int,int> location) {
        thePlayer = player;
        theLocation = location;
    }
};

class BoardSquare {
private:
    // 0 for empty, 1 for player, 2 for player 2
    int value; 

public:
    // constructor to initialize a square as empty
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
    int maxBoardSize;

public:
    // init the board with size
    Board(int size) : maxBoardSize(size) {
        for (int row = 0; row < maxBoardSize; ++row) {
            for (int col = 0; col < maxBoardSize; ++col) {
                // create an empty BoardSquare
                board[{row, col}] = BoardSquare();
            }
        }
    }

    // places a piece
    void placePiece(const std::pair<int, int>& position, int player) {
        board.at(position).setValue(player); // set the value to 1 or 2 based on the player
    }

    int getBoardPlaceValue(const std::pair<int, int>& position) const {
        return board.at(position).getValue();
    }

    // print the board for debugging or display (ASCII representation)
    void printBoard() const {
        for (int row = 0; row < maxBoardSize; ++row) {
            for (int col = 0; col < maxBoardSize; ++col) {
                int value = getBoardPlaceValue({row, col});
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

    // get the max board size
    int getMaxBoardSize() const {
        return maxBoardSize;
    }
};

// checks if a move is valid
bool isPlayerMoveValid(Board &theBoard, int player, std::pair<int, int> location) {
    bool moveValid = true;
    int currentValue = theBoard.getBoardPlaceValue(location);
    if (currentValue != 0) {
        moveValid = false;
    }
    return moveValid;
}

bool areValidMovesLeftForPlayer(
    std::stack<PlayerMove> &gameHistory, 
    Board &theBoard,
    int currentPlayer
) {
    // by default the move is valid
    bool validMoves = true;
    
    // if the board is full
    if (gameHistory.size() >= theBoard.getMaxBoardSize() * theBoard.getMaxBoardSize()) {
        validMoves = false;
    }
    
    // return the boolean
    return validMoves;
}

std::pair<int, int> getPlayerMove(int player, Board &theBoard) {
    // error message to print when we clear the screen
    std::string errorMessage;
    
    // the player's move location
    std::pair<int, int> playerMoveLocation;
    int maxSize = theBoard.getMaxBoardSize();

    while (true) {
        // clear the screen and set cursor to the upper left
        // ANSI escape codes 
        // clear the screen \033[2J
        // reset cursor \033[H
        std::cout << "\033[2J\033[H"; 
        std::cout << "Player " << player << "'s turn.\n";
        theBoard.printBoard();

        // print the error message if there is one
        if (!errorMessage.empty()) {
            std::cout << errorMessage << "\n";
        }

        // prompt for input
        std::cout << "Enter your move (row and column, e.g., '3 4'): ";

        // check if the input is valid
        if (!(std::cin >> playerMoveLocation.first >> playerMoveLocation.second)) {
            errorMessage = "Invalid input. Please enter two numbers.";
            // clear error flags
            std::cin.clear();
            // ignore invalid input
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
            continue;
        }

        // adjust for 0-based indexing
        playerMoveLocation.first -= 1;
        playerMoveLocation.second -= 1;

        // check if the row is within bounds
        if (playerMoveLocation.first < 0 || playerMoveLocation.first >= maxSize) {
            errorMessage = "Invalid row: " + std::to_string(playerMoveLocation.first + 1);
            errorMessage += (playerMoveLocation.first < 0) ? " (too small)." : " (too large).";
            continue;
        }

        // check if the column is within bounds
        if (playerMoveLocation.second < 0 || playerMoveLocation.second >= maxSize) {
            errorMessage = "Invalid column: " + std::to_string(playerMoveLocation.second + 1);
            errorMessage += (playerMoveLocation.second < 0) ? " (too small)." : " (too large).";
            continue;
        }

        // check if the move is valid using isPlayerMoveValid function
        if (!isPlayerMoveValid(theBoard, player, playerMoveLocation)) {
            errorMessage = "Invalid move. Please choose an empty and valid spot.";
            continue;
        }

        // if the move is valid, clear the error message and return the move
        errorMessage.clear();
        return playerMoveLocation;
    }
}

int main() {
    int currentPlayer = 1;
    int maxBoardSize;
    
    std::cout << "Enter board size: ";
    std::cin >> maxBoardSize;
    
    Board theBoard(maxBoardSize);
    std::stack<PlayerMove> gameHistory;
    
    while (true) {
        // check if there are valid moves for this player
        if (!areValidMovesLeftForPlayer(gameHistory, theBoard, currentPlayer)) {
            // if there are no valid moves we break the loop
            break;
        }
        // get a valid move for this player
        std::pair<int, int> playerMove = getPlayerMove(currentPlayer, theBoard);
        
        // place the piece
        theBoard.placePiece(playerMove, currentPlayer);
        
        // add this move to the gameHistory
        gameHistory.push(PlayerMove(currentPlayer, playerMove));
        
        // swap to next player
        currentPlayer = (currentPlayer % 2) + 1;
    }
    return 0;
}
