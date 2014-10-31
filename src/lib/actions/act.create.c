/* ************************************************************************
*   File: act.create.c					Part of CircleMUD *
*  Usage: Player-level object creation stuff				  *
*									  *
*  All rights reserved.	 See license.doc for complete information.	  *
*									  *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.		  *
************************************************************************ */
 
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "actions/fight.h"

/* struct for syls */
struct syllable {
  char *org;
  char *new;
};

/* extern variables */
extern struct spell_info_type spell_info[];
extern struct syllable syls[];
extern struct index_data *obj_index;

/* extern procedures */
int mag_manacost(CharData * ch, int spellnum, int spellCostImmutable);

/* Forward declarations. */
static char *garble_spell(int spellnum);

#if 0
char *get_spell_name(char *argument)
{
  char *s;

  s = strtok(argument, "'");
  s = strtok(NULL, "'");
  
  return s;
}
#endif

char *potion_names[] = {
	"milky white",
	"bubbling white",
	"glowing ivory",
	"glowing blue",
	"bubbling yellow",
	"light green",
	"gritty brown",
	"blood red",
	"swirling purple",
	"flickering green",
	"cloudy blue",
	"glowing red",
	"sparkling white",
	"incandescent blue"  
};

static int can_create_spell(int spell)
{
    return (spell == SPELL_PORTAL ||
            spell == SPELL_GROUP_HASTE ||
            spell == SPELL_GATE ||
            spell == SPELL_DOOM_BOLT ||
            spell == SPELL_CONJURE_ELEMENTAL ||
            spell == SPELL_MONSTER_SUMMON ||
            spell == SPELL_SUMMON ||
            spell == SPELL_WRATH_OF_THE_ANCIENTS ||
            spell == SPELL_RELOCATE ||
            spell == SPELL_NEXUS ||
            spell == SPELL_BLINK ||
            spell == SPELL_ASSISTANT ||
            spell == SPELL_FORTIFY ||
            spell == SPELL_SACRIFICE ||
            spell == SPELL_CHARM    ||
	    spell == SPELL_CREATE_WARDING ) ? 0 : 1;
}

