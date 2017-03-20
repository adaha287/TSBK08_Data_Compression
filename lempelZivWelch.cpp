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
        cout << "Error when opening inputfile" << endl;
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
    if(debug) cout << "Initialize the dictionary" << endl;
    initializeDictionary(dictionary);
    if(debug) cout << "Initialization complete" << endl;


    vector<uint16_t> knownWord;
    vector<uint16_t> testWord;
    vector<int16_t> output; //temporary output vector when not having a file to write to
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
        if(debug) cout << endl << "START: new symbol is: " << (uint16_t)symbol << endl;


        //if end_of_file, push what we haven't pushed yet and then the PSEUDO_EOF
        if(input.eof()) {
            if(debug) cout << "END_OF_FILE!!" << endl;
            if(debug) cout << "getIndex EOF" << endl;
            sizeOfCompressed += ceil(log2(dictionary.size()));
            sendToOutput(outputStream, getIndex(dictionary, knownWord), ceil(log2(dictionary.size())), intToSend, counter);
            output.push_back(getIndex(dictionary, knownWord));
            if(debug) cout << "getIndex EOF_END" << endl;
            sizeOfCompressed += ceil(log2(dictionary.size()));
            output.push_back(257); //index of PSEUDO_EOF
            //Maybe need padding??

            sendToOutput(outputStream, 257, ceil(log2(dictionary.size())), intToSend, counter);
            if(counter != 0){
                intToSend = intToSend << (8-counter);
                outputStream.put(intToSend);
            }
            break;
        }

        testWord = knownWord;
        testWord.push_back((uint16_t)symbol);

        if(debug) cout << "getIndex 1" << endl;
        index = getIndex(dictionary, testWord);
        if(debug) cout << "Index1 is: " << index << endl;
        if(debug) cout << "getIndex 1 end" << endl;

        //if testWord exist in dictionary
        if(index != -1){
            knownWord = testWord;

            if(debug) cout << "Word exist at index: " << index;/* << " and is:" ;
            for(int a = 0; a < knownWord.size(); a++){
                cout << knownWord.at(a) << " ";
            }
            cout << endl;
             */
        }

           // if testWord dont exist in dictionary
        else{

            knownIndex = getIndex(dictionary, knownWord);
            if(knownIndex == (int16_t)570){
               if(debug) cout << "This is weird?" << endl;
            }
            output.push_back(knownIndex);

            sizeOfCompressed += ceil(log2(dictionary.size()));
            if(dictionary.size() == 513){
                if(debug )cout << "now what have we  here" << endl;
            }
            //cout << "Sending " << (int)knownIndex << " from ";
            //for(int i = 0; i  <knownWord.size(); i++){
            //    cout << (char)knownWord[i] << " ";
            //}
            //cout << endl;
            sendToOutput(outputStream, knownIndex, ceil(log2(dictionary.size())), intToSend, counter);

            /*
            if(dictionary.size() == 4096){
                cout << endl;
                cout << "Dictionary is full, empty it!!" << endl;

                vector<uint16_t> emptyDict;
                emptyDict.push_back((uint16_t)NEW_DICT);
                int indexNewDict = getIndex(dictionary, emptyDict);
                //last argument will always be 12 when dictionary has size 4096
                sizeOfCompressed += ceil(log2(dictionary.size()));
                sendToOutput(outputStream, indexNewDict, ceil(log2(dictionary.size())), intToSend, counter);
                emptyDictionary(dictionary);
                testWord.clear();
                knownWord.clear();
                continue;
            }

             */
                //add tuple with index of knownWord and last symbol of testWord to dictionary
            addWord(dictionary, knownIndex, (uint16_t)symbol);

            if(dictionary.size() == 4096){
                emptyDictionary(dictionary);
            }

            /*
            if(debug){ cout << "Dictionary now looks like: " << endl;
                for(int q = 1; q < dictionary.size(); q++){
                    cout << "Index: " << q << "<" << get<0>(dictionary[q]) << ", " << get<1>(dictionary[q]) << ">" << endl;
                }
                cout << endl << endl;
            }
             */
            testWord.clear();
            knownWord.clear();
            knownWord.push_back((uint16_t)symbol);
        }


    }
    input.close();

    for(int i = 0; i < output.size(); i++){
        cout << output[i] << " ";
    }
    cout << endl;
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

    int8_t readingSymbol = (int8_t)input.get();
    int8_t haveRead = 0;
    int8_t shouldRead = (int8_t)ceil(log2(dictionary.size()));
    int16_t index = readIndex(input, dictionary, shouldRead, haveRead, readingSymbol);

    int16_t symbol = get<1>(dictionary[index]);
    if(symbol < 256) {
        cout << "putting out: " << (uint8_t) symbol << endl;
        output.put((uint8_t) symbol);
    }
    else{
        cout << "Error reading first character" << endl;
    }

    int16_t wordIndex = index;
    while(!input.eof()){
        shouldRead = (int8_t)ceil(log2(dictionary.size()+1));
        if(dictionary.size() == 511){
            cout << "We are here" << endl;
        }
        index = readIndex(input, dictionary, shouldRead, haveRead, readingSymbol);
        if( index == 0){
            cout << "Could not find index" << endl;
        }
        else if(index < dictionary.size()){
            //Make check for EMPTY_DICT and PSEUDO_EOF
            if(index == (int16_t)257){ //PSEUDO_EOF
                cout << "Ending" << endl;
                output.close();
                break;
            }
            else if(index == (int16_t)258){ //CLEAR OUR DICT
                //this should never be used
                cout << "THIS SHOULD NOT BE VIEWED IF IMPLEMENTED CORRECT" << endl;
                emptyDictionary(dictionary);
                cout << "Dictionary is now size: " << (int)dictionary.size() << endl;
            }
            else {
                //returns the last symbol in the index-chain (first letter in word)
                uint16_t newSymbol = writeSymbols(output, dictionary, index);

                addWord(dictionary, wordIndex, newSymbol);
                wordIndex = index;
            }
        }
        else if(index == (int16_t)dictionary.size()){
            cout << "INDEX == DICT_SIZE" << endl;
            cout << "Dictionary is now2 size: " << (int)dictionary.size() << endl;
            uint16_t newSymbol = writeSymbols(output, dictionary, wordIndex);

            output.put((int8_t) newSymbol);

            addWord(dictionary, wordIndex, newSymbol);
        }
        else{

            cout << "Error in compressed index!" << endl;
            output.close();
            exit(-2);
        }
    }
}


