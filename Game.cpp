#include "Game.h"
#include "Player.h"
#include "Tile.h"
#include "Board.h"
#include "MainMenu.h"
#include "Word.h"
#include "split.h"
#include <iostream>
#include <tuple>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <regex>
#include <tr1/unordered_map>
#include <cctype>
#include <map>
#include <numeric>

#define PLAYER_HAND_SIZE 7
#define MIDDLE 7

using std::string;
using std::tuple;
using std::vector;
using std::cin;
using std::endl;
using std::cout;
using std::make_tuple;
using std::get;
using std::stoi;
using std::ifstream;
using std::ofstream;
using std::abs;

Game::Game(){
    this->player1 = new Player("Player1");
    this->player2 = new Player("Player2");
    currentPlayer = player1;
    fillTileBag();
    dealTiles(PLAYER_HAND_SIZE);
    consecutivePasses = 0;
    cout << "Please make your first move across H7" << endl;
    main();
}


Game::Game(string player1Name, string player2Name){
    this->player1 = new Player(player1Name);
    this->player2 = new Player(player2Name);
    currentPlayer = player1;
    fillTileBag();
    loadDictionary();
    dealTiles(PLAYER_HAND_SIZE);
    consecutivePasses = 0;
    cout << "Please make your first move across H7" << endl;
    main();
}

// Load game constructor
Game::Game(Player* player1, Player* player2, String boardStateStr){
    // Performing normal construction
    this->player1 = new Player(player1);
    this->player2 = new Player(player2);
    currentPlayer = player1;
    consecutivePasses = 0;
    fillTileBag();
    loadDictionary();

    // Popping tiles in player1's hand from tileBag
    for(int i = 0; i<player1->hand.size(); i++){
        String tileLetter = player1->hand.get(i)->letter;
        tileBag.pop(tileBag.index(tileLetter));
    }

    // Popping tiles in player2's hand from tileBag
    for(int i = 0; i<player2->hand.size(); i++){
        String tileLetter = player2->hand.get(i)->letter;
        tileBag.pop(tileBag.index(tileLetter));
    }

    // Placing board state tiles on the board
    vector<String> boardState;
    // Seperating boardStateStr
    split(boardStateStr, " ", boardState, true);

    for (String tileInfo: boardState){
        Tile* tileOnBoard;
        // Popping tile from tile bag
        String letter(1, tileInfo[0]);
        tileBag.pop(tileBag.index(letter), tileOnBoard);
        // Placing tile on the board
        placeTile(tileOnBoard, tileInfo.substr(2,2));
        delete tileOnBoard;
        tileOnBoard = nullptr;
    }

    main();
}

Game::~Game(){
    delete player1;
    delete player2;
    player1 = nullptr;
    player2 = nullptr;

    // Delete words in placedWords
    for (Word* word: placedWords){
        delete word;
    }
}

bool Game::replaceTile(Player* player, String letter){
    bool replacementValid = false;
    // Removing tile from player's hand
    int tileIndex = player->hand.index(letter);
    if (tileIndex != -1){
        if (tileBag.size() > 0){
            replacementValid = true;
            player->hand.pop(tileIndex);
            // Getting a random tile from tile bag
            Tile* tile;
            std::srand(0);
            tileBag.pop(std::rand() % tileBag.size(), tile);
            // Adding tile to hand
            player->hand.append(tile);
            delete tile;

            // Sort the player's new hand
            currentPlayer->hand.sort();

            // Giving the player the chance to read their new
            // hand by waiting for user to press enter 
            // before continuing

        }
    }
    return replacementValid;
}

bool Game::placeTile(Player* player, String letter, string pos){

    bool placementValid = false;
    // Get Tile from players hand   
    int tileIndex = player->hand.index(letter);
    if (tileIndex != -1){
        // Derives the integer x value buy substracting the ascii value 
        // of 'A' from the character in the pos string
        int row = pos[0]-65; 
        int col = stoi(pos.substr(1, 3));

        // Checking that the position values
        // are not out of range
        if (row < 15 && col < 15){
            // Removing the tile from the player's hand
            Tile* tile;
            player->hand.pop(player->hand.index(letter), tile);

            // Updating the tile's position
            tile->posX = row;
            tile->posY = col;

            // Placing tile on the board and recording
            // placement success in a flag
            placementValid = board.placeTile(tile, tile->posX, tile->posY);
            if (placementValid == true){
                currentPlayer->score = currentPlayer->score + tile->value;
            }
            delete tile;
        }
    }
    return placementValid;
}

void Game::placeTile(Tile* tile, string pos){
    // Derives the integer x value buy substracting the ascii value 
    // of 'A' from the character in the pos string
    int row = pos[0]-65; 
    int col = stoi(pos.substr(1, 3));

    // Updating the tile's position
    tile->posX = row;
    tile->posY = col;

    board.placeTile(tile, row, col);
}

bool Game::placeTile(Tile tile){
    bool placementValid = false;
        placementValid = board.placeTile(&tile, tile.posX, tile.posY);
        return placementValid;
    }   

