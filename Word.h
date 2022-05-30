#ifndef ASSIGN2_WORD_H
#define ASSIGN2_WORD_H

#include "TileList.h"
#include "Tile.h"
#include <iostream>
#include <string>

class Word {
    public:
    
    Word(std::string word, TileList& tiles, bool direction);
    Word(Word*& other);
    std::string word;
    TileList tiles;
    // Direction of the word (horizontal = 0, vertical = 1)
    bool direction;
};

#endif //ASSIGN2_WORD_H