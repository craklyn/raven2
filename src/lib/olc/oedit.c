/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - oedit.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*. Original author: Levork .*/
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/class.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "specials/boards.h"
#include "specials/shop.h"
#include "olc/olc.h"
#include "olc/oedit.h"
#include "olc/medit.h"
#include "magic/missile.h"
#include "magic/trap.h"
#include "scripts/dg_olc.h"
#include "general/handler.h"

const ApplyRestrictType apply_restrictions[NUM_APPLIES];
const ValueRestrictType obj_value_restrictions[NUM_ITEM_TYPES][4];
char *container_bits[NUM_CONTAINER_BITS + 1] = {
  "CLOSEABLE", "PICKPROOF", "CLOSED", "LOCKED", "LOCKER", "DIRTY",
  "\n"
};

/*------------------------------------------------------------------------*/
/* external variables */
extern struct shop_data *shop_index;			/*. shop.c	.*/
extern int top_shop;					/*. shop.c	.*/
extern struct board_info_type board_info[];

/*------------------------------------------------------------------------*/

void oContFlagMenu(DescriptorData * d);
void oWeaponMenu(DescriptorData * d);
void oVal1Menu(DescriptorData * d);
void oVal2Menu(DescriptorData * d);
void oVal3Menu(DescriptorData * d);
void oVal4Menu(DescriptorData * d);
void oTypeMenu(DescriptorData * d);
void oExtraMenu(DescriptorData * d);
void oWearMenu(DescriptorData * d);
void oAffectMenu(DescriptorData * d);
void oSpellsMenu(DescriptorData * d, int theSpell); 
void oLiquidMenu(DescriptorData * d);
void oSaveInternally(DescriptorData *d);
void oExtraDescMenu(DescriptorData *d);
void oMenu(DescriptorData *d);


/*------------------------------------------------------------------------*\
  Utility and exported functions
\*------------------------------------------------------------------------*/

void
oSetupNew( DescriptorData *d )
{
  CREATE (OLC_OBJ(d), ObjData, 1);
  clear_object(OLC_OBJ(d));
  OLC_OBJ(d)->name = str_dup("unfinished object");
  OLC_OBJ(d)->description = str_dup("An unfinished object is lying here.");
  OLC_OBJ(d)->short_description = str_dup("an unfinished object");
  SET_BIT_AR(GET_OBJ_WEAR(OLC_OBJ(d)), ITEM_WEAR_TAKE);
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
  oMenu(d);
}


void
oSetupExisting(DescriptorData *d, int real_num)
{ struct extra_descr_data *this, *temp, *temp2;
  ObjData *obj;

  /* allocate object */
  CREATE (obj, ObjData, 1);
  clear_object (obj);
  *obj = obj_proto[real_num];
 
  /* copy all strings over */
  if (obj_proto[real_num].name)
    obj->name = str_dup (obj_proto[real_num].name);
  if (obj_proto[real_num].short_description)
    obj->short_description = str_dup (obj_proto[real_num].short_description);
  if (obj_proto[real_num].description)
    obj->description = str_dup (obj_proto[real_num].description);
  if (obj_proto[real_num].action_description)
    obj->action_description = str_dup (obj_proto[real_num].action_description);

  /*. Extra descriptions if necessary .*/
  if (obj_proto[real_num].ex_description)
  { /* temp is for obj being edited */
    CREATE (temp, struct extra_descr_data, 1);
    obj->ex_description = temp;
    for (this = obj_proto[real_num].ex_description; this; this = this->next)
    { if (this->keyword)
	temp->keyword = str_dup (this->keyword);
      if (this->description)
	temp->description = str_dup (this->description);
      if (this->next)
      { CREATE (temp2, struct extra_descr_data, 1);
        temp->next = temp2;
        temp = temp2;
      } else
	temp->next = NULL;
    }
  }

  /*. Attatch new obj to players descriptor .*/
  OLC_OBJ(d) = obj;
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
  dg_olc_script_copy(d);
  oMenu(d);
}

/*------------------------------------------------------------------------*/

#define ZCMD zone_table[zone].cmd[cmd_no]

void oSaveInternally(DescriptorData *d)
{
	int i, shop, robj_num, found = FALSE, zone, cmd_no;
	struct extra_descr_data *this, *next_one;
	ObjData *obj, *swap, *new_obj_proto;
	struct index_data *new_obj_index;
	DescriptorData *dsc;

	/* write to internal tables */
	robj_num = real_object(OLC_NUM(d));
	if (robj_num > 0) {
		/* we need to run through each and every object currently in the
		* game to see which ones are pointing to this prototype */

		// if object is pointing to this prototype, then we need to replace
		// with the new one
		CREATE(swap, ObjData, 1);
		for (obj = object_list; obj; obj = obj->next) {
			if (obj->item_number == robj_num) {
				CharData *wearer = obj->worn_by;
				int worn_at = obj->worn_at;
				if(wearer != NULL) unequip_char(wearer, worn_at);
				*swap = *obj;
				*obj = *OLC_OBJ(d);
				// copy game-time dependent vars over
				obj->in_room = swap->in_room;
				obj->item_number = robj_num;
				obj->carried_by = swap->carried_by;
				obj->worn_by = swap->worn_by;
				obj->worn_at = swap->worn_at;
				obj->in_obj = swap->in_obj;
				obj->contains = swap->contains;
				obj->next_content = swap->next_content;
				obj->next = swap->next;
                                GET_OBJ_RENAME(obj) = GET_OBJ_RENAME(swap);
				obj->name = str_dup(swap->name);
				obj->short_description = str_dup(swap->short_description);
				if (wearer != NULL) equip_char(wearer, obj, worn_at);
			}
		}
                
		free_obj(swap);
		/* now safe to free old proto and write over */
		if (obj_proto[robj_num].name)
			free(obj_proto[robj_num].name);
		if (obj_proto[robj_num].description)
			free(obj_proto[robj_num].description);
		if (obj_proto[robj_num].short_description)
			free(obj_proto[robj_num].short_description);
		if (obj_proto[robj_num].action_description)
			free(obj_proto[robj_num].action_description);
		if (obj_proto[robj_num].ex_description)
			for (this = obj_proto[robj_num].ex_description;
				this; this = next_one) 
			{ next_one = this->next;
		if (this->keyword)
			free(this->keyword);
		if (this->description)
			free(this->description);
		free(this);
		}
		obj_proto[robj_num] = *OLC_OBJ(d);
		obj_proto[robj_num].item_number = robj_num;
		obj_proto[robj_num].proto_script = OLC_SCRIPT(d);
	} else {
		/*. It's a new object, we must build new tables to contain it .*/

		CREATE(new_obj_index, struct index_data, top_of_objt + 2);
		CREATE(new_obj_proto, ObjData, top_of_objt + 2);
		/* start counting through both tables */
		for (i = 0; i <= top_of_objt; i++) {
			/* if we haven't found it */
			if (!found) {
				/* check if current virtual is bigger than our virtual */
				if (obj_index[i].virtual > OLC_NUM(d)) 
				{ found = TRUE;
				robj_num = i;
				OLC_OBJ(d)->item_number = robj_num;
				new_obj_index[robj_num].virtual = OLC_NUM(d);
				new_obj_index[robj_num].number = 0;
				new_obj_index[robj_num].func = NULL;
				new_obj_proto[robj_num] = *(OLC_OBJ(d));
				new_obj_proto[robj_num].proto_script = OLC_SCRIPT(d);
				new_obj_proto[robj_num].in_room = NOWHERE;
				/*. Copy over the mob that should be here .*/
				new_obj_index[robj_num + 1] = obj_index[robj_num];
				new_obj_proto[robj_num + 1] = obj_proto[robj_num];
				new_obj_proto[robj_num + 1].item_number = robj_num + 1;
				} else {
					/* just copy from old to new, no num change */
					new_obj_proto[i] = obj_proto[i];
					new_obj_index[i] = obj_index[i];
				}
			} else {
				/* we HAVE already found it.. therefore copy to object + 1 */
				new_obj_index[i + 1] = obj_index[i];
				new_obj_proto[i + 1] = obj_proto[i];
				new_obj_proto[i + 1].item_number = i + 1;
			}
		}
		if (!found)
		{ robj_num = i;
		OLC_OBJ(d)->item_number = robj_num;
		new_obj_index[robj_num].virtual = OLC_NUM(d);
		new_obj_index[robj_num].number = 0;
		new_obj_index[robj_num].func = NULL;
		new_obj_proto[robj_num] = *(OLC_OBJ(d));
		new_obj_proto[robj_num].proto_script = OLC_SCRIPT(d);
		new_obj_proto[robj_num].in_room = NOWHERE;
		}

		/* free and replace old tables */
		free (obj_proto);
		free (obj_index);
		obj_proto = new_obj_proto;
		obj_index = new_obj_index;
		top_of_objt++;

		/*. Renumber live objects .*/
		for (obj = object_list; obj; obj = obj->next)
			if (GET_OBJ_RNUM (obj) >= robj_num)
				GET_OBJ_RNUM (obj)++;

		/*. Renumber zone table .*/
		for (zone = 0; zone <= top_of_zone_table; zone++)
			for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
				switch (ZCMD.command)
			{ case 'P':
		if(ZCMD.arg3 >= robj_num)
			ZCMD.arg3++;
		/*. No break here - drop into next case .*/
				case 'O':
				case 'G':
				case 'E':
					if(ZCMD.arg1 >= robj_num)
						ZCMD.arg1++;
					break;
				case 'R':
					if(ZCMD.arg2 >= robj_num)
						ZCMD.arg2++;
					break;
			}

			/*. Renumber notice boards */
			for (i = 0; i < NUM_OF_BOARDS; i++)
				if (BOARD_RNUM(i) >= robj_num)
					BOARD_RNUM(i) = BOARD_RNUM(i) + 1;

			/*. Renumber shop produce .*/
			for(shop = 0; shop < top_shop; shop++)
				for(i = 0; SHOP_PRODUCT(shop, i) != -1; i++)
					if (SHOP_PRODUCT(shop, i) >= robj_num)
						SHOP_PRODUCT(shop, i)++;

			/*. Renumber produce in shops being edited .*/
			for(dsc = descriptor_list; dsc; dsc = dsc->next)
				if(dsc->connected == CON_SEDIT)
					for(i = 0; S_PRODUCT(OLC_SHOP(dsc), i) != -1; i++)
						if (S_PRODUCT(OLC_SHOP(dsc), i) >= robj_num)
							S_PRODUCT(OLC_SHOP(dsc), i)++;

  } 
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_OBJ);
}
/*------------------------------------------------------------------------*/