// Reads the tiles from tile set file and populates
// the tile bag
void Game::fillTileBag(){
    // Current line of file
    string line;
    // tile info string that have already been read from
    // the file
    vector<string> tiles;

    // Iterate through tiles file and append each tile to
    // tiles vector
    ifstream myfile ("ScrabbleTiles.txt");
    if (myfile.is_open()){
        while ( getline (myfile,line) ){
            tiles.push_back(line);
        }
        myfile.close();
    }

    // If the file doesn't exist
    else cout << "Unable to open ScrabbleTiles.txt"; 


    // For each tile read from, append its data to the tileBag
    for (string tile: tiles){
        string letter(1, tile[0]);
        
        int value = stoi(tile.substr(2, tile.size()-1));
        tuple<string, int> tileData = make_tuple(letter, value);
        tileBag.append(tileData);
    }
}

void Game::saveGame(Player* player1, Player* player2, String currentPlayer, TileList tileBag,  TileList boardTiles){
    // Create save file name string
    String fileName = player1->name + "-VS-" + player2->name + ".txt";
    cout << "Saving game as: " << fileName << endl;

    // Strings containing the contents of the
    // player's hands and the tileBag 
    // TileList objects
    String player1Hand;
    String player2Hand;
    String tileBagString;    

    // Save File stream
    ofstream saveFile (fileName);

    // Writing player1 name and score to save file
    saveFile << player1->name << endl;
    saveFile << player1->score << endl;

    // Building player1's hand string and 
    // writing it to the save file
    while(player1->hand.size() > 0){
        // Adding letter-value to the hand string
        player1Hand += player1->hand.get(0)->letter + "-";
        player1Hand += std::to_string(player1->hand.get(0)->value);
        if (player1->hand.size() != 1){
            player1Hand += ", ";
        }

        player1->hand.pop(0);
    }
    saveFile << player1Hand << endl;

    // Writing player2 name and score to save file
    saveFile << player2->name << endl;
    saveFile << player2->score << endl;

    // Building player2's hand string and 
    // writing it to the save file
    while(player2->hand.size() > 0){
        // Adding letter-value to the hand string
        player2Hand += player2->hand.get(0)->letter + "-";
        player2Hand += std::to_string(player2->hand.get(0)->value);
        if (player2->hand.size() != 1){
            player2Hand += ", ";
        }

        player2->hand.pop(0);
    }
    saveFile << player2Hand << endl;

    // Recording current player
    saveFile << currentPlayer << endl;

    
    while(tileBag.size() > 0){
        tileBagString += tileBag.get(0)->letter;
        tileBag.pop(0);
    }
    saveFile << tileBagString << endl;

    // Writing info of tiles on the board
    // to the save file
    while(boardTiles.size() > 0){
        saveFile << boardTiles.getHead()->letter;
        saveFile << "@";
        char pos_letter = 'A' + boardTiles.getHead()->posX;
        saveFile << pos_letter;
        saveFile << boardTiles.getHead()->posY;
        boardTiles.pop(0);
        saveFile << " ";
    }
}


// Deals each player a given number of tiles
void Game::dealTiles(int numTiles){
    if (numTiles < tileBag.size()){
            // For each tile to be dealt
        for (int i = 0; i < numTiles; i++){
            // Pop two random tiles from the tile bag
            Tile* tile1; 
            std::srand(0);
            tileBag.pop(std::rand() % tileBag.size(), tile1);
            Tile* tile2; 
            std::srand(0);
            tileBag.pop(std::rand() % tileBag.size(), tile2);

            // Insert them into the players hands
            player1->hand.insert(tile1->data, -1);
            player2->hand.insert(tile2->data, -1);

            // Clean up the transfer pointers
            delete tile1;
            delete tile2;
        }
    }
}

// Deals the specified player a given number of tiles
void Game::dealTiles(int numTiles, Player* player){
    if (numTiles < tileBag.size()){
        // For each tile to be dealt
        for (int i = 0; i < numTiles; i++){
            // Pop a random tile from the tile bag
            Tile* tile; 
            std::srand(0);
            tileBag.pop(std::rand() % tileBag.size(), tile);
            currentPlayer->hand.insert(tile->data, -1);
            delete tile;
        }
    }
}

void Game::switchCurrentPlayer(){
    // If the current player is player1 set the current player
    // to player2
    if (currentPlayer->name == player1->name){
        currentPlayer = player2;
    }
    // Otherwise set the current player to player1
    else{
        currentPlayer = player1;
    }
}

void Game::main(){
    // Each iteration of this loop represents a turn
    while (player1->hand.size() != 0 && player2->hand.size() != 0 && consecutivePasses < 4){
        // print game info at the start of a turn
        printGameInfo();

        // Process the player's move from input
        getPlayerMove();

        // Switch the players for the next turn
        switchCurrentPlayer();
    }

    if (player1->hand.size() == 0){
        cout<< player1->name << " WINS!" << endl;
    }
    else if (player2->hand.size() == 0){
        cout<< player1->name << " WINS!" << endl << endl;;
    }
    else {
        cout << "Stalemate."<<endl << endl;
    }
}

