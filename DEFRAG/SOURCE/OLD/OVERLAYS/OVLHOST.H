#ifndef OVLHOST_H_
#define OVLHOST_H_

/* In ovlhost.asm */
void  SaveHostState(void);
void  GetDispatcher(int* segment, int* offset);

/* Host routine selector */
int SelectHostRoutine(int ax, int bx, int cx, int dx, int ds);

/* Overlay spawn routine */
int SpawnOverlay(char* overlay, char* parameters);

#endif
