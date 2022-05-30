#include <iostream>
#include <string>
#include "Word.h"
#include "TileList.h"
#include "Tile.h"

Word::Word(std::string word, TileList& tiles, bool direction){
    this->tiles.copy(tiles);
    this->word = word;
    this->direction = direction;
};

Word::Word(Word*& other){
    tiles.copy(other->tiles);
    word = other->word;
    direction = other->direction;
}