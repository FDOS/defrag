#ifndef SCANNER_H_
#define SCANNER_H_

/* Return codes. */
#define SCANFILENOTFOUND  0
#define SCANFILEERROR    -1
#define SCANSUCCESS       1

/* ini file name. */
#define INIFILENAME "defrag.ini"

#define INIVARIABLELOW "defrini"
#define INIVARIABLEHI  "DEFRINI"

int   ReadScannerFile(char* filename);
void  CloseScanner(void);
void  RestartScanning(void);
TOKEN GetNextToken(void);
char* ScannerString(void);
TOKEN PeekToken(void);
int   GetScannerLine(void);

char* GetIniDir(char* filename, char* buffer);

#endif
