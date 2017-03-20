//
//  huffman.cpp
//  TSBK08-CompressionAlgorithm
//
//  Created by Adam Hansson on 2017-01-18.
//  Copyright © 2017 Adam Hansson. All rights reserved.
//

#include "huffman.h"
#include "bitWrite.h"
#include <map>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <stdio.h>
#include <stdint.h>


using namespace std;
bool debug = false;


/********************************************************
*                  Huffman compress                     *
********************************************************/

void Huffman(const char* fileName){
    map<uint16_t, unsigned int> frequencyMap;
    HuffmanNode* CodeTree;
    map<uint16_t, string> codeMap;
    string code = "";

    // Add '.huf' to the filename of the compressed file
    const char *outputEnd(".huf");
    string tmp(fileName);
    tmp.append(outputEnd);
    const char* outputName = tmp.c_str();
    if(debug)cout << "outputName: " << outputName << endl;

    cout << "Building frequency table" << endl;
    frequencyMap = frequencyTable(fileName);
    cout << "Done!" << endl;
    int summa = 0;
    map<uint16_t, unsigned int>::const_iterator it1;
    for(it1 = frequencyMap.begin(); it1 != frequencyMap.end(); it1++){
        summa += it1->second;
        cout << to_string(it1->first) << ":" << to_string(it1->second) << endl;
    }
    cout << "Antal symboler" << summa << endl;
    //antal bytes/antal symboler = rate


    cout << "Building code tree" << endl;
    CodeTree = buildCodeTree(&frequencyMap);
    cout << "Done!" << endl;

    cout << "Building code map" << endl;
    buildCodeMap(CodeTree, code, &codeMap);
    cout << "Done!" << endl;

    //Print the codeMap
    int n = 0;
    map<uint16_t, string>::const_iterator it2;
    for(it2 = codeMap.begin(); it2 != codeMap.end(); it2++){
        cout << "length of code:" << it2->second.length() << endl;
        n += (frequencyMap.at(it2->first) * it2->second.length());
    }
    float rate = (float)n/(float)summa;
    cout << "Rate: " << rate << " bits/character" << endl;

    cout << "Write header " << endl;
    writeHeader(outputName, &frequencyMap);
    cout << "Done!" << endl;

    cout << "Write bits" << endl;
    writeBits(outputName, fileName, &codeMap);
    cout << "Done!" << endl;


    freeHuffmanTree(CodeTree);
    cout << "Free memory done!" << endl;
}

/********************************************************
*                  Huffman decompress                   *
********************************************************/
/* Decompress a file fileName */

void HuffmanDecompress(const char* fileName){

    map<uint16_t, unsigned int> frequencyTable;
    ifstream inStream;
    inStream.open(fileName, ios::in | ifstream::binary);
    cout << "Reading header..." << endl;
    readHeader(fileName, &frequencyTable);
    cout << "Done" <<endl;

    //Print the frequencytable
    /*
    map<uint8_t, unsigned int>::iterator it = frequencyTable.begin();
    cout << "This is the frequencytable!" << endl;
    while(it != frequencyTable.end()){
        cout << it->first << ": " << it->second << endl;
        it++;
    }
    */

    cout << "Building code tree..." << endl;
    HuffmanNode* CodeTree;
    CodeTree = buildCodeTree(&frequencyTable);
    cout << "Done!" << endl;

    //open input and output, read byte, mask out a bit and traverse the codeTree according to that bit, if leaf: print char to output
    //
    string tmp(fileName);
    tmp.append(".out");
    const char* outFileName = tmp.c_str();
    cout << "Decoding..." <<endl;
    decode(fileName, outFileName, CodeTree);
    cout << "Done!" << endl;

    freeHuffmanTree(CodeTree);
    cout << "Free memory done!" << endl;
}


/********************************************************
*                         decode                        *
********************************************************/

void decode(const char* fileName, const char* outFileName, const HuffmanNode* rootNode){

    ifstream inStream;
    ofstream outStream;
    const HuffmanNode *currentNode = rootNode;

    inStream.open(fileName, ios::in | ifstream::binary);
    outStream.open(outFileName, ios::out | ofstream::trunc);
    inStream.ignore(numeric_limits<streamsize>::max(), '}');
    int counter = -1;
    uint8_t curByte = inStream.get();
    while(true){
        counter++;
        if(counter == 8){
            counter = 0;
            curByte = inStream.get();
            if(debug)cout << "curbyte=" << curByte<<endl;
            if(inStream.eof()){
                cout << "Reached end of inputfile" <<endl;
                break;
            }
        }
        int bit = getBit(counter, curByte);
        if(debug)cout << "Bit is " << bit << endl;
        if(bit == 0){
            currentNode = currentNode->zero;
            if(currentNode->isLeaf()){
                if(currentNode->character == PSEUDO_EOF){
                    break;
                }
                else{
                    if(debug)cout << "writing: " << currentNode->character << endl;
                    outStream.put((uint8_t)currentNode->character);
                    currentNode = rootNode;
                }
            }
        }
        else if(bit == 1){
            currentNode = currentNode->one;
            if(currentNode->isLeaf()){
                if(currentNode->character == PSEUDO_EOF){
                    break;
                }
                else{
                    if(debug)cout << "writing: " << currentNode->character << endl;
                    outStream.put((uint8_t)currentNode->character);
                    currentNode = rootNode;
                }
            }
        }
    }
    inStream.close();
    outStream.close();
}