void oSaveToDisk(DescriptorData *d)
{
	int counter, counter2, realcounter;
	FILE *fp;
	ObjData *obj;
	struct extra_descr_data *ex_desc;

	sprintf(buf, "%s/%d.obj", OBJ_PREFIX, zone_table[OLC_ZNUM(d)].number);
	if (!(fp = fopen(buf, "w+")))
	{ 
		mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: Cannot open objects file!");
		return;
	}

	/* start running through all objects in this zone */
	for (counter = zone_table[OLC_ZNUM(d)].number * 100;
		counter <= zone_table[OLC_ZNUM(d)].top;
		counter++) 
	{ 
		/* write object to disk */
		realcounter = real_object(counter);
		if (realcounter >= 0) {
			obj = (obj_proto + realcounter);

			/* Mortius : If the item is named DELETEME then don't save it to disk */
			if (obj->name && strcmp(obj->name, "DELETEME") == 0)
				continue;

			if (obj->action_description)
			{
				strcpy(buf1, obj->action_description);
				strip_string(buf1);
			} else
				*buf1 = 0;

			fprintf(fp, 
				"#%d\n"			/* <vnum> */
				"%s~\n"			/* name list */
				"%s~\n"			/* short description */
				"%s~\n"			/* room description */
				"%s~\n"			/* action description */
				"%d %d %d %d %d %d %d %d %d\n" /*  <type> <extras> <wears> */
				"%d %d %d %d\n"		/* <val0> <val1> <val2> <val3> */
				"%d %d %d\n",		/* <weight> <value> <rent> */

				GET_OBJ_VNUM(obj),
				obj->name ? obj->name : "undefined",
				obj->short_description ? obj->short_description : "undefined",
				obj->description ? obj->description : "undefined",
				buf1,
				GET_OBJ_TYPE(obj),
				GET_OBJ_EXTRA_AR(obj, 0), GET_OBJ_EXTRA_AR(obj, 1),
				GET_OBJ_EXTRA_AR(obj, 2), GET_OBJ_EXTRA_AR(obj, 3),
				GET_OBJ_WEAR_AR(obj, 0), GET_OBJ_WEAR_AR(obj, 1),
				GET_OBJ_WEAR_AR(obj, 2), GET_OBJ_WEAR_AR(obj, 3),
				GET_OBJ_VAL(obj, 0),
				GET_OBJ_VAL(obj, 1),
				GET_OBJ_VAL(obj, 2),
				GET_OBJ_VAL(obj, 3),
				GET_OBJ_WEIGHT(obj),
				GET_OBJ_COST(obj),
				GET_OBJ_RENT(obj)
				);

			script_save_to_disk(fp, obj, OBJ_TRIGGER);

			/* Do we have extra descriptions? */
			if (obj->ex_description)
			{
				for (ex_desc = obj->ex_description; ex_desc; ex_desc = ex_desc->next) 
				{
					if( ex_desc->keyword == NULL || !*ex_desc->keyword )
					{
						mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: NULL extra description keyword on object %d", GET_OBJ_VNUM(obj));
						ex_desc->keyword = strdup( "EMPTY_KEYWORD" );
					}

					if( ex_desc->description == NULL || !*ex_desc->description )
					{
						mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: NULL extra description on object %d", GET_OBJ_VNUM(obj) );
						ex_desc->description = strdup( "EMPTY_DESCRIPTION" );
					}
					strcpy(buf1, ex_desc->description);
					strip_string(buf1);
					fprintf(fp,   "E\n" "%s~\n" "%s~\n", ex_desc->keyword, buf1 );
				}
			}

			/* Do we have affects? */
			for (counter2 = 0; counter2 < MAX_OBJ_AFFECT; counter2++)
				if (obj->affected[counter2].modifier) 
					fprintf(fp,   "A\n"
					"%d %d\n", 
					obj->affected[counter2].location,
					obj->affected[counter2].modifier
					);
			/* Do we have permanent object affects? */
			if (obj->obj_flags.bitvector)
				fprintf(fp, 	"B\n"
				"%d %d %d %d\n", 
				GET_OBJ_AFFECT_AR(obj, 0), GET_OBJ_AFFECT_AR(obj, 1),
				GET_OBJ_AFFECT_AR(obj, 2), GET_OBJ_AFFECT_AR(obj, 3));
			/* Do we have a trap? */
			if (GET_TRAP(obj) > 0) fprintf(fp, "T%d\n", GET_TRAP(obj));

			/* Save the timer field if necessary */
			if (GET_OBJ_TIMER(obj)) {
				fprintf(fp, "D%d\n", GET_OBJ_TIMER(obj));
			}
		}
	}

	/* write final line, close */
	fprintf(fp, "$~\n");
	fclose(fp);
	olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_OBJ);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
** For container flags
*/
void
oContFlagMenu( DescriptorData *d )
{
  bitTable( d, container_bits,
               NUM_CONTAINER_BITS,
               "container",
               (u_int)d->olc->obj->obj_flags.value[1] );
}

/* Mortius : Display direction data for traps */
const char *trap_dirs[] =
{
  "left in room",
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};


/*
** For extra descriptions
*/
void
oExtraDescMenu( DescriptorData *d )
{
  ExtraDescrData *extra_desc = OLC_DESC(d);
  
  if( !extra_desc->next )
    strcpy( buf1, "<Not set>\r\n" );
  else
    strcpy( buf1, "Set." );

  send_to_char("[H[J", d->character);
  sprintf(buf, 
	"Extra desc menu\r\n"
  	"&021)&06 Keyword:&00 %s\r\n"
  	"&022)&06 Description:&00\r\n%s\r\n"
        "&023)&06 Goto next description:&00 %s\r\n"
  	"&020)&06 Quit&00\r\n"
        "Enter choice : ",

	extra_desc->keyword ? extra_desc->keyword : "<NONE>",
	extra_desc->description ? extra_desc->description : "<NONE>",
        buf1
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

/* Ask for *which* apply to edit */
void oedit_disp_prompt_apply_menu(DescriptorData * d)
{
  int counter;

  send_to_char("[H[J", d->character);
  for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
    if (OLC_OBJ(d)->affected[counter].modifier) {
      sprinttype(OLC_OBJ(d)->affected[counter].location, apply_types, buf2, sizeof(buf2));
      sprintf(buf, " &02%d)&06 %+d to %s&00\r\n", 
      	  	counter + 1,
		OLC_OBJ(d)->affected[counter].modifier, buf2
      );
      send_to_char(buf, d->character);
    } else {
      sprintf(buf, " &02%d)&06 None.&00\r\n", counter + 1);
      send_to_char(buf, d->character);
    }
  }
  send_to_char("\r\nEnter affect to modify: ", d->character);
  OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

/*
void oedit_disp_prompt_apply_menu(DescriptorData *d)
{
  int counter, columns = 0;

  for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
       if (OLC_OBJ(d)->affected[counter].modifier) {
           sprinttype(OLC_OBJ(d)->affected[counter].location,
	              apply_types, buf2, sizeof(buf2));
           sprintf(buf, "&02%d&06 %+d to %s %s\r\n", counter + 1,
		   OLC_OBJ(d)->affected[counter].modifier, buf2,
		   !(++columns % 4) ? "\r\n" : "");
           send_to_char(buf, d->character);

       } else {
	   sprintf(buf, "&02%d)&06 None.&00\r\n", counter + 1);
	   send_to_char(buf, d->character);
       }
  }
  send_to_char("\r\nEnter affect to modify: ", d->character);
  OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

*/

/*. Ask for liquid type .*/
/*
void oLiquidMenu(DescriptorData * d)
{
  intTable(d, drinks, NUM_DRINKS, "liquid type", d->olc->obj->obj_flags.value[2]);
  OLC_MODE(d) = OEDIT_VALUE_3;
}
*/
void oLiquidMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_LIQ_TYPES; counter++) {
    sprintf(buf, " %s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
            drinks[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n%sEnter drink type : ", nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_VALUE_3;
}


/* Ask for missile type */
/*
void oMissileMenu(DescriptorData *d)
{
  intTable(d, missile_types, NUM_MISSILE_TYPES, "missile type", d->olc->obj->obj_flags.value[0]);
}
*/
void oMissileMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_MISSILE_TYPES; counter++) {
       sprintf(buf, "%s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
               missile_types[counter], !(++columns % 3) ? "\r\n" : "");
       send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n%sEnter ammo choice (0 for none) : ", nrm);
  send_to_char(buf, d->character);
}

/* The actual apply to set */
/*
void oedit_disp_apply_menu(DescriptorData * d, int thisApply)
{
  intTable(d, apply_types, NUM_APPLIES, "apply location", d->olc->obj->affected[thisApply].location);
  OLC_MODE(d) = OEDIT_APPLY;
  d->olc->tableDisp = FALSE;
}
*/

void oedit_disp_apply_menu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_APPLIES; counter++) {
    sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter, nrm,
                apply_types[counter], !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter apply type : ", d->character);
}

/* weapon type */
/*
void oWeaponMenu(DescriptorData * d)
{
  if (d->olc->tableDisp)  clear screen and print table 
    attackTable(d);
  sprintf(buf, attack_hit_text[d->olc->obj->obj_flags.value[3]].singular);
  sendChar(d->character, "Object[%d] weapon type: &03%s&00\r\n", d->olc->vnum, buf);
  sendChar(d->character, "Enter new weapon type: ");
}
*/
void oWeaponMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_WEAPON_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
                attack_hit_text[counter].singular,
                !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter weapon type : ", d->character);
}

