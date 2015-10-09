#include <stdlib.h>

#include "aalloc.h"
#include "huffman.h"
#include "bool.h"

#define DEBUG

static HuffmanNode CreateHuffmanNode (char token)
{
	HuffmanNode node = (HuffmanNode) amalloc(sizeof(struct __HuffmanNode));
        if (node == NULL) return NULL;

	node->character = token;
	node->frequency = 0;
	node->left      = 0;
	node->right     = 0;

        return node;
}

static HuffmanNode FindNode(FrequencyList list, char token)
{
	if (!list->used) return FALSE;

	while (list)
	{
	   if (list->info->character == token)
	      return list->info;
	   list = list->next;
	}
	
	return NULL;
}

static int DeleteHuffmanNode(FrequencyList list, HuffmanNode node)
{
    if (!list)
       return FALSE;

    if (list->info == node)
    {
       if (list->next == NULL)
       {
          list->info = NULL;
          list->used = FALSE;
       }
       else
       {
          list->info = list->next->info;
          list->next = list->next->next;
       }

       return TRUE;
    }

    while (list->next)
    {
       if (list->next->info == node)
       {
          list->next = list->next->next;
          return TRUE;
       }

       list = list->next;
    }

    return FALSE;
}

static FrequencyList CreateFrequencyList()
{
	FrequencyList result;
	result = (FrequencyList) amalloc(sizeof(struct __FrequencyNode));

	if (!result)
	    return NULL;

	result->used = FALSE;
	result->next = NULL;

	return result;
}

static int AddToFrequencyList(FrequencyList list, HuffmanNode node)
{
     FrequencyList newnode;
	
     if (!list) return FALSE;	

     if (!list->used)
     {
	list->used = TRUE;
	list->info = node;
	return TRUE;
     }	
	
     newnode = (FrequencyList) amalloc(sizeof(struct __FrequencyNode));
     if (!newnode)
	return FALSE;

     newnode->next = list->next;
     list->next = newnode;
     newnode->info = node;
     
     return TRUE;	
}

static int HuffmanProcessCharacter(FrequencyList list, char token)
{
    HuffmanNode node = FindNode(list, token);

    if (!node)
    {	
        node = CreateHuffmanNode(token);
   	if (!node)
           return FALSE;
	
	if (!AddToFrequencyList(list, node))
	   return FALSE;
   }
	
   node->frequency++;
   return TRUE;
}

#ifdef DEBUG

void PrintFrequencyList(FrequencyList list)
{
    printf("After building frequency list:\n");

    if (!list)
       printf("No frequency list!");

    if (!list->used)
       printf("Empty frequency list!");

    while (list)
    {
       if (list->info->character > 32)
          printf("Character %c, frequency %d\n", list->info->character,
                                                 list->info->frequency);
       else
          printf("Character %d, frequency %d\n", (int)list->info->character,
                                                 list->info->frequency);
       
       list = list->next;
   }
}

#endif


static void FindLowestFreq(FrequencyList list, HuffmanNode* node,
			  HuffmanNode except)
{
   int lowest = 32767;

   while (list)
   {
	 if ((list->info->frequency <= lowest) &&
	     (list->info != except))
	 {
	    *node = list->info;
	    lowest = list->info->frequency;
	 }

	 list = list->next;
   }
}

static int FindTwoLowestFreqs(FrequencyList list, HuffmanNode* first, HuffmanNode* second)
{
   if (!list || (list->next == 0))
	return FALSE;

   FindLowestFreq(list, first, NULL);
   FindLowestFreq(list, second, *first);

   return TRUE;
}

#ifdef DEBUG

void PrintHuffmanTree(HuffmanNode node)
{
    if (node->left)
	PrintHuffmanTree(node->left);
    if (node->right)
	PrintHuffmanTree(node->right);

    if (!node->left && !node->right)
       if (node->character > 32)
          printf("%c\n", node->character);
       else
          printf("%d\n", (int)node->character);
}

#endif

/*
  returns root of huffman tree.
*/
static HuffmanNode CreateHuffmanTree(FrequencyList list)
{
   HuffmanNode first, second, node;

   if (!list) return NULL;

   while (FindTwoLowestFreqs(list, &first, &second))
   {
        node = CreateHuffmanNode(MIDNODECHAR);
   	if (!node)
           return NULL;

        node->left      = first;
        node->right     = second;
        node->frequency = first->frequency + second->frequency;

        if (!AddToFrequencyList(list, node))
	   return NULL;

	if (!DeleteHuffmanNode(list, first) ||
	    !DeleteHuffmanNode(list, second))
	{
	   printf("Algorithm failure: deletion failed!\n");
	}
   }

   return list->info;
}

