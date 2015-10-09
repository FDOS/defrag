/*******************************************************************************
** Copyright 2001, Imre Leber (imre.leber@worldonline.be)
**
** Help file decompression. Based on a file by Shaun Case.
**
** The original code stated:
**
/*******************************************************************************
*                                                                              *
* UNHUF.C   by Shaun Case   April 1991                                         *
*                                                                              *
* Written in Borland C++ 2.0 under MS-DOS 3.3                                  *
*                                                                              *
* Decompresses a single file encoded with companion program HUF,               *
* which uses Huffman encoding.                                                 *
*                                                                              *
* This program is in the public domain.                                        *
*                                                                              *
* atman%ecst.csuchico.edu@RELAY.CS.NET                                         *
*                                                                              *
*                                                                              *
*******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>

#define FALSE 0
#define TRUE !FALSE
#define MAX_DECODE_TABLE_SIZE 520       /* 512 should be enough           */


/* uncomment the next line to see the decode table at runtime. */


//#define DEBUG



/* uncomment the next line to only uncompress the exact number of bytes   */
/* that were originally encoded.  If it is not defined, the routine will  */
/* faster, but will generate up to 8 extra bytes at the end of the        */
/* decompressed data.                                                     */

#define EXACT


typedef struct decode_table_element {   /* template for decode table element (wow)  */
    unsigned char letter;               /* which character to decode to             */
    char spare;                         /* force 16-bit word alignment              */
    short left;                         /* index of lower left element from tree    */
    short right;                        /* index of lower right element from tree   */
}decode_table_element;


static short array_max_index;                  /* max number of elements in array (to be   */
                                        /* determined in create_decode_table() )    */

static unsigned long  total;                   /* total number of unencoded bytes          */

static struct decode_table_element             /* array implementation of huffman tree     */
    decode_table[MAX_DECODE_TABLE_SIZE];

/*************************************************************************
** The following allow to use file operations on a buffer in memory.
************************************************************************/

struct InMemoryFile
{
   int Ptr;
   unsigned char* Address;
};

static struct InMemoryFile InFile;
static struct InMemoryFile OutFile;

static void OpenBuffer(struct InMemoryFile* memfile, unsigned char* buf)
{
     memfile->Address = buf;
     memfile->Ptr = 0;
}

static void ReadBuffer(struct InMemoryFile* memfile, int length, void* out)
{
     memcpy(out, memfile->Address+memfile->Ptr, length);
     memfile->Ptr += length;
}

static void WriteBuffer(struct InMemoryFile* memfile, int length, void* in)
{
     memcpy(memfile->Address+memfile->Ptr, in, length);
     memfile->Ptr += length;
}

/******
 *
 * Datafile:  All 16/32 bit quantities in Intel byte ordering
 *
 *  13 bytes    : original filename (8.3 + '\0')
 *  16 bits     : number of array elements needed, N (N == 511 means 512 array
 *                elements -> 0..511)
 *  32 bits     : size of uncompressed original data in bytes
 *  N * 6 bytes : Array elements in order 0 .. N
 *                struct decode_table_element {
 *                     char letter;      8 bits
 *                     char spare;       8 bits
 *                     short left;      16 bits
 *                     short right;     16 bits
 *                 }
 *  <?>          : compressed data, effectively a bit stream
 *
 ******/

int UncompressBuffer(unsigned char* bufin, unsigned char* bufout)
{
    short read_header(unsigned char*);                /* prototype */
    short uncompress(unsigned char*);                 /* prototype */

    /* read in file name, size, decode table */
    if (read_header(bufin) != 0) return 1;
     /* uncompress the data & write to file  */
    if (uncompress(bufout) != 0)  return 1;

    return 0;
}

/*
 * read in the original filename, size, and decode table
 *
 */

short read_header(unsigned char* bufin)
{
    OpenBuffer(&InFile, bufin);

    ReadBuffer(&InFile, sizeof(short), &array_max_index);
    ReadBuffer(&InFile, sizeof(unsigned long), &total);
    
    /* get decode table        */
    ReadBuffer(&InFile, sizeof(decode_table_element)*(array_max_index + 1),
               decode_table);

    return 0;
}

/*
 * all the fputc() calls are unrolled for speed.
 *
 */

short uncompress(unsigned char* outbuf)
{

    /* tcc68 can assign 7 register variables */

    register short index         = 0;               /* "ptr" to "node" in "tree"        */
                                                    /* actually an array index          */
    unsigned unsigned int buffer = 0;               /* 8 bit buffer                     */
    register unsigned short fastleft;               /* fast ptr to next left  element   */
    register unsigned short  fastleft0;             /* fast ptr to left of root         */
    register long running_total = 0L;

    OpenBuffer(&OutFile, outbuf);

    fastleft0 = decode_table[0].left;                   /* setup frequently used vars       */
    fastleft = fastleft0;

    while (1)
    {   
        /* get 8 bits                       */
        ReadBuffer(&InFile, sizeof(unsigned char), &buffer);

        /* branch left if current bit == 1, else branch right                               */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;                                   /* rotate next bit to test postion  */

        fastleft = decode_table[index].left;            /* set up frequently used var       */

        if (fastleft == 0)                              /* if we have decoded a char        */
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);

            index = 0;                                  /* set "ptr" to top of "tree"       */
            fastleft = fastleft0;                       /* set up freq. used variable       */
#ifdef EXACT
            if (++running_total == total) goto finished;  /* if we are done, quit.          */
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }


        /* and do it again for 2nd bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);
            
            index = 0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }
 
        /* and 3rd bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);
            
            index = 0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }

        /* and 4th bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);
            
            index=0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }

        /* and 5th bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);        

            index = 0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }

        /* and 6th bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);

            index = 0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }

        /* and 7th bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);        

            index = 0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
        ++running_total;
#endif EXACT
        }

        /* and finally, the 8th bit. */

        index = ( (buffer & 0x0080) ? fastleft : decode_table[index].right);

        buffer <<= 1;

        fastleft = decode_table[index].left;

        if (fastleft == 0)
        {
            WriteBuffer(&OutFile, sizeof(unsigned char), &decode_table[index].letter);

            index = 0;
            fastleft = fastleft0;
#ifdef EXACT
            if (++running_total == total) goto finished;
#endif EXACT
#ifndef EXACT
            ++running_total;
#endif EXACT
        }
#ifndef EXACT
        if (running_total >= total)
            goto finished;
#endif EXACT

    }

finished:

    return 0;
}

