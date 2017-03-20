//
//  bitWrite.cpp
//  TSBK08-CompressionAlgorithm
//
//  Created by Adam Hansson on 2017-01-19.
//  Copyright © 2017 Adam Hansson. All rights reserved.
//

#include "bitWrite.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;



void bitWrite(char bit, uint8_t &byte_to_write, ofstream &outputStream, int &bitCounter){
    bool debug = false;
    //if(debug)cout << "before:"<< (int)*byte_to_write << endl;

    byte_to_write <<= 1;
    if(bit == '0') byte_to_write |= 0x00;
    else if (bit == '1') byte_to_write |= 0x01;
    //if(debug)cout << "after:"<< (int)*byte_to_write << endl;
    bitCounter += 1;

    if(bitCounter == 8)
    {
        if(debug)cout << "8 bits! It's time to write!" << endl;
        if(debug)cout << "gonna write:" << (int)byte_to_write << endl;
        //if(debug)cout << "in binany is that: " << (int) *byte_to_write << endl;
        //outputStream->write(to_string(byte_to_write), 1);
        //outputStream->write(to_string((unsigned char*)byte_to_write), sizeof(byte_to_write));
        outputStream.put(byte_to_write);
        bitCounter = 0;
        byte_to_write = 0;
    }
}

int getBit(int counter, char currentChar){
    int tmp = (int)(currentChar);
    tmp  >>= (7-counter);
    tmp &= 1;
    return tmp;
}