void Game::getPlayerMove(){
    // String containing player's move
    String playerMove = "";

    // While the users input is valid
    bool inputValid = false;
    while (inputValid == false){
        // Recieivng input
        cout << "> ";
        std::getline(std::cin, playerMove);
        cout << endl;
        playerMove = std::regex_replace(playerMove, std::regex("\\r\\n|\\r|\\n"), "");
        // Checking for placement
    
        if (playerMove.substr(0, 5) == "Place"){
            // Recieving player's word placement
            place(playerMove);
            inputValid = true;
            consecutivePasses = 0;
        }
        // Checking for replacement
        else if (playerMove.substr(0, 7) == "Replace"){
            // Extracting tile letter from command string
            String letter(1, playerMove[8]);
            replaceTile(currentPlayer, letter);
            inputValid = true;
            consecutivePasses = 0;
        }

        // Checking for pass
        else if (playerMove == "pass"){
            inputValid = true;
            consecutivePasses++;
        }

        else if (playerMove == "save game" || playerMove == "Save Game"){
            inputValid = false;
            
            TileList boardTiles = board.getTiles();

            saveGame(player1, player2, currentPlayer->name, tileBag, boardTiles);

            exit(EXIT_SUCCESS);
        }

        // Checking for EOF
        else if (cin.eof()){
            cout << endl << "Goodbye!" << endl;
            exit(0);
        }

        // Printing invalid input if the input is still invalid
        if (inputValid == false){
            cout << "Invalid Input"<< endl;
        }

    }
    cout << endl;
}


void Game::printGameInfo(){
    // Printing current player and player scores
    cout << currentPlayer->name << ", it's your turn" <<endl;
    cout << "Score for " << player1->name << ": " << player1->score << endl;
    cout << "Score for " << player2->name << ": " << player2->score << endl<<endl;
    board.printBoard();
    cout<<endl;

    // Current player's hand
    cout << "Your hand is" << endl;
    currentPlayer->hand.sort();
    currentPlayer->hand.printTiles2();
    cout << endl;
}


void Game::place(String playerMove){
    vector<String> placedTilesPositions;
    vector<String> placedLetters;
    int numTilesPlaced = 0;

    // while the move has not been finished
    while (playerMove != "Place Done"){
        bool inputValid = false;
        // Checking for EOF
        if (cin.eof()){
            cout << endl << "Goodbye!" << endl;
            exit(0);
        }

        // Check that the length of the arguments is valid
        if (playerMove.size() == MIN_ARG_LENGTH || playerMove.size() == MAX_ARG_LENGTH){
            // Check that arguments follow the correct syntax
            if (playerMove.substr(8, 2) == "at"){

                // Check that the specified letter is in the player's hand
                String commandLetter(1, playerMove[6]); 
                int indexOfTile = currentPlayer->hand.index(commandLetter);
                if (indexOfTile != -1){
                    // pos is the coordinates of the move
                    String pos = playerMove.substr(11, 3);
                    // Place the specified tile at the position specified at index 11
                    // of the command if the placement succeeds, the input is valid.
                    if (validatePlacement(pos, commandLetter) == true){
                        inputValid = true;
                        placedTilesPositions.push_back(pos);
                        placedLetters.push_back(commandLetter);
                        // Forcing player to follow through with place move
                        cout << "> Place ";
                        std::getline(std::cin, playerMove);
                        cout << endl;
                        playerMove = std::regex_replace( playerMove, std::regex("\\r\\n|\\r|\\n"), "");
                        playerMove = std::regex_replace(playerMove, std::regex("\\r\\n|\\r|\\n"),"");
                        // Adding place to the beginning of player command
                        playerMove = "Place " + playerMove;
                        numTilesPlaced++;
                    }
                }
            }
        }
        // Input was invalid
        if (inputValid == false) {
            cout << "Invalid Input" << endl;
            // Forcing player to follow through with place move
            cout << "> Place ";
            std::getline(std::cin, playerMove);
            cout << endl;
            playerMove = std::regex_replace( playerMove, std::regex("\\r\\n|\\r|\\n"), "");
            playerMove = std::regex_replace(playerMove, std::regex("\\r\\n|\\r|\\n"),"");
            // Adding place to the beginning of player command
            playerMove = "Place " + playerMove;
        }
    }
    if (placedLetters.size() >0){
        bool legalPlacement = true;
        int i = 0;
            
        // TODO write func to build a tilelist for the word
        Word* wordPtr = new Word(make_word_from_letters_and_positions(placedLetters, placedTilesPositions));
        bool placementConsectutive = tilePlacementIsConsecutive(placedTilesPositions, wordPtr);
        bool placementAdjacent = tilePlacementIsAdjacent(placedTilesPositions);
        bool formedWordsValid = validateFormedWords(wordPtr);

        if (placementConsectutive == true && placementAdjacent == true  && formedWordsValid == true){
            while (i < numTilesPlaced && legalPlacement == true){
                placeTile(currentPlayer,
                placedLetters[i],
                placedTilesPositions[i]);
                dealTiles(1, currentPlayer);
                i++;
            }
            if (placedTilesPositions.size() == 7){
                cout << "BINGO!!!" << endl;
                currentPlayer->score++;
            }
            placedWords.push_back(new Word(wordPtr));
        }
        else{
            legalPlacement = false;
            if (!placementConsectutive){
                cout << "Please ensure that you the enter word ";
                cout << "start to finish and that your placements are ";
                cout << "in a horizontal or vertical straight line ";
                cout << "with no gaps." << endl;
            }
            if (!placementAdjacent){
                cout << "All words placed must connect with another or be ";
                cout << "placed over the center tile (H7) if there are no " ;
                cout << "existing words." << endl;
            }
            if (!formedWordsValid){
                cout << "'" + wordPtr->word + "' does not form valid words ";
                cout << "with all of its neighbours" << endl;
                if (!checkDictionary(wordPtr->word)){
                    cout << "'" + wordPtr->word + "' was not found in the ";
                    cout << "dictionary, please try again." << endl;
                }
            }
            // Forcing player to follow through with place move
            cout << "> Place ";
            std::getline(std::cin, playerMove);
            cout << endl;
            playerMove = std::regex_replace( playerMove, std::regex("\\r\\n|\\r|\\n"), "");
            playerMove = std::regex_replace(playerMove, std::regex("\\r\\n|\\r|\\n"),"");
            // Adding place to the beginning of player command
            playerMove = "Place " + playerMove;
            place(playerMove);
        }
        delete wordPtr;
        wordPtr = nullptr;
    }
}