/* spell type */
void oSpellsMenu(DescriptorData * d, int theSpell)
{
  intTable(d, spells, NUM_SPELLS, "spell", theSpell);
}

/* room flag type */
void oRoomFlagMenu(DescriptorData *d, int theFlag)
{
    intTable(d, none_room_bits, NUM_ROOM_FLAGS + 1, "flag", theFlag);
}

/* object value 1 */
void oVal1Menu(DescriptorData * d)
{
  if (OLC_MODE(d) != OEDIT_VALUE_1) /* Then this is first time we been here. */
      d->olc->tableDisp = TRUE;
  OLC_MODE(d) = OEDIT_VALUE_1;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
      case ITEM_AFFECT:
          oRoomFlagMenu(d, d->olc->obj->obj_flags.value[0]);
          break;
  case ITEM_LIGHT:
    /* values 0 and 1 are unused.. jump to 2 */
    oVal3Menu(d);
    break;
  case ITEM_WEAPON: /* value 0 unused, jump to 1. */
    oVal2Menu(d);
    break;
  case ITEM_SCROLL:
  case ITEM_WAND:
  case ITEM_DUST:
  case ITEM_STAFF:
  case ITEM_POTION:
    send_to_char("Spell level : ", d->character);
    break;
  case ITEM_ARMOR:
    send_to_char("Apply to AC : ", d->character);
    break;
  case ITEM_CONTAINER:
    send_to_char("Max weight to contain : ", d->character);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    send_to_char("Max drink units : ", d->character);
    break;
  case ITEM_FOOD:
    send_to_char("Hours to fill stomach : ", d->character);
    break;
  case ITEM_MONEY:
    send_to_char("Number of gold coins : ", d->character);
    break;
  case ITEM_FIREWEAPON:
  case ITEM_MISSILE:
    oMissileMenu(d);
    break;
  case ITEM_PORTAL:
    send_to_char("Destination room : ", d->character);
    break;
  default:
    oMenu(d);
  }
  d->olc->tableDisp = FALSE;
}

/* object value 2 */
void oVal2Menu(DescriptorData * d)
{
  if (OLC_MODE(d) != OEDIT_VALUE_2) /* then this is first time we been here. */
      d->olc->tableDisp = TRUE;
  OLC_MODE(d) = OEDIT_VALUE_2;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
      case ITEM_AFFECT:
          oRoomFlagMenu(d, d->olc->obj->obj_flags.value[1]);
          break;
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_DUST:
    oSpellsMenu(d, d->olc->obj->obj_flags.value[1]); 
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char("Max number of charges : ", d->character);
    break;
  case ITEM_MISSILE:
  case ITEM_WEAPON:
    send_to_char("Number of damage dice : ", d->character);
    break;
  case ITEM_FIREWEAPON:
    sendChar(d->character, "Range : ");
    break;
  case ITEM_FOOD:
    sendChar(d->character, "Spell Level : ");
    break;
  case ITEM_CONTAINER:
    /* these are flags, needs a bit of special handling */
    oContFlagMenu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    send_to_char("Initial drink units : ", d->character);
    break;
  case ITEM_PORTAL:
    send_to_char("Maximum uses(-1 infinite) : ", d->character);
    break;
  default:
    oMenu(d);
  }
  d->olc->tableDisp = FALSE;
}

/* object value 3 */
void oVal3Menu(DescriptorData * d)
{
  if (OLC_MODE(d) != OEDIT_VALUE_3) /* Then this is first time we been here. */
      d->olc->tableDisp = TRUE;
  OLC_MODE(d) = OEDIT_VALUE_3;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    send_to_char("Number of hours (0 = burnt, -1 is infinite) : ", d->character);
    break;
      case ITEM_AFFECT:
          oRoomFlagMenu(d, d->olc->obj->obj_flags.value[2]);
          break;
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_FOOD:
  case ITEM_DUST:
    oSpellsMenu(d, d->olc->obj->obj_flags.value[2]); 
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char("Number of charges remaining : ", d->character);
    break;
  case ITEM_MISSILE:
  case ITEM_WEAPON:
    send_to_char("Size of damage dice : ", d->character);
    break;
  case ITEM_FIREWEAPON:
    sendChar(d->character, "Stun delay: ");
    break;
  case ITEM_CONTAINER:
    send_to_char("Vnum of key to open container (-1 for no key) : ", d->character);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    oLiquidMenu(d);
    break;
  case ITEM_PORTAL:
    send_to_char("Reliability(0-10, 0 never works, 10+ always works) : ", d->character);
    break;
  default:
    oMenu(d);
  }
  d->olc->tableDisp = FALSE;
}

/* object value 4 */
void oVal4Menu(DescriptorData * d)
{
  if (OLC_MODE(d) != OEDIT_VALUE_4) /* then this is first time we been here. */
	d->olc->tableDisp = TRUE;
  OLC_MODE(d) = OEDIT_VALUE_4;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_WAND:
  case ITEM_STAFF:
  case ITEM_DUST:
    oSpellsMenu(d, d->olc->obj->obj_flags.value[3]); 
    break;
  case ITEM_WEAPON:
    oWeaponMenu(d);
    break;
  case ITEM_MISSILE:
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
  case ITEM_FOOD:
    send_to_char("Poisoned (0 = not poison) : ", d->character);
    break;
  case ITEM_CONTAINER:
    send_to_char("Trap level (0 = not trapped) : ", d->character);
    break;
  default:
    oMenu(d);
  }
  d->olc->tableDisp = FALSE;
}

/* object type */
void oTypeMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_ITEM_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter, nrm,
                item_types[counter], !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter object type : ", d->character);
}


/* object extra flags */
/*
void oExtraMenu(DescriptorData * d)
{
  bitTable(d, extra_bits, NUM_ITEM_FLAGS, "extra", d->olc->obj->obj_flags.extra_flags);
}
*/

void oExtraMenu(DescriptorData * d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_ITEM_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter + 1, nrm,
                extra_bits[counter], !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(GET_OBJ_EXTRA(d->olc->obj), extra_bits, EF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nObject flags: %s%s%s\r\n"
          "Enter object extra flag (0 to quit) : ",
          cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/* object wear flags */
#ifdef OLDWAY
void
oWearMenu(DescriptorData * d)
{
  bitTable(d, wear_bits, NUM_ITEM_WEARS, "wear", d->olc->obj->obj_flags.wear_flags);
}
#else
void oWearMenu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_ITEM_WEARS; counter++) {
    sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter + 1, nrm,
                wear_bits[counter], !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, TW_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nWear flags: %s%s%s\r\n"
          "Enter wear flag, 0 to quit : ", cyn, buf1, nrm);
  send_to_char(buf, d->character);
}
#endif


/*. Display aff-flags menu .*/
#ifdef OLDWAY
void oAffectMenu(DescriptorData *d)
{
  bitTable(d, affected_bits, NUM_AFF_FLAGS, "affect", d->olc->obj->obj_flags.bitvector);
}
#else
void oAffectMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_AFF_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter + 1, nrm,
                affected_bits[counter], !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(GET_OBJ_AFFECT(OLC_OBJ(d)), affected_bits, AF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nAffect flags: %s%s%s\r\n"
          "Enter affect flag, 0 to quit : ", cyn, buf1, nrm);
  send_to_char(buf, d->character);
}
#endif

/* display main menu */
void
oMenu(DescriptorData * d)
{ 
  char buf3[MAX_INPUT_LENGTH];
  ObjData *obj;
  obj = OLC_OBJ(d);

  /*. Build buffers for first part of menu .*/
  sprinttype(GET_OBJ_TYPE(obj), item_types, buf1, sizeof(buf1));
  sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf2);

  /*. Build first hallf of menu .*/
  sprintf(buf, "[H[J"
  	"-- Item number : [%d]\r\n"
  	"&021)&06 Namelist :&00 %s\r\n"
  	"&022)&06 S-Desc   :&00 %s\r\n"
  	"&023)&06 L-Desc   :&00-\r\n%s\r\n"
  	"&024)&06 A-Desc   :&00-\r\n%s\r\n"
  	"&025)&06 Type        :&03 %s\r\n"
  	"&026)&06 Extra flags :&03 %s&00\r\n",

	OLC_NUM(d), 
	obj->name,
 	obj->short_description,
	obj->description,
	obj->action_description ?  obj->action_description : "<not set>\r\n",
	buf1,
 	buf2
  );
  /*. Send first half .*/
  send_to_char(buf, d->character);

  /*. Build second half of menu .*/
  sprintbitarray(GET_OBJ_WEAR(obj), wear_bits, TW_ARRAY_MAX, buf1);
  oPrintValues(obj, buf2);
 
  
  if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
    sprintf(buf, "                 Trapped at level %d\r\n", GET_TRAP(obj));
    send_to_char(buf, d->character);
  }
  sprintbitarray(GET_OBJ_AFFECT(obj), affected_bits, AF_ARRAY_MAX, buf3);

  sprintf(buf,
  	"&027)&06 Wear flags  :&03 %s\r\n"
  	"&028)&06 Weight      :&03 %d\r\n"
  	"&029)&06 Cost        :&03 %d\r\n"
  	"&02A)&06 Cost/Day    :&03 %d\r\n"
  	"&02B)&06 Timer       :&03 %d\r\n"
  	"&02D)&06 Values      :&03 %s\r\n"
  	"&02E)&06 Applies menu\r\n"
  	"&02F)&06 Extra descriptions menu\r\n"
	"&02G)&06 Object Affects  :&03 %s\r\n"
        "&02H)&06 Script      :&03 %s\r\n"
  	"&02Q)&06 Quit&00\r\n"
  	"Enter choice : ",

 	buf1,
 	GET_OBJ_WEIGHT(obj),
 	GET_OBJ_COST(obj),
 	GET_OBJ_RENT(obj),
 	GET_OBJ_TIMER(obj),
	buf2,
	buf3,
        obj->proto_script?"Set.":"Not set."
  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_MAIN_MENU;
}