static void RecurseCreateLookupTable(HuffmanNode root, CharLookupTable table,
           	                     char* buf, int bitcount)
{
   if (root->left)
   {
      buf[bitcount / 8] &= ~(1 << (bitcount % 8));
      RecurseCreateLookupTable(root->left, table, buf, bitcount+1);
   }
   if (root->right)
   {
      buf[bitcount / 8] |= 1 << (bitcount % 8);
      RecurseCreateLookupTable(root->right, table, buf, bitcount+1);
   }

   if (!root->left && !root->right)
   {
int i, j;

if (root->character > 32)
printf("%c: ", root->character);
else
printf("%d: ", (int)root->character);


for (i = 0; i < bitcount / 8; i++)
    for (j = 0; j < 8; j++)
	if (buf[i] & (1 << j))
	   printf("1");
	else
	   printf("0");

for (j = 0; j < bitcount % 8; j++)
	if (buf[bitcount / 8] & (1 << j))
	   printf("1");
	else
	   printf("0");

printf("\n");

      table[root->character].nrbits = bitcount;
      memcpy(table[root->character].code, buf, 32);
   }
}

static CharLookupTable CreateLookupTable(HuffmanNode root)
{
   char buf[32];
   CharLookupTable table =
             (CharLookupTable) acalloc(256, sizeof(struct __CharLookupEntry));

   if (!table)
      return FALSE;

   memset(table, '\0', 256 * sizeof(struct __CharLookupEntry));

   RecurseCreateLookupTable(root, table, buf, 0);

   return table;
}

static struct __CharLookupEntry LookupChar(CharLookupTable table, char character)
{
   return table[(int) character];
}

static void MergeByte(char before, int beforelen, char after, int afterlen,
		      char* result, char* spil, int* spillen)
{
  unsigned big;

  char andtable[] = {0,1,3,7,15,31,63,127,255};

/*

  0                                 8                              16
  -------------------------------------------------------------------
  |   before                   |         after            |/////////|
  -------------------------------------------------------------------
			    beforelen                   afterlen
  |                                 |                     |
  ---------------------------------------------------------
	      result                       spil

*/

  before &= andtable[beforelen];
  after  &= andtable[afterlen];

  big  = (unsigned) before;
  big += ((unsigned) after) << beforelen;

  *result = (char) big;
  *spil   = big >> 8;

  *spillen = (afterlen + beforelen) - 8; /* negative if result not fully filled */
}

static void StreamOutputBuf(struct __CharLookupEntry* charentry,
			    char* spilbuffer, int* spilcount,
			    char* output, int* olen)
{
   int i;

   *olen = 0;
   for (i = 0; i < ((int)charentry->nrbits / 8); i++)
   {
       MergeByte(*spilbuffer, *spilcount, charentry->code[i], 8,
                 output, spilbuffer, spilcount);

       if (*spilcount >= 0)
       {
          output++;
	  (*olen)++;
       }
       else
       {
          *spilbuffer = *output;
	  *spilcount  = 8 + *spilcount;
       }
   }

   if ((int)charentry->nrbits % 8)
   {
      MergeByte(*spilbuffer, *spilcount, charentry->code[i],
                ((int)charentry->nrbits % 8),
                output, spilbuffer, spilcount);

       if (*spilcount >= 0)
       {
	  (*olen)++;
       }
       else
       {
          *spilbuffer = *output;
	  *spilcount  = 8 + *spilcount;
       }
   }
}

static void RecurseSerializeHuffmanTree(HuffmanNode node, CharLookupTable table,
					char* outbuf, size_type* position)
{
     size_type tocopy;
     struct __CharLookupEntry entry;

     if (node->left)
	RecurseSerializeHuffmanTree(node->left, table, outbuf, position);
     if (node->right)
	RecurseSerializeHuffmanTree(node->right, table, outbuf, position);

     if ((node->left == NULL) && (node->right == NULL))
     {
	 /*   ---------------------------------
	      | nr of bits | character | code |
	      ---------------------------------  */
printf("  position = %u\n", *position);
	entry  = LookupChar(table, node->character);
	tocopy = entry.nrbits / 8;
	if (entry.nrbits % 8) tocopy++;
	*((unsigned char*) (outbuf + *position)) = entry.nrbits;
	(*position) += sizeof(unsigned char);
	*((char*) (outbuf + *position)) = node->character;
	(*position) += sizeof(char);
	memcpy(outbuf + *position, &entry.code, tocopy);
	(*position) += tocopy;


if (node->character > 32)
   printf("%c : to copy: %d, ", node->character, tocopy);
else
   printf("%d : to copy: %d, ", (int) node->character, tocopy);

{
int i, j;
for (i = 0; i < tocopy; i++)
    for (j = 0; (j < 8) && (i*8+j < (int) entry.nrbits); j++)
       if (entry.code[i] & (1 << j))
	  printf("1");
       else
	  printf("0");
printf("\n");
}
     }
}

