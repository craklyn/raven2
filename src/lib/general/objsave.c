/* ************************************************************************
*   File: objsave.c                                     Part of CircleMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "general/mudconfig.h" /* for the default config values. */
#include "magic/spells.h"
#include "util/weather.h"
#include "general/objsave.h"
#include "magic/sing.h"
#include "general/color.h"        /* for procColor */
#include "actions/act.alias.h"

/* Local file scope functions */
static void write_saved_corpses(void);
static void Crash_report_rent(CharData *ch, CharData *recep, ObjData *obj,
        long *cost, long *nitems, int display, int factor, int modifier );

/* these factors should be unique integers */
#define RENT_FACTOR 	1
#define CRYO_FACTOR 	4

/* ============================================================================ 
A list of room vnum => % modifier, that % items can be stored for that % cost
============================================================================ */
struct rent_modifiers {
  int vnum;
  int modifier;
} rent_modifiers[] = {
  {13826, 120},
  {33191, 90},
  {-1, 100}         /* terminate the list */
};


ObjData *
Obj_from_store( ObjFileElem store )
{
  int taeller;

  if( real_object(store.item_number) > -1 )
  {
    int j;
    ObjData *obj = read_object( store.item_number, VIRTUAL );

#define UPDATE_NECRO_FLAGS
#ifdef UPDATE_NECRO_FLAGS
    if (IS_OBJ_STAT(obj, ITEM_ANTI_NECROMANCER))
        SET_BIT_AR(store.extra_flags, ITEM_ANTI_NECROMANCER);
#endif

#define zVERIFY_OBJ_FLAGS
#ifdef VERIFY_OBJ_FLAGS
    int ignore_flags[] = {
      ITEM_CURSED, ITEM_MAGIC, ITEM_NOSELL, ITEM_EXPLODES, ITEM_TIMED, -1
    };

    for (j = 0; ignore_flags[j] != -1; j++) {
      REMOVE_BIT_AR(obj->obj_flags.extra_flags, ignore_flags[j]);
      if (IS_SET_AR(store.extra_flags, ignore_flags[j]))
        SET_BIT_AR(obj->obj_flags.extra_flags, ignore_flags[j]);
    }

    for (taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
      if (obj->obj_flags.extra_flags[taeller] != store.extra_flags[taeller])
        break;
    if (taeller != EF_ARRAY_MAX) {
      char buf[512];
      int pos;

      pos = sprintf(buf, "Object #%d '%s' has updated flags: ",
          store.item_number, obj->short_description);
      for (taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
        for (j = 0; j < 32; j++) {
          if ((obj->obj_flags.extra_flags[taeller] & (1 << j))
              != (store.extra_flags[taeller] & (1 << j)))
            pos += sprintf(buf + pos, "%c%s ",
                store.extra_flags[taeller] & (1 << j) ? '-' : '+',
                extra_bits[taeller * 32 + j]);
        }
      buf[pos - 1] = '\0';
      mudlog( CMP, LVL_IMMORT, TRUE, buf);
    }
#endif

    for(taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
      obj->obj_flags.extra_flags[taeller] = store.extra_flags[taeller];

    for(taeller = 0; taeller < AF_ARRAY_MAX; taeller++)
      obj->obj_flags.bitvector[taeller] = store.bitvector[taeller];


    for( j = 0; j < MAX_OBJ_AFFECT; j++ )
      obj->affected[j] = store.affected[j];

    GET_OBJ_VAL(obj, 0) = store.value[0];
    GET_OBJ_VAL(obj, 1) = store.value[1];
    GET_OBJ_VAL(obj, 2) = store.value[2];
    GET_OBJ_VAL(obj, 3) = store.value[3];

    obj->worn_at        = store.position;
    obj->container      = store.container;
    GET_OBJ_WEIGHT(obj) = (GET_OBJ_WEIGHT(obj) < 0 ? 1 : store.weight );
    GET_OBJ_TIMER(obj)  = store.timer;

    GET_TRAP(obj) = store.traplevel;

    obj->rename_slot    = store.rename_slot;
    if (obj->rename_slot > 0 && obj->rename_slot <= top_of_rename_t) {
        obj->short_description = strdup(renames[obj->rename_slot - 1].name);
        desc_to_aliases(obj);
    } else
        obj->rename_slot = 0;

    return obj;
  }

  else
    return NULL;
}

void desc_to_aliases(ObjData *obj)
{
    char *aliases = strdup(obj->short_description);

    procColor(aliases, 0);
    obj->name = aliases;
}

int Obj_into_store(ObjData * obj, ObjFileElem *store)
{
    int j, taeller;

    store->item_number = GET_OBJ_VNUM(obj);
    store->value[0]    = GET_OBJ_VAL(obj, 0);
    store->value[1]    = GET_OBJ_VAL(obj, 1);
    store->value[2]    = GET_OBJ_VAL(obj, 2);
    store->value[3]    = GET_OBJ_VAL(obj, 3);

    for (taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
         store->extra_flags[taeller] = obj->obj_flags.extra_flags[taeller];

    for(taeller = 0; taeller < AF_ARRAY_MAX; taeller++)
        store->bitvector[taeller] = obj->obj_flags.bitvector[taeller];

    store->weight      = ( GET_OBJ_WEIGHT(obj) > 0 ? GET_OBJ_WEIGHT(obj) : 1 );
    store->timer       = GET_OBJ_TIMER(obj);

    store->position    = obj->worn_at;
    store->traplevel   = GET_TRAP(obj);
    store->container   = obj->container;

    store->rename_slot = obj->rename_slot;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        store->affected[j] = obj->affected[j];

    return 0;
}

int Obj_to_store(ObjData * obj, FILE * fl)
{
    struct obj_file_elem object;

    Obj_into_store(obj, &object);

    if( fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1 ){
        perror("Error writing object in Obj_to_store");
        return 0;
    }
    return 1;
}


int
crashDeleteFile(char *name)
{
  char filename[50];
  char crash_file[60];
  FILE *fl;

  if( !get_filename( name, filename, CRASH_FILE )) return 0;

  if( !(fl = fopen(filename, "rb")) )
  {
    if( errno != ENOENT )
      mudlog(NRM, LVL_IMMORT, TRUE, "crashDeleteFile errno %d on %s", errno, filename );
    return 0;
  }
  fclose(fl);

  strcpy( crash_file, filename );
  strcat( crash_file, ".BAK"   );

  if( rename( filename, crash_file ))
  {
    if( errno != ENOENT )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "crashDeleteFile errno %d renaming %s", errno, filename );
    }
  }

  return (1);
}


int
deleteCrashFile( CharData * ch )
{
  char fname[MAX_INPUT_LENGTH];
  struct rent_info rent;
  FILE *fl;

  if( !get_filename( GET_NAME(ch), fname, CRASH_FILE )) return 0;

  if( !(fl = fopen(fname, "rb")) )
  {
    if( errno != ENOENT )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: checking for crash file %s (3)", fname);
    }
    return 0;
  }

  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  fclose(fl);

  if (rent.rentcode == RENT_CRASH)
    crashDeleteFile(GET_NAME(ch));

  return 1;
}


int Crash_clean_file(char *name)
{
  char fname[MAX_STRING_LENGTH], filetype[20];
  struct rent_info rent;
  FILE *fl;

  if (!get_filename(name, fname, CRASH_FILE))
    return 0;
  /*
   * open for write so that permission problems will be flagged now, at boot
   * time.
   */
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: OPENING OBJECT FILE %s (4)", fname);
      perror(buf1);
    }
    return 0;
  }
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);
  fclose(fl);

  if ((rent.rentcode == RENT_CRASH) ||
      (rent.rentcode == RENT_FORCED) || (rent.rentcode == RENT_TIMEDOUT)) {
    if (rent.time < time(0) - (CONFIG_CRASH_TIMEOUT * SECS_PER_REAL_DAY)) {
      crashDeleteFile(name);
      switch (rent.rentcode) {
      case RENT_CRASH:
	strcpy(filetype, "crash");
	break;
      case RENT_FORCED:
	strcpy(filetype, "forced rent");
	break;
      case RENT_TIMEDOUT:
	strcpy(filetype, "idlesave");
	break;
      default:
	strcpy(filetype, "UNKNOWN!");
	break;
      }
      mlog("    Deleting %s's %s file.", name, filetype);
      return 1;
    }
    /* Must retrieve rented items w/in 30 days */
  } else if (rent.rentcode == RENT_RENTED)
    if (rent.time < time(0) - (CONFIG_RENT_TIMEOUT * SECS_PER_REAL_DAY)) {
      crashDeleteFile(name);
      mlog("    Deleting %s's rent file.", name);
      return 1;
    }
  return (0);
}



