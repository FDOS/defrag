#ifndef MISC_H_
#define MISC_H_

/* In Switchch.asm */
char SwitchChar(void);

/* In critical.asm */
void SetCriticalHandler(int (*handler)(int status));
void RenewCriticalHandler(int (*handler)(int status));

/* In hicritical.asm */
void CriticalHandlerOn(void);
int  CriticalErrorOccured(void);
int  GetCriticalCause(void);
int  GetCriticalStatus(void);

/* In gtdrvnms.c */
void GetDriveNames(char* drives);

/* In gtcdex.asm */
int CDEXinstalled(void);
char* GetCDROMLetters(void);
int IsCDROM(int drive);

#endif