/***
Put first an EMPTY_CHAR, then 0-255, then also PSEDUO_EOF and NEW_DICT
***/
void initializeDictionary(vector<tuple<int16_t, uint16_t>> &dictionary){
    dictionary.push_back(tuple<int16_t, uint16_t>(NULL, EMPTY_CHAR));

    for(int i = 0; i < 256; i++){
        dictionary.push_back(tuple<int16_t, uint16_t>((int16_t)0, (uint16_t)i));
    }

    dictionary.push_back(tuple<int16_t, uint16_t>(0, PSEUDO_EOF));
    dictionary.push_back(tuple<int16_t, uint16_t>(0, NEW_DICT));
}


/***
 Emties the dictionary and initializes it with all starting values
***/
void emptyDictionary( vector<tuple<int16_t, uint16_t>> &dictionary){
    //dictionary.clear();

    dictionary.erase(dictionary.begin(), dictionary.end());
    initializeDictionary(dictionary);
}


/***
 Add a new word to the dictionary
***/
void addWord(vector<tuple<int16_t, uint16_t>> &Dictionary, int16_t earlierCharIndex, uint16_t newChar){
    if(debug) cout << "Adding <" << (int16_t)earlierCharIndex <<", " << (uint16_t)newChar << ">" << endl;
    Dictionary.push_back(tuple<int16_t, uint16_t>(earlierCharIndex, newChar));
}


/***
Send an index to the output stream
***/
void sendToOutput(ofstream &out, int16_t last_index, int bits_needed, uint8_t &intToSend, int &counter){

    if(bits_needed == 10){
        cout << "interesting stuff" << endl;
    }
    if(debug) cout << "Bits needed:" << bits_needed << endl;
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

    for(int i = 0; i < wordVector.size(); i++ ){
        if(wordVector[i] < 256){
            output.put((uint8_t)wordVector[i]);
        }
        else{
            cout << "We have a problem, char is bigger than 256!!" << endl;
        }
    }
    return wordVector[0];
}


bool existInDict(std::vector<std::tuple<int16_t, uint16_t>> &dictionary, int index){
    return false;
}


/***
 Get index of word in dictionary or -1 if word do not exist.
***/
int getIndex(vector<tuple<int16_t, uint16_t>> &dictionary, vector<uint16_t> &word){
    if(debug){
        cout << "Word is: " << endl;
        for(int j = 0; j < word.size(); j++){
            cout << j << ":" << word[j] << endl;
        }
        cout << endl << endl;
    }


    for(int index = 1; index < dictionary.size(); index++){

        tuple<int16_t, uint16_t> tmpTuple = dictionary.at(index);
        uint16_t symbol = get<1>(tmpTuple);
        int16_t place = word.size()-1;
        while(place > (int16_t)-1 && symbol != EMPTY_CHAR){
            if(word[place] != symbol){
                break;
            }
            else{
                if(debug) cout << "It's a letter match" << endl;
                int16_t newerIndex = get<0>(tmpTuple);
                if(debug)cout << "NewerIndex(index of next symbol) is: " << newerIndex << endl;
                if(debug)cout << "and place is: " << place << endl;
                tmpTuple = dictionary.at(newerIndex);
                symbol = get<1>(tmpTuple);
                place--;
                if(debug)cout << "Next symbol is:" << symbol << endl;
                if(symbol == (uint16_t)EMPTY_CHAR && place == (int16_t)-1){
                    if(debug) cout << "Full word match, returning index: " << index << endl;
                    return index;
                }
                if(debug)cout << "Check next letter" << endl;
            }
        }
    }
    //Never reached! (Unless word is empty?)
    //cout << "Reaching end -1 of getIndex!" << endl;
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
int16_t readIndex(ifstream &input, vector<tuple<int16_t, uint16_t>> &dictionary, int8_t shouldRead, int8_t &haveRead, int8_t &symbol){

    int16_t returnIndex = 0;
    if(debug) cout << "Symbol is:" << symbol << endl;

    int8_t i = 0;
    while(i < shouldRead){


        //if(debug) cout << "i is: " << (int)i << endl;
        //if(debug) cout << "haveRead is: " << (int)haveRead << endl;

        if(haveRead == 8){

            symbol = (int8_t)input.get();
            if(debug) cout << "Getting new symbol: " << (uint8_t)symbol << endl;

            haveRead = 0;
        }
        //Shift leftmost bit of symbol into rightmost bit of returnIndex
        int8_t tmpSymbol = symbol;
        int8_t leftmost = int8_t((tmpSymbol >> (7-haveRead)) & 0x01);
        //if(debug) cout << "Leftmost is:" << (int)leftmost << endl;
        returnIndex = (returnIndex << 1) | leftmost;
        //if(debug) cout << "returnIndex is:" << (int)returnIndex << endl;


        haveRead++;
        i++;
    }
    return returnIndex;
}