void
update_obj_file()
{
  int i;

  for( i = 0; i <= top_of_p_table; i++ )
    Crash_clean_file(( player_table + i)->name );
}


void
Crash_listrent(CharData * ch, char *name)
{
  FILE *fl;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  ObjData *obj;
  struct rent_info rent;

  if( !get_filename( name, fname, CRASH_FILE )) return;

  if( !(fl = fopen(fname, "rb")) )
  {
    sendChar( ch, "%s has no rent file.\r\n", name );
    return;
  }

  sprintf(buf, "%s\r\n", fname);
  if( !feof(fl) )
    fread(&rent, sizeof(struct rent_info), 1, fl);

  switch( rent.rentcode )
  {
  case RENT_RENTED: strcat(buf, "Rent\r\n"); break;
  case RENT_CRASH:  strcat(buf, "Crash\r\n"); break;
  case RENT_CRYO:   strcat(buf, "Cryo\r\n"); break;
  case RENT_TIMEDOUT:
  case RENT_FORCED: strcat(buf, "TimedOut\r\n"); break;
  default:          strcat(buf, "Undef\r\n"); break;
  }

  send_to_char(buf, ch);

  while( !feof(fl) )
  {
    fread( &object, sizeof(struct obj_file_elem), 1, fl );
    if( ferror(fl) )
    {
      fclose(fl);
      return;
    }

    if( !feof(fl) )
    {
      if( real_object(object.item_number) > -1 )
      {
        obj = read_object(object.item_number, VIRTUAL);
        sprintf( buf, " [%5d] (%5dau) %-20s\r\n",
                       object.item_number, GET_OBJ_RENT(obj),
                       obj->short_description);
        send_to_char( buf, ch );
        extract_obj(obj);
      }
    }
  }
  fclose(fl);
}



int
Crash_write_rentcode( CharData* ch,
                      FILE* fl,
                      RentInfo* rent )
{
  if( fwrite( rent, sizeof( *rent ), 1, fl) < 1 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "%s in Crash_write_rentcode for %s", strerror(errno), GET_NAME(ch) );

    return 0;
  }
  else
    return 1;
}


