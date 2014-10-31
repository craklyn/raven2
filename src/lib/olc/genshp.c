/**************************************************************************
*  File: genshp.c                                          Part of tbaMUD *
*  Usage: Generic OLC Library - Shops.                                    *
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
#include "olc/genolc.h"
#include "olc/genshp.h"
#include "olc/genzon.h"


/* NOTE (gg): Didn't modify sedit much. Don't consider it as 'recent' as the
 * other editors with regard to updates or style. */

/* local (file scope) functions */
static void copy_shop_list(IDXTYPE **tlist, IDXTYPE *flist);
static void copy_shop_type_list(struct shop_buy_data **tlist, struct shop_buy_data *flist);
static void free_shop_strings(struct shop_data *shop);
static void free_shop_type_list(struct shop_buy_data **list);

