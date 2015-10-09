#ifndef KEYBOARD_H_
#define KEYBOARD_H_

int  AltKeyDown    (void);
int  KeyPressed    (void); /* Returns boolean. */
int  ReadKey       (void);
void ClearKeyboard (void);

int AltKeyPressed (void);

#endif
