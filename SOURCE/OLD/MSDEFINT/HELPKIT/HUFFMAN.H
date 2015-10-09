#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#define MIDNODECHAR '\0'

typedef struct __HuffmanNode {

	char character;
	int frequency;
	
	struct __HuffmanNode* left;
	struct __HuffmanNode* right;
} *HuffmanNode;

typedef struct __FrequencyNode {

	HuffmanNode info;
	int used;
	struct __FrequencyNode* next; 

} *FrequencyList;


typedef struct __CharLookupEntry {

       unsigned char nrbits;
       char code[32];

} *CharLookupTable;

typedef unsigned short size_type;

int Compress(char* inbuf, char* outbuf, size_type inlen, size_type *outlen);

#endif