/*
** return values:
**   0 - successful load, keep char in rent room.
**   1 - load failure or load of crash items -- put char in temple.
**   2 - rented equipment lost (no $)
*/
int
Crash_load( CharData *ch )
{
  void Crash_crashsave(CharData * ch);

  FILE *fl;
  char fname[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  struct rent_info rent;
  int cost, orig_rent_code;
  float num_of_days;

  load_aliases( ch );

  if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
    return 1;
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) {	/* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
      perror(buf1);
      send_to_char("\r\n********************* NOTICE *********************\r\n"
		   "There was a problem loading your objects from disk.\r\n"
		   "Contact a God for assistance.\r\n", ch);
    }
    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s entering game with no equipment.", GET_NAME(ch));
    return 1;
  }

  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

/*
** An old OLD bug existed here since we weren't converting the
** room number to the real number we have no idea what lights
** were being turned on in the mud. This was never a problem
** until the mud was shuffled around a bit. 3/14/98 - Digger
*/
  ch->in_room = real_room( ch->in_room );

/*
** Digger
*/
  if( rent.rentcode == RENT_RENTED   ||
      rent.rentcode == RENT_TIMEDOUT ||
      rent.rentcode == RENT_FORCED   )
  {
      num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
      cost = (int) (rent.net_cost_per_diem * num_of_days);
      
      if( cost > GET_GOLD(ch) + GET_BANK_GOLD(ch) )
      {
          if(GET_LEVEL(ch) > 40) {
              // Let's stop taking their equipment if they're flat broke.  We'll
              // extend them a line of credit instead.
              GET_BANK_GOLD(ch) -= MAX(cost - GET_GOLD(ch), 0);
              GET_GOLD(ch) = MAX(GET_GOLD(ch) - cost, 0);
              save_char(ch, NOWHERE);
              if(GET_BANK_GOLD(ch) < 0) {
                  mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE,
                          "%s entering game with line of credit extended (no $ for rent).", GET_NAME(ch));
                  sendChar(ch, "\r\n\007You could not afford your rent!\r\n"
                          "You have been extended a line of credit and allowed to keep your possesions.\r\n");
              }
          }
          else
          {
              fclose(fl);
              mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE,
              "%s entering game, rented equipment lost (no $).", GET_NAME(ch));
              Crash_crashsave(ch);
              GET_BANK_GOLD(ch) = 0;
              GET_GOLD(ch)      = 0;
              return 2;
          }
      }
      else
      {
          GET_BANK_GOLD(ch) -= MAX(cost - GET_GOLD(ch), 0);
          GET_GOLD(ch) = MAX(GET_GOLD(ch) - cost, 0);
          save_char(ch, NOWHERE);
      }
          
  }
  else
  {
    num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
    cost = (int) (rent.net_cost_per_diem * num_of_days);
#if 0
    mudlog(NRM, LVL_IMMORT, TRUE, "RENT: Freeloader ? %s rentcode %d GP[%d]", GET_NAME(ch), rent.rentcode, cost );
#endif
  }

  switch( orig_rent_code = rent.rentcode )
  {
  case RENT_RENTED:
    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s un-renting and entering game.", GET_NAME(ch));
    break;
  case RENT_CRASH:
    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
    break;
  case RENT_CRYO:
    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s un-cryo'ing and entering game.", GET_NAME(ch));
    break;
  case RENT_FORCED:
  case RENT_TIMEDOUT:
    mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
    break;
  default:
    mudlog( BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE,
           "WARNING: %s entering game with undefined rent code[%d].", GET_NAME(ch), rent.rentcode);
    break;
  }

  while( !feof(fl) )
  {
    fread( &object, sizeof( ObjFileElem ), 1, fl );
    if( ferror(fl) )
    {
      perror("Reading crash file: Crash_load.");
      fclose(fl);
      return 1;
    }
    if( !feof(fl) )
    {
      ObjData *obj = Obj_from_store(object);

      if( obj != NULL )
      {
#define RIDDING_BAD_EQ 1
#ifdef RIDDING_BAD_EQ
        if( GET_OBJ_VNUM(obj) == 10030 && GET_OBJ_TIMER(obj) <= 0 )
        {
          mudlog(NRM, LVL_IMMORT, TRUE, "Object %d set to decay from %s",
                    GET_OBJ_VNUM(obj), GET_NAME(ch) );
	  SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_EXPLODES);
          SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_TIMED);
          GET_OBJ_TIMER(obj) = number(4,16);

        }
#endif
        if( obj->worn_at < 0 )
          obj_to_char( obj, ch);
        else
        { /* Make those Sl itch ... for now ;) */
          equip_char(ch, obj, obj->worn_at );
          if( IS_SHOU_LIN(ch) && objAntiShouLin( obj) )
          {
            act( "&08$p starts to itch.&00", 
                 FALSE, ch, obj, 0, TO_CHAR);
          }
        }
      }
    }
  }

    /* turn this into a crash file by re-writing the control block */
    rent.rentcode = RENT_CRASH;
    rent.time = time(0);
    rewind(fl);
    Crash_write_rentcode(ch, fl, &rent);

    fclose(fl);
    if( (orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO) )
        return 0;
    else
        return 1;
}



int Crash_save(ObjData * obj, FILE * fp)
{
    ObjData *tmp;
    int result;

    if( obj ){
        Crash_save( obj->contains, fp );
        Crash_save( obj->next_content, fp );
        result = Obj_to_store( obj, fp );

        for( tmp = obj->in_obj; tmp; tmp = tmp->in_obj )
            GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

        if( !result ) return 0;
    }
    return TRUE;

}/* Crash_save */


void Crash_restore_weight(ObjData * obj)
{
  if (obj) {
    Crash_restore_weight(obj->contains);
    Crash_restore_weight(obj->next_content);
    if (obj->in_obj)
      GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
  }
}



void Crash_extract_objs(ObjData * obj)
{
  if (obj) {
    Crash_extract_objs(obj->contains);
    Crash_extract_objs(obj->next_content);
    extract_obj(obj);
  }
}


int Crash_is_unrentable(ObjData * obj)
{
  if( obj == NULL ){ return 0; }

  if( IS_OBJ_STAT( obj, ITEM_NORENT ) || GET_OBJ_RENT(obj) < 0 ||
      GET_OBJ_RNUM(obj) <= NOTHING    || GET_OBJ_TYPE(obj) == ITEM_KEY ||
      (GET_OBJ_VNUM(obj) == 6908 && IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_TIMED)))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


