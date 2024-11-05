/***********************************************************************************
 * Over-Engineered Othello
 * 
 * Author: Rukundo Kaganda (rkaganda@gmail.com)
 * Date: 11/4/2024
 * 
 * Description:
 * - An implementation of Othello with for 2 players 
 *   or 1 player and "AI" opponent,
 *   with varible board size and, optional move assistance.
 * 
 ***********************************************************************************/
#include <iostream>
#include <map>
#include <utility>
#include <stack>
#include <limits>
#include <list>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>


// list of directions used to check for flippable pieces around a position on the board
// each pair represents a direction (e.g., {-1, 0} is north, {1, 1} is southeast)
// used to check for anchor points
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

// struct to store player moves, 
// used in the game history stack
struct PlayerMove {
    int thePlayer;
    std::pair<int,int> theLocation;
    
    PlayerMove(int player, std::pair<int,int> location) {
        thePlayer = player;
        theLocation = location;
    }
};

// class representing a single square on the board
// which can be empty or occupied by a player
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

    // sets a piece for the specified player, throws an error if the square is already occupied
    //
    // parameters:
    // int player - the player placing a piece (1 for 'X', 2 for 'O')
    //
    // throws:
    // std::runtime_error if the square is already occupied
    void setPiece(int player) {
        if (value != 0) {
            throw std::runtime_error("square is already occupied.");
        }
        value = player;
    }

    // flips the piece on the square (from 'X' to 'O' or vice versa),
    // throws an error if the square is empty
    //
    // throws:
    // std::runtime_error if the square is empty
    void flipPiece() {
        if (value == 0) {
            throw std::runtime_error("cannot flip an empty square.");
        }
        value = (value == 1) ? 2 : 1; // flip between 1 and 2
    }

    // retrieves the value of the square
    //
    // returns:
    // int - the value of the square (0 for empty, 1 for 'X', 2 for 'O')
    int getValue() const {
        return value;
    }

    // checks if the square is empty
    //
    // returns:
    // bool - true if the square is empty, false otherwise
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
    // initializes the board with a given size
    // sets up an empty board, ensuring the size is at least 4x4, and places starting pieces at the center
    //
    // parameters:
    // int size - the size of the board (must be at least 4)
    //
    // throws:
    // std::invalid_argument if the board size is less than 4
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

    // places a piece on the board at the specified location for a given player
    // flips the pieces in the set of positions provided
    //
    // parameters:
    // const std::pair<int, int>& position - the position to place the piece
    // int player - the player number (1 for 'X', 2 for 'O')
    // const std::set<std::pair<int, int>>& toFlip - the set of positions to flip after placing the piece
    //
    // returns:
    // void - does not return a value
    void placePiece(
        const std::pair<int, int>& position, 
        int player, 
        const std::set<std::pair<int, int>>& toFlip
    ) {
        // set the value to 1 or 2 based on the player
        board.at(position).setPiece(player); 
        
        // flip each piece in the list
        std::for_each(toFlip.begin(), toFlip.end(), [this](const auto& flipPosition) {
            board.at(flipPosition).flipPiece();
        });
    }
    
    // retrieves the value at a specified board position
    //
    // parameters:
    // const std::pair<int, int>& position - the position on the board
    //
    // returns:
    // int - the value at the specified position (0 for empty, 1 for 'X', 2 for 'O')
    int getBoardPlaceValue(const std::pair<int, int>& position) const {
        return board.at(position).getValue();
    }
    
    // checks if there are pieces to flip in a given direction for a move by the specified player
    // if there are, populates 'toFlip' with the locations of those pieces
    //
    // parameters:
    // std::pair<int, int> position - the starting position for checking flips
    // int player - the player number (1 for 'X', 2 for 'O')
    // std::pair<int, int> direction - the direction to check for flippable pieces
    // std::set<std::pair<int, int>>& toFlip - a set to populate with flippable positions
    //
    // returns:
    // bool - true if there are pieces to flip, false otherwise
    bool findFlippablePieces(
        std::pair<int, int> position,
        int player,
        std::pair<int, int> direction,
        std::set<std::pair<int, int>>& toFlip
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
                toFlip.insert(position);
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
    // stores each valid move and its corresponding flippable pieces in a map
    //
    // parameters:
    // int player - the player number (1 for 'X', 2 for 'O')
    //
    // returns:
    // std::map<std::pair<int, int>, std::set<std::pair<int, int>>> - a map of valid moves
    // where each key is a board position and the associated value is a set of positions that would be flipped
    std::map<std::pair<int, int>, std::set<std::pair<int, int>>> getValidMoves(int player) const {
        // map to store valid moves and flippable pieces
        std::map<std::pair<int, int>, std::set<std::pair<int, int>>> validMovesMap;  
        
        // lambda empty check for find_if
        auto isSquareEmpty = [this](const auto& entry) {
            return entry.second.isEmpty();
        };
        
        // get iterator for the start of the board
        auto it = board.begin();

        // iterate using find_if so we dont enter the loop
        // if the square is empty
        // loops until iterator reached board.end
        while((it = std::find_if(it, board.end(), isSquareEmpty)) != board.end()) {
            // current position we check (row, col)
            std::pair<int, int> position = it->first;
            
            // set to accumulate flippable pieces in all directions
            std::set<std::pair<int, int>> totalFlippablePieces;

            // check each direction for flippable pieces
            for (const auto& direction : directions) {
                // temporary set for the current direction
                std::set<std::pair<int, int>> toFlip;
                if (findFlippablePieces(position, player, direction, toFlip)) {
                    // add flippable pieces in this direction to the total list
                    totalFlippablePieces.insert(toFlip.begin(), toFlip.end());
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
    
    // checks if there are valid moves left for the current player
    //
    // parameters:
    // std::stack<PlayerMove>& gameHistory - a stack representing the history of moves in the game
    // int currentPlayer - the current player's number (1 for 'X', 2 for 'O')
    //
    // returns:
    // bool - true if there are valid moves left, false otherwise
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

    // prints the board in ASCII format, showing 'X', 'O', and '.' for empty spaces
    //
    // parameters:
    // none
    //
    // returns:
    // void - does not return a value
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
    
    // counts the tokens for each player and displays the winner or if the game is a draw
    //
    // parameters:
    // none
    //
    // returns:
    // void - does not return a value
    void showWinner() const {
        // iterate over the board player TWICE to count X and O
        // NOTE: its more efficent to use for here iterate once
        // but count is used due to project requirements
        int countX = std::count_if(board.begin(), board.end(), [](const auto& pair) {
            return pair.second.getValue() == 1;
        });
        int countO = std::count_if(board.begin(), board.end(), [](const auto& pair) {
            return pair.second.getValue() == 2;
        });
        
        /*
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
        */
        
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

    // retrieves the maximum size of the board
    //
    // parameters:
    // none
    //
    // returns:
    // int - the maximum size of the board
    int getMaxBoardSize() const {
        return maxBoardSize;
    }
};


// prints all possible moves and their corresponding flip counts
// displays each valid move location and the number of pieces that would be flipped
//
// parameters:
// std::map<std::pair<int, int>, std::set<std::pair<int, int>>> validMoves
//                                - a map of valid moves where each key is a board position 
//                                  and the associated value is a set of positions that would 
//                                  be flipped by that move
//
// returns:
// void - does not return a value

void printPossibleMoves(std::map<std::pair<int, int>, std::set<std::pair<int, int>>> validMoves) {
    for (const auto& move : validMoves) {
        std::pair<int, int> key = move.first;
        int setSize = move.second.size();
        std::cout << "Move (" << key.first+1 << ", " << key.second+1 << ") has " << setSize << " possible flip(s).\n";
    }
}


// checks if the player's move is valid by ensuring the chosen location is empty
//
// parameters:
// Board& theBoard                 - reference to the game board object
// int player                       - the player number (1 for 'X', 2 for 'O')
// std::pair<int, int> location     - the coordinates of the move location on the board
//
// returns:
// bool                             - true if the move is valid (location is empty), 
//                                    false otherwise

bool isPlayerMoveValid(Board &theBoard, int player, std::pair<int, int> location) {
    bool moveValid = true;
    int currentValue = theBoard.getBoardPlaceValue(location);
    if (currentValue != 0) {
        moveValid = false;
    }
    return moveValid;
}


// gets the player's move, ensuring it is within bounds and valid
// repeatedly prompts for input until a valid move is entered
// clears the screen, displays the board, and provides move assistance if enabled
//
// parameters:
// int player                       - the player number (1 for 'X', 2 for 'O')
// Board& theBoard                  - reference to the game board object
// const std::map<std::pair<int, int>, std::set<std::pair<int, int>>>& validMoves
//                                   - map of valid moves, with each key as a board 
//                                     position and the associated value as positions to flip
// std::string statusMessage          - message to display from the previous turn
// bool moveAssistOn                - indicates if move assistance is enabled
//
// returns:
// std::pair<int, int>              - coordinates of the player's chosen move (row, column)

std::pair<int, int> getPlayerMove(
    int player, Board &theBoard, 
    const std::map<std::pair<int, int>, 
    std::set<std::pair<int, int>>>& validMoves,
    std::string statusMessage,
    bool moveAssistOn
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
        std::cout << statusMessage << "\n";
        // print X or O for the player
        std::cout << "Player " << (player == 1 ? "X" : "O") << "'s turn.\n";
        theBoard.printBoard();
        
        if (moveAssistOn) {
            printPossibleMoves(validMoves);
        }

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


// selects an ai move from the list of valid moves, aiming for the move with the highest flips
// has a 50/50 chance of choosing either the best move or a random valid move
//
// parameters:
// const std::map<std::pair<int, int>, std::set<std::pair<int, int>>>& validMoves
//                                 - a map of valid moves where each key is a board position 
//                                   and the associated value is a set of positions that would 
//                                   be flipped by that move
//
// returns:
// std::pair<int, int>             - the coordinates (row, column) of the selected ai move

std::pair<int, int> getAIMove(const std::map<std::pair<int, int>, std::set<std::pair<int, int>>>& validMoves) {
    // list to store moves with the count of pieces flipped
    std::list<std::pair<int, std::pair<int, int>>> moves;
    
    // the move
    std::pair<int, int> aiMove;
    
    // insert each move with its flip count into the list
    for (const auto& move : validMoves) {
        // count of pieces to flip for this move
        int flipCount = move.second.size(); 
        // push the move and flip count to the list
        moves.push_back({flipCount, move.first});
    }

    // sort the list in descending order based on flip count (largest flips in front)
    moves.sort([](const std::pair<int, std::pair<int, int>>& a, const std::pair<int, std::pair<int, int>>& b) {
        // sort by flip count in descending order
        return a.first > b.first; 
    });

    // priority queue to store sorted moves (best move at the top)
    std::priority_queue<std::pair<int, std::pair<int, int>>> moveQueue;

    // push sorted moves into the priority queue
    for (const auto& move : moves) {
        moveQueue.push(move);
    }

    // initialize random seed
    std::srand(std::time(0));

    // 50/50 to either pick the move frop the top of the prio queue
    /// or choose a move using a random iterator
    if (std::rand() % 2 == 0) {
        // pick a random move using a random iterator
        auto randomIt = std::next(moves.begin(), std::rand() % moves.size());
        aiMove = randomIt->second;
    } else {
        // pick the move from the top of the priority queue
        aiMove = moveQueue.top().second;
    }
    
    // return the move
    return aiMove;
}


// prompts the user to enter the board size with a minimum value of 4
// repeatedly asks for input until the user enters a valid size (4 or greater)
//
// parameters:
// none
//
// returns:
// int - the board size entered by the user (an integer 4 or greater)

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


// prompts the user to enter the number of players (either 1 or 2)
// repeatedly asks for input until the user enters 1 or 2
//
// parameters:
// none
//
// returns:
// int - the number of players entered by the user (either 1 or 2)

int getPlayerCount() {
    int playerCount;
    do {
        std::cout << "How many players (1 or 2): ";
        std::cin >> playerCount;
        if (playerCount < 1 || playerCount > 2) {
            std::cout << "Either 1 or 2 players. Please try again.\n";
        }
    } while (playerCount < 1 || playerCount > 2);
    return playerCount;
}


// prompts the user to enable or disable the move assist feature
// repeatedly asks for input until the user enters 'y' (yes) or 'n' (no)
//
// parameters:
// none
//
// returns:
// bool - true if the user chooses 'y' to enable move assist, 
//        false if the user chooses 'n' to disable it

bool getMoveAssist() {
    char choice;
    do {
        std::cout << "Do you want move assist on? (Y/N): ";
        std::cin >> choice;
        // convert input to uppercase to compare
        choice = std::toupper(choice);
        if (choice != 'Y' && choice != 'N') {
            std::cout << "Please enter 'Y' or 'N'.\n";
        }
    } while (choice != 'Y' && choice != 'N');
    return choice == 'Y';
}


// prompts the user to decide if they want to play again
// repeatedly asks for input until the user enters 'y' (yes) or 'n' (no)
//
// parameters:
// none
//
// returns:
// bool - true if the user chooses 'y' to play again,
//        false if the user chooses 'n' to stop playing

bool getPlayAgain() {
    char choice;
    do {
        std::cout << "Do you want to play again? (Y/N): ";
        std::cin >> choice;
        // convert input to uppercase for comparison
        choice = std::toupper(choice);
        if (choice != 'Y' && choice != 'N') {
            std::cout << "Please enter 'Y' or 'N'.\n";
        }
    } while (choice != 'Y' && choice != 'N');
    return choice == 'Y';
}


// prints the rules of Othello
// provides players with an overview of the game objectives, piece placement,
// flipping mechanics, and victory conditions
//
// parameters:
// none
//
// returns:
// void - does not return a value

void printRules() {
    std::cout << "\033[2J\033[H"; // Clear screen and reset cursor
    std::cout << "Othello Game Rules:\n\n";
    std::cout << "1. Objective:\n";
    std::cout << "   - The goal is to have the most pieces of your color on the board when no moves remain.\n";
    std::cout << "   - Player X ('X') and Player O ('O') alternate turns, placing pieces on the board.\n\n";
    std::cout << "2. Piece Placement:\n";
    std::cout << "   - On your turn, place a piece on an empty square to trap one or more of your opponent's pieces\n";
    std::cout << "     in any direction (horizontal, vertical, or diagonal).\n\n";
    std::cout << "3. Flipping Pieces:\n";
    std::cout << "   - All opponent's pieces trapped between your new piece and any of your existing pieces will flip\n";
    std::cout << "     to your color.\n";
    std::cout << "   - To be valid, a move must flip at least one of your opponent's pieces.\n\n";
    std::cout << "4. Game End:\n";
    std::cout << "   - The game ends when neither player has a valid move remaining.\n";
    std::cout << "   - The player with the most pieces of their color on the board at the end of the game wins.\n";
    std::cout << "   - If both players have the same number of pieces, the game is a draw.\n\n";
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Wait for user input
}


// manages a single game session, setting up the board, prompting moves, 
// checking for valid moves, and determining the game outcome
//
// parameters:
// none
//
// returns:
// void - does not return a value

void playGame() {
    // players
    int currentPlayer = 1;
    int nextPlayer = 2;
    // get the board size
    int maxBoardSize = getBoardSize(); 
    // get player count
    int playerCount = getPlayerCount();
    bool moveAssistOn = getMoveAssist();
    // valid move
    bool prevPlayerMoved = true;
    
    // prev message
    std::string statusMessage = "Player X goes first.";
    
    
    Board theBoard(maxBoardSize);
    std::stack<PlayerMove> gameHistory;
    
    while (true) {
        // check if there are valid moves for this player
        std::map<std::pair<int, int>, std::set<std::pair<int, int>>> validMoves = theBoard.getValidMoves(currentPlayer);
        
        if (validMoves.size()==0) {
            // if the prev player didn't move
            if (!prevPlayerMoved) {
                // this means neither player has valid moves and the game is over
                statusMessage = "Neither player has valid moves, the game is over!";
                break;
            }
            // if there are no valid moves we go to the next player
            statusMessage = "Player " + std::string(currentPlayer == 1 ? "X" : "O") + " has no valid moves!";
            
            // the previous player was unable to move so we set the flag
            prevPlayerMoved = false;
            
            // swap to the next player
            std::swap(currentPlayer, nextPlayer);
            
            // contine the while loop for the next player
            continue;
        }
        
        // where we store the move for this player
        std::pair<int, int> playerMove;
        
        // check if its the second players move
        // and if we have an ai opponent
        if (currentPlayer == 2 && playerCount == 1) {
            // get the move from the "AI"
            playerMove = getAIMove(validMoves);
        }else {
            // get a valid move for human player
            playerMove = getPlayerMove(currentPlayer, theBoard, validMoves, statusMessage, moveAssistOn);
        }
        // the previous player moved
        prevPlayerMoved = true; 
        
        // update the message
        statusMessage = "Player " + std::string(currentPlayer == 1 ? "X" : "O") + 
                " placed piece @(" + std::to_string(playerMove.first+1) + "," + 
                std::to_string(playerMove.second+1) + ")";
        
        // place the piece, pass pieces to be flipped
        theBoard.placePiece(playerMove, currentPlayer, validMoves.at(playerMove));
        
        // add this move to the gameHistory
        gameHistory.push(PlayerMove(currentPlayer, playerMove));
        
        // swap to next player
        std::swap(currentPlayer, nextPlayer);
    }
    // display the statusMessage
    std::cout << statusMessage << std::endl;
    
    // count the X and O and display the winner
    theBoard.showWinner();
}


int main() {
    printRules();
    do {
        // clear the screen and set cursor to the upper left
        // ANSI escape codes
        // clear the screen \033[2J
        // reset cursor \033[H
        std::cout << "\033[2J\033[H"; 
        // start a new game
        playGame();
    } while (getPlayAgain());  // check if player wants to play again
    
    std::cout << "Thanks for playing." << std::endl;
    return 0;
}
