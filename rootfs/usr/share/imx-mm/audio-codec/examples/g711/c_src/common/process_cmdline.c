/***********************************************************************s
 * Copyright 2005-2010 by Freescale Semiconductor, Inc.
 * All modifications are confidential and proprietary information
 * of Freescale Semiconductor, Inc. ALL RIGHTS RESERVED.
 ***********************************************************************/

/*==================================================================================================

    Module Name:  process_cmdline.c

    General Description: test the G711 Codec.

====================================================================================================


Revision History:
                            Modification     Tracking
Author                          Date          Number     Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
Vineet Golchha              Apr/20/2006        1.0          Initial Version
Neha Srivastava             Jan/24/2007        1.1          Elinux Ported Version
====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/
#include "process_cmdline.h"
#include "stdio.h"
#include "string.h"

/*==================================================================================================

FUNCTION: G711ProcessCmdLineOptions
DESCRIPTION:

ARGUMENTS PASSED:
   int Argc    : number of command line  parameters
   char *Argv[]  : pointer to buffer containing command line arguments
   unsigned char *pMode : pointer to the type of mode

RETURN VALUE:
   status

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

IMPORTANT NOTES:
   None

==================================================================================================*/
char G711ProcessCmdLineOptions (int Argc, char *Argv[], unsigned char *pMode)
{
	if ( Argc < 4 )
    {
 	  CommandLineFormat();
      return G711_ARGUMENTS_FAIL;
    }
    else
    {
		if ( strcmp( Argv[1], "A" ) == 0 )
        {
          *pMode = 0;
        }
        else if ( strcmp( Argv[1], "M" ) == 0 )
        {
          *pMode = 1;
        }
        else if ( strcmp( Argv[1], "A2M" ) == 0 )
        {
            *pMode = 2;
        }
        else if ( strcmp( Argv[1], "M2A" ) == 0 )
        {
            *pMode = 3;
        }
        else
        {
			CommandLineFormat();
          return G711_ARGUMENTS_FAIL;
        }

    }

  return G711_ARGUMENTS_OK;
}
/*==================================================================================================

FUNCTION: CommandLineFormat
DESCRIPTION: G.711 Command line menu
ARGUMENTS PASSED:
    None
RETURN VALUE:
   None

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

IMPORTANT NOTES:
   None

==================================================================================================*/

void CommandLineFormat()
{

 printf ("\n G711 codec usage arguments : ");
 printf ("\n-------------------------------------");
 printf ("\n <exe>  [ Options ] <InFile> <OutFile> ");
 printf ("\n ");
 printf ("\n <exe>              : is the name of the G711 executable");
 printf ("\n Options :");
 printf ("\n ");
 printf ("\n Encoder :");
 printf ("\n A/M/A2M/M2A        : select A for A-law, M for Mu-law, A2M for Alaw to Mulaw, M2A for Mulaw to Alaw\n");
 printf ("\n Decoder :");
 printf ("\n A/M                : select A for A-law and M for Mu-law\n");
 printf ("\n InFile             : Input filename");
 printf ("\n OutFile            : Output filename\n\n");
}



