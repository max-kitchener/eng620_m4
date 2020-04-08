
/* vxWorks wdLib functions -- Rinat Khusainov */

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
#ifndef __VXW_WDLIB_H
#define __VXW_WDLIB_H

#if __cplusplus
extern "C" {
#endif

#include "vxWorks.h"

/*
**  wdLib Function Prototypes
*/
extern STATUS    wdCancel( WDOG_ID wdId );
extern WDOG_ID   wdCreate( void );
extern STATUS    wdDelete( WDOG_ID wdId );
extern STATUS    wdStart( WDOG_ID wdId, int delay, FUNCPTR funcptr, int parm );

#if __cplusplus
}
#endif

#endif // __VXW_HDRS_H
