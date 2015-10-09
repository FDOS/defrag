#ifndef MODULES_H_
#define MODULES_H_

int InfoFAT(char* drive);
int CheckFAT(char* drive);
int SortFAT(char* drive, int criterium, int order);
int DefragFAT(char* drive, int method);

#endif
