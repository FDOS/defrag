#ifndef SWAPFILE_H_
#define SWAPFILE_H_

int DiskInit(void);
void DiskClose(void);
unsigned long DiskCoreLeft(void);
int DiskWrite(unsigned long offset, unsigned int bytes, void* towrite);
int DiskRead(unsigned long offset, unsigned int bytes, void* toread);
int TruncateSwapFile(unsigned long offset);

#endif