// Checks if a given letter is in the palyer's hand
// and that the given position is free
bool Game::validatePlacement(String pos, String letter){
    bool placementValid = false;
    // Get Tile from players hand   
    int tileIndex = currentPlayer->hand.index(letter);
    if (tileIndex != -1){
        if (board.validatePos(pos) == true){
            placementValid = true;
        }
    }
    return placementValid;
}


// Recieves a vector containing the string positions of the tiles in
// a word placed ny the player and indicates if it is adjacent to another
// already placed tile
bool Game::tilePlacementIsAdjacent(std::vector<String> placedTilesPositions){
    // turns true if there is at least 1 adjacent tile
    bool adjacentExists = false;
    // Converting string positions
    // into 2 vectors of coordinates
    tuple<vector<int>, vector<int>> convertedPositions;
    convertedPositions = convertStringPositions(placedTilesPositions);
    vector<int> rows = std::get<0>(convertedPositions);
    vector<int> cols = std::get<1>(convertedPositions);

    // check if the word pass through the center. If so its the 1st word so it is legal.
    // We mark this by making adjacentExists true
    for(int i = 0; i < (int)rows.size(); i++){
            if(rows[i] == 7 && cols[i] == 7)
                adjacentExists = true;
    }

            //iterate through the x coords of the tiles in the word
    for(int i = 0; i < (int)rows.size(); i++){
        //iterate through the tiles placed on the board
        for(int j = 0; j < board.tiles.size(); j++){
            //check if x coord of tile in word is +1 or -1 of the x coord of a tile on the board. If so then it is adjacent.
            if((rows[i] == board.tiles.get(j)->posX + 1 || rows[i] == board.tiles.get(j)->posX - 1) && cols[i] == board.tiles.get(j)->posY)
                adjacentExists = true;
        }
    }

    //iterate through the y coords of the tiles in the word
    for(int i = 0; i < (int)cols.size(); i++){
        //iterate through the tiles placed on the board
        for(int j = 0; j < board.tiles.size(); j++){
            //check if y coord of tile in word is +1 or -1 of the y coord of a tile on the board. If so then it is adjacent.
            if((cols[i] == board.tiles.get(j)->posY + 1 || cols[i] == board.tiles.get(j)->posY - 1) && rows[i] == board.tiles.get(j)->posX)
                adjacentExists = true;
        }
    }
    return adjacentExists;
}


// Recieves a vector containing the string positions of the tiles in
// a word placed by the player and indicates whether or not
// they were placed consecutively in a straight line from
// right to left or top to bottom
bool Game::tilePlacementIsConsecutive(vector<String> placedTilesPositions, Word* word){
    // Flags to indicate if consecutive flow of tiles
    // has been broken
    bool verticallyConsecutive = true;
    bool horizontallyConsecutive = true;
    // Converting string positions
    // into 2 vectors of coordinates
    tuple<vector<int>, vector<int>> convertedPositions;
    convertedPositions = convertStringPositions(placedTilesPositions);

    // Contains the row and col vals of each tile being 
    // checked for consecutiveness
    vector<int> rows = std::get<0>(convertedPositions);
    vector<int> cols = std::get<1>(convertedPositions);

    // Position of the first tile in the word
    int firstTileRow = rows[0];
    int firstTileCol = cols[0];

    // Iterating through tiles to test consecutiveness

    {
        // Iterator index. Starting at 1 as the front
        // element has already been stored.
        int i = 1;

        // Checking vertical consecutiveness
        while (i < (int)rows.size() && verticallyConsecutive == true){
            // Checking that the current x value
            // matches the first x value

            if (cols[i] != firstTileCol){
                verticallyConsecutive = false;
            }

            // Checking that the current y value
            // i spaces below the original y
            // value

            if (rows[i] != firstTileRow+i){
                // Iterating through positions between rows[i-1]+1
                // and rows[i] to see whether or not they are
                // already occupied by other tiles
                for (int row = rows[i-1]+1; row < rows[i]; row++){

                    // If the spaces between placed tiles are empty
                    // then the placement is not consecutive
                    if(board.boardState[row][cols[i]] == " "){
                        verticallyConsecutive = false;
                    }
                    else{
                        // Add the letters to the word that already exist on the board
                        Tile* newTile = new Tile(board.boardState[row][cols[i]], -1);
                        newTile->posX = row;
                        newTile->posY = cols[i];
                        word->tiles.insert(newTile, i);
                        delete newTile;
                        newTile = nullptr;
                        word->word.insert(i, board.boardState[row][cols[i]]);
                    }
                }
            }
            i++;
        }
    }
    {
        // Iterator index. Starting at 1 as the front
        // element has already been stored.
        int i = 1;

        // Checking horizontal consecutiveness
        while (i < (int)rows.size() && horizontallyConsecutive == true){
            // Checking that the current y value
            // matches the first y value

            if (rows[i] != firstTileRow){    
                horizontallyConsecutive = false;
            }

            // Checking that the current x value
            // i spaces to the right the original x
            // value

            if (cols[i] != firstTileCol+i){
                // Iterating through positions between cols[i-1]+1
                // and cols[i] to see whether or not they are
                // already occupied by other tiles
                for (int col = cols[i-1]+1; col < cols[i]; col++){
                    // If the space between the two tiles is empty
                    if(board.boardState[rows[i]][col] == " "){
                        horizontallyConsecutive = false;
                    }

                    else{
                        // Add the letters to the word that already exist on the board
                        Tile* newTile = new Tile(board.boardState[rows[i]][col]);
                        newTile->posX = rows[i];
                        newTile->posY = col;
                        word->tiles.insert(newTile, i);
                        delete newTile;
                        newTile = nullptr;
                        word->word.insert(i, board.boardState[rows[i]][col]);
                    }
                }
            }
            i++;
        }
    }
    
    bool returnVal;

    if (verticallyConsecutive == true){
        returnVal = verticallyConsecutive;
    }
    else{
        returnVal = horizontallyConsecutive;
    }
    return returnVal;
}