void
Crash_extract_norents( ObjData* obj )
{
  if( obj != NULL )
  {
    Crash_extract_norents( obj->contains );
    Crash_extract_norents( obj->next_content );
    if( Crash_is_unrentable( obj ))
    {
      extract_obj(obj);
    }
  }
}


void
Crash_extract_expensive( ObjData* obj )
{
  ObjData *tobj, *max;

  max = obj;
  for( tobj = obj; tobj != NULL; tobj = tobj->next_content )
  {
    if( GET_OBJ_RENT(tobj) > GET_OBJ_RENT(max) )
    {
      max = tobj;
    }
  }
  extract_obj(max);
}


void
Crash_calculate_rent( ObjData* obj, int* cost )
{
  if( obj != NULL )
  {
    *cost += MAX(0, GET_OBJ_RENT(obj));
    Crash_calculate_rent( obj->contains, cost );
    Crash_calculate_rent( obj->next_content, cost );
  }
}


void Crash_crashsave(CharData * ch)
{
    char buf[MAX_INPUT_LENGTH];
    struct rent_info rent;
    int j;
    FILE *fp;

    if( IS_NPC( ch ) ||
        !get_filename( GET_NAME( ch ), buf, CRASH_FILE ) ||
        !(fp = fopen( buf, "wb" )))
        return;

    save_aliases( ch );

    rent.rentcode = RENT_CRASH;
    rent.time     = time(0);

    if( !Crash_write_rentcode(ch, fp, &rent) ){
        fclose(fp);
        return;
    }

    if( !Crash_save(ch->carrying, fp) ){
        fclose(fp);
        return;
    }

    Crash_restore_weight(ch->carrying);

    for( j = 0; j < NUM_WEARS; j++ )
        if( ch->equipment[j] ){
            if (!Crash_save(ch->equipment[j], fp)) {
	        fclose(fp);
	        return;
            }
        Crash_restore_weight(ch->equipment[j]);
    }

    fclose( fp );
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_CRASH);

}/* Crash_crashsave */

/* This function is not called anywhere in the code. I believe it is a stock
 * circle function. It was being used in mudlimits.c, which has been commented
 * out -Xiuh 08.28.09
 *
void Crash_idlesave( CharData* ch )
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  int cost;
  FILE *fp;

  if( IS_NPC(ch) ){ return; }

  save_aliases( ch );

  if( !get_filename(GET_NAME(ch), buf, CRASH_FILE) ){ return; }

  fp = fopen( buf, "wb" );

  if( fp == NULL ){ return; }

  for( j = 0; j < NUM_WEARS; j++ )
  {
    if( ch->equipment[j] )
    {
      obj_to_char( unequip_char(ch, j), ch );
    }
  }

  Crash_extract_norents(ch->carrying);

  cost = 0;
  Crash_calculate_rent( ch->carrying, &cost );
  cost <<= 1;			// forcerent cost is 2x normal rent

  while(( cost > GET_GOLD(ch) + GET_BANK_GOLD(ch) ) && ch->carrying )
  {
    Crash_extract_expensive(ch->carrying);
    cost = 0;
    Crash_calculate_rent(ch->carrying, &cost);
    cost <<= 1;
  }

  if( !ch->carrying )
  {
    fclose(fp);
    crashDeleteFile(GET_NAME(ch));
    return;
  }
  rent.net_cost_per_diem = cost;

  rent.rentcode = RENT_TIMEDOUT;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
}

 */

void
crashRentSave( CharData *ch, int cost )
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if( IS_NPC(ch) ) return;

  save_aliases( ch );

  if( !get_filename(GET_NAME(ch), buf, CRASH_FILE)) return;

  if( !(fp = fopen(buf, "wb"))) return;

  for( j = 0; j < NUM_WEARS; j++ )
  {
    if( ch->equipment[j] )
    {
      ObjData *obj = unequip_char( ch, j );
      if( obj != NULL )
      {
          obj_to_char( obj, ch );
          obj->worn_at = j;
      }
    }
  }

  Crash_extract_norents(ch->carrying);

  // Hack around to avoid losing char EQ in void.
  //
  if( cost < 0 )
  {
    Crash_calculate_rent(ch->carrying, &cost);
  }

  rent.net_cost_per_diem = cost;
  rent.rentcode = RENT_RENTED;
  rent.time     = time(0);
  rent.gold     = GET_GOLD(ch);
  rent.account  = GET_BANK_GOLD(ch);

  if( !Crash_write_rentcode(ch, fp, &rent))
  {
    fclose(fp);
    return;
  }

  if( !Crash_save(ch->carrying, fp))
  {
    fclose(fp);
    return;
  }

  fclose(fp);

  Crash_extract_objs(ch->carrying);
}


void Crash_cryosave(CharData * ch, int cost)
{
  char buf[MAX_INPUT_LENGTH];
  struct rent_info rent;
  int j;
  FILE *fp;

  if (IS_NPC(ch))
    return;

  save_aliases( ch );

  if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
    return;
  if (!(fp = fopen(buf, "wb")))
    return;

  for (j = 0; j < NUM_WEARS; j++)
    if (ch->equipment[j])
      obj_to_char(unequip_char(ch, j), ch);

  Crash_extract_norents(ch->carrying);

  GET_GOLD(ch) = MAX(0, GET_GOLD(ch) - cost);

  rent.rentcode = RENT_CRYO;
  rent.time = time(0);
  rent.gold = GET_GOLD(ch);
  rent.account = GET_BANK_GOLD(ch);
  rent.net_cost_per_diem = 0;
  if (!Crash_write_rentcode(ch, fp, &rent)) {
    fclose(fp);
    return;
  }
  if (!Crash_save(ch->carrying, fp)) {
    fclose(fp);
    return;
  }
  fclose(fp);

  Crash_extract_objs(ch->carrying);
  SET_BIT_AR(PLR_FLAGS(ch), PLR_CRYO);
}


