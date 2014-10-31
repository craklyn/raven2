/* ***********************************************************************
*    File:   genqst.c                                 Part of CircleMUD  *
* Version:   2.0 (November 2005) Written for CircleMud CWG / Suntzu      *
* Purpose:   To provide special quest-related code.                      *
* Copyright: Kenneth Ray                                                 *
* Original Version Details:                                              *
* Copyright 1996 by Harvey Gilpin                                         *
* Copyright 1997-2001 by George Greer (greerga@circlemud.org)             *
*                                                                         *
*  File mangled and defiled during RavenMUD OLC source code on 08.25.09   *
*  Upgrade base source code: tbaMUD 3.59                                  *
*  Ported by: Brooks(Xiuhtecuhtli)                                        *
**************************************************************************/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "actions/quest.h"
#include "olc/genolc.h"
#include "olc/genzon.h"


/*-------------------------------------------------------------------*/
