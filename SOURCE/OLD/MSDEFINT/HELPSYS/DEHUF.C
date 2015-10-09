#include <stdlib.h>

#include "huffman.h"
#include "bool.h"

#define DEBUG

struct BitPosition
{
   char* bytepos;
   int   bitoffset;
};

struct DecompressNode
{
    char character;

    void* left;
    void* right;

    int leftisnode;
    int rightisnode;
};

static int bitset(char* bitfield, unsigned char bit)
{
   return bitfield[bit / 8] & (1 << (bit % 8));
}


static void FreeDecodeTree(struct DecompressNode* tree)
{
   if (tree->leftisnode)
      FreeDecodeTree((struct DecompressNode*) tree->left);

   if (tree->rightisnode)
      FreeDecodeTree((struct DecompressNode*) tree->right);
}

static char* SeekFirstBit(char* table, int tablelen, unsigned char bits,
			  int findwhat)
{
    int i=0, toskip;
    unsigned char nrbits;

    while (i < tablelen)
    {
	nrbits  = *((unsigned char*) table);
	table  += sizeof(unsigned char)+sizeof(char);
	toskip  = nrbits / 8;
	if (nrbits % 8) toskip++;

	if (nrbits >= bits)
	{
	   if (bitset(table, bits))
	   {
	      if (findwhat)
		 return table - sizeof(unsigned char) - sizeof(char);
	   }
	   else
	   {
	      if (!findwhat)
		 return table - sizeof(unsigned char) - sizeof(char);
	   }
	}

	table += toskip;
	i     += toskip;
    }

    return NULL;
}

static struct DecompressNode* RecurseDeserializeHuffmanTree(char* table,
							    unsigned char bits,
							    size_type tablelen)
{
    unsigned char nrbits;
    char *pos0, *pos1;
    struct DecompressNode* node;

    /* Seek first position where there is a 0 at this position. */
    pos0 = SeekFirstBit(table, tablelen, bits, 0);

    /* Seek first position where there is a 1 at this position. */
    pos1 = SeekFirstBit(table, tablelen, bits, 1);

    node = (struct DecompressNode*) malloc(sizeof(struct DecompressNode));
    if (!node) return NULL;

    if (pos0)
    {
       nrbits = *((unsigned char*) pos0);
       if (nrbits == bits)                 /* If full code reached */
       {
	  node->leftisnode  = FALSE;
	  node->rightisnode = FALSE;

	  node->left  = NULL;
	  node->right = NULL;

	  node->character = *(pos0+1);

	  return node;
       }

       if (pos1)
	  node->left = (void*)
		      RecurseDeserializeHuffmanTree(pos0, bits+1, pos1-pos0);
       else
	  node->left = (void*)
		      RecurseDeserializeHuffmanTree(pos0, bits+1, tablelen);

       if (node->left)
	  node->leftisnode = TRUE;
       else
       {
	  node->left = (void*) pos0; /* If there is not enough memory left
	  node->leftisnode = FALSE;     to create the node */

	  node->right = (void*) pos1;
	  node->rightisnode = FALSE;

	  return node;
       }
    }
    else
       node->leftisnode = FALSE;

    if (pos1)
    {
       nrbits = *((unsigned char*) pos1);
       if (nrbits == bits)                 /* If full code reached */
       {
	  node->leftisnode  = FALSE;
	  node->rightisnode = FALSE;

	  node->left  = NULL;
	  node->right = NULL;

	  node->character = *(pos1+1);

	  return node;
       }

       node->right = (void*)
		    RecurseDeserializeHuffmanTree(pos1, bits+1,
						  tablelen - (pos1-table));

       if (node->right)
	  node->rightisnode = TRUE;
       else
       {
	  node->right = (void*) pos1; /* If there is not enough memory left
	  node->rightisnode = FALSE;     to create the node */

          node->left = (void*) pos0; /* If there is not enough memory left
	  node->leftisnode = FALSE;     to create the node */
       }
    }
    else
       node->rightisnode = FALSE;

    return node;
}