/* ************************************************************************
* Routines used for the receptionist					  *
************************************************************************* */

void Crash_rent_deadline(CharData * ch, CharData * recep,
			      long cost)
{
  long rent_deadline;

  if (!cost)
    return;

  rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / cost);
  sprintf(buf,
      "$n tells you, 'You can rent for %ld day%s with the gold you have\r\n"
	  "on hand and in the bank.'\r\n",
	  rent_deadline, (rent_deadline > 1) ? "s" : "");
  act(buf, FALSE, recep, 0, ch, TO_VICT);
}

int Crash_report_unrentables(CharData *ch, CharData *recep,
			         ObjData* obj)
{
  char buf[128];
  int has_norents = 0;

  if (obj) {
    if (Crash_is_unrentable(obj)) {
      has_norents = 1;
      sprintf(buf, "$n tells you, 'You cannot store %s.'", OBJS(obj, ch));
      act(buf, FALSE, recep, 0, ch, TO_VICT);
    }
    has_norents += Crash_report_unrentables(ch, recep, obj->contains);
    has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
  }
  return (has_norents);
}

static void Crash_report_rent(CharData *ch, CharData *recep, ObjData *obj,
   long *cost, long *nitems, int display, int factor, int modifier )
{
  static char buf[256];

  if (obj) {
    if (!Crash_is_unrentable(obj)) {
      (*nitems)++;
      *cost += MAX(0, (GET_OBJ_RENT(obj) * factor * modifier / 100 ));
      if (display) {
	sprintf(buf, "$n tells you, '%5d coins for %s..'",
                (GET_OBJ_RENT(obj) * factor * modifier / 100 ), OBJS(obj, ch));
	act(buf, FALSE, recep, 0, ch, TO_VICT);
      }
    }
    Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor, modifier);
    Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor, modifier);
  }
}

int Crash_offer_rent( CharData * ch,
                      CharData * receptionist,
                      int display,
                      int factor,
                      int max_obj_save, int modifier  )
{
  char buf[MAX_INPUT_LENGTH];
  int i;
  long totalcost = 0, numitems = 0, norent = 0;

  norent = Crash_report_unrentables(ch, receptionist, ch->carrying);
  for (i = 0; i < NUM_WEARS; i++)
    norent += Crash_report_unrentables(ch, receptionist, ch->equipment[i]);

  if( norent ) return 0;

  totalcost = CONFIG_MIN_RENT_COST * factor;

  Crash_report_rent(ch, receptionist, ch->carrying, &totalcost, &numitems, display, factor, modifier);

  for (i = 0; i < NUM_WEARS; i++)
    Crash_report_rent(ch, receptionist, ch->equipment[i], &totalcost, &numitems, display, factor, modifier);

  if (!numitems) {
    act("$n tells you, 'But you are not carrying anything!  Just quit!'",
	FALSE, receptionist, 0, ch, TO_VICT);
    return (0);
  }
  /*
  ** Temporary rent obj fix
  ** Digger
  */
  max_obj_save += (GET_LEVEL(ch) < 20 ? 0 : 5 );
  max_obj_save += (GET_LEVEL(ch) < 30 ? 0 : 5 );
  max_obj_save = max_obj_save * modifier / 100;

  if (numitems > max_obj_save) {
    sprintf(buf, "$n tells you, 'Sorry, but I cannot store more than %d items.'", max_obj_save);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    return (0);
  }
  if (display) {
    sprintf(buf, "$n tells you, 'Plus, my %d coin fee..'", CONFIG_MIN_RENT_COST * factor);
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    sprintf(buf, "$n tells you, 'For a total of %ld coins%s.'",
	    totalcost, (factor == RENT_FACTOR ? " per day" : ""));
    act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    if (totalcost > GET_GOLD(ch)) {
      act("$n tells you, '...which I see you can't afford.'",
	  FALSE, receptionist, 0, ch, TO_VICT);
      return (0);
    } else if (factor == RENT_FACTOR)
      Crash_rent_deadline(ch, receptionist, totalcost);
  }
  return (totalcost);
}