/********************************************************
*                 FREQUENCY TABLE                       *
********************************************************/
map<uint16_t, unsigned int> frequencyTable(const char* fileName){
    map<uint16_t, unsigned int> freqTbl;
    ifstream inputFile (fileName);
    if (inputFile.is_open())
    {
        uint8_t symbol;
        symbol = (uint8_t)inputFile.get();
        while (!inputFile.eof()){
            freqTbl[(uint16_t)symbol]++;
            symbol = inputFile.get();
        }

        if(!freqTbl.empty()){
            ++freqTbl[(uint16_t)PSEUDO_EOF];
        }

        inputFile.close();

    }
    else{
        cout << "Unable to open file" << endl;
        exit(-1);
    }
    return freqTbl;
}

/*********************************************************
 *                 BUILD CODE TREE                       *
 ********************************************************/
/* Builds a code tree and returns the root node of the tree */
HuffmanNode* buildCodeTree(const map<uint16_t, unsigned int> *frequencyTable){

    priority_queue<HuffmanNode> prioQueue;
    map<uint16_t, unsigned int>::const_iterator mapIterator;
    HuffmanNode current;

    for(mapIterator = frequencyTable->begin(); mapIterator != frequencyTable->end(); mapIterator++){
        prioQueue.push(HuffmanNode(mapIterator->first, mapIterator->second, nullptr, nullptr));
    }

    while(prioQueue.size() > 1){

        current = prioQueue.top();
        //Create the left subnode
        HuffmanNode *zero = new HuffmanNode(current.character, current.count, current.zero, current.one);
        prioQueue.pop();

        current = prioQueue.top();
        //Creates the right subnode of the new node.
        HuffmanNode* one = new HuffmanNode(current.character, current.count, current.zero, current.one);
        prioQueue.pop();

        // Create the node that will be return
        HuffmanNode* newNode = new HuffmanNode(0, zero->count+one->count, zero, one);

        if(prioQueue.empty()){
            return newNode;
        }
        else{
            prioQueue.push(*newNode);
        }
    }

    //This should never be used!
    return new HuffmanNode(NOT_A_CHAR, 0, nullptr, nullptr);

}


/*********************************************************
 *                 BUILD CODE MAP                        *
 ********************************************************/
/* Traverse a code tree and put the character(key) and code(value) in a codeMap */

void buildCodeMap(const HuffmanNode* node, string code, map<uint16_t, string>* codeMap){

    if(node->isLeaf()) {
        codeMap->insert(pair<uint16_t, string>(node->character, code));
    }
    else{
        if(node->zero != nullptr){
            string newCode = code;
            buildCodeMap(node->zero, newCode.append("0"), codeMap);
        }
        if(node->zero != nullptr){
            string newCode = code;
            buildCodeMap(node->one, newCode.append("1"), codeMap);
        }
    }

}


/* Write a header to the output file */
/*********************************************************
 *                     WRITE HEADER                      *
 ********************************************************/
void writeHeader(const char *fileOutputName, map<uint16_t, unsigned int> *frequencyTable){

    if(debug) cout << "Time to write the header with this method!" << endl;
    ofstream outFileStream;
    outFileStream.open(fileOutputName, ios::out);
    if(debug) cout << "Stream created" << endl;
    string header = "{";
    if(debug) cout << "Header started with {" << endl;

    map<uint16_t, unsigned int>::iterator mapIter;
    mapIter = frequencyTable->begin();
    while(mapIter != frequencyTable->end()){
        if(debug) cout << "first " << endl;
        uint16_t symbol = mapIter->first;
        if(debug) cout << "symbol:" << symbol << endl;
        int amount = mapIter->second;
        if(debug)cout << "amount:" << amount << endl;
        if(debug)cout << endl;
        header.append(to_string(symbol));
        header.append(":");
        header.append(to_string(amount));
        if(debug)cout << "Current header:" << header << endl;
        if((++mapIter) != frequencyTable->end()){
            header.append(",");
        }
        else{
            header.append("}");
        }
        if(debug)cout << "and now:" << header << endl;
    }
    if(debug) cout << "Header created, gonna write to stream" << endl;
    for(char c : header){
        outFileStream.put(c);
    }
    outFileStream.close();
}