static void make_dust(CharData *ch, int spell, ObjData *dust)
{
    ObjData *final_dust;
    struct extra_descr_data *new_descr;
    int mana, dam;

    if (!can_create_spell(spell)) {
        send_to_char("That spell cannot be mixed into dust.\r\n", ch);
        return;
    } else if (!number(0,2) && (GET_LEVEL(ch) < LVL_IMMORT)) {
        send_to_char("As you mix your magic into the dust, it violently"
                     " explodes!\r\n",ch);
        act("$n begins to mix some dust, but it suddenly explodes!",
                FALSE, ch, 0,0, TO_ROOM);
        extract_obj(dust);
        dam = number(15, mag_manacost(ch, spell, TRUE) * 2);
        GET_HIT(ch) -= dam;
        update_pos(ch);
        if( GET_POS(ch) == POS_DEAD ) {
            raw_kill(ch, NULL);
        }
        return;
    }

    /* requires x3 mana to mix a potion than the spell */
    mana = mag_manacost(ch, spell, TRUE) * 3;
    if (GET_MANA(ch) - mana > 0) {
        GET_MANA(ch) -= mana;
        sprintf(buf, "You create some %s dust.\r\n", spells[spell]);
        send_to_char(buf, ch);
        act("$n creates some dust!", FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(dust);
    } else {
        send_to_char("You don't have enough mana to create that dust!\r\n", ch);
        return;
    }
	
    final_dust = create_obj();

    final_dust->item_number = NOTHING;
    final_dust->in_room = NOWHERE;
    sprintf(buf2, "%s %s dust", spells[spell], garble_spell(spell));
    final_dust->name = str_dup(buf2);

    sprintf(buf2, "Some magic %s dust is in a pile here.",
		garble_spell(spell));
    final_dust->description = str_dup(buf2);

    sprintf(buf2, "some %s dust", garble_spell(spell));
    final_dust->short_description = str_dup(buf2);

    /* extra description coolness! */
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(final_dust->name);
    sprintf(buf2, "It appears to be some %s dust.", spells[spell]);
    new_descr->description = str_dup(buf2);
    new_descr->next = NULL;
    final_dust->ex_description = new_descr;
 
    GET_OBJ_TYPE(final_dust)   = ITEM_DUST;
    SET_BIT_AR(GET_OBJ_WEAR(final_dust), ITEM_WEAR_TAKE);
    SET_BIT_AR(GET_OBJ_WEAR(final_dust), ITEM_WEAR_HOLD);
    SET_BIT_AR(GET_OBJ_EXTRA(final_dust), ITEM_NORENT);
    SET_BIT_AR(GET_OBJ_EXTRA(final_dust), ITEM_TIMED);
    SET_BIT_AR(GET_OBJ_EXTRA(final_dust), ITEM_NOSELL);
	SET_BIT_AR(GET_OBJ_EXTRA(final_dust), ITEM_ARTIFACT);
	GET_OBJ_COST(final_dust)   = 0;
    GET_OBJ_WEIGHT(final_dust) = 1;
    GET_OBJ_RENT(final_dust)   = 0;
    GET_OBJ_TIMER(final_dust)  = 50;

    /* set spell level and types */
    GET_OBJ_VAL(final_dust, 0) = GET_LEVEL(ch);
    GET_OBJ_VAL(final_dust, 1) = spell;
    GET_OBJ_VAL(final_dust, 2) = 0;
    GET_OBJ_VAL(final_dust, 3) = 0;

    /* 33% chance there'll be two loads of spell on it if mixer is level 40 or higher*/
    if (number(1,100) <= 33 && GET_LEVEL(ch) >= 40) {
        GET_OBJ_VAL(final_dust, 2) = spell;
        /* further 25% chance that there'll be three loads if mixer is level 50*/
        if (number(1,100) <= 25 && GET_LEVEL(ch) >= 50) {
            GET_OBJ_VAL(final_dust, 3) = spell;
        }
    }

    obj_to_char(final_dust, ch);
}

void make_potion(CharData *ch, int potion, ObjData *container)
{
    ObjData *final_potion;
    struct extra_descr_data *new_descr;
    int mana, dam;

    if (!can_create_spell(potion)) {
        send_to_char("That spell cannot be mixed into a potion.\r\n", ch);
        return;
    } else if ((number(1, 3) == 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
        send_to_char("As you begin mixing the potion, it violently"
                     " explodes!\r\n",ch);
        act("$n begins to mix a potion, but it suddenly explodes!",
                FALSE, ch, 0,0, TO_ROOM);
        extract_obj(container);
        dam = number(15, mag_manacost(ch, potion, TRUE) * 2);
        GET_HIT(ch) -= dam;
        update_pos(ch);
        if( GET_POS(ch) == POS_DEAD ) {
            raw_kill(ch, NULL);
        }
        return;
    }

    /* requires x3 mana to mix a potion than the spell */
    mana = mag_manacost(ch, potion, TRUE) * 3;
    if (GET_MANA(ch) - mana > 0) {
        GET_MANA(ch) -= mana;
        sprintf(buf, "You create a %s potion.\r\n", spells[potion]);
        send_to_char(buf, ch);
        act("$n creates a potion!", FALSE, ch, 0, 0, TO_ROOM);
        extract_obj(container);
    } else {
        send_to_char("You don't have enough mana to mix that potion!\r\n", ch);
        return;
    }
	
    final_potion = create_obj();

    final_potion->item_number = NOTHING;
    final_potion->in_room = NOWHERE;
    sprintf(buf2, "%s %s potion", spells[potion], garble_spell(potion));
    final_potion->name = str_dup(buf2);

    sprintf(buf2, "A potion inscribed with the runes '%s' lies here.",
		garble_spell(potion));
    final_potion->description = str_dup(buf2);

    sprintf(buf2, "a %s potion", garble_spell(potion));
    final_potion->short_description = str_dup(buf2);

    /* extra description coolness! */
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(final_potion->name);
    sprintf(buf2, "It appears to be a %s potion.", spells[potion]);
    new_descr->description = str_dup(buf2);
    new_descr->next = NULL;
    final_potion->ex_description = new_descr;
 
    GET_OBJ_TYPE(final_potion)   = ITEM_POTION;
    SET_BIT_AR(GET_OBJ_WEAR(final_potion), ITEM_WEAR_TAKE);
    SET_BIT_AR(GET_OBJ_EXTRA(final_potion), ITEM_NORENT);
    SET_BIT_AR(GET_OBJ_EXTRA(final_potion), ITEM_TIMED);
    SET_BIT_AR(GET_OBJ_EXTRA(final_potion), ITEM_NOSELL);
    GET_OBJ_VAL(final_potion, 0) = GET_LEVEL(ch);
    GET_OBJ_VAL(final_potion, 1) = potion;
    GET_OBJ_VAL(final_potion, 2) = 0;
    GET_OBJ_VAL(final_potion, 3) = 0;
    GET_OBJ_COST(final_potion)   = 0;
    GET_OBJ_WEIGHT(final_potion) = 1;
    GET_OBJ_RENT(final_potion)   = 0;
    GET_OBJ_TIMER(final_potion)  = 50;

    obj_to_char(final_potion, ch);
}

ACMD(do_mix)
{
    ObjData *dust = NULL;
    ObjData *obj, *next_obj;
    char item_name[MAX_STRING_LENGTH];
    char spell_name[MAX_STRING_LENGTH];
    int spell, found = FALSE;

    item_name[0] = '\0'; /* The easy way :) Vex. */
    spell_name[0] = '\0';
    two_arguments(argument, spell_name, item_name);
     
    if(!IS_FAERIE(ch)) {
        send_to_char("You have no idea how to mix potions!\r\n", ch);
        return;
    }

    if (!*item_name || !*spell_name) {
        send_to_char("What do you wish to mix in where?\r\n", ch);
        return;
    }

    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(dust = get_obj_in_list_vis(ch, item_name,
                        ch->carrying))) 
            continue;
        else
            found = TRUE;
    }
    if (found != FALSE && (GET_OBJ_TYPE(dust) != ITEM_DUST)) {
        send_to_char("That item is not a pile of dust!\r\n", ch);
        return;
    }

    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
            item_name);
        send_to_char(buf, ch);
        return;
    }

    if (!spell_name || !*spell_name) {
        send_to_char("Spell names must be enclosed in single quotes!\r\n",
                ch);
        return;
    }
 
    spell = find_skill_num(spell_name);	

    if ((spell < 1) || (spell > MAX_SPELLS)) {
        send_to_char("Mix what spell?!?\r\n", ch);
        return;
    }
    if (GET_LEVEL(ch) < spell_info[spell].min_level[(int) GET_CLASS(ch)]) {
        send_to_char("You do not know how to mix that spell!\r\n", ch);
        return;
    }
    if (GET_SKILL(ch, spell) == 0) {
        send_to_char("You are unfamiliar with that spell.\r\n", ch);
        return;
    }
    make_dust(ch, spell, dust);
}

