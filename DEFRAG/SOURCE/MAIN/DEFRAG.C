#include <stdio.h>
#include <string.h>

#include "..\misc\misc.h"
#include "..\misc\version.h"
#include "..\msdefint\main\msdefint.h"
#include "..\cmdefint\main\cmdefint.h"

static void ShowNoOps (char switchchar);
static void ShowHelp (char switchchar);
static void ShowContributors(void);

static char* Contributors[] = CONTRIBUTORS;

int main(int argc, char *argv[])
{
    char switchchar = SwitchChar();

    /* Check parameters */
    if (argc == 2)
    {
       if ((argv[1][0] == switchchar) || (argv[1][0] == '/'))
       {
          if (argv[1][1] == '?')
          {
             ShowHelp(switchchar);
             return 0;
          }
       }

       /* Show no-ops if requested. */
       if (stricmp(&argv[1][1], "no-ops") == 0)
       {
          ShowNoOps(switchchar);
          return 0;
       }

       if (stricmp(&argv[1][1], "CONTRIB") == 0)
       {
          ShowContributors();
          return 0;
       }
    }

    /* Show copyright on the screen. */
    printf("This program is free software. It comes with ABSOLUTELY NO WARANTIES.\n"
           "You are welcome to redistribute it under the terms of the\n" 
           "GNU General Public License, see http://www.GNU.org for details.\n\n");

    /* See whih interface we should use. */
    if ((argc > 1) && (argv[1][0] == '/') &&
        (stricmp(&argv[1][1], "c") == 0))
    {
       return CMDefint(argc-1, &argv[1]);   /* Command line interface */
    }
    else
    {
       return MSDefint(argc, argv);         /* Interactive user interface */
    }
}

static void ShowNoOps (char switchchar)
{
     printf("FreeDOS defrag " VERSION "\n"
            "Optimizes file loading times by moving file fragments together.\n"
            "\n"
            "(C) 2000 by Imre Leber under the GNU General Public License.\n"
            "\n"
            "List of no-ops included for MS-DOS compatibility.\n"
            "\n"
            "%cSKIPHIGH : this option doesn't do anything.\n"
            "%cLCD      : this option doesn't do anything either.\n"
            "%cBW       : nope, nothing!\n"
            "%cG0       : just forget about this one.\n",
            switchchar,
            switchchar,
            switchchar,
            switchchar);
}

static void ShowHelp (char switchchar)
{
     printf("FreeDOS defrag " VERSION "\n"
            "Optimizes file loading times by moving file fragments together.\n"
            "\n"
            "(C) 2000, 2002 by Imre Leber under the GNU General Public License.\n"
            "\n"
            "defrag [%cC] [<drive>:] [{%cF|%cU}] [%cSorder[-]] [%cB] [%cX] [%cA] [%cFO]\n"
            "\n"
            "drive : drive letter of drive to optimize.\n"
            "\n"
            "%cC : Use command line version only.\n"
            "%cF : Fully optimizes specified drive.\n"
            "%cU : Unfragments files, possibly leaving space between files.\n"
            "%cS : Sort files by specified order.\n"
            "      order: N  by Name (alphabetic)            E by Extension (alphabetic)\n"
            "             D  by Date & time (earliest first) S by Size (smallest first)\n"
            "      - suffix to sort in descending order.\n"
            "%cB : Restarts computer after optimization.\n"
            "%cX : Allways automatically exits.\n"
            "%cA : Audible warning before user action.\n"
            "%cFO: Full textual output (only relevant when used with %cC option).\n"
            "\n"
            "Remarks:\n"
            "    Type defrag %cno-ops for a list of no-ops.\n"
            "    Type defrag %ccontrib for a list of contributors.",
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,
            switchchar,            
            switchchar);
}

static void ShowContributors()
{
    int i;

    printf("Main programmer: Imre Leber\n\n");
    
    printf("Contributors to FreeDOS defrag:\n");
    
    for (i = 0; i < AMOFCONTRIBUTORS; i++)
        printf("%s\n", Contributors[i]);

    puts("\nWeb site: " WEBSITE);
    puts("E-Group:  "  EGROUP);
}