/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/
void
oParse( DescriptorData *d, char *arg )
{
  int number;
  int nr;
  char chIn = toupper( *arg );

  switch( OLC_MODE(d) )
  {
  case OEDIT_CONFIRM_SAVESTRING:
    switch( chIn )
    {
    case 'Y':
      send_to_char("Saving object to memory.\r\n", d->character);

      for (nr = 0; nr <= top_of_objt; nr++) {
        if (obj_index[nr].virtual == OLC_NUM(d)) {
            // We only want to record old stats if the item already existed.
            sprintf(buf, "%s saves edit to item #%d internally (not to disk).\r\n\r\nOld Stats:\r\n", GET_NAME(d->character), OLC_NUM(d));
            general_log(buf, "misc/itemEdits");
            logItem(OLC_NUM(d), "misc/itemEdits");
        }
      }

      oSaveInternally(d);

      general_log("\r\nNew stats:\r\n", "misc/itemEdits");
      logItem(OLC_NUM(d), "misc/itemEdits");
      general_log("----------------------------------------------------------------------\r\n", "misc/itemEdits");

      mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s edits obj %d", GET_NAME(d->character), OLC_NUM(d));
      cleanup_olc(d, CLEANUP_STRUCTS);
      return;
    case 'N':
      /*. Cleanup all .*/
      cleanup_olc(d, CLEANUP_ALL);
      return;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save this object internally?\r\n", d->character);
      return;
    }

  case OEDIT_MAIN_MENU:
    /* throw us out to whichever edit mode based on user input */
    d->olc->tableDisp = TRUE;
    switch( chIn )
    {
    case 'Q':
// Hack to force ANY change to save - Bean 9-25-06
//      if (OLC_VAL(d))
//      { /*. Something has been modified .*/
        send_to_char("Do you wish to save this object internally? : ", d->character);
        OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
//      } else
//        cleanup_olc(d, CLEANUP_ALL);
//      return;
        break;
    case '1':
      send_to_char("Enter namelist : ", d->character);
      OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
      break;
    case '2':
      send_to_char("Enter short desc : ", d->character);
      OLC_MODE(d) = OEDIT_SHORTDESC;
      break;
    case '3':
      send_to_char("Enter long desc :-\r\n| ", d->character);
      OLC_MODE(d) = OEDIT_LONGDESC;
      break;
    case '4':
      send_to_char("Enter long desc :-\r\n| ", d->character);
      OLC_MODE(d) = OEDIT_ACTDESC;
      /*
      write_to_output(d, "Enter action description: (/s saves /h for help)\r\n\r\n");
      d->backstr = NULL;
      if (OLC_OBJ(d)->action_description) {
        write_to_output(d, "%s", OLC_OBJ(d)->action_description);
      d->backstr = str_dup(OLC_OBJ(d)->action_description);
      }
      d->str = &OLC_OBJ(d)->action_description;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
      */
      break;
    case '5':
      oTypeMenu(d);
      OLC_MODE(d) = OEDIT_TYPE;
      break;
    case '6':
      oExtraMenu(d);
      OLC_MODE(d) = OEDIT_EXTRAS;
      break;
    case '7':
      oWearMenu(d);
      OLC_MODE(d) = OEDIT_WEAR;
      break;
    case '8':
      weightTable(d);
      sendChar(d->character, "Enter item weight: ");
      OLC_MODE(d) = OEDIT_WEIGHT;
      break;
    case '9':
      send_to_char("Enter cost : ", d->character);
      OLC_MODE(d) = OEDIT_COST;
      break;
    case 'A':
      send_to_char("Enter cost per day : ", d->character);
      OLC_MODE(d) = OEDIT_COSTPERDAY;
      break;
    case 'B':
      send_to_char("Enter timer : ", d->character);
      OLC_MODE(d) = OEDIT_TIMER;
      break;
    case 'D':
      /*. Clear any old values .*/
      GET_OBJ_VAL(OLC_OBJ(d), 0) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 1) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 2) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
      oVal1Menu(d);
      break;
    case 'E':
      oedit_disp_prompt_apply_menu(d);
      break;
    case 'F':
      /* if extra desc doesn't exist . */
      if (!OLC_OBJ(d)->ex_description) {
	CREATE(OLC_OBJ(d)->ex_description, struct extra_descr_data, 1);
	OLC_OBJ(d)->ex_description->next = NULL;
      }
      OLC_DESC(d) = OLC_OBJ(d)->ex_description;
      oExtraDescMenu(d);
      break;
    case 'G':
      oAffectMenu(d);
      OLC_MODE(d) = OEDIT_AFF_FLAGS;
      break;
    case 'H':
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      break;
    default:
      oMenu(d);
      break;
    }
    d->olc->tableDisp = FALSE;  /* We don't want to show tables now. */
    return;			/* end of OEDIT_MAIN_MENU */

  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;

  case OEDIT_EDIT_NAMELIST:
    if (OLC_OBJ(d)->name)
      free(OLC_OBJ(d)->name);
    OLC_OBJ(d)->name = str_dup(arg);
    break;

  case OEDIT_SHORTDESC:
    if (OLC_OBJ(d)->short_description)
      free(OLC_OBJ(d)->short_description);
    OLC_OBJ(d)->short_description = str_dup(arg);
    break;

  case OEDIT_LONGDESC:
    if (OLC_OBJ(d)->description)
      free(OLC_OBJ(d)->description);
    OLC_OBJ(d)->description = str_dup(arg);
    break;
    
  case OEDIT_ACTDESC:
    if (OLC_OBJ(d)->action_description)
      free(OLC_OBJ(d)->action_description);
    OLC_OBJ(d)->action_description = str_dup(arg);
    break;
          
#ifdef OLDWAY
  case OEDIT_TYPE:
    number = getType( d, arg, item_types, NUM_ITEM_TYPES, "item type" );
    if( number == -2 ) /* -2 means they just hit retun so exit */
    {
      oMenu(d);
      return; 
    }
    else if( number >= 0 )
    {
      GET_OBJ_TYPE(OLC_OBJ(d)) = number;
      for (number = 0; number < 4; number++)
          d->olc->obj->obj_flags.value[number] = 0;
    }
    oTypeMenu(d);
    return;
    break;
#else
  case OEDIT_TYPE:
    number = atoi(arg);
    if ((number < 1) || (number >= NUM_ITEM_TYPES)) {
      send_to_char("Invalid choice, try again : ", d->character);
      return;
    } else
      GET_OBJ_TYPE(OLC_OBJ(d)) = number;
    break;
#endif

#ifdef OLDWAY
  case OEDIT_EXTRAS:
    number = toggleBit(d, arg, extra_bits, NUM_ITEM_FLAGS, "Extra flag", &GET_OBJ_EXTRA(OLC_OBJ(d)));
    if (number < 0) { /* Invalid entry */
	oExtraMenu(d);
        return;
    }
    break;
#else
  case OEDIT_EXTRAS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_FLAGS)) {
      oExtraMenu(d);
      return;
    } else if (number == 0)
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_EXTRA(OLC_OBJ(d)), (number - 1));
      oExtraMenu(d);
      return;
    }
#endif

#ifdef OLDWAY
  case OEDIT_WEAR:
    number = toggleBit(d, arg, wear_bits, NUM_ITEM_WEARS, "Wear flag", &GET_OBJ_WEAR(OLC_OBJ(d)));
    if (number < 0) { /* Invalid entry */
	oWearMenu(d);
        return;
    }
    break;
#else
  case OEDIT_WEAR:
number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_WEARS)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      oWearMenu(d);
      return;
    } else if (number == 0)     /* Quit. */
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_WEAR(OLC_OBJ(d)), (number - 1));
      oWearMenu(d);
      return;
    }
#endif

#ifdef OLDWAY
  case OEDIT_AFF_FLAGS:
    number = toggleBit(d, arg, affected_bits, NUM_AFF_FLAGS, "Affect flag", &GET_OBJ_AFFECT(OLC_OBJ(d)));
    if (number < 0) { /* Invalid entry */
	oAffectMenu(d);
        return;
    }
    oCheckAffectFlags(OLC_OBJ(d));
    break;
#else
  case OEDIT_AFF_FLAGS:
  number = atoi(arg);
    if ((number < 0) || (number > NUM_AFF_FLAGS)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      oAffectMenu(d);
      return;
    } else if (number == 0)     /* Quit. */
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_AFFECT(OLC_OBJ(d)), (number - 1));
      oAffectMenu(d);
      return;
    }
