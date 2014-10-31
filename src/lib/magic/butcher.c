
/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_BUTCHER

#define SKILL_ADVANCES_WITH_USE   FALSE
#define SKILL_ADVANCE_STRING      "You've discovered cleavage is your friend."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               FALSE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  4

/*
** MUD SPECIFIC INCLUDES
*/
#include <stdlib.h>

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/butcher.h"

int setButcherSpell(CharData *ch);

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      SKILL_NAME - butcher
**
**  FORMAL PARAMETERS:
**
**      ch:
**          A pointer to the character structure that is trying to use the skill/spell.
**
**  RETURN VALUE:
**
**      None
**
**  DESIGN:
**
**      A butchering skill allowing one to convert corpses into steaks.
**
**  NOTES:
**      The following standard macros can be used from skills.h:
**
**          IF_CH_CANT_SEE_VICTIM( string );             IF_VICTIM_NOT_WIELDING( string );
**          IF_CH_CANT_BE_VICTIM( string );              IF_VICTIM_CANT_BE_FIGHTING( string );
**          IF_CH_NOT_WIELDING( string );
**
**      The parameter `string' is the string that is sent to the user when the
**      error condition is met and the skill function exits. For example when an
**      attempt to disarm a weaponless victim is made:
**
**          IF_VICTIM_NOT_WIELDING( "That person isn't even wielding a weapon!\r\n" );
**
*/
ACMD(do_butcher)
{
  char   arg[100];
  ObjData *obj;	
  ObjData *steak;
  int i;
  int thespell = 0;

  IF_UNLEARNED_SKILL("You wouldn't know where to begin.");

  one_argument(argument, arg);

  if(!*arg)
  {
    sendChar(ch, "Butcher what?\r\n");
    return;
  }

  obj = 0;

  if( !(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)) )
  {
      sendChar( ch, "Butcher what?\r\n" );
      return;
  }

  if( (obj->obj_flags.type_flag != ITEM_CONTAINER) ||
     !(GET_OBJ_VAL(obj,3) != 0) || !CAN_WEAR(obj, ITEM_WEAR_TAKE))
  {
      sendChar(ch, "You can't butcher that!\r\n" );
      return;
  }

  if (obj->contains) {
      send_to_char("The corpse must be empty first.\r\n", ch);
      return;
  }

  if(skillSuccess(ch, THIS_SKILL ))
  {
    advanceSkill(ch, THIS_SKILL,SKILL_MAX_LEARN, SKILL_ADVANCE_STRING,
                      DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );

    act("You butcher $p into lean fillets.", FALSE, ch, obj, 0, TO_CHAR );
    act("$n butchers $p into lean fillets.", FALSE, ch, obj, 0, TO_ROOM );

    for(i = 0; i < ((GET_LEVEL(ch)/15) > 0 ? (GET_LEVEL(ch)/15) : 1); i++)
    {
      CREATE(steak, ObjData, 1);
      clear_object(steak);
      steak->name = str_dup("steak");
      steak->short_description = str_dup("a juicy steak");
      steak->description =
        str_dup("A really delicious steak has been discarded here.");
      steak->obj_flags.type_flag = ITEM_FOOD;
      SET_BIT_AR(GET_OBJ_WEAR(steak), ITEM_WEAR_TAKE);
      SET_BIT_AR(GET_OBJ_EXTRA(steak), ITEM_NOSELL);
      SET_BIT_AR(GET_OBJ_EXTRA(steak), ITEM_NODONATE);
      SET_BIT_AR(GET_OBJ_EXTRA(steak), ITEM_ARTIFACT);
      steak->obj_flags.value[0] = 5 + GET_LEVEL(ch);
      steak->obj_flags.weight   = 1;
      steak->obj_flags.cost     = -1;
      steak->obj_flags.cost_per_day = -1;

      SET_BIT_AR(steak->obj_flags.extra_flags, ITEM_TIMED);
      GET_OBJ_TIMER(steak) = MAX(1, number(GET_LEVEL(ch)/4, GET_LEVEL(ch)/2));

      // Butcher now has bonus affects on food
      if((thespell = setButcherSpell(ch))) {
          GET_OBJ_VAL(steak, 0) = GET_OBJ_WEIGHT(steak);
          GET_OBJ_VAL(steak, 1) = GET_LEVEL(ch);
          GET_OBJ_VAL(steak, 2) = thespell;
          GET_OBJ_VAL(steak, 3) = 0;
      }

      steak->next = object_list;
      object_list = steak;

      steak->item_number = -1;

      obj_to_char(steak, ch);
    }
  }

  else
  {
    sendChar( ch, "You mangle the corpse into unusable fragments of flesh.\r\n");
    act( "$n mangles $p into useless fragments of flesh.", FALSE, ch, obj, 0, TO_ROOM);
  }

  extract_obj(obj);

}/* do_butcher */

int setButcherSpell(CharData *ch) {
	int level = GET_LEVEL(ch);
	
	if (IS_NATURALIST(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL)
		level += number(1, 20);

	if(percentSuccess(level - 56))
		return SPELL_REVIVE;
	else if(percentSuccess(level - 48))
		return SPELL_PRAYER_OF_LIFE;
	else if(percentSuccess(level - 40))
		return SPELL_HEAL;
	else if(percentSuccess(level - 32))
		return SPELL_REFRESH;
	else if(percentSuccess(level - 24))
		return SPELL_CURE_CRITIC;
	else if(percentSuccess(level - 16))
		return SPELL_CURE_SERIOUS;
	else if(percentSuccess(level - 8))
		return SPELL_CURE_LIGHT;
        else return 0;
}