int gen_receptionist( CharData * ch,
                      CharData * recep,
                      int cmd, char *arg, int mode, int classes)
{
    extern int max_obj_save;
    int max_obj = 0;
    int cost    = 0;
    int modifier = 100;
    sh_int save_room;
    int i;

    char *action_table[] = { "smile",
                             "dance",
                             "sigh",
                             "blush",
                             "burp",
                             "cough",
                             "fart",
                             "twiddle",
                             "yawn",
                             "sneeze",
                             "shiver",
                             "puke",
                             "moon" };
    ACMD(do_action);

    if( !ch->desc || IS_NPC(ch)) return FALSE;

    if (!cmd && !number(0, 5)) {
        do_action(recep, "", find_command(action_table[number(0, 12)]), 0);
        return FALSE;
    }

    if( !CMD_IS("offer") && !CMD_IS("rent") ) return FALSE;

    if (!AWAKE(recep)) {
        send_to_char("They are unable to talk to you...\r\n", ch);
        return TRUE;
    }

    if( !CAN_SEE(recep, ch) && (GET_LEVEL(ch) <= MAX_MORTAL)){
        act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
        return TRUE;
    }

  if (((1 << GET_CLASS(ch)) & classes) == 0) {
    act("$n says, 'Sorry, we have no room.'", FALSE, recep, 0, ch, TO_VICT);
    return TRUE;
  }

   if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED)) {
       act("$n says, 'Naw, I wouldn't want to dissapoint those people looking for you.'", FALSE, recep, 0, 0, TO_ROOM);
       return TRUE;
   }

    if (CONFIG_FREE_RENT) {
        act("$n tells you, 'Rent is free here.  Just quit, and your objects will be saved!'",
        FALSE, recep, 0, ch, TO_VICT);
        return 1;
    }

    if( IS_SET_AR( ROOM_FLAGS( IN_ROOM(ch)), ROOM_CLAN ) &&
        ( world[IN_ROOM(ch)].clan_id != GET_CLAN(ch)))  {
        act("$n tells you, 'You are not allowed to rent here, infidel!'", FALSE, recep, 0, ch, TO_VICT);
        return 1;
    }

    max_obj = ( IS_SET_AR( ROOM_FLAGS( IN_ROOM(ch)), ROOM_CLAN ) ? world[IN_ROOM(ch)].clan_recept_sz : max_obj_save );

    for (i = 0; rent_modifiers[i].vnum != -1; i++)
      if (IN_ROOM(ch) == real_room(rent_modifiers[i].vnum))
        modifier = rent_modifiers[i].modifier;

    if( CMD_IS( "rent" )){
        if( !(cost = Crash_offer_rent(ch, recep, FALSE, mode, max_obj, modifier ))) return TRUE;

        if(IS_SET_AR( ROOM_FLAGS( IN_ROOM(ch)), ROOM_CLAN ) || (GET_LEVEL(ch) <= 10))
		{ cost = (cost >> 1); }

        if (mode == RENT_FACTOR)
            sprintf(buf, "$n tells you, 'Rent will cost you %d gold coins per day.'", cost);
        else if (mode == CRYO_FACTOR)
            sprintf(buf, "$n tells you, 'It will cost you %d gold coins to be frozen.'", cost);

        act(buf, FALSE, recep, 0, ch, TO_VICT);
        if (cost > GET_GOLD(ch)) {
            act("$n tells you, '...which I see you can't afford.'", FALSE, recep, 0, ch, TO_VICT);
            return TRUE;
        }

        if (cost && (mode == RENT_FACTOR))
            Crash_rent_deadline(ch, recep, cost);

        if (mode == RENT_FACTOR) {
            act("$n stores your belongings and helps you into your private chamber.", FALSE, recep, 0, ch, TO_VICT);
            crashRentSave(ch, cost);
            mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s has rented (%d/day, %d tot.)", GET_NAME(ch),
            cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
        }

        else {   /* cryo */
            act( "$n stores your belongings and helps you into your private chamber.\r\n"
	         "A white mist appears in the room, chilling you to the bone...\r\n"
	         "You begin to lose consciousness...",
	         FALSE, recep, 0, ch, TO_VICT);
            Crash_cryosave(ch, cost);
            mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s has cryo-rented.", GET_NAME(ch));
	    SET_BIT_AR(PLR_FLAGS(ch), PLR_CRYO);
        }
        
        act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
        save_room = ch->in_room;
        extract_char(ch);
        ch->in_room = world[save_room].number;
        save_char(ch, ch->in_room);
    }

    else {
        int cost = Crash_offer_rent(ch, recep, TRUE, mode, max_obj, modifier );
        act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
        if( IS_SET_AR( ROOM_FLAGS( IN_ROOM(ch)), ROOM_CLAN )){
            cost = (cost >> 1);
            sprintf(buf, "$n tells you, 'But, since you're a fellow clansman it will only be %d.'", cost);
            act(buf, FALSE, recep, 0, ch, TO_VICT);
        }
    }

  return TRUE;
}


SPECIAL(rogue_receptionist)
{
  int classes = (1 << CLASS_THIEF) | (1 << CLASS_SHADOW_DANCER) |
    (1 << CLASS_ASSASSIN);
  return (gen_receptionist(ch, me, cmd, argument, RENT_FACTOR, classes));
}

SPECIAL(rogue_cryogenicist)
{
  int classes = (1 << CLASS_THIEF) | (1 << CLASS_SHADOW_DANCER) |
    (1 << CLASS_ASSASSIN);
  return (gen_receptionist(ch, me, cmd, argument, CRYO_FACTOR, classes));
}

SPECIAL(receptionist)
{
  return (gen_receptionist(ch, me, cmd, argument, RENT_FACTOR, -1));
}


SPECIAL(cryogenicist)
{
  return (gen_receptionist(ch, me, cmd, argument, CRYO_FACTOR, -1));
}


void Crash_save_all(void)
{
  DescriptorData *d;
  LockerList *l;

  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLAYING) && !IS_NPC(d->character)) {
      if (PLR_FLAGGED(d->character, PLR_CRASH)) {
	Crash_crashsave(d->character);
	save_char(d->character, NOWHERE);
	REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRASH);
      }
    }
  }

  for (l = lockers; l; l = l->next) {
    if (IS_SET(GET_OBJ_VAL(l->locker, 1), CONT_DIRTY)) { //GDB points crash ending on this line.
      save_locker(l->locker);
      REMOVE_BIT(GET_OBJ_VAL(l->locker, 1), CONT_DIRTY);
    }
  }

  write_saved_corpses();
}

static int save_locker_obj(ObjData *obj, FILE *fp, int offset)
{
  int index = offset;
  ObjData *item;

  for (item = obj->contains; item; item = item->next_content) {
    item->container = offset;
    Obj_to_store(item, fp);
    index++;
    if (IS_CONTAINER(item))
      index = save_locker_obj(item, fp, index);
  }
  return index;
}

void save_locker(ObjData *obj)
{
  FILE *fp;
  char filename[70];
  char backupname[70];

  sprintf(filename, "lockers/%d.objs", GET_OBJ_VNUM(obj));

  // keep a backup of the locker
  strcpy(backupname, filename);
  strcat(backupname, ".BAK");
  rename(filename, backupname);

  // simply write out the object list
  fp = fopen(filename, "wb");
  if (fp == NULL) return;
  save_locker_obj(obj, fp, 0);
  fclose(fp);
}

