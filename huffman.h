//
//  huffman.hpp
//  TSBK08-CompressionAlgorithm
//
//  Created by Adam Hansson on 2017-01-18.
//  Copyright © 2017 Adam Hansson. All rights reserved.
//
#ifndef COMPRESSION_HUFFMAN_H
#define COMPRESSION_HUFFMAN_H


#include <stdio.h>
#include <map>

/**********************************************
                STRUCTURES
 FREQUENCYTABLE is a map<uint16_t, unsigned int> that have the read byte as a key and the number of apperances of the byte in the input file as the value.

 CODEMAP is a map<uint8_t, string> where the key is the read byte from the inputfile and the value is the code that corresponds to the key.
***********************************************/
#define PSEUDO_EOF 256
#define NOT_A_CHAR 257

using namespace std;

struct HuffmanNode{
    uint16_t character;
    unsigned count;
    struct HuffmanNode* zero;
    struct HuffmanNode* one;

    HuffmanNode(uint16_t character = NOT_A_CHAR, unsigned int count = 0, HuffmanNode* zero = nullptr, HuffmanNode* one = nullptr);

    //Returns true if this node is a leaf (has no children)
    bool isLeaf() const;

    bool operator <(const HuffmanNode &rhs) const;
};

void Huffman(const char* fileName);

void HuffmanDecompress(const char* fileName);
void decode(const char* fileName, const char* outFileName, const HuffmanNode* rootNode);


map<uint16_t, unsigned int> frequencyTable(const char* fileName);


HuffmanNode* buildCodeTree(const map<uint16_t, unsigned int> *freqTable);
void buildCodeMap(const HuffmanNode* node, string code, map<uint16_t , string> *codeMap);

void writeHeader(const char *fileOutputName, map<uint16_t, unsigned int> *frequencyTable);

void writeBits(const char *fileOutputName, const char* fileName, map<uint16_t, string> *codeMap);

void readHeader(const char *fileName, map<uint16_t, unsigned int>* frequencyTable);

void freeHuffmanTree(HuffmanNode *HuffmanTree);

#endif //COMPRESSION_HUFFMAN_H
