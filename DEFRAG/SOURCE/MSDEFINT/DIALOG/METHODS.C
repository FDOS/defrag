/*    
   Methods.c - method dialog box.
   Copyright (C) 2000 Imre Leber

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be
*/

#include <stdlib.h>

#include "dialog.h"
#include "methods.h"
#include "..\screen\screen.h"
#include "..\winman\control.h"
#include "..\winman\window.h"
#include "..\winman\winman.h"
#include "..\winman\controls.h"
#include "..\winman\slctbtn.h"

#include "..\..\misc\bool.h"

#include "..\..\modlgate\defrpars.h"

#include "..\helpsys\hlpindex.h"
#include "..\helpsys\idxstack.h"

#define DIALOG_X      10
#define DIALOG_Y       7
#define DIALOG_X_LEN  60
#define DIALOG_Y_LEN   9 
#define SELECTION_X   DIALOG_X + 2
#define TEXT_X        DIALOG_X + (DIALOG_X_LEN / 2) + 2
#define BUTTON_LENGTH 10

#define BUTTON_X1     DIALOG_X + (DIALOG_X_LEN / 2) - BUTTON_LENGTH - 2
#define BUTTON_X2     DIALOG_X + (DIALOG_X_LEN / 2) + 1

#define BUTTON_Y      DIALOG_Y + 7
#define PUSHLEN       (DIALOG_X_LEN / 2) - 2

#define HIGHLIGHTFOR WHITE

#define AMOFCONTROLS 7

static struct Control controls[AMOFCONTROLS];

#define CANCELBUTTON 6



extern struct CommandButton OkButton;
extern struct CommandButton CancelButton;

#define AD &controls[3]

static struct SelectButton
 SButtons[] = 
   {{"Full Optimization", FALSE, HIGHLIGHTFOR, PUSHLEN, 0, AD, 2, NULL},
    {"Unfragment Files only", FALSE, HIGHLIGHTFOR, PUSHLEN, 1, AD, 2, NULL}};

static struct Window MethodWin = {DIALOG_X, DIALOG_Y,
                                  DIALOG_X_LEN, DIALOG_Y_LEN,
                                  DIALOGBACKCOLOR, DIALOGFORCOLOR,
                                  " Select optimization method ",
                                  controls,
                                  AMOFCONTROLS};

static void Initialize(void)
{
    controls[0] = CreateLabel("Fully optimizes your disk",
                              DIALOGFORCOLOR, DIALOGBACKCOLOR,
                              TEXT_X, DIALOG_Y + 2);

    controls[1] = CreateLabel("Unfragment files only,",
                              DIALOGFORCOLOR, DIALOGBACKCOLOR,
                              TEXT_X, DIALOG_Y + 4);
    
    controls[2] = CreateLabel("possibly leaving holes",
                              DIALOGFORCOLOR, DIALOGBACKCOLOR,
                              TEXT_X, DIALOG_Y + 5);

    controls[3] = CreateSelectionButton(&SButtons[0],
                                        DIALOGFORCOLOR, DIALOGBACKCOLOR,
                                        SELECTION_X, DIALOG_Y + 2);

    controls[4] = CreateSelectionButton(&SButtons[1],
                                        DIALOGFORCOLOR, DIALOGBACKCOLOR,
                                        SELECTION_X, DIALOG_Y + 4);

     controls[5] = CreateCommandButton(&OkButton, 
                                       BUTTONFORCOLOR, BUTTONBACKCOLOR,
                                       BUTTON_X1, BUTTON_Y, TRUE, FALSE,
                                       FALSE);
     
     controls[6] = CreateCommandButton(&CancelButton, 
                                       BUTTONFORCOLOR, BUTTONBACKCOLOR,
                                       BUTTON_X2, BUTTON_Y, FALSE, TRUE, 
                                       TRUE);
}

int AskOptimizationMethod(Method* method)
{
    static int Initialized = FALSE;

    int control, result;

    PushHelpIndex(OPTIMIZATION_METHOD_INDEX);
    
    if (!Initialized)
    {
       Initialize();
       Initialized = TRUE;
    }
     
    SetStatusBar(RED, WHITE, "                                            ");
    SetStatusBar(RED, WHITE, " Change the current optimization method.");
    
    SButtons[GetOptimizationMethod()].selected = TRUE;
    
    OpenWindow(&MethodWin);
    control = ControlWindow(&MethodWin);
    CloseWindow();

    if ((control == CANCELBUTTON) || (control == -1))
       result = FALSE;
    else
       result = TRUE;

    *method = (SButtons[0].selected) ? FULL_OPTIMIZATION: UNFRAGMENT_FILES;
    
    SButtons[0].selected = FALSE;
    SButtons[1].selected = FALSE;
       
    PopHelpIndex();
    
    return result;
}
