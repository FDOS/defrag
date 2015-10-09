#ifndef UNIMEM_H_
#define UNIMEM_H_

/* All sizes are in bytes. */

/* Memory unification interface. */
struct UniMemType
{
       /* Data members. */
       int resizable; /* Does the memory need to be resized at allocation 
                         time.                                            */

       unsigned long reserved;  /* Size of the memory that is reserved.   */
       unsigned long allocated; /* Size of this memory that has been 
                                   allocated.                             */

       int handle;              /* Private handle of the memory type.     */ 

       void* buffer;            /* Extra buffer to put individual stuff.  */
       
       /* Function members. */
       int (*Alloc)  (struct UniMemType* this); /* Allocate initial memory. */
       int (*Release)(struct UniMemType* this); /* Release initial memory.  */
       
       int (*Resize)(unsigned long newsize,
                     struct UniMemType* this); /* Resize memory, if possible. */
       
       /* Write to and from memory type. */
       int (*Write) (unsigned long offset, unsigned bytes, void* buffer,
                     struct UniMemType* this);
       int (*Read)  (unsigned long offset, unsigned bytes, void* buffer,
                     struct UniMemType* this);
};

/* Memory set element structure. */

struct MemorySetElement {

       /* data members */
       int               idcode;      /* Memory Type identifier.     */
       int               active;      /* Can the memory be used.     */
       struct UniMemType memtype;     /* Memory type information.    */
       int               next;        /* Next element in the memory 
                                         allocation list.            */
       int               initialised; /* Has the memory type been 
                                         initalised.                 */
       
       /* function members */
       int (*init) (struct UniMemType* memtype);

};

#endif
