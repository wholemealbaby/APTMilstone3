#ifndef ASSIGN2_TILE_H
#define ASSIGN2_TILE_H

#include <iostream>
#include <tuple>
#include "Tile.h"

using std::string;
using std::tuple;

// Scrabble tile that has a letter, value and position
class Tile {
   public:

   Tile(tuple<string, int> data);
   Tile(const Tile& other);
   Tile(tuple<string, int> data, Tile* prev, Tile* next);
   Tile(const Tile& other, Tile* prev, Tile* next);
   Tile(string letter, int value);
   Tile(string letter, int value, Tile* prev, Tile* next);
   Tile(string letter);

   // Left and right neighbour pointers
   Tile* prev;
   Tile* next;
   tuple<string, int> data;
   string letter;
   int  value;
   // Position of tile on the board,  eg. C2
   // Empty string if the tile has not been placed yet
   int posX;
   int posY;


};

#endif // ASSIGN2_TILE_H
