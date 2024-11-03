#include <iostream>
#include <map>
#include <utility>
#include <stack>
#include <limits>
#include <list>
#include <algorithm>


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
    // init the board with a given size
    Board(int size) : maxBoardSize(size) {
        // check for invalid board
        if (size < 4) {
            throw std::invalid_argument("Board size must be at least 4.");
        }

        // init an empty board
        for (int row = 0; row < maxBoardSize; ++row) {
            for (int col = 0; col < maxBoardSize; ++col) {
                board[{row, col}] = BoardSquare();
            }
        }

        // find center to place starting pieces
        int center = maxBoardSize / 2;

        // set up starting pieces:
        // 0 1
        // 1 0
        try {
            board.at({center - 1, center - 1}).setPiece(1);  // Top-left of center with Player X (1)
            board.at({center - 1, center}).setPiece(2);      // Top-right of center with Player O (2)
            board.at({center, center - 1}).setPiece(2);      // Bottom-left of center with Player O (2)
            board.at({center, center}).setPiece(1);          // Bottom-right of center with Player X (1)
        } catch (const std::exception& e) {
            std::cerr << "Error initializing the board's center pieces: " << e.what() << std::endl;
        }
    }

    // places a piece
    void placePiece(
        const std::pair<int, int>& position, 
        int player, 
        const std::list<std::pair<int, int>>& toFlip
    ) {
        // set the value to 1 or 2 based on the player
        board.at(position).setPiece(player); 
        
        // flip each piece in the list
        for (const auto& flipPosition : toFlip) {
            board.at(flipPosition).flipPiece();
        }
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
        
        // lambda empty check for find_if
        auto isSquareEmpty = [this](const auto& entry) {
            return entry.second.isEmpty();
        };
        
        // get iterator for the start of the board
        auto it = board.begin();

        // iterate using find_if so we dont enter the loop
        // until iterator reached board.end
        while((it = std::find_if(it, board.end(), isSquareEmpty)) != board.end()) {
            // current position we check (row, col)
            std::pair<int, int> position = it->first;
            
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
            
            // increment the iterator
            it++;
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

std::pair<int, int> getPlayerMove(
    int player, Board &theBoard, 
    const std::map<std::pair<int, int>, 
    std::list<std::pair<int, int>>>& validMoves
) {
    // error message to print when we clear the screen
    std::string errorMessage;
    std::pair<int, int> playerMoveLocation;
    int maxSize = theBoard.getMaxBoardSize();

    while (true) {
        // clear the screen and set cursor to the upper left
        // ANSI escape codes
        // clear the screen \033[2J
        // reset cursor \033[H
        std::cout << "\033[2J\033[H"; 
        // print X or O for the player
        std::cout << "Player " << (player == 1 ? "X" : "O") << "'s turn.\n";
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

        // check if the row and column are within bounds
        if (playerMoveLocation.first < 0 || playerMoveLocation.first >= maxSize || 
            playerMoveLocation.second < 0 || playerMoveLocation.second >= maxSize) {
            errorMessage = "Move out of bounds. Please enter a valid position.";
            continue;
        }

        // check if the move is in the validMoves map
        if (validMoves.find(playerMoveLocation) == validMoves.end()) {
            errorMessage = "Invalid move. Please choose a position with available flips.";
            continue;
        }

        // clear error message and return the valid move
        errorMessage.clear();
        return playerMoveLocation;
    }
}


int getBoardSize() {
    int size;
    do {
        std::cout << "Enter board size (minimum 4): ";
        std::cin >> size;
        if (size < 4) {
            std::cout << "Board size must be at least 4. Please try again.\n";
        }
    } while (size < 4);
    return size;
}

int main() {
    // starting player
    int currentPlayer = 1;
    // get the board size
    int maxBoardSize = getBoardSize();  
    
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
        std::pair<int, int> playerMove = getPlayerMove(currentPlayer, theBoard, validMoves);
        
        // place the piece, pass pieces to be flipped
        theBoard.placePiece(playerMove, currentPlayer, validMoves.at(playerMove));
        
        // add this move to the gameHistory
        gameHistory.push(PlayerMove(currentPlayer, playerMove));
        
        // swap to next player
        currentPlayer = (currentPlayer % 2) + 1;
    }
    theBoard.showWinner();
    return 0;
}