// Converts a vector of string positions into 2
// x and y integer vectors
std::tuple<std::vector<int>, std::vector<int>> Game::convertStringPositions(vector<String> stringPositions){
    vector<int> rows;
    vector<int> cols;

    // Iterating through string positions
    // to convert them
    int row;
    int col;

    // Iterating through string positions
    // to convert them
    for (String pos: stringPositions){
        tuple<int, int> intPos = convertStringPosToInt(pos);
        row = get<0>(intPos);
        col = get<1>(intPos);
        // Storing resulting coordinates in
        // appropriate vectors
        rows.push_back(row);
        cols.push_back(col);
    }

    return make_tuple(rows, cols);

} 


// Recieves a string position such as C6 and
// converts it to an integer position (3, 6)
std::tuple<int, int> Game::convertStringPosToInt(String pos){
    int row = pos[0]-65; 
    int col = stoi(pos.substr(1, 3));
    return make_tuple(row, col);
}

// Loads the dictionary file into the game's Dictionary
void Game::loadDictionary(){
    // Current line of file
    string line;
    // tile info string that have already been read from
    // the file
    

    // Iterate through dictionary file and add each word
    // the Dictionary
    ifstream myfile ("Dictionary.txt");
    if (myfile.is_open()){
        while ( getline (myfile,line) ){
            for (auto & c: line) c = std::toupper(c);
            Dictionary[line] = line;
        }
        myfile.close();
    }

    // If the file doesn't exist
    else cout << "Unable to open Dictionary.txt";
}

// Returns truth value indicating if a string is in the
// dictionary
bool Game::checkDictionary(String word){

    bool returnVal = false;
    if (Dictionary[word] == word){
        returnVal = true;
    }
    return returnVal;
}

// Dictionary checks and updates any words formed by
// the addition of the incoming word
bool Game::dictionaryCheckPlacedWords(Word* incomingWord){
    bool returnValue = true;
    // Index in placedWords
    int i = 0;
    // While the update is succesful and the index is less than the
    // word list size
    while (returnValue ==  true && i < (int)placedWords.size()){
        //Word* existingWord = placedWords[i];
        returnValue = false;//updateWord(existingWord, incomingWord);
        i++;
    }
    return returnValue;
}

Word Game::make_word_from_letters_and_positions(vector<string> placedLetters, 
vector<string> placedTilesPositions){
    // Converting placedLetters into a single string; word
    String word = "";
    for(String letter: placedLetters){
        word += letter;
    }
    // Converting string positions
    // into 2 vectors of coordinates
    tuple<vector<int>, vector<int>> convertedPositions;
    convertedPositions = convertStringPositions(placedTilesPositions);
    TileList wordTiles;

    // Contains the row and col vals of each tile being 
    // checked for consecutiveness
    vector<int> rows = std::get<0>(convertedPositions);
    vector<int> cols = std::get<1>(convertedPositions);


    for (int i =0; i < (int)placedLetters.size(); i ++){
        Tile* currentTile = new Tile(placedLetters[i]);
        currentTile->posX = rows[i];
        currentTile->posY = cols[i];
        currentTile->data = make_tuple(placedLetters[i], 999);
        wordTiles.append(currentTile);
        delete currentTile;
    
    }

    bool direction = true;
    if (rows.size() > 0){
        if (rows.front() == rows.back()){
            // Direction == horizontal
            direction = false;
        }
    }
                
    Word returnWord(word, wordTiles, direction);

    return returnWord;
}

// // Checks if the given word intersects with this word
// bool Game::updateWord(Word* existingWord, Word* incomingWord){
//     cout << existingWord->word << " VS " << incomingWord->word << endl;
//     bool returnVal = true;
//     // Flag indicating if the incomingWord
//     // extends an existingWord
//     bool extendsExistingWord = false;
//     bool foundInvalidExtension = false;

