/**************************************************************************
*  File: genmob.c                                          Part of tbaMUD *
*  Usage: Generic OLC Library - Mobiles.                                  *
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
#include "specials/shop.h"
#include "general/handler.h"
#include "olc/genolc.h"
#include "olc/genmob.h"
#include "olc/genzon.h"
#include "scripts/dg_olc.h"
#include "magic/spells.h"

/* local functions */
static void extract_mobile_all(mob_vnum vnum);