#ifdef DEBUG

static int bitset(char* bitfield, unsigned char bit)
{
   return bitfield[bit / 8] & (1 << (bit % 8));
}

void PrintTableIndex(char* table)
{
     unsigned char nrbits, i, j;
     char character;

     character = *(table+1);
     if (character <= 32)
	printf("Character %d ", (int) character);
     else
	printf("Character %c ", character);

     nrbits = (int) *((unsigned char*) table);

     printf("Number of bits = %d ", (int) nrbits);

     printf("Code: ");
     for (i = 0, j=0; i < nrbits; i++, j++)
     {
	 if (j == 8)
	 {
	    j = 0;
	    table++;
	 }

	 if (bitset(table+2, j))
	    printf("1");
	 else
	    printf("0");
     }
     puts("");
}

void PrintTable(char* table, size_type tablelen)
{
    size_type position=0;
    unsigned char nrbits;

    while (tablelen > 0)
    {
	printf(" position=%u\n", position);

	PrintTableIndex(table);

	nrbits = (int) *((unsigned char*) table);
	table += sizeof(unsigned char) + sizeof(char) + nrbits / 8 +
		 ((nrbits % 8) > 0);

	position += sizeof(unsigned char) + sizeof(char) + nrbits / 8 +
		    ((nrbits % 8) > 0);

	tablelen -= sizeof(unsigned char) + sizeof(char) + nrbits / 8 +
		    ((nrbits % 8) > 0);
    }
}

#endif

static void SerializeHuffmanTree(HuffmanNode root, CharLookupTable table,
				 char* outbuf, size_type* olen)
{
   size_type position = 0;
   
   RecurseSerializeHuffmanTree(root, table, outbuf, &position);

   *olen = position;
}

int Compress(char* inbuf, char* outbuf, size_type inlen, size_type *outlen)
{
    size_type i;
    HuffmanNode root;
    FrequencyList freqlist;
    CharLookupTable table;
    struct __CharLookupEntry charentry;

    char spilbuffer = 0;
    int  spilcount = 0;
    int  olen;

    *((size_type*) outbuf) = inlen;
    *outlen = sizeof(size_type) * 3;

    freqlist = CreateFrequencyList();
    if (!freqlist)
    {
        autofree();
        return FALSE;
    }

    for (i = 0; i < inlen; i++)
    {
	if (!HuffmanProcessCharacter(freqlist, inbuf[i]))
	{
	   autofree();
	   return FALSE;
	}
    }
#ifdef DEBUG
    PrintFrequencyList(freqlist);
#endif
    root = CreateHuffmanTree(freqlist);
    if (!root)
    {
       autofree();
       return FALSE;
    }
    
#ifdef DEBUG
    PrintHuffmanTree(root);
#endif

    table = CreateLookupTable(root);
    if (!table)
    {
       autofree();
       return FALSE;
    }

    for (i = 0; i < inlen; i++)
    {
        charentry = LookupChar(table, inbuf[i]);

        StreamOutputBuf(&charentry, &spilbuffer,  &spilcount,
                        &outbuf[*outlen], &olen);
        *outlen += olen;
    }

    if (spilcount)
    {
       outbuf[*outlen] = spilbuffer;
       *outlen += 1;
    }

    *((size_type*) (outbuf+sizeof(size_type))) = *outlen;

    SerializeHuffmanTree(root, table, &outbuf[*outlen], (size_type*)(outbuf+sizeof(size_type)*2));

#ifdef DEBUG
    PrintTable(&outbuf[*outlen], *(size_type*)(outbuf+sizeof(size_type)*2));
#endif

    *outlen += *(size_type*)(outbuf+sizeof(size_type)*2);

    autofree();
    return TRUE;
}