void save_locker_of(CharData *ch)
{
  LockerList *l;

  if (GET_LOCKER(ch) == 0) return;

  for (l = lockers; l; l = l->next) {
    if (GET_LOCKER(ch) == GET_OBJ_VNUM(l->locker) &&
        IS_SET(GET_OBJ_VAL(l->locker, 1), CONT_DIRTY)) {
      save_locker(l->locker);
      REMOVE_BIT(GET_OBJ_VAL(l->locker, 1), CONT_DIRTY);
    }
  }
}

void load_object_list(FILE *fp, int nitems, ObjData *obj)
{
    ObjData **index;
    int count = 1;

    index = (ObjData **)malloc(sizeof(ObjData *) * (nitems + 1));
    index[0] = obj;

    while (nitems-- > 0) {
        struct obj_file_elem object;

        fread(&object, sizeof(object), 1, fp);
        if (ferror(fp)) {
            fclose(fp);
            free(index);
            return;
        }
        if (!feof(fp)) {
            ObjData *item = Obj_from_store(object);
            index[count++] = item;
            if (item) {
                if (item->container >= 0 && item->container < count
                        && index[item->container]) {
                    int weight = GET_OBJ_WEIGHT(item);
                    GET_OBJ_WEIGHT(item) = 0; // mmm, hackery.
                    obj_to_obj(item, index[item->container]);
                    if (item->container > 0)
                        obj_index[item->item_number].number -= 1;
                    GET_OBJ_WEIGHT(item) = weight;
                } else
                    obj_to_obj(item, obj);
            }
        }
    }
}

void load_locker(ObjData *obj)
{
  char filename[70];
  long items;
  FILE *fp;

  sprintf(filename, "lockers/%d.objs", GET_OBJ_VNUM(obj));
  fp = fopen(filename, "rb");
  if (fp == NULL) return;

  // work out how many items are in this file
  fseek(fp, 0, SEEK_END);
  items = ftell(fp) / sizeof(struct obj_file_elem);
  rewind(fp);

  load_object_list(fp, items, obj);

  fclose(fp);
}

/*****************************************************************************
 * Auto corpse saving facility.
 *
 * This section of code allows corpses to be added to a list to be
 * saved periodically.  Calling boot_corpses() will load this list, and
 * restore all corpses to their original location.  Corpses past a
 * certain age will NOT be loaded.  Items within corpses that can't be
 * rented will NOT be saved.  Corpses that are not in a room (ie,
 * carried) will NOT be saved.
 ****************************************************************************/

/* Max age of an object */
#define MAX_AGE                 (5 * SECS_PER_REAL_DAY)
#define MORT_AGE                (2 * SECS_PER_REAL_DAY)
#define MORT_ROOM               18096
#define CORPSES_FILE             "corpses/corpses.db"

#define MAX_CORPSE_NAME         80

/* This structure is written to corpses/corpses.db */
typedef struct saved_corpse
{
    u_char name[MAX_CORPSE_NAME+1];     /* name of the corpse object */
    u_char keyword[MAX_NAME_LENGTH+1];  /* exdesc keyword - player name */
    int load_location;                  /* room to load corpse into */
    time_t created;                     /* original creation time */
    int n_items;                        /* number of items saved */
} SavedCorpse;

/* This structure maintains a list of active corpses */
typedef struct saved_corpses
{
    time_t created;                     /* creation time */
    ObjData *corpse;                    /* corpse object */
    struct saved_corpses *next;         /* next in list */
} SavedCorpses;

static SavedCorpses *corpses = NULL;

static int is_corpse_saveable(ObjData *obj)
{
    if (Crash_is_unrentable(obj)) return 0;

    return 1;
}

static int save_corpse_obj(ObjData *obj, FILE *fp, int offset)
{
    int index = offset;
    ObjData *item, *money;

    for (item = obj->contains; item; item = item->next_content) {
        if (is_corpse_saveable(item)) {
            item->container = offset;
            Obj_to_store(item, fp);
            index++;
            if (IS_CONTAINER(item))
                index = save_corpse_obj(item, fp, index);
        } else if (GET_OBJ_TYPE(item) == ITEM_MONEY) {
            money = read_object(2, VIRTUAL);
            if (money) {
                GET_OBJ_VAL(money, 0) = GET_OBJ_VAL(item, 0);
                GET_OBJ_COST(money)   = GET_OBJ_COST(item);
                Obj_to_store(money, fp);
                index++;
            }
        }
    }
    return index;
}

