/*
 ***********************************************************************
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************
 */

#include "strfunc.h"
#include <stdlib.h>
#include <string.h>

#ifdef CYCLE_MEASUREMENT
int mystrcmp(const char* pcStr1, const char* pcStr2)
{
    const char* pcLocal1 = pcStr1;
    const char* pcLocal2 = pcStr2;
    int   diff;

    /* TEDebug ("Comparing strings %s and %s\n", pcStr1, pcStr2); */

    do
    {
        diff = (int)(*pcLocal1 - *pcLocal2);

        if (diff != 0)
        {
            /* TEDebug ("Returning %d at position %d\n", diff, pcLocal1 - pcStr1
            ); */
            return diff;
        }

        if (*pcLocal1 == '\0') /* means other is also null */
            return 0;

        pcLocal1++;
        pcLocal2++;
    } while (1);

    /* return 0; */  /* matched, but code should not come here */
}

#endif