ACMD(do_brew)
{
    ObjData *container = NULL;
    ObjData *obj, *next_obj;
    char bottle_name[MAX_STRING_LENGTH];
    char spell_name[MAX_STRING_LENGTH];
    int potion, found = FALSE;

    bottle_name[0] = '\0'; /* The easy way :) Vex. */
    spell_name[0] = '\0';
    two_arguments(argument, spell_name, bottle_name);
     
    if (!IS_GNOME(ch)) {
        send_to_char("You have no idea how to mix potions!\r\n", ch);
        return;
    }
    if (!*bottle_name || !*spell_name) {
        send_to_char("What do you wish to mix in where?\r\n", ch);
        return;
    }

    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(container = get_obj_in_list_vis(ch, bottle_name,
                        ch->carrying))) 
            continue;
        else
            found = TRUE;
    }
    if (found != FALSE && (GET_OBJ_TYPE(container) != ITEM_DRINKCON)) {
        send_to_char("That item is not a drink container!\r\n", ch);
        return;
    }
    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
            bottle_name);
        send_to_char(buf, ch);
        return;
    }

    if (!spell_name || !*spell_name) {
        send_to_char("Spell names must be enclosed in single quotes!\r\n",
                ch);
        return;
    }
 
    potion = find_skill_num(spell_name);	

    if ((potion < 1) || (potion > MAX_SPELLS)) {
        send_to_char("Mix what spell?!?\r\n", ch);
        return;
    }
    if (GET_LEVEL(ch) < spell_info[potion].min_level[(int) GET_CLASS(ch)]) {
        send_to_char("You do not know how to make that potion!\r\n", ch);
        return;
    }
    if (GET_SKILL(ch, potion) == 0) {
        send_to_char("You are unfamiliar with potion brewing.\r\n", ch);
        return;
    }
    make_potion(ch, potion, container);
}


