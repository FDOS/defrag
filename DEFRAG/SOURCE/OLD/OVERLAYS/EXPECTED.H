#ifndef EXPECTED_H_ 
#define EXPECTED_H_

/* Routines expected to be implemented in the interface. */

void UpdateInterfaceState(void);
void SmallMessage(char* buffer);
void LargeMessage(char* buffer);
void DrawOnDriveMap(int cluster, int symbol);
void DrawDriveMap(unsigned maxcluster);

#define WRITESYMBOL     1
#define READSYMBOL      2
#define USEDSYMBOL      3
#define UNUSEDSYMBOL    4
#define BADSYMBOL       5
#define UNMOVABLESYMBOL 6   
#define OPTIMIZEDSYMBOL 7

#endif
