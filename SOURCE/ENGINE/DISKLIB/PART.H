/*
 * part.h
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#pragma pack(1)

struct partition {
   unsigned char bootable;			/* 80h or 0 */
   unsigned char start_head;		/* dh location of first sector (boot_sector) */
   unsigned char start_sector;		/* ch */
   unsigned char start_cylinder;	/* cl */
   unsigned char system;
   unsigned char end_head;			/* location of last sector */
   unsigned char end_sector;
   unsigned char end_cylinder;
   unsigned long start_sector_abs;	/* start_cylinder * heads * sectors */
									/*	+ start_head * sectors + start_sector - 1 */
   unsigned long no_of_sectors_abs; /* end_cylinder * heads * sectors + end_head * sectors */
									/*	+ end_sector - start_sector_abs */
};

struct mbr {
    unsigned char code[446];
	struct partition part[4];
	unsigned short sig;
};

#define PRIMARY_TYPE_OFFSET 446+5

#pragma pack()


struct PARTITION_TYPE {
    int type;
    char *name;
    char *desc;
};

struct PARTITION_TYPE *partition_type(int type);
