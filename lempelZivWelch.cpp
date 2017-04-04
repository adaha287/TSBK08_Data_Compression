//  lempelziv.cpp
//  TSBK08-CompressionAlgorithm
//
//  Created by Adam Hansson on 2017-01-25.
//  Copyright © 2017 Adam Hansson. All rights reserved.
//
#include "lempelZivWelch.h"
#include <string>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <stdint.h> //int8_t, int16_t...
#include <fstream> // input and output from file
#include <iostream>
#include <cmath> //ceil, log2
#include "bitWrite.h"

using namespace std;

#define debug false

void lzwCompress(const char *fileName){
    ifstream input(fileName, ifstream::in | ios::binary);
    if(!input.is_open()){
        cout << "Error when opening inputfile, exiting." << endl;
        exit(1);
    }
    const char *outputEnd(".lzw");
    string tmp(fileName);
    tmp.append(outputEnd);
    const char* outputName = tmp.c_str();

    ofstream outputStream;
    outputStream.open(outputName, ofstream::binary | ofstream::trunc);
    if(!outputStream.is_open()){
        cout << "Could not open outputstream" << endl;
    }

    vector<tuple<int16_t, uint16_t>> dictionary;
    initializeDictionary(dictionary);

    vector<uint16_t> knownWord;
    vector<uint16_t> testWord;
    int16_t index = 0;
    uint8_t symbol;
    int sizeOfCompressed = 0;
    int signs = 0;
    int16_t knownIndex = 0;
    int counter = 0;
    uint8_t intToSend=0;


    while(!input.eof()){

        symbol = (uint8_t)input.get();
        signs += 1;

        //if end_of_file, send what we haven't send yet and then send the PSEUDO_EOF
        if(input.eof()) {

            sizeOfCompressed += ceil(log2(dictionary.size())); //For calculating statistics
            sendToOutput(outputStream, getIndex(dictionary, knownWord), (int) ceil(log2(dictionary.size())), intToSend, counter);

            sizeOfCompressed += ceil(log2(dictionary.size()));

            sendToOutput(outputStream, 257, (int) ceil(log2(dictionary.size())), intToSend, counter);
            //Maybe need padding??
            if(counter != 0){
                intToSend = intToSend << (8-counter);
                outputStream.put(intToSend);
            }
            break;
        }

        testWord = knownWord;
        uint16_t tmp_symbol = (uint16_t) symbol;
        testWord.push_back(tmp_symbol);
        index = getIndex(dictionary, testWord);


        //if testWord exist in dictionary
        if(index != -1){
            knownWord = testWord;
        }

           // if testWord don't exist in dictionary
        else{

            knownIndex = (int16_t) getIndex(dictionary, knownWord);
            sizeOfCompressed += ceil(log2(dictionary.size()));
            sendToOutput(outputStream, knownIndex, (int) ceil(log2(dictionary.size())), intToSend, counter);
            addWord(dictionary, knownIndex, tmp_symbol);

            if(dictionary.size() == 4096){
                emptyDictionary(dictionary);
                outputStream.flush();
            }


            testWord.clear();
            knownWord.clear();
            knownWord.push_back(tmp_symbol);
        }


    }
    input.close();

    cout << endl;
    cout << "Dictionary size: " << dictionary.size() << endl;
    cout << "Bits needed now:" << ceil(log2(dictionary.size())) << endl;
    cout << "size in bits: " << sizeOfCompressed << endl;
    cout << "Size in bytes: " << sizeOfCompressed/8 << endl;
    cout << "Signs: " << signs << endl;
    cout << "Rate: " << (double)sizeOfCompressed/(double)signs << " bits/symbol." << endl;
    cout << "End of Lempel-Ziv-Welch compress" << endl;
}

