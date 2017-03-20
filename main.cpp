//
//  main.cpp
//  TSBK08-CompressionAlgorithm
//
//  Created by Adam Hansson on 2017-01-18.
//  Copyright © 2017 Adam Hansson. All rights reserved.
//  Author adaha287


#include <queue>
#include <iostream>
#include "huffman.h"
#include "lempelZivWelch.h"
#include "bitWrite.h"

using namespace std;

// Syntax: CompressionAlgorithm Algorithm compress/decrompress filename

int main(int argc, const char * argv[]) {

    bool huffman;
    bool compress;

    if(argc != 4){
        cout << "Please enter three arguments" << endl;
        cout << "Syntax: Huffman(H)/Lempel-Ziv(LZ) Compress(C)/Decrompress(D) filename" << endl;
        return 0;
    }
    if(strcmp(argv[1], "H") == 0 || strcmp(argv[1], "Huffman") == 0 || strcmp(argv[1], "huffman") == 0){
        huffman = true;
    }
    else if(strcmp(argv[1], "LZW") == 0 ||strcmp(argv[1], "Lempel-Ziv-Welch") == 0 || strcmp(argv[1], "lempel-ziv-welch") == 0){
        huffman = false;

    }
    else{
        cout << "First argument must be 'Huffman'(H) or 'Lempel-Ziv-Welch'(LZW)" << endl;
        return 0;
    }
    if(strcmp(argv[2], "C") == 0 || strcmp(argv[2], "Compress") == 0 || strcmp(argv[2], "compress") == 0){
        compress = true;
    }
    else if(strcmp(argv[2], "D") == 0 ||strcmp(argv[2], "Decompress") == 0 || strcmp(argv[2], "decompress") == 0){
        compress = false;
    }
    else{
        cout << "Second argument must be 'Compress'(C) or 'Decompress'(D)" << endl;
        return 0;
    }
    cout << "Trying to ";
    if(compress) cout << "compress ";
    else cout << "decompress ";
    cout << argv[3] << " with ";
    if(huffman) cout << "Huffman algorithm!";
    else cout << "Lempel-Ziv-Welch algorithm!";
    cout << endl;

    if(huffman){
        if(compress){
            Huffman(argv[3]);
            cout << "Compression done!" << endl;
        }
        else{
            HuffmanDecompress(argv[3]);
            cout << "Decompression done!" << endl;
        }
    }
    else{
        if(compress){
            lzwCompress(argv[3]);
            cout << "Compression done!" << endl;
        }
        else{
            lzwDecompress(argv[3]);
            cout << "Decompression done!" << endl;
        }
    }
    return 0;
}
