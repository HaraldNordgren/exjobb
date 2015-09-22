#ifndef D65_DEBLOCKING_FILTER_H
#define D65_DEBLOCKING_FILTER_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for deblocking filtering
*
******************************************************************************/
#include "string.h"
#include "d65_api.h"
#include "d65_internal.h"
#include "d65_init.h"

#if MULTI_THREADED
#include "pthread.h"
#endif

/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        DeblockingFilter
*
* Parameters:  Sessiondata_p  [in/out]  The decoder
*
* Returns:     void
*
* Description: Deblocking filtering for current picture
*
******************************************************************************/
void
DeblockingFilter(SessionData_t *Sessiondata_p);
#endif