//     // The words have the same direction
//     if (incomingWord->direction == existingWord->direction){
//         // If the direction is horizontal
//         if (incomingWord->direction == 0){
//             cout << "horizontal" << endl;
//             // If the given word consumes the existing word
//             if (incomingWord->tiles.getHead()->posY <= existingWord->tiles.getHead()->posY
//             && incomingWord->tiles.getTail()->posY >= existingWord->tiles.getTail()->posY){
//                 cout << "a-1" << endl;
//                 // If the incoming word is in the dictionary
//                 if (checkDictionary(incomingWord->word)){
//                     // The existingWord becomes the incomingWord
//                     existingWord->tiles.copy(existingWord->tiles);
//                     existingWord->word = incomingWord->word;
//                     existingWord->direction = incomingWord->direction;
//                 }
//             }

//             // If the given word is a prefix for the existing
//             // word
//             else if (incomingWord->tiles.getHead()->posY <= existingWord->tiles.getHead()->posY-1 &&
//             incomingWord->tiles.getTail()->posY == existingWord->tiles.getHead()->posY-1){
//                 String newWord = incomingWord->word + existingWord->word;
//                 cout << "a0" << endl;
//                 if (checkDictionary(newWord)){
//                     for (int i =0; i < existingWord->tiles.size(); i++){
//                         existingWord->tiles.insert(incomingWord->tiles[i], i);
//                     }
//                     existingWord->word = newWord;
//                     extendsExistingWord = true;
//                 }
//                 else{
//                     returnVal = false;
//                 }
//             }

//             // If the given word is a suffix for the existing word
//             else if (incomingWord->tiles.getHead()->posY == existingWord->tiles.getTail()->posY+1 &&
//             incomingWord->tiles.getTail()->posY >= existingWord->tiles.getHead()->posY+1){
//                 // Add the suffix to the existing word to create newWord
//                 String newWord = existingWord->word + incomingWord->word;
//                 // If the word is in the dictionary
//                 cout << "a1" << endl;
//                 if (checkDictionary(newWord)){
//                     for (int i =0; i < existingWord->tiles.size(); i++){
//                         // Inserting each character at the end of the string
//                         existingWord->tiles.insert(incomingWord->tiles[i], -1);
//                     }
//                     existingWord->word = newWord;
//                     extendsExistingWord = true;
//                 }
//                 // If the word is not found in the dictionary,
//                 // return false
//                 else{
//                     returnVal = false;
//                 }
//             }

//         }

//         else if (incomingWord->direction == 1){
//             cout << "vertical" << endl;
//             // If the given word consumes the existing word
//             if (incomingWord->tiles.getHead()->posX <= existingWord->tiles.getHead()->posX && 
//             incomingWord->tiles.getTail()->posX >= existingWord->tiles.getTail()->posX){
//                 cout << "a2" << endl;
//                 // If the incoming word is in the dictionary
//                 if (checkDictionary(incomingWord->word)){
//                     // The existingWord becomes the incomingWord
//                     existingWord->tiles.copy(incomingWord->tiles);
//                     existingWord->word = incomingWord->word;
//                     existingWord->direction = incomingWord->direction;
//                 }
//             }

//             // If the given word is a prefix for the existing
//             // word
//             else if (incomingWord->tiles.getHead()->posX <= existingWord->tiles.getHead()->posX-1 &&
//             incomingWord->tiles.getTail()->posX== existingWord->tiles.getHead()->posX-1){
//                 // The prefix is added to the existing word and it is 
//                 // looked up in the dictionary
//                 String newWord = incomingWord->word + existingWord->word;
//                 cout << "a3" << endl;
//                 if (checkDictionary(newWord)){
//                     // If the word is found, insert the prefix
//                     // into the word and its tilelist
//                     for (int i =0; i < incomingWord->tiles.size(); i++){
//                         existingWord->tiles.insert(incomingWord->tiles[i], i);
//                     }
//                     existingWord->word = newWord;
//                     extendsExistingWord = true;
//                 }
            

//             }

//             // If the given word is a suffix for the existing word
//             else if (incomingWord->tiles.getHead()->posX == existingWord->tiles.getTail()->posX+1 &&
//             incomingWord->tiles.getTail()->posX >= existingWord->tiles.getTail()->posX+1){
//                 String newWord = existingWord->word + incomingWord->word;
//                 extendsExistingWord = true;
//                 cout << "a4" << endl;
//                 if (checkDictionary(newWord)){
//                     for (int i =0; i < incomingWord->tiles.size(); i++){
//                         existingWord->tiles.insert(incomingWord->tiles[i], -1);
//                     }
//                     existingWord->word = newWord;
//                 }
//                 // If the word is not found, return false
//                 else{
//                     returnVal = false;
//                 }
//             }

//         }
//     }
//     // If the incoming directions is horizontal
//     else {
//         if (incomingWord->direction == 0){
//             for (int tileIndex = 0; tileIndex < (int)incomingWord->tiles.size(); tileIndex++){
//             Tile* tile = incomingWord->tiles[tileIndex];
//             // Checks if any letter in the given word prefix's
//             // this word
//                 if (tile->posX == existingWord->tiles.getHead()->posX - 1
//                     && tile->posY == existingWord->tiles.getHead()->posY){
//                     extendsExistingWord = true;
//                     String newWord = tile->letter + existingWord->word;
//                     cout << "a5" << endl;
//                     if (checkDictionary(newWord)){
//                         existingWord->tiles.insert(tile, 0);
//                         existingWord->word = newWord;
//                     }
//                     // If the word is not found, return false
//                     else{
//                         returnVal = false;
//                     }
//                 }