#endif

  case OEDIT_WEIGHT:
    if (!is_integer(arg)) {
	sendChar(d->character, "%s is not a valid weight.\r\n", arg);
	sendChar(d->character, "Enter weight: ");
	return;
    }
    number = MIN(MAX_OBJ_WEIGHT, MAX(0, atoi(arg)));
    GET_OBJ_WEIGHT(OLC_OBJ(d)) = number;
    break;

  case OEDIT_COST:
    if (!is_integer(arg)) {
	sendChar(d->character, "%s is not a valid cost.\r\n", arg);
	sendChar(d->character, "Enter cost: ");
	return;
    }
    number = MIN(MAX_OBJ_COST, MAX(0, atoi(arg)));
    GET_OBJ_COST(OLC_OBJ(d)) = number;
    break;

  case OEDIT_COSTPERDAY:
    if (!is_integer(arg)) {
	sendChar(d->character, "%s is not a valid rent cost.\r\n", arg);
	sendChar(d->character, "Enter rent: ");
	return;
    }
    number = MIN(MAX_OBJ_RENT, MAX(0, atoi(arg)));
    GET_OBJ_RENT(OLC_OBJ(d)) = number;
    break;

  case OEDIT_TIMER:
    if (!is_integer(arg)) {
	sendChar(d->character, "%s is not a valid timer value.\r\n", arg);
	sendChar(d->character, "Enter timer: ");
	return;
    }
    number = MIN(MAX_OBJ_TIMER, MAX(0, atoi(arg)));
    GET_OBJ_TIMER(OLC_OBJ(d)) = number;
    break;

  case OEDIT_VALUE_1:
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
        case ITEM_AFFECT:
    case ITEM_FIREWEAPON:
    case ITEM_MISSILE:
        number = atoi(arg);
	if (number < 0) {
	    oVal1Menu(d);
	    return;
	}
	break;
    default:
	if (!is_integer(arg)) {
	    sendChar(d->character, "%s isn't a valid value.\r\n", arg);
	    oVal1Menu(d);
	    return;
	}
        number = atoi(arg);
    } /* switch */
    number = objValueLimit(d->olc->obj->obj_flags.type_flag, 0, number);
    GET_OBJ_VAL(OLC_OBJ(d), 0) = number;
    /* proceed to menu 2 */
    oVal2Menu(d);
    return;
  case OEDIT_VALUE_2:
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
        case ITEM_AFFECT:
    case ITEM_POTION:
    case ITEM_DUST:
        number = atoi(arg);
	if (number < 0) {
	    oVal2Menu(d);
	    return;
	}
	break; /* drop out of switch with number */
    case ITEM_CONTAINER:
	number = toggleBit(d, arg, container_bits, NUM_CONTAINER_BITS, "container", &d->olc->obj->obj_flags.value[1]);
	if (number < 0) {
	    oVal2Menu(d);
	    return;
	}
	oVal3Menu(d);
	return; /* we don't want to drop out of switch here, we are done. */
    default:
	if (!is_integer(arg)) {
	    sendChar(d->character, "%s isn't a valid value.\r\n", arg);
	    oVal2Menu(d);
	    return;
	}
        number = atoi(arg); /* drop out of switch with number */
    } /* switch */
    number = objValueLimit(d->olc->obj->obj_flags.type_flag, 1, number);
    GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
    oVal3Menu(d);
    return; /* move to next value menu */

  case OEDIT_VALUE_3:
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_FOOD:
    case ITEM_DUST:
        number = atoi(arg);
	if (number < 0) {
	    oVal3Menu(d);
	    return;
	}
        break; /* drop out of switch with number */
      case ITEM_DRINKCON:
      case ITEM_FOUNTAIN:
        number = atoi(arg);
	if (number < 0) {
	    oVal3Menu(d);
	    return;
	}
        break; /* drop out of switch with number */
      default:
	if (!is_integer(arg)) {
	    sendChar(d->character, "%s isn't a valid value.\r\n", arg);
	    oVal3Menu(d);
	    return;
	} 
        number = atoi(arg); /* drop out of switch with number */
    }
    number = objValueLimit(d->olc->obj->obj_flags.type_flag, 2, number);
    GET_OBJ_VAL(OLC_OBJ(d), 2) = number;
    if (GET_OBJ_TYPE(OLC_OBJ(d)) == ITEM_AFFECT) break;
    oVal4Menu(d);
    return;

  case OEDIT_VALUE_4:
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_DUST:
	number = atoi(arg);
	if (number < 0) {
	    oVal4Menu(d);
	    return;
	}
        break; /* drop out of switch with number */
    case ITEM_WEAPON:
      number = atoi(arg);
      if (number < 0) {
	oVal4Menu(d);
	return;
      }
      break; /* drop out of switch with number */
    default:
	if (!is_integer(arg)) {
	    sendChar(d->character, "%s isn't a valid value.\r\n", arg);
	    oVal4Menu(d);
	    return;
	} 
        number = atoi(arg); /* drop out of switch with number */
    }
    number = objValueLimit(d->olc->obj->obj_flags.type_flag, 3, number);
    GET_OBJ_VAL(OLC_OBJ(d), 3) = number;
    if (GET_OBJ_TYPE(OLC_OBJ(d)) == ITEM_CONTAINER)
      OLC_OBJ(d)->traplevel = number;
    break;

  case OEDIT_PROMPT_APPLY:
    if (!*arg) 
        oMenu(d);
    else if( !is_integer(arg) )
    {
      sendChar(d->character, "Invalid entry [%s].\r\n", arg);
      sendChar(d->character, "Enter affect to modify: " );
    }
    else
    {
      number = atoi(arg);
      if( number < 0 || number > MAX_OBJ_AFFECT )
      {
        oedit_disp_prompt_apply_menu(d);
      }
      else
      {
        OLC_VAL(d) = number - 1;
        OLC_MODE(d) = OEDIT_APPLY;
        d->olc->tableDisp = TRUE;
//        oedit_disp_apply_menu(d, OLC_VAL(d));
	oedit_disp_apply_menu(d);
      }
    }
    return;

  case OEDIT_APPLY:
    number = atoi(arg);
    if( number == -2 ) /* -2 means they just hit return */
    {
      oedit_disp_prompt_apply_menu(d);
    }
    else if( number == 0 )
    {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = 0;
      OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = 0;
      oedit_disp_prompt_apply_menu(d);
    } 
    else if( number == -1 ) /* -1 means it was invalid input */
    {
//      oedit_disp_apply_menu(d, OLC_VAL(d));
	oedit_disp_apply_menu(d);
    }
    else
    {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = number;
      sendChar( d->character, "Enter modifier: " );
      OLC_MODE(d) = OEDIT_APPLYMOD;
    }
    return;

  case OEDIT_APPLYMOD:
    /* Digger note to self:
    ** We could impose limits for the various apply types which
    ** could exist in a lookup table and then allow additional
    ** or a widening of the ranges for higher level imms.
    */
    if( !*arg )
    {
//      oedit_disp_apply_menu(d, OLC_VAL(d));
	oedit_disp_apply_menu(d);
    }
    else if( !is_integer(arg) )
    {
      sendChar(d->character, "Invalid modifier [%s].\r\n", arg);
      sendChar(d->character, "Enter modifier: ");
    }
    else
    {
      number = atoi(arg);
      d->olc->obj->affected[d->olc->value].modifier =
        applyModLimit(OLC_OBJ(d)->affected[OLC_VAL(d)].location, number);
      oedit_disp_prompt_apply_menu(d);
    }
    return;

  case OEDIT_EXTRADESC_KEY:
    if (OLC_DESC(d)->keyword)
      free(OLC_DESC(d)->keyword);
    OLC_DESC(d)->keyword = str_dup(arg);
    oExtraDescMenu(d);
    return;

  case OEDIT_EXTRADESC_MENU:
    if (!is_integer(arg)) {
	sendChar(d->character, "%s isn't a valid menu choice.\r\n", arg);
	sendChar(d->character, "Enter choice :");
	return;
    }
    number = atoi(arg);
    switch (number) {
    case 0:
      { /* if something got left out */
	if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) 
        { struct extra_descr_data **tmp_desc;

	  if (OLC_DESC(d)->keyword)
	    free(OLC_DESC(d)->keyword);
	  if (OLC_DESC(d)->description)
	    free(OLC_DESC(d)->description);

          /*. Clean up pointers .*/
	  for(tmp_desc = &(OLC_OBJ(d)->ex_description); *tmp_desc;
	      tmp_desc = &((*tmp_desc)->next))
          { if (*tmp_desc == OLC_DESC(d))
            { *tmp_desc = NULL;
              break;
            }
          }
	  free(OLC_DESC(d));
	}
      }
      break;

    case 1:
      OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
      send_to_char("Enter keywords, separated by spaces :-\r\n| ", d->character);
      return;

    case 2:
      OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
	 write_to_output(d, "Enter the extra description: (/s saves /h for help)\r\n\r\n");
	 d->backstr = NULL;
	 if (OLC_DESC(d)->description) {
           write_to_output(d, "%s", OLC_DESC(d)->description);
	   d->backstr = str_dup(OLC_DESC(d)->description);
	 }
	 d->str = &OLC_DESC(d)->description;
	 d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
      return;

    case 3:
      /*. Only go to the next descr if this one is finished .*/
      if (OLC_DESC(d)->keyword && OLC_DESC(d)->description)
      { struct extra_descr_data *new_extra;

	if (OLC_DESC(d)->next)
	  OLC_DESC(d) = OLC_DESC(d)->next;
	else 
        { /* make new extra, attach at end */
	  CREATE(new_extra, struct extra_descr_data, 1);

	  OLC_DESC(d)->next = new_extra;
	  OLC_DESC(d) = OLC_DESC(d)->next;
	}
      }
      /*. No break - drop into default case .*/
    default:
      oExtraDescMenu(d);
      return;
    }
    break;
  default:
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: Reached default case in oParse()!");
    break;
  }

  /*. If we get here, we have changed something .*/
  OLC_VAL(d) = 1; /*. Has changed flag .*/
  oMenu(d);
}

