#ifndef DEFRAG_PARAMETERS_H_
#define DEFRAG_PARAMETERS_H_

/* Constants for defragmentation method. */
#define FULL_OPTIMIZATION 0
#define UNFRAGMENT_FILES  1

/* Constants for sort options. */
#define UNSORTED          0
#define NAMESORTED        1
#define EXTENSIONSORTED   2
#define DATETIMESORTED    3
#define SIZESORTED        4

/* Sort orders. */       
#define ASCENDING         0
#define DESCENDING        1

void SetOptimizationDrive(char drive);
char GetOptimizationDrive(void);
void SetSortOptions(int criterium, int order);
int  GetSortCriterium (void);
void SetOptimizationMethod(int method);
int  GetOptimizationMethod(void);
int  GetSortOrder (void);

#endif