#if 0
static struct DecompressNode* RecurseDeserializeHuffmanTree(char* buffer,
							    unsigned char bits,
							    size_type buflen)
{
    int toskip;
    char *pos0 = NULL, *pos1 = NULL;
    unsigned char nrbits;

    struct DecompressNode* node;

    /*   ---------------------------------
	 | nr of bits | character | code |
	 ---------------------------------  */

    /* Seek first position where there is a 0 at this position. */
    pos0 = SeekFirstBit(buffer, buflen, bits, 0);

    /* Seek first position where there is a 1 at this position. */
    pos1 = SeekFirstBit(buffer, buflen, bits, 1);

    node = (struct DecompressNode*) malloc(sizeof(struct DecompressNode));
    if (!node) return NULL;

    if (pos0)
    {
       if (pos1)
	  node->left = (void*)
		      RecurseDeserializeHuffmanTree(pos0, bits+1, pos1-pos0);
       else
	  node->left = (void*)
		      RecurseDeserializeHuffmanTree(pos0, bits+1, buflen);

       if (node->left)
	  node->leftisnode = TRUE;
       else
       {
	  node->left = (void*) pos0; /* If there is not enough memory left
	  node->leftisnode = FALSE;     to create the node */
	  node->tofar      = FALSE;
       }
    }
    else
    {
       node->tofar = TRUE; /* No more codes of this length */
       node->left  = (void*) *(((char*)buffer)+sizeof(unsigned char));
    }

    if (pos1)
    {
       node->right = (void*)
		    RecurseDeserializeHuffmanTree(pos1, bits+1,
						  buflen - (pos1-buffer));

       if (node->right)
	  node->rightisnode = TRUE;
       else
       {
	  node->right = (void*) pos1; /* If there is not enough memory left
	  node->rightisnode = FALSE;     to create the node */
	  node->tofar       = FALSE;
       }
    }
    else
    {
       node->right = (void*) *(((char*)buffer)+sizeof(unsigned char));
       node->tofar = TRUE;  /* No more codes of this length */
    }

    return node;
}

#endif

#ifdef DEBUG

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

void PrintDecompressTree(struct DecompressNode* tree)
{
     if (tree->leftisnode)
     {


	PrintDecompressTree((struct DecompressNode*) tree->left);
     }

     if (tree->rightisnode)
     {

	PrintDecompressTree((struct DecompressNode*) tree->right);
     }

}

#endif

static struct DecompressNode* DeserializeHuffmanTree(char* buffer,
						     size_type buflen)
{
      return RecurseDeserializeHuffmanTree(buffer, 0, buflen);
}

static void IncrementBitPos(struct BitPosition* pos)
{
    pos->bitoffset++;
    if (pos->bitoffset == 8)
    {
       pos->bytepos++;
       pos->bitoffset=0;
    }
}

static void AddBitPos(struct BitPosition* pos, int amount)
{
    int i;

    for (i = 0; i < amount; i++)
	IncrementBitPos(pos);
}

#if 0