//                 // Checks if any letter in the given word suffix's
//                 // this word
//                 else if (tile->posX == existingWord->tiles.getTail()->posX + 1
//                     && tile->posY == existingWord->tiles.getTail()->posY){
//                     extendsExistingWord = true;
//                     String newWord = existingWord->word + tile->letter;
//                     cout << "a6" << endl;
//                     if (checkDictionary(newWord)){
//                         existingWord->tiles.insert(tile, -1);
//                         existingWord->word = newWord;
//                     }
//                     // If the word is not found, return false
//                     else{
//                         returnVal = false;
//                     }                   
//                 }

//             }
//         }
        
//         // If the incoming direction is vertical
//         else if (incomingWord->direction == 1){
//             cout << "incoming vert" << endl;
//             // Checks if any letter in the given word prefix/suffix's
//             // this word
            
//             for (int tileIndex = 0; tileIndex < (int)incomingWord->tiles.size(); tileIndex++){
//                 Tile* tile = incomingWord->tiles[tileIndex];

//                 if (tile->posY == existingWord->tiles.getHead()->posY
//                     && tile->posX == existingWord->tiles.getHead()->posX-1){
//                     extendsExistingWord = true;
//                     String newWord = tile->letter + existingWord->word;
//                     cout << "a7" << endl;
//                     if (checkDictionary(newWord)){
//                         existingWord->tiles.insert(tile, 0);
//                         existingWord->word = newWord;


//         // If the incomingWord is not in the dictionary and it 
//         // does not extend any existing words, return false
//         if (!checkDictionary(incomingWord->word) && !extendsExistingWord){
//             returnVal = false;
//         }

//         // If the word extends an existing word but the word formed is
//         // nto found within the dictionary
//         if (foundInvalidExtension == true){
//             returnVal = false;
//         }
//     }

//     return returnVal;
// }