char
applyModLimit(int location, int theMod)
{
    if ( (location < 0) || (location >= NUM_APPLIES) ) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Invalid location %d passed to applyModLimit", location);
	return 0;
    }
    if (theMod < apply_restrictions[location].min)
	theMod = apply_restrictions[location].min;
    else if (theMod > apply_restrictions[location].max)
	theMod = apply_restrictions[location].max;
    return(theMod);
}

int
objValueLimit(int itemType, int valueIndex, int theValue)
{
    if ( (valueIndex < 0) || (valueIndex > 3) ) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Invalid object value index %d passed to objValueLimit", valueIndex);
	return 0;
    }
    if ( (itemType < 0) || (itemType >= NUM_ITEM_TYPES) ) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Invalid item type %d passed to objValueLimit", itemType);
	return 0;
    }
    if (theValue < obj_value_restrictions[itemType][valueIndex].min)
	theValue = obj_value_restrictions[itemType][valueIndex].min;
    else if (theValue > obj_value_restrictions[itemType][valueIndex].max)
	theValue = obj_value_restrictions[itemType][valueIndex].max;
    return theValue;
}

void
oPrintValues(ObjData *obj, char *result)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH]; /* using global buffers is a bit dodgy when a buffer could be passed in... */


  /* First pass. */
  switch(obj->obj_flags.type_flag) {
  case ITEM_SCROLL:
  case ITEM_WAND:
  case ITEM_STAFF:
  case ITEM_POTION:
  case ITEM_DUST:
	sprintf(result, "Spell lvl: %d ", obj->obj_flags.value[0]);
	break;
  case ITEM_ARMOR:
	sprintf(result, "AC apply: %d", obj->obj_flags.value[0]);
	return; /* nothing more to add. */
  case ITEM_CONTAINER:
	sprintbit(obj->obj_flags.value[1], container_bits, buf);
	obj->obj_flags.value[2] > 0 ? sprintf(buf2, "%d", obj->obj_flags.value[2]) : sprintf(buf2, "No Key");
	sprintf(result, "Capacity: %dlbs Flags: %s Key: %s Is corpse? %s",
	obj->obj_flags.value[0], buf, buf2, (obj->obj_flags.value[3] != 0 ? "Yes" : "No"));
	return;
  case ITEM_KEY:
	sprintf(result, "Key type: %d", obj->obj_flags.value[0]);
	return; /* Nothing further to add. */
  case ITEM_FOOD:
	sprintf(result, "No. hours to fill stomach: %d", obj->obj_flags.value[0]);
	sprintf(buf, "(%s)", (obj->obj_flags.value[3] != 0 ? "poison" : "&02Edible&00"));
	strcat(result, buf);
	sprintf(buf, " Level: %d Spell: %s", obj->obj_flags.value[1], spells[obj->obj_flags.value[2]]);
	strcat(result, buf);
	return; /* We done. */
  case ITEM_MONEY:
	sprintf(result, "Gold coins: %d", obj->obj_flags.value[0]);
	return; /* Nothing further to add. */
  case ITEM_MISSILE:
  case ITEM_FIREWEAPON:
	sprintf(result, "Type: %s ", missile_types[obj->obj_flags.value[0]]);
	break;
  case ITEM_LIGHT:
	sprintf(result, "Duration: ");
	obj->obj_flags.value[2] < 0 ? sprintf(buf, "Infinite") : sprintf(buf, "%d hours", obj->obj_flags.value[2]);
	strcat(result, buf);
	return; /* Finished. */
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
	sprintf(result, "Max. Units: %d Units left: %d Liquid: %s(%s)",
	obj->obj_flags.value[0], obj->obj_flags.value[1], drinks[obj->obj_flags.value[2]],
	(obj->obj_flags.value[3] != 0 ? "poison" : "clear"));
	return; /* finished. */
  case ITEM_PORTAL:
	sprintf(result, "Portal item: %d %d %d %d\r\n", obj->obj_flags.value[0],
	obj->obj_flags.value[1], obj->obj_flags.value[2], obj->obj_flags.value[3]);
	return;
  case ITEM_TREASURE:
  case ITEM_WORN:
  case ITEM_OTHER:
  case ITEM_TRASH:
  case ITEM_NOTE:
  case ITEM_PEN:
  case ITEM_BOAT:
  case ITEM_SCRIBE:
  case ITEM_POLE:
  case ITEM_UNDEFINED:
	sprintf(result, "Values not used for this item.");
	return;
  case ITEM_AFFECT:
        sprintf(result, "flag1: %s flag2: %s flag3: %s",
                none_room_bits[obj->obj_flags.value[0]],
                none_room_bits[obj->obj_flags.value[1]],
                none_room_bits[obj->obj_flags.value[2]]);
        return;
  case ITEM_WEAPON:
	sprintf(result, "Forge level: %d ", obj->obj_flags.value[0]);
	break;
  default:
	sprintf(result, "Unknown object!");
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Unknown item type passed to oPrintValue.");
	return;
  } /* stage 1 switch */

  /* Stage 2 */
  switch(obj->obj_flags.type_flag) {
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_DUST:
        if( obj->obj_flags.value[1] == SPELL_WORD_OF_RECALL ||
            obj->obj_flags.value[1] == SPELL_GROUP_RECALL )
        {
	  sprintf( buf, "spell: %s.", spells[obj->obj_flags.value[1]]);
        }
        else
        {
	  sprintf( buf, "spell1: %s spell2: %s spell3: %s",
                   spells[obj->obj_flags.value[1]],
	           spells[obj->obj_flags.value[2]],
                   spells[obj->obj_flags.value[3]]);
        }
	strcat(result, buf);
	return;
  case ITEM_WAND:
  case ITEM_STAFF:
	sprintf(buf, "Max charges: %d Charges left: %d spell: %s", obj->obj_flags.value[1],
	obj->obj_flags.value[2], spells[obj->obj_flags.value[3]]);
	strcat(result, buf);
	return;
  case ITEM_FIREWEAPON:
	sprintf(buf, "Range: %d Stun duration: %d", obj->obj_flags.value[1],
	obj->obj_flags.value[2]);
	strcat(result, buf);
	return;
  case ITEM_MISSILE:
  case ITEM_WEAPON:
	sprintf(buf, "No. dice: %d Dice size: %d ", obj->obj_flags.value[1],
	obj->obj_flags.value[2]);
	strcat(result, buf);
	break; /* little more to do */
  default: /* we should never reach here, no matter what. */
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Reached stage 2 switch in oPrintValue, uhmm, ACK??");
	return;
  } /* stage 2 switch */
  /* Just have couple of types to finish off. */
  switch (obj->obj_flags.type_flag) {
  case ITEM_MISSILE:
	sprintf(buf, "Ammo: %d", obj->obj_flags.value[3]);
	strcat(result, buf);
	return;
  case ITEM_WEAPON:
	sprintf(buf, "Type: %s", attack_hit_text[obj->obj_flags.value[3]].singular);
	strcat(result, buf);
	return;
  default:
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Reached stage 3 switch in oPrintValue, uhmm, ACK??");
	return;
  } /* stage 3 switch */
} /* oPrintValue */

/* This function strips off any undesirable affect flags from a object. */
void
oCheckAffectFlags(ObjData *obj)
{
/*128: PROBLEM: Why do we need this?
  if (!obj) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: NULL objected passed to oCheckAffectFlags!!");
	return;
  }
  obj->obj_flags.bitvector &= ~STRIPBITS;
*/
}


/* Vex May 1997
** Since builders tend to screw up item rent, we shall
** calculate them instead, from the objects stats. To keep things
** interesting, a random element will be thrown in after the object
** is created from its proto-type, so item values and rent costs will
** vary from object to object, making each object a little different.
** I decided it was either to complex, or pointless to muck around with
** the following items to much:
** Scroll, Wand, Staff, Potion, Trap, Note, Key, Money, Pen, Boat, and
** Fountain.
** Basically leave it up to their creators to work out sensible costs, althou
** if these items have stats bonuses, these will still be added on top.
** The defines below reflect the number of coins I think it should cost to
** rent each 'point' of the attribute in question.
*/
#define STAT1RENT 30 /* Str, Int, Wis, Dex, Con, Cha */
#define STAT2RENT 60
#define STAT3RENT 100
#define STAT4RENT 180
#define STAT5RENT 280
#define STAT6RENT 400
#define AGE_RENT 25
#define MANA10RENT 4
#define MANA20RENT 10
#define MANA30RENT 16
#define MANA40RENT 26
#define MANA50RENT 40
#define MANA51RENT 65
#define HP5RENT 4
#define HP10RENT 10
#define HP15RENT 16
#define HP20RENT 26
#define HP25RENT 40
#define HP30RENT 60
#define HP31RENT 72
#define MOVE_RENT 5
#define AC5RENT 5
#define AC10RENT 15
#define AC15RENT 40
#define AC16RENT 70
#define HIT1RENT 40
#define HIT2RENT 70
#define HIT3RENT 90
#define HIT4RENT 150
#define HIT5RENT 200
#define HIT6RENT 350
#define DAM1RENT  60
#define DAM2RENT 100
#define DAM3RENT 160
#define DAM4RENT 240
#define DAM5RENT 350
#define DAM6RENT 500
#define SAVE_RENT 50 /* Para, Rod, Petri, Breath, Spell */
#define CONT_RENT 1
#define DRINK_RENT 10
#define DRINK_MAX 200
#define FOOD_RENT 3
#define WPN5RENT 2
#define WPN10RENT 10
#define WPN15RENT 25
#define WPN20RENT 100
#define WPN25RENT 200
#define WPN30RENT 300
#define WPN31RENT 500
#define LIGHT_RENT 1000 /* Infinite source */