static int FullCodeReached(char* table, struct BitPosition* bitpos, int bitspassed,
			   int tablelen)
{
   unsigned char nrbits;

    /*   ---------------------------------
	 | nr of bits | character | code |
	 ---------------------------------  */

   /* First case: this is the only remaining entry in the table */
   if (tablelen == sizeof(unsigned char) + sizeof(char) +
		   nrbits / 8 + (nrbits % 8 != 0))
      return TRUE;

   /* Second case: bit is 0 and bit of following is 1 */
   if ((!bitset(bitpos->bytepos, bitpos->bitoffset) &&
       (!Bit1InTableReached(table, bitspassed))
   {
      nrbits  = *((unsigned char*) table);

      table = table + sizeof(unsigned char) + sizeof(char) +
	      nrbits / 8;
      if (nrbits % 8) table++;

      if (Bit1InTableReached(table, bitspassed))
	 return TRUE;
      else
	 return FALSE;
   }
   return FALSE;
}

static int Bit1InTableReached(char* table, int bitspassed)
{
   unsigned char nrbits;

   nrbits  = *((unsigned char*) table);

   if (nrbits >= bitspassed)
   {
      return bitset(table+sizeof(unsigned char)+sizeof(char), bitspassed);
   }
   else
      return FALSE;
}

/*
** Do a linear search over the codes until a full code is found.
*/

static char SlowGetNextChar(char* table, int bitspassed,
			    struct BitPosition* bitpos, int tablelen)
{
    unsigned char nrbits;
    char* orig;

    while (!FullCodeReached(table, bitpos, bitspassed, tablelen))
    {
	  if (bitset(bitpos->bytepos, bitpos->bitoffset))
	  {
	     /* In order for the following not to exceed buffer
		length we assume that the input is correct and
		a code is found before being here.              */
	     while (!Bit1InTableReached(table, bitspassed))
	     {
		   orig    = table;
		   nrbits  = *((unsigned char*) table);
		   table  += sizeof(unsigned char) +
			     sizeof(char)          +
			     nrbits / 8;
		   if (nrbits % 8) table++;
		   tablelen -= (table - orig);
	     }
	  }

	  IncrementBitPos(bitpos);
	  bitspassed++;
    }

    /*   ---------------------------------
	 | nr of bits | character | code |
	 ---------------------------------  */

    return *(table+sizeof(unsigned char));
}

#endif

static int BitEqual(struct BitPosition* pos0, struct BitPosition* pos1)
{
    return bitset(pos0->bytepos, pos0->bitoffset) !=
	   bitset(pos1->bytepos, pos1->bitoffset);
}

static int FullCodeReached(char* table, struct BitPosition* bitpos,
			   int bitspassed)
{
    int i;
    unsigned char nrbits;
    struct BitPosition tablepos, datapos;

    /*   ---------------------------------
	 | nr of bits | character | code |
	 ---------------------------------  */
    memcpy(&datapos, bitpos, sizeof(struct BitPosition));
    tablepos.bytepos   = table+sizeof(unsigned char)+sizeof(char);
    tablepos.bitoffset = 0;

    nrbits = *((unsigned char*) table);
    if (bitspassed  == nrbits)
    {
       for (i = 0; i < bitspassed; i++)
       {
	   if (!BitEqual(&tablepos, &datapos))
	      return FALSE;

	   IncrementBitPos(&tablepos);
	   IncrementBitPos(&datapos);
       }

       return TRUE;
    }

    return FALSE;
}

/*
** Do a linear search over the codes until a full code is found.
*/

static char SlowGetNextChar(char* table, int bitspassed,
			    struct BitPosition* bitpos, int tablelen)
{
    unsigned char nrbits;
    char* orig, *origtable=table;
    int origlen = tablelen;

    do {
	 while (tablelen)
	 {
	       if (FullCodeReached(table, bitpos, bitspassed))
	       {
		  AddBitPos(bitpos, bitspassed);
		  return *(table+sizeof(unsigned char));
	       }

	       orig    = table;
	       nrbits  = *((unsigned char*) table);
	       table  += sizeof(unsigned char) +
			 sizeof(char)          +
			 nrbits / 8;
	       if (nrbits % 8) table++;
	       tablelen -= (table - orig);
	 }
	 tablelen = origlen;
	 table    = origtable;
    } while (++bitspassed);  /* Infinite loop that ends when there is a
				overrun on the number                   */

    return '\0';
}

static char GetNextChar(struct BitPosition* bitpos,
			struct DecompressNode* tree,
			char* table, size_type tablelen)
{
    int bitspassed = 0;
    struct BitPosition workingpos;

    memcpy(&workingpos, bitpos, sizeof(struct BitPosition));

    if (!tree)
       return SlowGetNextChar(table, 0, bitpos, tablelen);

    while (1)
    {
	 if ((tree->left = NULL) && (tree->right == NULL))
	    return tree->character;

	 if (!bitset(workingpos.bytepos, workingpos.bitoffset))
	 {
	    if (tree->leftisnode)
	       tree = (struct DecompressNode*) tree->left;
	    else              /* If there wasn't enough memory left. */
	       return SlowGetNextChar((char*) tree->left, bitspassed, bitpos,
				      tablelen-((char*)tree->left-table));
	 }
	 else
	 {
	    if (tree->rightisnode)
	       tree = (struct DecompressNode*) tree->right;
	    else /* If there wasn't enough memory left. */
	       return SlowGetNextChar((char*) tree->right, bitspassed, bitpos,
				      tablelen-((char*)tree->right-table));
	 }

	 IncrementBitPos(&workingpos);
	 bitspassed++;
    }
}

size_type GetDecompressedBufSize(char* inbuf)
{
    return *((size_type*) inbuf);
}

int Decompress(char* inbuf, char* outbuf, size_type inlen, size_type* outlen)
{
    int i;
    char* tabledata;
    size_type complen, tablelen;
    struct BitPosition bitpos;
    struct DecompressNode* tree;

    complen = *((size_type*) inbuf);
    *outlen = complen;

    bitpos.bytepos   = inbuf + sizeof(size_type)*3;
    bitpos.bitoffset = 0;

    tablelen  = *(((size_type*) inbuf)+2);
    tabledata = inbuf + *(((size_type*) inbuf)+1);

#ifdef DEBUG
    PrintTable(tabledata, tablelen);
#endif

//    tree = DeserializeHuffmanTree(tabledata, tablelen);

    for (i = 0; i < complen; i++)
//	*outbuf++ = SlowGetNextChar(&bitpos, tree, tabledata, tablelen);
	*outbuf++ = SlowGetNextChar(tabledata, 0, &bitpos, tablelen);

//    FreeDecodeTree(tree);

    return TRUE;
}

