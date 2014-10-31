/**************************************************************************
*  File: genzon.c                                          Part of tbaMUD *
*  Usage: Generic OLC Library - Zones.                                    *
*                                                                         *
*  Copyright 1996 by Harvey Gilpin, 1997-2001 by George Greer.            *
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
#include "olc/genolc.h"
#include "olc/genzon.h"
#include "scripts/dg_scripts.h"

/* local functions */
static void remove_cmd_from_list(struct reset_com **list, int pos);

