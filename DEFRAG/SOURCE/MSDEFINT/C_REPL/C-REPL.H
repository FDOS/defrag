#ifndef C_REPL_H_
#define C_REPL_H_

/* strings.asm */
int  StringLength(char*);
void StringCopy(char* dest, char* src);

/* time.asm */
void WasteTime(int delay);

/* higdscreen.c */
void GrabScreen(int left, int top, int right, int bottom, void* dest);
void DumpScreen(int left, int top, int right, int bottom, void* src);

/* inttostr.c */
char* IntToString(int value, char* buf);

#endif
