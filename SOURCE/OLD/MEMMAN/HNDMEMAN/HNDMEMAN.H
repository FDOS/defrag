#ifndef HNDMEMAN_H_
#define HNDMEMAN_H_

/* Block information struct: */
struct BlockInfo {
    unsigned long Offset;   /* Offset of the block in the unified memory. */
    unsigned long Length;   /* Length of the block in the unified memory. */

    int  next;         /* forward pointer in the handle allocation list.  */
    int  previous;     /* backward pointer in the handle allocation list. */
};

#define AMOFHANDLES 8       /* Amount of managed handles. */

#endif
