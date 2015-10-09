#ifndef OVLSLAVE_H_ 
#define OVLSLAVE_H_

/* Overlay main routine. */
int  OvlMain(int argc, char** argv);

/* Slave initialisation. */
void SetHostAddress(int segment, int offset);

/* Stub routines. */
void SendNOP (void);
void SmallMessage (char* message);
void LargeMessage (char* message);

#endif
