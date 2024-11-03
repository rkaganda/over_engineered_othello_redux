#include <iostream>
#include <map>
#include <utility>
#include <stack>
#include <limits>
#include <list>


// this list is used to store the directions that 
// we check for anchor points
const std::list<std::pair<int, int>> directions = {
    {-1, 0},  // north
    {1, 0},   // south
    {0, -1},  // west
    {0, 1},   // east
    {-1, -1}, // northwest
    {-1, 1},  // northeast
    {1, -1},  // southwest
    {1, 1}    // southeast
};

// this struct is used to store player moves
// in the stack of player history
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
    // 0 for empty
    // 1 for player X
    // 2 for player 0
    int value; 

public:
    // constructor to initialize a square as empty
    BoardSquare() {
        value = 0;
    } 

    // set the piece, throw an error if the square is already occupied
    void setPiece(int player) {
        if (value != 0) {
            throw std::runtime_error("square is already occupied.");
        }
        value = player;
    }

    // flip the piece, allowed only if there is already a piece present
    void flipPiece() {
        if (value == 0) {
            throw std::runtime_error("cannot flip an empty square.");
        }
        value = (value == 1) ? 2 : 1; // flip between 1 and 2
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
        board.at(position).setPiece(player); // set the value to 1 or 2 based on the player
    }

    int getBoardPlaceValue(const std::pair<int, int>& position) const {
        return board.at(position).getValue();
    }
    
    // use the provided position to check if
    // there are pieces to flip
    // populates toFlip with the location of these
    // returns true if there are pieces to flip
    bool findFlippablePieces(
        std::pair<int, int> position,
        int player,
        std::pair<int, int> direction,
        std::list<std::pair<int, int>>& toFlip
    ) const {
        bool piecesToFlip = false;
        // clear the flip list
        toFlip.clear();
        
        // get value of opponent
        int opponent = (player == 1) ? 2 : 1;

        // move to the next position in the specified direction
        position.first += direction.first;
        position.second += direction.second;

        // traverse the board in that direction
        // until we reach the end of the board
        while (position.first >= 0 && position.first < maxBoardSize &&
               position.second >= 0 && position.second < maxBoardSize) {

            int value = board.at({position.first, position.second}).getValue();

            if (value == opponent) {
                // add opponent's piece to the flip path
                toFlip.push_back(position);
            } else if (value == player) {
                // found an anchor piece of the same color
                // return the negation of toFlip 
                // so if there are pieces to flip this returns true
                piecesToFlip = !toFlip.empty(); // Only valid if there are pieces to flip
                break;
            } else {
                // Empty square encountered, no anchor in this direction
                break;
            }

            // Move to the next position in the specified direction
            position.first += direction.first;
            position.second += direction.second;
        }

        // No anchor piece found in this direction
        if (!piecesToFlip) {
            toFlip.clear();   
        }
        return piecesToFlip;
    }
    
    // generates all valid moves for a given player based on Othello rules
    // if a move is valid its stored in a map
    // the location is used as the key, and the value is a list of the pieces that get flipped
    // this way if the player chooses that move we don't have to recalcuate the flipped pieces
    std::map<std::pair<int, int>, std::list<std::pair<int, int>>> getValidMoves(int player) const {
        // map to store valid moves and flippable pieces
        std::map<std::pair<int, int>, std::list<std::pair<int, int>>> validMovesMap;  

        // iterate over each position on the board using an iterator
        for (auto it = board.begin(); it != board.end(); ++it) {
            // current position (row, col)
            std::pair<int, int> position = it->first;

            // check if the square is empty
            if (it->second.isEmpty()) {
                // list to accumulate flippable pieces in all directions
                std::list<std::pair<int, int>> totalFlippablePieces;  

                // check each direction for flippable pieces
                for (const auto& direction : directions) {
                    // temporary list for the current direction
                    std::list<std::pair<int, int>> toFlip;  
                    if (findFlippablePieces(position, player, direction, toFlip)) {
                        // add flippable pieces in this direction to the total list
                        totalFlippablePieces.insert(totalFlippablePieces.end(), toFlip.begin(), toFlip.end());
                    }
                }

                // if there are any flippable pieces, store the move in the map
                if (!totalFlippablePieces.empty()) {
                    validMovesMap[position] = totalFlippablePieces;
                }
            }
        }
        return validMovesMap;  // return the map of valid moves with their flippable pieces
    }
    
    bool areValidMovesLeftForPlayer(
        std::stack<PlayerMove> &gameHistory, 
        int currentPlayer
    ) {
        // by default the move is valid
        bool validMoves = true;

        // if the board is full
        if (gameHistory.size() >= maxBoardSize * maxBoardSize) {
            validMoves = false;
        }

        // return the boolean
        return validMoves;
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
    
    // counts the number of player tokens then displays the winner
    void showWinner() const {
        // store for the player counts
        int countX = 0;
        int countO = 0;

        // iterate over the board to count tokens
        for (auto it = board.begin(); it != board.end(); ++it) {
            // the value of the player at that location
            int value = it->second.getValue();
            // if value is player X
            if (value == 1) { 
                countX++;  // Player X
            } else if (value == 2) {
                countO++;  // Player O
            }
        }
        
        this->printBoard();

        // display player X tokens
        std::cout << "X: ";
        for (int i = 0; i < countX; ++i) {
            std::cout << ". ";
        }
        std::cout << std::endl;

        // display player O tokens
        std::cout << "O: ";
        for (int i = 0; i < countO; ++i) {
            std::cout << ". ";
        }
        std::cout << std::endl;

        // display the winner or draw
        if (countX > countO) {
            std::cout << "Player X wins (" << countX << "-" << countO << ")\n";
        } else if (countO > countX) {
            std::cout << "Player O wins (" << countO << "-" << countX << ")\n";
        } else {
            std::cout << "It's a draw (" << countX << "-" << countO << ")\n";
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
        std::map<std::pair<int, int>, std::list<std::pair<int, int>>> validMoves = theBoard.getValidMoves(currentPlayer);
        if (validMoves.size()==0) {
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
    theBoard.showWinner();
    return 0;
}
