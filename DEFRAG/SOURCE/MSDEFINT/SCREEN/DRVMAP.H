#ifndef DRIVEMAP_H_
#define DRIVEMAP_H_

void DrawDrvMap(CLUSTER maxcluster);

void DrawWriteBlock(CLUSTER cluster);
void DrawReadBlock(CLUSTER cluster);
void DrawBadBlock(CLUSTER cluster);
void DrawUnmovableBlock(CLUSTER cluster);
void DrawUnusedBlock(CLUSTER cluster);
void DrawOptimizedBlock(CLUSTER cluster);
void DrawUsedBlock(CLUSTER cluster);

#endif
