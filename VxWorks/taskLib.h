
/* VxWorks taskLib functions -- Rinat Khusainov */


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
#ifndef __VXW_TLIB_H
#define __VXW_TLIB_H

#if __cplusplus
extern "C" {
#endif

#include "vxWorks.h"

/*
**  taskLib Function Prototypes
*/

extern int       taskSpawn( char *name, int pri, int opts, int stksize,
                            FUNCPTR entry, int arg1, int arg2, int arg3,
                            int arg4, int arg5, int arg6, int arg7, int arg8,
                            int arg9, int arg10 );

extern STATUS    taskInit( WIND_TCB *tcb, char *name, int pri,
                           int opts, char *pstack, int stksize,
                           FUNCPTR entry, int arg1, int arg2, int arg3,
                           int arg4, int arg5, int arg6, int arg7, int arg8,
                           int arg9, int arg10 );
extern STATUS    taskActivate( int taskId );
extern STATUS    taskDelete( int taskId );
extern STATUS    taskDeleteForce( int taskId );
extern STATUS    taskSuspend( int taskId );
extern STATUS    taskResume( int taskId );
extern STATUS    taskRestart( int taskId );
extern STATUS    taskPrioritySet( int taskId, int priority );
extern STATUS    taskPriorityGet( int taskId, int *priority );
extern STATUS    taskLock( void );
extern STATUS    taskUnlock( void );
extern STATUS    taskSafe( void );
extern STATUS    taskUnsafe( void );
extern STATUS    taskDelay( int ticks_to_wait );
extern char      *taskName( int taskId );
extern int       taskNameToId( char *task_name );
extern STATUS    taskIdVerify( int taskId );
extern int       taskIdSelf( void );
extern int       taskIdDefault( int taskId );
extern BOOL      taskIsReady( int taskId );
extern BOOL      taskIsSuspended( int taskId );
extern WIND_TCB  *taskTcb( int taskId );
extern int       taskIdListGet( int list[], int maxIds );

#if __cplusplus
}
#endif

#endif // __VXW_HDRS_H