/* read all objects from obj file; generate index and prototypes */
void calculateRent(ObjData *obj)
{
  int t[4], wpn_average, j;

  t[0] = obj->obj_flags.value[0];
  /* Ok, see if we need to adjust rent cost. */
  switch (obj->obj_flags.type_flag) {
	case ITEM_ARMOR :
                GET_OBJ_RENT(obj) = 0;
		if (abs(t[0]) <= 5)
			GET_OBJ_RENT(obj) += abs(t[0]) * AC5RENT;
		else if (abs(t[0]) <= 10)
			GET_OBJ_RENT(obj) += (5 * AC5RENT) + ((abs(t[0]) - 5) * AC10RENT);
		else if (abs(t[0]) <= 15)
			GET_OBJ_RENT(obj) += (5 * (AC5RENT + AC10RENT)) + ((abs(t[0]) -10) * AC15RENT);
		else
			GET_OBJ_RENT(obj) += (5 * (AC5RENT + AC10RENT + AC15RENT)) + ((abs(t[0]) - 15) * AC16RENT);
		break;
	case ITEM_CONTAINER:
                GET_OBJ_RENT(obj) = 0;
		GET_OBJ_RENT(obj) += abs(t[0]) * CONT_RENT;
		break;
	case ITEM_DRINKCON:
                GET_OBJ_RENT(obj) = 0;
		if (abs(t[0]) > DRINK_MAX)
		  obj->obj_flags.cost_per_day += DRINK_MAX * DRINK_RENT;
		else
		  obj->obj_flags.cost_per_day += abs(t[0]) * DRINK_RENT;
 		break;
	case ITEM_FOOD: 
                GET_OBJ_RENT(obj) = 0;
		obj->obj_flags.cost_per_day  += abs(t[0]) * FOOD_RENT;
		break;
	case ITEM_POTION: 
	case ITEM_SCROLL: 
	case ITEM_WAND: 
        case ITEM_DUST:
           GET_OBJ_RENT(obj) = GET_OBJ_COST(obj); /* Base it on the item's cost */
           break;
/*
** Digger
*/
	default: /* No need to do anything */
           GET_OBJ_RENT(obj) = 0;
  }
  t[1] = obj->obj_flags.value[1];
  t[2] = obj->obj_flags.value[2];
  /* Adjust rent costs, if needed. */
  switch (obj->obj_flags.type_flag) {
	case ITEM_WEAPON:
		wpn_average = abs( ((t[1] * (t[2] + 1))/2) );
		if ((t[1] <= 0) || (t[2] <= 0))
			mudlog(NRM, LVL_IMMORT, TRUE, "0 ERROR: %s", buf2);
		if (wpn_average <= 5)
			obj->obj_flags.cost_per_day += wpn_average * WPN5RENT;
		else if (wpn_average <= 10)
			obj->obj_flags.cost_per_day += (5 * WPN5RENT) + ((wpn_average - 5) * WPN10RENT);
		else if (wpn_average <= 15)
			obj->obj_flags.cost_per_day += (5 * (WPN5RENT + WPN10RENT)) + ((wpn_average - 10) * WPN15RENT);
		else if (wpn_average <= 20)
			obj->obj_flags.cost_per_day += (5 * (WPN5RENT + WPN10RENT + WPN15RENT)) + ((wpn_average - 15) * WPN20RENT);
		else if (wpn_average <= 25)
			obj->obj_flags.cost_per_day += (5 * (WPN5RENT + WPN10RENT + WPN15RENT + WPN20RENT)) + ((wpn_average - 20) * WPN25RENT);
		else if (wpn_average <= 30)
			obj->obj_flags.cost_per_day += (5 * (WPN5RENT + WPN10RENT + WPN15RENT + WPN20RENT + WPN25RENT)) + ((wpn_average - 25) * WPN30RENT);
		else
			obj->obj_flags.cost_per_day += (5 * (WPN5RENT + WPN10RENT + WPN15RENT + WPN20RENT + WPN25RENT + WPN30RENT)) + ((wpn_average - 30) * WPN31RENT);
		break;
	case ITEM_LIGHT:
		if ((t[2] < 0) || (t[2] > 1000))
			obj->obj_flags.cost_per_day += LIGHT_RENT;
		else
			obj->obj_flags.cost_per_day += t[2];
		break;
	default: /* No need to do anything. */
		break;
  }
  t[3] = obj->obj_flags.value[3];

  t[0] = obj->obj_flags.weight;
  t[1] = obj->obj_flags.cost; 
  t[2] = obj->obj_flags.cost_per_day;

  /* Ok, we will only use the builders specefied rent cost for some items. */
  if (   (obj->obj_flags.type_flag == ITEM_SCROLL)
      || (obj->obj_flags.type_flag == ITEM_WAND)
      || (obj->obj_flags.type_flag == ITEM_STAFF)
      || (obj->obj_flags.type_flag == ITEM_FIREWEAPON)
      || (obj->obj_flags.type_flag == ITEM_MISSILE)
      || (obj->obj_flags.type_flag == ITEM_TREASURE)
      || (obj->obj_flags.type_flag == ITEM_POTION)
      || (obj->obj_flags.type_flag == ITEM_DUST)
      || (obj->obj_flags.type_flag == ITEM_NOTE)
      || (obj->obj_flags.type_flag == ITEM_KEY)
      || (obj->obj_flags.type_flag == ITEM_PEN)
      || (obj->obj_flags.type_flag == ITEM_BOAT)
      || (obj->obj_flags.type_flag == ITEM_FOUNTAIN)
      || (obj->obj_flags.type_flag == ITEM_POLE))
  {
    obj->obj_flags.cost_per_day = abs( (t[2]/10) );
  }

  for ( j = 0; j < MAX_OBJ_AFFECT; j++) {
      t[0] = obj->affected[j].location; 
      t[1] = obj->affected[j].modifier; 

      /* Ok, now lets adjust the rent cost according to what the modifier is */
      if ((t[0] >= APPLY_STR) && (t[0] <= APPLY_CHA))
      {
	if (abs(t[1]) > 5)
		obj->obj_flags.cost_per_day += (abs(t[1]) - 5) * STAT6RENT;
	if (abs(t[1]) >= 5)
		obj->obj_flags.cost_per_day += STAT5RENT;
	if (abs(t[1]) >= 4)
		obj->obj_flags.cost_per_day += STAT4RENT;
	if (abs(t[1]) >= 3)
		obj->obj_flags.cost_per_day += STAT3RENT;
	if (abs(t[1]) >= 2)
		obj->obj_flags.cost_per_day += STAT2RENT;
	if (abs(t[1]) >= 1)
		obj->obj_flags.cost_per_day += STAT1RENT;
      }
      else if (t[0] == APPLY_AGE)
	obj->obj_flags.cost_per_day += abs(t[1]) * AGE_RENT;
      else if (t[0] == APPLY_MANA)
      {
	if (abs(t[1]) >= 0)
	{
		if (abs(t[1]) < 10)
			obj->obj_flags.cost_per_day += abs(t[1]) * MANA10RENT;
		else
			obj->obj_flags.cost_per_day += 10 * MANA10RENT;
	}
	if (abs(t[1]) >= 10)
	{
		if (abs(t[1]) < 20)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 10) * MANA20RENT;
		else
			obj->obj_flags.cost_per_day += 10 * MANA20RENT;
	}
	if (abs(t[1]) >= 20)
	{
		if (abs(t[1]) < 30)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 20) * MANA30RENT;
		else
			obj->obj_flags.cost_per_day += 10 * MANA30RENT;
	}
	if (abs(t[1]) >= 30)
	{
		if (abs(t[1]) < 40)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 30) * MANA40RENT;
		else
			obj->obj_flags.cost_per_day += 10 * MANA40RENT; 
	}
	if (abs(t[1]) >= 40)
	{
		if (abs(t[1]) < 50)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 40) * MANA50RENT;
		else
			obj->obj_flags.cost_per_day += 10 * MANA50RENT;
	}
	if (abs(t[1]) > 50)
		obj->obj_flags.cost_per_day += (abs(t[1]) - 50) * MANA51RENT;
      }
      else if (t[0] == APPLY_HIT)
      {
	if (abs(t[1]) >= 0)
	{
		if (abs(t[1]) < 5)
			obj->obj_flags.cost_per_day += abs(t[1]) * HP5RENT;
		else
			obj->obj_flags.cost_per_day += 5 * HP5RENT;
	}
	if (abs(t[1]) >= 5)
	{
		if (abs(t[1]) < 10)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 5) * HP10RENT;
		else
			obj->obj_flags.cost_per_day += 5 * HP10RENT;
	}
	if (abs(t[1]) >= 10)
	{
		if (abs(t[1]) < 15)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 10) * HP15RENT;
		else
			obj->obj_flags.cost_per_day += 5 * HP15RENT;
	}
	if (abs(t[1]) >= 15)
	{
		if (abs(t[1]) < 20)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 15) * HP20RENT;
		else
			obj->obj_flags.cost_per_day += 5 * HP20RENT; 
	}
	if (abs(t[1]) >= 20)
	{
		if (abs(t[1]) < 25)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 20) * HP25RENT;
		else
			obj->obj_flags.cost_per_day += 5 * HP25RENT;
	}
	if (abs(t[1]) >= 25)
	{
		if (abs(t[1]) < 30)
			obj->obj_flags.cost_per_day += (abs(t[1]) - 25) * HP30RENT;
		else
			obj->obj_flags.cost_per_day += 5 * HP30RENT;
	}
	if (abs(t[1]) > 30)
		obj->obj_flags.cost_per_day += (abs(t[1]) - 30) * HP31RENT;
      }
      else if (t[0] == APPLY_MOVE)
	obj->obj_flags.cost_per_day += abs(t[1]) * MOVE_RENT;
      else if (t[0] == APPLY_AC)
      {
	if (abs(t[1]) <= 5)
		obj->obj_flags.cost_per_day += abs(t[1]) * AC5RENT;
	else if (abs(t[1]) <= 10)
		obj->obj_flags.cost_per_day += (5 * AC5RENT) + ((abs(t[1]) - 5) * AC10RENT);
	else if (abs(t[1]) <= 15)
		obj->obj_flags.cost_per_day += (5 * (AC5RENT + AC10RENT)) + ((abs(t[1]) -10) * AC15RENT);
	else
		obj->obj_flags.cost_per_day += (5 * (AC5RENT + AC10RENT + AC15RENT)) + ((abs(t[1]) - 15) * AC16RENT);
      }
      else if (t[0] == APPLY_HITROLL)
      {
	if (abs(t[1]) > 5)
		obj->obj_flags.cost_per_day += (abs(t[1]) - 5) * HIT6RENT;
	if (abs(t[1]) >= 5)
		obj->obj_flags.cost_per_day += HIT5RENT;
	if (abs(t[1]) >= 4)
		obj->obj_flags.cost_per_day += HIT4RENT;
	if (abs(t[1]) >= 3)
		obj->obj_flags.cost_per_day += HIT3RENT;
	if (abs(t[1]) >= 2)
		obj->obj_flags.cost_per_day += HIT2RENT;
	if (abs(t[1]) >= 1)
		obj->obj_flags.cost_per_day += HIT1RENT;
      }
      else if (t[0] == APPLY_DAMROLL)
      { 
	if (abs(t[1]) > 5)
		obj->obj_flags.cost_per_day += (abs(t[1]) - 5) * DAM6RENT;
	if (abs(t[1]) >= 5)
		obj->obj_flags.cost_per_day += DAM5RENT;
	if (abs(t[1]) >= 4)
		obj->obj_flags.cost_per_day += DAM4RENT;
	if (abs(t[1]) >= 3)
		obj->obj_flags.cost_per_day += DAM3RENT;
	if (abs(t[1]) >= 2)
		obj->obj_flags.cost_per_day += DAM2RENT;
	if (abs(t[1]) >= 1)
		obj->obj_flags.cost_per_day += DAM1RENT;
      }
      else if ((t[0] >= APPLY_SAVING_PARA) && (t[0] <= APPLY_SAVING_SPELL))
	obj->obj_flags.cost_per_day += abs(t[1]) * SAVE_RENT;
      /* Nothing else will have any effect on rent... */
  } /* for loop */
} /* calculateRent */

