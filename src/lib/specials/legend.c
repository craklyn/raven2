
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"


SPECIAL(una)
{
  CharData *una = me;
  ObjData*  obj = NULL;

  if( !ch->desc || IS_NPC(ch) ) return FALSE;

  /*
  ** Una is looking for a kiss
  */
  if( !CMD_IS("kiss" )) return FALSE;

  /*
  ** Rather than hardcoding an object Id let's use a sybolic name
  */
  if( !(obj = get_obj_in_list_vis(una, "unakey", una->carrying)) ) return FALSE;

  /*
  ** Ok, she has it, ch kissed her, fork it over
  */
  perform_give( una, ch, obj);

  return( TRUE );
}


SPECIAL(alicorn)
{
  CharData* alicorn     = NULL;
  char      tgtname[80] = "";
  char      objname[80] = "";
  ObjData*  obj         = NULL;
  
  if( !ch->desc || IS_NPC(ch) ) return FALSE;

  if( !CMD_IS("give" )) return FALSE;

  argument = one_argument(argument, objname);

  if( !*objname ) return FALSE;

  one_argument(argument, tgtname);

  if(( alicorn = give_find_vict( ch, tgtname )) == NULL ) return TRUE;

  if(( obj = get_obj_in_list_vis( ch, objname, ch->carrying )) == NULL ) return FALSE;

  perform_give( ch, alicorn, obj );

  if( GET_OBJ_VNUM(obj) == 15722 )
  {
    ObjData* key = get_obj_in_list_vis( alicorn, "alicornkey", alicorn->carrying);

    if( key != NULL )
      perform_give( alicorn, ch, key);
  }

  return( TRUE );
}

