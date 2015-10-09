
#include "..\..\misc\bool.h"

#define AMOFMEMTYPES        /* amount of elements in the memory set/array */

extern int UnifyXMS(struct UniMemType* memtype);
extern int UnifyEMS(struct UniMemType* memtype);
extern int UnifySwapFile(struct UniMemType* memtype);
extern int UnifyMainMem(struct UniMemType* memtype);

#define XMS_CODE      1
#define EMS_CODE      2
#define MAINMEM_CODE  3 
#define SWAPFILE_CODE 4

struct MemorySetElement MemorySet[] =
/* This table lists all the memory types in the system.              */
/*                                                                   */
/* All memory types are identified by an identification code.        */
/* Every memory type can be active.                                  */
/* All memory types should have a memory unification function        */
/* which installs the corresponding memory driver.                   */
/*                                                                   */
/*         id.code        | active | filler | filler | init func     */
	   {{XMS_CODE,      TRUE,    {0},     0,       UnifyXMS},
	    {EMS_CODE,      TRUE,    {0},     0,       UnifyEMS},
	    {SWAPFILE_CODE, TRUE,    {0},     0,       UnifySwapFile},
	    {MAINMEM_CODE,  TRUE,    {0},     0,       UnifyMainMem}};