static char *garble_spell(int spellnum)
{
  char lbuf[256];
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }
  return buf;
}

void make_scroll(CharData *ch, int scroll, ObjData *paper)
{
	ObjData *final_scroll;
	struct extra_descr_data *new_descr;
	int mana, dam = 0;

	if (!can_create_spell(scroll)) {
		send_to_char("That spell cannot be scribed into a"
			     " scroll.\r\n", ch);
		return;
	}
	else if ((number(1, 10) == 1) && (GET_LEVEL(ch) < LVL_IMMORT)) {
		send_to_char("As you begin inscribing the final rune, the"
			     " scroll violently explodes!\r\n",ch);
		act("$n tries to scribe a spell, but it explodes!",
		    FALSE, ch, 0,0, TO_ROOM);
		extract_obj(paper);
		dam = number(15, mag_manacost(ch, scroll, TRUE) * 2);
		GET_HIT(ch) -= dam;
		update_pos(ch);
          if( GET_POS(ch) == POS_DEAD )
            {
            raw_kill(ch, NULL);
            }
		return;
	}
	/* requires x3 mana to scribe a scroll than the spell */
	mana = mag_manacost(ch, scroll, TRUE) * 3;

	if (GET_MANA(ch) - mana > 0) {
		GET_MANA(ch) -= mana;
		sprintf(buf, "You create a scroll of %s.\r\n",
			spells[scroll]);
		send_to_char(buf, ch);
		act("$n creates a scroll!", FALSE, ch, 0, 0, TO_ROOM);
		extract_obj(paper);
	}
	else {
		send_to_char("You don't have enough mana to scribe such"
			     " a powerful spell!\r\n", ch);
		return;
	}
	
	final_scroll = create_obj();

	final_scroll->item_number = NOTHING;
	final_scroll->in_room = NOWHERE;
	sprintf(buf2, "scribedscroll%s %s %s scroll", GET_NAME(ch),
		spells[scroll], garble_spell(scroll));
	final_scroll->name = str_dup(buf2);

	sprintf(buf2, "Some parchment inscribed with the runes '%s' lies here.",
		garble_spell(scroll));
	final_scroll->description = str_dup(buf2);

	sprintf(buf2, "a %s scroll", garble_spell(scroll));
	final_scroll->short_description = str_dup(buf2);

	/* extra description coolness! */
	CREATE(new_descr, struct extra_descr_data, 1);
	new_descr->keyword = str_dup(final_scroll->name);
	sprintf(buf2, "It appears to be a %s scroll.", spells[scroll]);
	new_descr->description = str_dup(buf2);
	new_descr->next = NULL;
	final_scroll->ex_description = new_descr;
 
	GET_OBJ_TYPE(final_scroll) = ITEM_SCROLL;
	SET_BIT_AR(GET_OBJ_WEAR(final_scroll), ITEM_WEAR_TAKE);
	SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_NORENT);
        SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_TIMED);
	SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_NOSELL);
	GET_OBJ_VAL(final_scroll, 0) = GET_LEVEL(ch);
	GET_OBJ_VAL(final_scroll, 1) = scroll;
	GET_OBJ_VAL(final_scroll, 2) = 0;
	GET_OBJ_VAL(final_scroll, 3) = 0;
	GET_OBJ_COST(final_scroll) = GET_LEVEL(ch) * 100;
	GET_OBJ_WEIGHT(final_scroll) = 1;
	GET_OBJ_RENT(final_scroll) = 0;
	GET_OBJ_TIMER(final_scroll)  = 50;
	
	obj_to_char(final_scroll, ch);
}