/***
 Decompress a file that is compressed with the lzwCompress algorithm
 @param fileName is the name of the file
***/
void lzwDecompress(const char *fileName){

    ifstream input(fileName, ifstream::in | ios::binary);
    if(!input.is_open()){
        cout << "Error when opening the compressed file" << endl;
        exit(-1);
    }
    const char *outputEnd(".out");
    string tmp(fileName);
    tmp.append(outputEnd);
    const char* outputName = tmp.c_str();

    ofstream output(outputName);
    if(!output.is_open()){
        cout << "Error! Could not open output stream!" << endl;
        exit(-1);
    }

    vector<tuple<int16_t, uint16_t>> dictionary;
    cout << "Initialize the dictionary" << endl;
    initializeDictionary(dictionary);
    cout << "Initialization complete" << endl;

/*
 *   [1] Initialize string table;
     [2] get first code: <code>;
     [3] output the string for <code> to the charstream;
     [4] <old> = <code>;
     [5] <code> <- next code in codestream;
     [6] does <code> exist in the string table?
      (yes: output the string for <code> to the charstream;
            [...] <- translation for <old>;
            K <- first character of translation for <code>;
            add [...]K to the string table;
            <old> <- <code>;  )
      (no: [...] <- translation for <old>;
            K <- first character of [...];
            output [...]K to charstream and add it to string table;
            <old> <- <code>  )
     [7] go to [5];
     */

    vector<int16_t > indexes;
    int8_t readingSymbol = (int8_t)input.get();
    int8_t haveRead = 0;
    int shouldRead = (int) ceil(log2(dictionary.size()));
    int16_t index = readIndex(input, dictionary, shouldRead, haveRead, readingSymbol);
    if(debug) cout << index << " with " << ceil(log2(dictionary.size())) << " bits. ";

    int16_t symbol = get<1>(dictionary[index]);
    if(symbol < 256) {

        output.put((uint8_t) symbol);
    }
    else if(symbol == 257) cout << "END, no input detected" << endl;
    else{
        cout << "Error reading first character" << endl;
    }

    int16_t wordIndex = index;
    while(!input.eof()){

        if(dictionary.size() == 4095){
            shouldRead = 9;
        }
        else{
            shouldRead = (int) ceil(log2(dictionary.size()+1));
        }


        index = readIndex(input, dictionary, shouldRead, haveRead, readingSymbol);
        //cout << index << " with " << ceil(log2(dictionary.size())) << " bits." << endl;


        if( index == 0){
            cout << "Could not find index" << endl;
        }
        else if(index < dictionary.size()){
            if(index == (int16_t)257){ //PSEUDO_EOF
                cout << "Ending" << endl;
                output.close();
                break;
            }
            else {
                //returns the last symbol in the index-chain (first letter in word)
                uint16_t newSymbol = writeSymbols(output, dictionary, index);

                addWord(dictionary, wordIndex, newSymbol);
                if(debug) cout << index << " with " << ceil(log2(dictionary.size()-1)) << " bits. ";

                wordIndex = index;

                if(dictionary.size() == 4096){
                    emptyDictionary(dictionary);
                    output.flush();
                    vector<uint16_t> tmp_vector;
                    tmp_vector.push_back(newSymbol);
                    wordIndex = (int16_t)getIndex(dictionary, tmp_vector);
                }
            }
        }
        else if(index == (int16_t)dictionary.size()){
            uint16_t newSymbol = writeSymbols(output, dictionary, wordIndex);
            //cout << " " << (char)newSymbol << endl;
            output.put((int8_t) newSymbol);

            addWord(dictionary, wordIndex, newSymbol);
            if(debug) cout << index << " with " << ceil(log2(dictionary.size()-1)) << " bits. ";

            wordIndex = index;

            if(dictionary.size() == 4096){
                emptyDictionary(dictionary);
                output.flush();
                vector<uint16_t> tmp_vector;
                tmp_vector.push_back(newSymbol);
                wordIndex = (int16_t)getIndex(dictionary, tmp_vector);
            }
        }
        else{
            cout << "index: " << index << ". Dictionary size: " << dictionary.size() << "." << endl;
            cout << "Error in compressed index, index bigger than dictionary size" << endl;
            output.close();
            /*
            cout <<"Dictionary size: " << dictionary.size() << endl;
            for(int i = 0; i < indexes.size(); i++){
                if(i % 3 == 0) cout << endl;
                cout << indexes[i]<< " ";
            }
            cout << endl;
             */
            exit(-1);
        }
    }
    if(debug) cout <<"Dictionary size: " << dictionary.size() << endl;
    for(int i = 0; i < indexes.size(); i++){
        cout << indexes[i];
    }
    cout << endl;
}


/***
Put first an EMPTY_CHAR, then 0-255, then also PSEDUO_EOF in the dictionary
***/
void initializeDictionary(vector<tuple<int16_t, uint16_t>> &dictionary){
    dictionary.push_back(tuple<int16_t, uint16_t>(NULL, EMPTY_CHAR));

    for(int i = 0; i < 256; i++){
        dictionary.push_back(tuple<int16_t, uint16_t>((int16_t)0, (uint16_t)i));
    }
    dictionary.push_back(tuple<int16_t, uint16_t>(0, PSEUDO_EOF));
}


