/**************************************************************************
*  File: genobj.c                                          Part of tbaMUD *
*  Usage: Generic OLC Library - Objects.                                  *
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
#include "specials/boards.h"
#include "specials/shop.h"
#include "olc/genolc.h"
#include "olc/genobj.h"
#include "olc/genzon.h"
#include "scripts/dg_olc.h"
#include "magic/spells.h"
#include "general/handler.h"


/* local functions */
static int update_all_objects(struct obj_data *obj);
static void copy_object_strings(struct obj_data *to, struct obj_data *from);