/* ****************************************************************************
** Constants                                                                  
**************************************************************************** */

const ApplyRestrictType apply_restrictions[NUM_APPLIES] = {
{0, 0},		/* 0 APPLY_NONE */
{-5, 5},	/* 1 APPLY_STRENGTH */
{-5, 5},	/* 2 APPLY_DEXTERITY */
{-5, 5},	/* 3 APPLY_INTELLIGENCE */
{-5, 5},	/* 4 APPLY_WISDOM */
{-5, 5},	/* 5 APPLY_CONSTITUTION */
{-5, 5},	/* 6 APPLY_CHARISMA */
{0, 0},		/* 7 APPLY_CLASS */
{0, 0},		/* 8 APPLY_LEVEL */
{-100, 100},	/* 9 APPLY_AGE */
{-100, 100},	/* 10 APPLY_CHAR_WEIGHT */
{-100, 100},	/* 11 APPLY_CHAR_HEIGHT */
{-100, 50},	/* 12 APPLY_MANA */
{-100, 50},	/* 13 APPLY_MAX_HIT */
{-100, 100},	/* 14 APPLY_MAX_MOVE */
{0, 0},		/* 15 APPLY_GOLD */
{0, 0},		/* 16 APPLY_EXP */
{-50, 100},	/* 17 APPLY_AC */
{-10, 5},	/* 18 APPLY_HITROLL */
{-10, 5},	/* 19 APPLY_DAMROLL */
{-20, 20},	/* 20 APPLY_SAVING_PARA */
{-20, 20},	/* 21 APPLY_SAVING_ROD */
{-20, 20},	/* 22 APPLY_SAVING_PETRI */
{-20, 20},	/* 23 APPLY_SAVING_BREATH */
{-20, 20},	/* 24 APPLY_SAVING_SPELL */
{0, 0},		/* 25 APPLY_POISON */
{0, 0},		/* 26 APPLY_PLAGUE */
{-80, 100},	/* 27 APPLY_SPELL_COST */
{-50, 50},	/* 28 APPLY_SPELL_SAVE */
{-30, 30},	/* 29 APPLY_SPELL_DAMAGE */
{-30, 30},	/* 30 APPLY_SPELL_DURATION */
{-15, 15},	/* 31 APPLY_SPELL_SUCCESS */
{-15, 15},	/* 32 APPLY_SKILL_SUCCESS (DEPRECATED) */
{-60, 60}	/* 33 APPLY_USELEVEL */
};

const ValueRestrictType obj_value_restrictions[NUM_ITEM_TYPES][4] = {
/* 0 ITEM_UNDEFINED */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 1 ITEM_LIGHT */
{ {0, 0}, {0, 0}, {-1, 100000}, {0, 0} },
/* 2 ITEM_SCROLL */
{ {1, MAX_MORTAL}, {0, NUM_SPELLS -1}, {0, NUM_SPELLS -1}, {0, NUM_SPELLS -1} },
/* 3 ITEM_WAND */
{ {1, MAX_MORTAL}, {0, 10}, {0, 10}, {0, NUM_SPELLS -1} },
/* 4 ITEM_STAFF */
{ {1, MAX_MORTAL}, {0, 10}, {0, 10}, {0, NUM_SPELLS -1} },
/* 5 ITEM_WEAPON */
/* forge           num dice  sz dice   type */
{ {0, MAX_MORTAL}, {1, 100}, {1, 100}, {0, NUM_WEAPON_TYPES -1} },
/* 6 ITEM_FIREWEAPON */
{ {0, NUM_MISSILE_TYPES -1}, {0, 4}, {1, 4}, {0, 0} },
/* 7 ITEM_MISSILE */
{ {0, NUM_MISSILE_TYPES -1}, {1, 100}, {1, 100}, {1, 20} },
/* 8 ITEM_TREASURE */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 9 ITEM_ARMOR */
{ {-15, 15}, {0, 0}, {0, 0}, {0, 0} },
/* 10 ITEM_POTION */
{ {1, MAX_MORTAL}, {0, TOP_SPELL_DEFINE}, {0, TOP_SPELL_DEFINE}, {0, TOP_SPELL_DEFINE} },
/* 11 ITEM_WORN */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 12 ITEM_OTHER */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 13 ITEM_TRASH */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 14 ITEM_TRAP */
/* type   Cont Flags  In ROOM    Room Num */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },

/* 15 ITEM_CONTAINER */
{ {0, 10000}, {0, 63}, {-1, 99999}, {0, LVL_IMMORT} },
/* 16 ITEM_NOTE */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 17 ITEM_DRINKCON */
{ {0, 10000}, {0, 10000}, {0, NUM_LIQ_TYPES -1}, {-1, MAX_MORTAL} },
/* 18 ITEM_KEY */
{ {0, 2}, {0, 0}, {0, 0}, {0, 0} },
/* 19 ITEM_FOOD */
{ {-24, 24}, {0, MAX_MORTAL}, {0, TOP_SPELL_DEFINE}, {-1, MAX_MORTAL} },
/* 20 ITEM_MONEY */
{ {1, 1000000}, {0, 0}, {0, 0}, {0, 0} },
/* 21 ITEM_PEN */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 22 ITEM_BOAT */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 23 ITEM_FOUNTAIN */
{ {0, 1000000}, {0, 1000000}, {0, NUM_LIQ_TYPES -1}, {-1, MAX_MORTAL} },
/* 24 ITEM_PORTAL */
{ {-1, 99999}, {-1, 99999}, {-1, 99999}, {-1, 99999} },
/* 25 ITEM_SCRIBE */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
/* 26 ITEM_AFFECT */
{ {0, NUM_ROOM_FLAGS}, {0, NUM_ROOM_FLAGS}, {0, NUM_ROOM_FLAGS}, {0, 0} },
/* 27 ITEM_DUST */
{ {1, MAX_MORTAL}, {0, TOP_SPELL_DEFINE}, {0, TOP_SPELL_DEFINE}, {0, TOP_SPELL_DEFINE} },
/* 28 ITEM_POLE */
{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
};

void oedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
  /*
  case OEDIT_ACTDESC:
    oMenu(d);
    break;
  */
  case OEDIT_EXTRADESC_DESCRIPTION:
    oExtraDescMenu(d);
    break;
  }
}