/***
 Emties the dictionary and initializes it with all starting values
***/
void emptyDictionary( vector<tuple<int16_t, uint16_t>> &dictionary){
    dictionary.erase(dictionary.begin(), dictionary.end());
    initializeDictionary(dictionary);
}


/***
 Add a new word to the dictionary
***/
void addWord(vector<tuple<int16_t, uint16_t>> &dictionary, int16_t earlierCharIndex, uint16_t newChar){
    if(debug)cout << (int16_t)earlierCharIndex <<" " << (uint16_t)newChar << endl;
    dictionary.push_back(tuple<int16_t, uint16_t>(earlierCharIndex, newChar));
    if(debug)cout << "Size: " << dictionary.size() << endl;
}


/***
Send an index to the output stream
***/
void sendToOutput(ofstream &out, int16_t last_index, int bits_needed, uint8_t &intToSend, int &counter){
    //cout << "Sending " << last_index << " with " << bits_needed << " bits" << endl;

    int bits[12] = {0};
    for(int i = 0; i < bits_needed; i++){
        int16_t  tmpInt = last_index;
        tmpInt = tmpInt >> i;
        int bit = tmpInt & 0x0001;
        bits[bits_needed-i-1] = bit;
    }
    char bitToWrite;
    for(int j = 0; j < bits_needed; j++){
        if(bits[j] == 0) bitToWrite = '0';
        else if(bits[j] == 1) bitToWrite = '1';
        else cout << "Error in sendToOutput, bitToWrite not 0 or 1!" << endl;

        bitWrite(bitToWrite, intToSend, out, counter);
    }
}


/***
 Write the word which end at index, return the first letter in the word
***/
uint16_t writeSymbols(ofstream &output, vector<std::tuple<int16_t, uint16_t>> &dictionary, int index){
    vector<uint16_t> wordVector;
    tuple<int16_t ,uint16_t > currentTuple = dictionary[index];
    uint16_t symbol = get<1>(currentTuple);
    while(symbol != (uint16_t)EMPTY_CHAR){
        wordVector.insert(wordVector.begin(), symbol);
        currentTuple = dictionary[get<0>(currentTuple)];
        symbol = get<1>(currentTuple);
    }

    for(int i = 0; i < wordVector.size(); i++){
        if(wordVector[i] < 256){
            output.put((uint8_t)wordVector[i]);
        }
        else{
            cout << "We have a problem, char is bigger than 256!!" << endl;
        }
    }
    return wordVector[0];
}


/***
 Get index of word in dictionary or -1 if word do not exist.
 @param dictionary is dictionary with all words
 @param word is a vector with the letters of the word whose index you want to find
***/
int getIndex(vector<tuple<int16_t, uint16_t>> &dictionary, vector<uint16_t> &word){

    for(int index = 1; index < dictionary.size(); index++){
        tuple<int16_t, uint16_t> tmpTuple = dictionary[index];
        uint16_t symbol = get<1>(tmpTuple);
        int16_t place = word.size()-1;
        while(place > (int16_t)-1 && symbol != (uint16_t)EMPTY_CHAR){
            if(word[place] != symbol){
                break;
            }
            else{
                int16_t newerIndex = get<0>(tmpTuple);
                tmpTuple = dictionary[newerIndex];
                symbol = get<1>(tmpTuple);
                place--;
                if(symbol == (uint16_t)EMPTY_CHAR && place == (int16_t)-1){
                    return index;
                }
            }
        }
    }

    //If no match or word is empty
    return (int16_t)-1;
}


/***
 Read from input stream, and return next shouldRead amount of bits which is an index in the dictionary
 @param input is the input stream to read from
 @param dictionary is the dictionary with all the known words
 @param shouldRead is how many bits we need to read to get the next index
 @param haveRead is the global variable that tells how many bits that we have read out from symbol
 @param symbol is the byte that we read bits from
***/
int16_t readIndex(ifstream &input, vector<tuple<int16_t, uint16_t>> &dictionary, int shouldRead, int8_t &haveRead, int8_t &symbol){

    int16_t returnIndex = 0;
    int i = 0;

    while(i < shouldRead){

        if(haveRead == 8){
            symbol = (int8_t)input.get();
            haveRead = 0;
        }

        //Shift leftmost bit of symbol into rightmost bit of returnIndex
        int8_t tmpSymbol = symbol;
        int8_t leftmost = int8_t((tmpSymbol >> (7-haveRead)) & 0x01);
        returnIndex = (returnIndex << 1) | leftmost;

        haveRead++;
        i++;
    }
    //cout << "Reading " << returnIndex << " with " << (int)shouldRead << " bits" << endl;
    return returnIndex;
}