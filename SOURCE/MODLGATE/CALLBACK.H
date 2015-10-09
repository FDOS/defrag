#ifndef CALLBACK_H_
#define CALLBACK_H_

struct CallBackStruct 
{
    void (*UpdateInterface)(void);
    void (*SmallMessage)(char* buffer);
    void (*LargeMessage)(char* buffer);
    void (*DrawDriveMap)(CLUSTER maxcluster);
    void (*DrawOnDriveMap)(CLUSTER cluster, int symbol);
    void (*LogMessage)(char* message);
};

void SetCallBacks(struct CallBackStruct* callbacks);

#endif