bool Game::validateFormedWords(Word* incomingWord){
    // Value returned at the end of the function.
    // Returns true UNLESS a formed word is found
    // that does not exist in the dictionary.
    bool returnValue = true;

    // XY cords of head an tail of incomingWord
    int incomingHeadPosX = incomingWord->tiles[0]->posX;
    int incomingHeadPosY = incomingWord->tiles.getHead()->posY;
    int incomingTailPosX = incomingWord->tiles.getTail()->posX;
    int incomingTailPosY = incomingWord->tiles.getTail()->posY;

    // HORIZONTAL INCOMING WORDS
    if (incomingWord->direction == 0){

        // Flags indicating end of adjacent letters
        bool endOfRight = false;
        bool endOfLeft = false;

        // PosY values of the current tile on the board
        // being checked (To the left and right of the 
        // incomingWord)
        int stepRight = incomingTailPosY + 1;
        int stepLeft = incomingHeadPosY - 1;

        // CHECKING FOR HORIZONTAL EXTENSIONS OF INCOMING WORD
        // Accumulation of the incoming word and letters horizontally
        // adjacent on the board
        String extendedWord = incomingWord->word;

        // Flag to indicate if d causes the program to go out of bounds
        int outOfBounds = false;
        while ((!endOfRight || !endOfLeft) && !outOfBounds){
            
            // Check if the next space to the right is empty
            // If it isn't, append the character to the extendedWord
            if (!endOfRight){
                // If there is a tile to the right, add it to the extendedWord
                if (board.boardState[incomingTailPosX][stepRight] != " "){
                    extendedWord += board.boardState[incomingTailPosX][stepRight];
                }
                else {
                    endOfRight = true;
                }
            }

            // Check if the next space to the left is empty
            // If it isn't, prepend the character to the extendedWord
            if (!endOfLeft){
                // If there is a tile to the left, add it to the extendedWord
                if (board.boardState[incomingHeadPosX][stepLeft] != " "){
                    extendedWord.insert(0, board.boardState[incomingHeadPosX][stepLeft]);
                }
                else {
                    endOfLeft = true;
                }
            }
            stepRight++;
            stepLeft--;
            
            // Checking to see if d causes the index to go out of bounds
            if (stepRight > 14  || stepLeft < 0){
                outOfBounds = true;
            }
        }
        

        /*
        If the horizontally formed word is valid, begin
        checking vertically formed words at each letter
        of the incoming Word
        */



        // Index in incomingWord
        int incWrdIndex = 0;
        bool wordFound = true;
        String intersectingWord;
        while (wordFound && incWrdIndex < (int)incomingWord->word.size()){
            // currentTile in incomingWord
            Tile* currentTile = incomingWord->tiles[incWrdIndex];
            intersectingWord = incomingWord->word[incWrdIndex];

            // PosY values of the current tile on the board
            // being checked (Above and Below the incoming word)
            int stepUp = currentTile->posX - 1;
            int stepDown = currentTile->posX + 1;

            // Flags indicating end of adjacent letters
            bool endOfUp = false;
            bool endOfDown = false;

            // Flag to indicate if d causes the program to go out of bounds
            outOfBounds = false;

            // Iterate through incomingWord, checking above and
            // below each tile for any formed words
            while((!endOfUp || !endOfDown) && !outOfBounds){

                // Check if the next space above is empty
                // If it isn't, prepend the character to the intersectingWord
                if (!endOfUp){
                    if (board.boardState[stepUp][currentTile->posY] != " "){
                        intersectingWord = board.boardState[stepUp][currentTile->posY] + intersectingWord;
                        cout << intersectingWord << endl;
                    }
                    else{
                        endOfUp = true;
                    }
                }

                // Check if the next space below is empty
                // If it isn't, append the character to the intersectingWord
                if (!endOfDown){
                    if (board.boardState[stepDown][currentTile->posY] != " "){
                        intersectingWord += board.boardState[stepDown][currentTile->posY];
                    }
                    else {
                        endOfDown = true;
                    }
                }

                stepUp--;
                stepDown++;
                // Updating out of bounds flag
                if (stepDown > 14  || stepUp < 0){
                    outOfBounds = true;
                }
            }
            incWrdIndex++;

            wordFound = checkDictionary(intersectingWord);
        }
        if (!checkDictionary(extendedWord)){
            returnValue = false;
        }

        else if (intersectingWord.size() > 1 && !wordFound){
            returnValue = false;
        }
    }

    // VERTICAL INCOMING WORDS
    else {

        // Flags indicating end of adjacent letters
        bool endOfUp = false;
        bool endOfDown = false;

        // PosY values of the current tile on the board
        // being checked (Above and Below the incoming word)
        int stepDown = incomingTailPosX + 1;
        int stepUp = incomingHeadPosX - 1;

        
        // CHECKING FOR VERTICAL EXTENSIONS OF INCOMING WORD

        // Accumulation of the incoming word and letters
        // adjacent on the board
        String extendedWord = incomingWord->word;
        // Flag to indicate if d causes the program to go out of bounds
        int outOfBounds = false;
        while ((!endOfUp || !endOfDown) && !outOfBounds){

            // Check if the next space below is empty
            // If it isn't, append the character to the extendedWord
            if (!endOfDown){
                // If there is a tile to the right, add it to the extendedWord
                if (board.boardState[stepDown][incomingTailPosY] != " "){
                    extendedWord += board.boardState[stepDown][incomingTailPosY];
                }
                else {
                    endOfDown = true;
                }
            }

            // Check if the next space above is empty
            // If it isn't, prepend the character to the extendedWord            
            if (!endOfUp){
                // If there is a tile to the left, add it to the extendedWord
                if (board.boardState[stepUp][incomingTailPosY] != " "){
                    extendedWord = board.boardState[stepUp][incomingTailPosY] + extendedWord;
                }
                else {
                    endOfUp = true;
                }
            }
            stepDown++;
            stepUp--;
            
            // Checking to see if d causes the index to go out of bounds
            if (stepDown > 14 || stepUp < 0){
                outOfBounds = true;
            }
        }
        

        /*
        If the vertically formed word is valid, begin
        checking horizontally formed words at each letter
        of the incoming Word
        */

        // Index in incomingWord
        int incWrdIndex = 0;
        bool wordFound = true;
        String intersectingWord;
        while (wordFound && incWrdIndex < (int)incomingWord->word.size()){
            // currentTile in incomingWord
            Tile* currentTile = incomingWord->tiles[incWrdIndex];
            // Word formed by the incoming word and adjacent tiles
            intersectingWord = incomingWord->word[incWrdIndex];

            // PosY values of the current tile on the board
            // being checked (To the left and right of the
            // incomingWord)
            int stepRight = currentTile->posY + 1;
            int stepLeft = currentTile->posY - 1;

            // Flags indicating end of adjacent letters
            bool endOfRight = false;
            bool endOfLeft = false;

            // Flag to indicate if d causes the program to go out of bounds
            outOfBounds = false;

            // Iterate through incomingWord, checking to the right and
            // and left of each tile for any formed words
            while((!endOfRight || !endOfLeft) && !outOfBounds){
                if (!endOfRight){
                    // If there is a tile to the right, add it to the intersectingWord
                    if (board.boardState[currentTile->posX][stepRight] != " "){
                        intersectingWord += board.boardState[currentTile->posX][stepRight];
                    }
                    else {
                        endOfRight = true;
                    }
                }

                if (!endOfLeft){
                    // If there is a tile to the left, add it to the intersectingWord
                    if (board.boardState[currentTile->posX][stepLeft] != " "){
                        intersectingWord.insert(0, board.boardState[currentTile->posX][stepRight]);
                    }
                    else {
                        endOfLeft = true;
                    }
                }

                stepRight++;
                stepLeft++;
                
                // Updating out of bounds flag
                if (stepRight > 14  || stepLeft < 0){
                    outOfBounds = true;
                }
            }
            incWrdIndex++;
            wordFound = checkDictionary(intersectingWord);
        }

        if (!checkDictionary(extendedWord)){
            cout << "A5" << endl;
            cout << extendedWord << endl;
            cout << Dictionary[extendedWord] << endl;
            returnValue = false;
        }

        else if (intersectingWord.size() > 1 && !wordFound){
            cout << "A4" << endl;
            cout << intersectingWord << endl;
            cout << Dictionary[intersectingWord] << endl;
            returnValue = false;
        }
    }

    
    cout << returnValue << endl;
    return returnValue;
}