ACMD(do_scribe)
{
#define MIN_SCRIBE_LEVEL 35
	ObjData *paper = NULL;
	ObjData *obj, *next_obj;
	char paper_name[MAX_STRING_LENGTH];
	char spell_name[MAX_STRING_LENGTH];
	int scroll = 0, found = FALSE;

	paper_name[0] = '\0';
	spell_name[0] = '\0';
	two_arguments(argument, spell_name, paper_name);

	if (!(((GET_CLASS(ch) == CLASS_MAGIC_USER) && (GET_LEVEL(ch) >= MIN_SCRIBE_LEVEL))
	    || GET_LEVEL(ch) >= LVL_IMMORT)) {
		send_to_char("You have no idea how to scribe scrolls!\r\n", ch);
		return;
	}
	if (!*paper_name || !*spell_name) {
		send_to_char("What do you wish to scribe where?\r\n", ch);
		return;
	}

	for (obj = ch->carrying; obj; obj = next_obj) {
		next_obj = obj->next_content;
		if (obj == NULL)
			return;
		else if (!(paper = get_obj_in_list_vis(ch, paper_name,
			ch->carrying))) 
			continue;
		else
			found = TRUE;
	}
	if (found && (GET_OBJ_TYPE(paper) != ITEM_SCRIBE)) {
		send_to_char("That item is not suitable for scroll making.\r\n", ch);
		return;
	}
	if (found == FALSE) {
		sprintf(buf, "You don't have %s in your inventory!\r\n",
			paper_name);
		send_to_char(buf, ch);
		return;
	}

	if (!spell_name || !*spell_name) {
	    send_to_char("Spell names must be enclosed in single quotes!\r\n",
			 ch);
	    return;
	} 

	scroll = find_skill_num(spell_name);	

	if ((scroll < 1) || (scroll > MAX_SPELLS)) {
		send_to_char("Scribe what spell?!?\r\n", ch);
		return;
	}
	if (GET_LEVEL(ch) < spell_info[scroll].min_level[(int) GET_CLASS(ch)]) {
		 send_to_char("You are not schooled enough to cast that spell!\r\n",
			      ch);
		return;
	}
	if (GET_SKILL(ch, scroll) == 0) {
		 send_to_char("You don't know any spell like that!\r\n",
		 ch);
		return;
	}
	if (!(ch->equipment[WEAR_HOLD] && (GET_OBJ_TYPE(ch->equipment[WEAR_HOLD]) == ITEM_PEN)))
	{
		send_to_char("You must be holding something to write with.\r\n", ch);
		return;
	}
	make_scroll(ch, scroll, paper);
}