static void write_saved_corpses(void)
{
    FILE *f = fopen(CORPSES_FILE, "wb");
    SavedCorpses *temp;
    SavedCorpse sc;
    long mark;
    int saved = 0;

    if (!f) {
        mlog("SYSERR: Could not create corpse file for writing!");
        return;
    }

    for (temp = corpses; temp; temp = temp->next) {
        /* If the corpse doesn't exist, there's something wrong */
        if (!temp->corpse) {
            mlog("SYSERR: There's a missing corpse object here!");
            /* XXX should remove it, too, but with a single linked
             * list that's tricky to do in the middle of an iteration */
            continue;
        }

        /* If the corpse isn't in an actual room, skip it quietly */
        if (temp->corpse->in_room == NOWHERE) continue;

        /* If there's no description, we don't know what this is */
        if (!temp->corpse->description) {
            mlog("SYSERR: Corpse to be saved is indescribable!");
            continue;
        }

        /* If there's no exdesc, something's wrong! */
        if (!temp->corpse->ex_description) {
            mlog("SYSERR: '%s' has no ex_description",
                    temp->corpse->description);
            continue;
        }

        /* If there's no keyword, we don't know whose corpse this is */
        if (!temp->corpse->ex_description->keyword) {
            mlog("SYSERR: '%s' has no ex_description keyword",
                    temp->corpse->description);
            continue;
        }

        /* If the keyword is too long or doesn't map to a player ID,
         * this isn't a (valid) player's corpse */
        if (strlen(temp->corpse->ex_description->keyword) > MAX_NAME_LENGTH ||
                get_id_by_name(temp->corpse->ex_description->keyword) == -1) {
            mlog("SYSERR: '%s' is an unknown player",
                    temp->corpse->ex_description->keyword);
            continue;
        }

        /* See if we need to make a shorter corpse object name ... */
        if (strlen(temp->corpse->description) > MAX_CORPSE_NAME)
            sprintf(sc.name, "The corpse of %s is rotting here.",
                    temp->corpse->ex_description->keyword);
        else
            strcpy(sc.name, temp->corpse->description);

        /* having verified these fields, we can now safely copy them */
        strcpy(sc.keyword, temp->corpse->ex_description->keyword);
        sc.load_location = world[temp->corpse->in_room].number;
        sc.created = temp->created;
        sc.n_items = 0;

        /* it would be nice not to need to dance around the file like this */
        mark = ftell(f);
        fwrite(&sc, sizeof(sc), 1, f);
        sc.n_items = save_corpse_obj(temp->corpse, f, 0);
        fseek(f, mark, SEEK_SET);
        fwrite(&sc, sizeof(sc), 1, f);
        fseek(f, 0, SEEK_END);

        saved++;
    }

    //mlog("CORPSES: Saved %d corpses", saved);

    fclose(f);
}

void add_saved_corpse(time_t created, ObjData *obj)
{
    SavedCorpses *corpse;

    CREATE(corpse, SavedCorpses, 1);
    corpse->created = created;
    corpse->corpse = obj;
    corpse->next = corpses;
    corpses = corpse;
}

void del_saved_corpse(ObjData *obj)
{
    SavedCorpses *mark, *temp = NULL;

    /* Look for a saved corpse matching the requested removal */
    for (mark = corpses; mark && mark->corpse != obj; mark = mark->next)
        temp = mark;

    /* Nope, none here */
    if (!mark) return;

    /* Relink list */
    if (!temp) corpses = mark->next;
    else temp->next = mark->next;

    /* Reclaim the memory */
    free(mark);
}

void boot_corpses(void)
{
    FILE *f = fopen(CORPSES_FILE, "rb");
    int ncorpses = 0;

    if (!f) {
        mlog("Corpses database does not exist - no corpses loaded");
        return;
    }

    /* load all corpses, one by one */
    while (!feof(f)) {
        SavedCorpse corpse;
        CharFileU owner;
        ObjData *obj;
        int room;

        /* Might've run out of file at this point. */
        if (fread(&corpse, sizeof(SavedCorpse), 1, f) != 1) break;

        /* "close enough" to being a valid corpse */
        ncorpses++;

        /* if it's expired, skip over the objects and keep going */
        if (corpse.created + MAX_AGE < time(0)) {
            fseek(f, sizeof(ObjFileElem) * corpse.n_items, SEEK_CUR);
            continue;
        }

        /* Create the new corpse object */
        obj = create_obj();

        /* It's nothing, and it's nowhere, and it can't be revived */
        obj->item_number  = NOTHING;
        obj->in_room      = NOWHERE;
        GET_OBJ_TYPE(obj) = ITEM_CONTAINER;

        /* Don't let this corpse be revivable or explodable */
        obj->obj_flags.cost         = -1;
        obj->obj_flags.cost_per_day = -1;

        /* set up aliases, description and short description */
        sprintf(buf, "corpse %s PC_CORPSE", corpse.keyword);
        obj->name = str_dup(buf);
        obj->description = str_dup(corpse.name);
        sprintf(buf, "the corpse of %s", corpse.keyword);
        obj->short_description = str_dup(buf);

        /* add the player name exdesc */
        CREATE(obj->ex_description, struct extra_descr_data, 1);
        obj->ex_description->keyword = str_dup(corpse.keyword);
        sprintf(buf, "Ewwww! What ever killed %s sure made it messy!",
                corpse.keyword);
        obj->ex_description->description = str_dup(buf);
        obj->ex_description->next = NULL;

        /* set up object flags and values */
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODONATE);
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_TIMED);
        GET_OBJ_VAL(obj, 0) = 0;
        GET_OBJ_VAL(obj, 3) = 1;
        GET_OBJ_TIMER(obj) = 13;

        /* load the contents of the corpse */
        load_object_list(f, corpse.n_items, obj);

        /* put it in its room */
        room = real_room(MORT_ROOM);
        if (load_char(corpse.keyword, &owner) != -1) {
            if (owner.player_specials_saved.clan_id > 0
             && owner.player_specials_saved.clan_id < clan_count) {
                int r = clan_list[owner.player_specials_saved.clan_id].home;
                r = real_room(r);
                if (r != -1) room = r;
            }
        }
        obj_to_room(obj, room);

        /* add it to saved list */
        add_saved_corpse(corpse.created, obj);
    }

    mlog("CORPSES: %d corpses loaded successfully", ncorpses);

    /* Uhoh, didn't reach EOF, that means there was an incomplete record! */
    if (!feof(f)) {
        mlog("CORPSES: More data in file, corpse db may be corrupt!");
    }

    fclose(f);
}

ObjData *find_player_corpse(CharData *ch)
{
    SavedCorpses *temp;

    for (temp = corpses; temp; temp = temp->next) {
        if (!temp->corpse ||                            /* no corpse */
            !temp->corpse->ex_description ||            /* no ex_desc */
             temp->corpse->in_room == NOWHERE ||        /* not in a room */
            !temp->corpse->ex_description->keyword)     /* no keyword */
                continue;
        if (!strcmp(temp->corpse->ex_description->keyword, GET_NAME(ch)))
                return temp->corpse;
    }
    return NULL;
}
