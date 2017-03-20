//
//  bitWrite.hpp
//  TSBK08-CompressionAlgorithm
//
//  Created by Adam Hansson on 2017-01-19.
//  Copyright © 2017 Adam Hansson. All rights reserved.
//

#ifndef COMPRESSION_BITWRITE_H
#define COMPRESSION_BITWRITE_H

#include <stdio.h>
#include <fstream>

using namespace std;

void bitWrite(char bit, uint8_t &cur_byte, ofstream &outputStream, int &bitCounter);

int getBit(int counter, char currentChar);



#endif //COMPRESSION_BITWRITE_H
