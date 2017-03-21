//
// Created by Adam Hansson on 2017-03-03.
//

#ifndef COMPRESSION_LEMPELZIVWELCH_H
#define COMPRESSION_LEMPELZIVWELCH_H


#define PSEUDO_EOF 256 // index = 257
#define EMPTY_CHAR 258 // index = 0



#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <stdint.h>
/**************************************************************
                        DATA STRUCTURES
 The encoder will need a dictionary, the structure for that will be a vector. In every position there will be a tuple with reference to earlier character and the new character
 When we initialize the map we will insert all possible characters,
 i.e. the values 0-255 and the own values (PSEUDO_EOF, NEW_DICT and EMPTY_CHAR)
***************************************************************/
void lzwCompress(const char *fileName);
void lzwDecompress(const char *fileName);

void initializeDictionary(std::vector<std::tuple<int16_t, uint16_t>> &dictionary);

void emptyDictionary(std::vector<std::tuple<int16_t, uint16_t>> &dictionary);

void addWord(std::vector<std::tuple<int16_t, uint16_t>> &dictionary, int16_t earlierCharIndex, uint16_t newChar);

void sendToOutput(std::ofstream &out, int16_t last_index, int bits_needed, uint8_t &intToSend, int &counter);

int getIndex(std::vector<std::tuple<int16_t, uint16_t>> &dictionary, std::vector<uint16_t> &word);

int16_t readIndex(std::ifstream &input, std::vector<std::tuple<int16_t, uint16_t>> &dictionary, int8_t shouldRead, int8_t &haveRead, int8_t &symbol);

//Returns the first letter in the word written
uint16_t writeSymbols(std::ofstream &output, std::vector<std::tuple<int16_t, uint16_t>> &dictionary, int index);

#endif //COMPRESSION_LEMPELZIVWELCH_H
