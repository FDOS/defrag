#ifndef PARSER_H_
#define PARSER_H_

typedef int TOKEN;

/* Parser input structure. */

struct LValueArray {
       TOKEN       token;
       void (*func)(TOKEN rvalue);
};

struct HeaderArray {
       int                 amount;
       TOKEN               header;
       struct LValueArray* lvalues;
};


/* Tokens. */
#define TknNONE         -1           /* Invalid input.         */
#define TknDONE          0           /* End of buffer reached. */

#define TknSPACE         1           /* white space  */
#define TknASSIGN        2           /* =            */
#define TknYES           3           /* yes          */
#define TknNO            4           /* no           */
#define TknRETURN        5           /* Return.      */

#define TknOPTIONHEADER  6           /* [- OPTIONS -] */
#define TknAUDIBLE       7           /* audible       */
#define TknREBOOT        8           /* reboot        */
#define TknOUTPUT        9           /* output        */
#define TknAUTOEXIT     10            /* autoexit      */

#define TknDEFRAGHEADER 11           /* [- DEFRAGMENTATION -] */
#define TknMETHOD       12           /* audible               */

#define TknSORTHEADER   13           /* [- SORT -] */
#define TknCRITERIUM    14           /* criterium  */
#define TknSORTORDER    15           /* order      */

#define TknASCENDING    16           /* ascending  */
#define TknDESCENDING   17           /* descinding */
#define TknFULL         18           /* full       */
#define TknNORMAL       19           /* normal     */
#define TknUNFRAGMENT   20           /* unfragment */
#define TknSELECTIVE    21           /* selective  */
#define TknNAME         22           /* name       */
#define TknEXTENSION    23           /* extension  */ 
#define TknSIZE         24           /* size       */ 
#define TknDATEANDTIME  25           /* date&time  */

#define PARSERSUCCESS       1

#define YES    1
#define NO     0

int ParseIniFile(char* filename);

#endif