/*********************************************************
 *                     WRITE BITS                        *
 ********************************************************/
/* Read the file and write the compressed version */

void writeBits(const char *fileOutputName, const char* fileName, map<uint16_t, string> *codeMap){

    if(debug) cout << "Lets write bits in this function! :D!" << endl;
    ofstream outFileStream;
    outFileStream.open(fileOutputName, ofstream::binary | ios::out | ios::app);
    if(!outFileStream){
        cout << "Could not open outputfile" << endl;
        exit(-1);
    }
    ifstream inputFile(fileName);

    uint8_t byte_to_write = 0;
    int bitCounter = 0;
    uint8_t symbol;
    string code;

    //Do all the writing here
    //Get each character
    symbol = (uint8_t)inputFile.get();
    while(!inputFile.eof()){
        if(debug) cout << "The symbol was: " << symbol << endl;
        code = codeMap->at((uint16_t)symbol);
        if(debug) cout << "The code is: " << code << endl;
        for(char bit : code){
            if(bit == '0'){
                if(debug) cout << "Write a 0!" << endl;
                bitWrite(0, byte_to_write, outFileStream, bitCounter);

            }
            else if (bit == '1'){
                if(debug) cout << "Write a 1!" << endl;
                bitWrite(1, byte_to_write, outFileStream, bitCounter);
            }
        }
        symbol = (uint8_t)inputFile.get();
    }

    code = codeMap->at(PSEUDO_EOF);
    if(debug) cout << "The code for EOF is: " << code << endl;

    for(char bit : code){
        if(bit == '0'){
            bitWrite(0, byte_to_write, outFileStream, bitCounter);
            if(debug) cout << "I wrote a EOF 0!" << endl;
        }
        else if (bit == '1'){
            bitWrite(1, byte_to_write, outFileStream, bitCounter);
            if(debug) cout << "I wrote a EOF 1!" << endl;
        }
    }


    //Padding if the bits are an even multiple of 8
    if(bitCounter > 0)
    {
        // pad the last byte with zeroes
        cout << "Gonna add padding of size: " << 8 - bitCounter << endl;
        byte_to_write <<= 8 - bitCounter;
        if(debug)cout << "cur_byte är nu: " << byte_to_write << endl;
        if(debug)cout << "binary: " << (int) byte_to_write << endl;
        outFileStream.put(byte_to_write);
    }
    outFileStream.close();
    inputFile.close();
}

/*********************************************************
*                     READ HEADER                        *
*********************************************************/
/* Read in a header from a compressed file and create a frequency table from it */

void readHeader(const char *fileName, map<uint16_t, unsigned int>* frequencyTable){

    ifstream inStream;
    inStream.open(fileName);
    char symbol;
    char number;
    string totalSymbol = "";
    string totalAmount = "";

    symbol = inStream.get();
    if(debug)cout << "First symbol is:"<<symbol<<endl;


    if(symbol == '{'){
        symbol = inStream.get();
        if(debug)cout << "Symbol read:" << symbol<<endl;

        //start read header
        while(symbol != '}'){

            while(symbol != ':'){
                totalSymbol.push_back(symbol);
                symbol = inStream.get();
            }
            number = inStream.get();

            while(number != ',' && number != '}'){
                totalAmount.push_back(number);
                number = inStream.get();
            }

            frequencyTable->insert(pair<uint16_t, unsigned int>((uint16_t)stoi(totalSymbol), (unsigned int)stoi(totalAmount)));
            totalSymbol = "";
            totalAmount = "";
            if(number == '}'){
                break;
            }
            symbol = inStream.get();
        }
    }
    else{
        cout << "Error: Header problem" << endl;
        inStream.close();
        exit(-1);
    }
    inStream.close();
}

void freeHuffmanTree(HuffmanNode *HuffmanTree){
    if(HuffmanTree->isLeaf()){
        free(HuffmanTree);
    }
    else{
        freeHuffmanTree(HuffmanTree->zero);
        freeHuffmanTree(HuffmanTree->one);
        free(HuffmanTree);
    }
}

HuffmanNode::HuffmanNode(uint16_t character, unsigned int count, HuffmanNode* zero, HuffmanNode* one) {
    this->character = character;
    this->count = count;
    this->zero = zero;
    this->one = one;
}

bool HuffmanNode::isLeaf() const {
    return zero == nullptr && one == nullptr;
}

bool HuffmanNode::operator <(const HuffmanNode &rhs) const {
    return this->count > rhs.count;
}
