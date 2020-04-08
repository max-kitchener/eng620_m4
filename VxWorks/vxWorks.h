
/* vxWorks.h - based on the original one below but modified to split the standard
               WindRiver header files  -- Rinat Khusainov
 */

/*****************************************************************************
 * vxw_hdrs.h - includes the Wind River VxWorks (R) definitions and
 *              function prototypes needed to compile applications programs
 *              in the v2pthreads environment.
 *              NOTE that the actual original VxWorks (R) header files
 *              introduce conflicts with some native Linux header files
 *              and consequently cannot be used in the pthreads environment.
 *  
 * Copyright (C) 2000, 2001  MontaVista Software Inc.
 *
 * Author : Gary S. Robertson
 *
 * VxWorks is a registered trademark of Wind River Systems, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 ****************************************************************************/
#ifndef __VXW_HDRS_H
#define __VXW_HDRS_H

#if __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "vxw_defs.h"
#include "v2pthread.h"

/*
**  Object Compatibility data types
**
**  These data types are defined here to provide source code compatibility with
**  existing vxWorks code.  NOTE that they do not necessarily correspond to the
**  actual Wind River definitions for the defined types, nor to the actual
**  types of the objects they reference as defined in the v2pthreads
**  environment.  THIS MAY CAUSE LINKER ERRORS IF TYPE-SAFE LINKAGE IS USED!
**  It works okay with standard ANSII C-style linkage, however. 
*/
typedef void     *MSG_Q_ID;
typedef void     *SEM_ID;
typedef void     *WDOG_ID;
typedef void     WIND_TCB;
typedef int      SEM_B_STATE;
typedef void     *FUNCPTR;


/*
**
** This function must be called ASAP in main()
**
*/
extern int v2lin_init( void );

/*
**  Round-Robin Scheduling Control
**
**  The following three functions are unique to v2pthreads. 
**  They are used to manipulate a global system setting which affects all
**  tasks spawned or initialized after the round-robin control call is made.
**  They have no effect on tasks spawned or initialized prior to the call.
**  Round-Robin scheduling causes tasks at the same priority level to be
**  scheduled on a 'time-sliced' basis within that priority level, so that
**  all tasks at a given priority level get an equal opportunity to execute.
**  Round-robin scheduling is TURNED OFF by default.
*/
extern void      disableRoundRobin( void );
extern void      enableRoundRobin( void );
extern BOOL      roundRobinIsEnabled( void );
extern STATUS    kernelTimeSlice( int ticks_per_quantum );



/* Adjust the return value here to get the correct delays */
int sysClkRateGet() {
    return 200;
}

#if __cplusplus
}
#endif

#endif // __VXW_HDRS_H