ACMD(do_forge)
{
    /* PLEASE NOTE!!!  This command alters the object_values of the target
     * 	   weapon, and this will save to the rent files.  It should not cause
     * 	   a problem with stock Circle, but if your weapons use the first
     * 	   position [ GET_OBJ_VAL(weapon, 0); ], then you WILL have a problem.
     * 	   This command stores the character's level in the first value to
     * 	   prevent the weapon from being "forged" more than once by mortals.
     * 	   Install at your own risk.  You have been warned...
     */

    ObjData *weapon = NULL;
    ObjData *obj, *next_obj;
    char weapon_name[MAX_STRING_LENGTH];
    char optional_level[MAX_STRING_LENGTH];
    int found = FALSE, prob = 0, dam = 0;

    int level = GET_LEVEL(ch);

    two_arguments (argument, weapon_name, optional_level);
        
    if (IS_NPC(ch))
        return;

    if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
        send_to_char("Relax and enjoy being retired.\r\n", ch);
        return;
    }

    if (!(IS_DWARF(ch) || GET_LEVEL(ch) >= LVL_IMMORT)) {
        send_to_char("You have no idea how to forge weapons!\r\n", ch);
        return;
    }
    if (!*weapon_name) {
        send_to_char("What do you wish to forge?\r\n", ch);
        return;
    }

    for (obj = ch->carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj == NULL)
            return;
        else if (!(weapon = get_obj_in_list_vis(ch, weapon_name,
                ch->carrying)))
            continue;
        else
            found = TRUE;
    }

    if (found == FALSE) {
        sprintf(buf, "You don't have %s in your inventory!\r\n",
                weapon_name);
        send_to_char(buf, ch);
        return;
    }

    if (found && (GET_OBJ_TYPE(weapon) != ITEM_WEAPON)) {
        sprintf(buf, "It doesn't look like %s would make a"
                " good weapon...\r\n", weapon_name);
        send_to_char(buf, ch);
        return;
    }

    if (GET_OBJ_VAL(weapon, 0) > 0) {
        send_to_char("You are not powerful enough to forge this weapon!\r\n", ch);
        return;
    }

    if (IS_SET_AR(GET_OBJ_EXTRA(weapon), ITEM_MAGIC)) {
        send_to_char("The weapon is imbued with magical powers beyond"
                " your grasp.\r\nYou cannot further affect its form.\r\n", ch);
        return;
    }

    // Super dwarves may choose the level they forge to, even below level 30!
    if(GET_RACE(ch) == RACE_SDWARF) {
        if(*optional_level && is_number(optional_level))
            level = atoi(optional_level);
        level  = CLAMP_VALUE(1, level, GET_LEVEL(ch));
    }
    
    /* determine success probability */
    prob += ( ((level/8) * 10) + GET_DEX(ch) + GET_WIS(ch));
    
    if ((number(60, 100) > prob) && (level < LVL_IMMORT)) {
        send_to_char("As you pound out the dents in the weapon,"
                " you hit a weak spot and it explodes!\r\n", ch);
        act("$n tries to forge a weapon, but it explodes!",
        FALSE, ch, 0,0, TO_ROOM);
        extract_obj(weapon);
        dam = number(20, 60);
        GET_HIT(ch) -= dam;
        update_pos(ch);
        return;
    }

    int initialWeapon1 = GET_OBJ_VAL(weapon, 1);
    int initialWeapon2 = GET_OBJ_VAL(weapon, 2);

    GET_OBJ_VAL(weapon, 0) = level;
    GET_OBJ_VAL(weapon, 1) += number(-1, 2);
    GET_OBJ_VAL(weapon, 2) += number(-1, 1);
    GET_OBJ_RENT(weapon) += (level << 3);
    weapon->affected[5].location = APPLY_USELEVEL;
    weapon->affected[5].modifier = level;
    send_to_char("You have forged new life into the weapon!\r\n", ch);
    act("$n vigorously pounds on a weapon!", FALSE, ch, 0, 0, TO_ROOM);

    if(initialWeapon1 + 2 == GET_OBJ_VAL(weapon, 1) &&
            initialWeapon2 + 1 == GET_OBJ_VAL(weapon, 2))
    {
        act("You have forged $p into a very fine weapon!", FALSE, ch, weapon, 0, TO_CHAR);
        act("$n has forged $p into a very fine weapon!", FALSE, ch, weapon, 0, TO_ROOM);
    }

}

