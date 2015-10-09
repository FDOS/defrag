#ifndef ABSDSK_H_
#define ABSDSK_H_

int abs16supported(void);

int absread12  (int drive, int nsects, unsigned lsect, void* buffer);
int abswrite12 (int drive, int nsects, unsigned lsect, void* buffer);

int absread16  (int drive, int nsects, unsigned long lsect, void* buffer);
int abswrite16 (int drive, int nsects, unsigned long lsect, void* buffer);

int AbsRead  (int drive, int nsects, unsigned long lsect, void* buffer);
int AbsWrite (int drive, int nsects, unsigned long lsect, void* buffer);

#endif
