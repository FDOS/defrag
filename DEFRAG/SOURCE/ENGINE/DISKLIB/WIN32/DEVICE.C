/*
 * win32\device.c   WIN32 device driver I/O
 *
 * This file is part of the BETA version of DISKLIB
 * Copyright (C) 1998, Gregg Jennings
 *
 * See README.TXT for information about re-distribution.
 * See DISKLIB.TXT for information about usage.
 *
 */

#ifdef TRACE
#include <stdio.h>
#endif

#include "win32.h"
#include "dosio.h"
#include "debug.h"

/*
 * win_device_io    device IOCTL
 *
 */

/*
    From Microsoft Knowledge Base Article ID: Q125712

    Windows 95 also does not support the DeviceIoControl() IOCTL
    APIs.... Instead, low-level disk access in Windows 95 can be
    achieved through DeviceIoControl() calls to the VWIN32 VxD.
*/

extern int win_device_io(DWORD control, DIOC_REGISTERS *regs)
{
BOOL i;
DWORD cb;
static HANDLE VxDevice = 0;

    if (VxDevice == 0) {
        VxDevice = CreateFile("\\\\.\\VWIN32", 0, 0, NULL, 0, 0, NULL);
        if (VxDevice == INVALID_HANDLE_VALUE) {
            DBG_err_dump("device");
            VxDevice = 0;
            return DEVICE_ERR;

            /*
            Under Windows NT CreateFile() fails and GetLastError()
            returns "The parameter is incorrect." rather than
            something sane like "File/Device not found.".
            */
        }
    }

#ifdef TRACE
    printf("(%d)",control);
#endif
    DBG_reg_dump(regs);

    i = DeviceIoControl(VxDevice, control,
                        regs, sizeof(DIOC_REGISTERS),
                        regs, sizeof(DIOC_REGISTERS), &cb, 0);

    DBG_reg_dump(regs);

    if (i == FALSE) {
        DBG_err_dump("deviceio");
        return DOS_ERR;
    }

    return DEVICE_OK;
}
