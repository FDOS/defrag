#ifndef MAINMEM_H_
#define MAINMEM_H_

#define MainMemCoreLeft farcoreleft

int  MainMemAlloc(unsigned long initial);
void MainMemClose(void);
int  MainMemRead(unsigned long offset, unsigned bytes, void* toread);
int  MainMemWrite(unsigned long offset, unsigned bytes, void* towrite);
int MainMemTruncate(unsigned long newsize);

#endif
