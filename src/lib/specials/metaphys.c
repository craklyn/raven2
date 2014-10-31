
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/metaphys.h"

/*
 * Liam Nov 3, 1996
 * 
 * First time I've written a special for a shopkeeper...
 * This is probably all wrong.
 *
 * Digger Dec 7, 1996
 *
 * That boy did such a bang up job I'm gonna use his healer as
 * the basis for the Metaphys. Thanks Li ... you're the best man.
 * *wipe tear from eye*sniff*
 *
 * Mortius 16-May-2000
 *
 * Added in race Max stat checks.  Any race could meta upto 18
 *
 */

static CharData *meta;
#define BOOST_CHA(ch) ch->real_abils.cha
#define BOOST_CON(ch) ch->real_abils.con
#define BOOST_DEX(ch) ch->real_abils.dex
#define BOOST_INT(ch) ch->real_abils.intel
#define BOOST_STR(ch) ch->real_abils.str
#define BOOST_ADD(ch) ch->real_abils.str_add
#define BOOST_WIS(ch) ch->real_abils.wis

int reroll(CharData *ch, int choice);

int freedom( CharData *ch,
             ObjData  *obj,
             int    metaCondition,
             char  *metaImage,
             char  *metaEffect )
{
    if(( obj = get_obj_in_list_vis( ch, "sustenance", ch->carrying )) == NULL ){
        sendChar( ch, "You must have a ring of sustenance to have this performed.\r\n" );
        return( FALSE );
    }

    if( GET_COND( ch, metaCondition ) >= 0 ){
        extract_obj( obj );
        GET_COND( ch, metaCondition ) = -1;
        act("$n takes $p and places it on the counter.", FALSE, meta, obj, ch, TO_VICT);
        act("$e makes several arcane gestures and the lights in the room dim.", FALSE, meta, obj, ch, TO_VICT);
        sendChar( ch, metaImage );
        sendChar( ch, "There is a sudden flash as energy arcs from the counter to your forehead.\r\n" );
        sendChar( ch, "You feel momentarily disoriented and then your mind clears.\r\n" );
        sendChar( ch, metaEffect );
        return(-100); /* Negative value will not affect Exp */
    }

    else {
        sendChar( ch, "You are not affected by this condition.\r\n" );
        return( FALSE );
    }
}

int
metaHun( CharData *ch, ObjData *obj )
{
    if (IS_VAMPIRE(ch)) {
        sendChar(ch, "You must feed in order to survive, there's nothing I can do for you.\r\n");
        return(FALSE);
   } else {
    return( freedom( ch, obj, HUNGER,
                     "You detect the faint odor of meat and bread wafting about the room.\r\n",
                     "You are satiated.\r\n" ));
   }
}

int
metaThr( CharData *ch, ObjData *obj )
{
    if (IS_VAMPIRE(ch)) {
        sendChar(ch, "You must feed in order to survive, there's nothing I can do for you.\r\n");
        return(FALSE);
   } else {
    return( freedom( ch, obj, THIRST,
                     "An image of a vast emerald reservior fills your mind's eye.\r\n",
                     "You have been quenched.\r\n" ));
   }
}


/*
** Attribute adjustments.
*/

typedef struct {
    int eqId;
    int eqRate;
} infT;

void
infuse( CharData *ch,
        ObjData  *obj,
        char  *metaImage,
        char  *metaEffect )
{
    extract_obj( obj );
    act("$n takes $p and places it on the counter.", FALSE, meta, obj, ch, TO_VICT);
    act("$e makes several arcane gestures and the lights in the room dim.", FALSE, meta, obj, ch, TO_VICT);
    sendChar( ch, "%s\r\n", metaImage );
    sendChar( ch, "There is a sudden flash as energy arcs from the counter to your forehead.\r\n" );
    sendChar( ch, "You feel momentarily disoriented and then your mind clears.\r\n" );
    sendChar( ch, "%s\r\n", metaEffect );
}


int
hasEnoughExp( CharData *ch )
{
    int minXp = (titles[(int) GET_CLASS(ch)][(int)GET_LEVEL(ch)].exp);
    int xpOk  = (minXp - 10000000);

    if( GET_LEVEL(ch) >= 50 ) return(1);

    if(xpOk < 0){
        act("$n tells you, 'You may not go into negative experience for your level.'",
             FALSE, meta, 0, ch, TO_VICT);
        return(0);
    }
    else
        return(1);
}

int
notNaked( CharData *ch )
{
    int i,p;
    for( i = 0, p = 0; i < NUM_WEARS; i++ ){
        p += ( ch->equipment[i] == 0 ? 0 : 1 );
    }
    /*
    ** p should equal the number of pieces
    ** of equip the char is wearing.
    */
    if( p != 0 ){
        act("$n tells you, 'You must be naked to have this procedure done.'",
             FALSE, meta, 0, ch, TO_VICT);
    }
    return(p);
}

int
okToInfuse( CharData *ch,
            ObjData  *obj,
               infT eqList[] )
{
    int i = 0, qp;

    if( !hasEnoughExp(ch) ) return(-1);

    if( notNaked(ch) ) return(-1);

    if( obj == NULL ){
        act("$n tells you, 'I need a piece of equipment to draw power from.'",
             FALSE, meta, obj, ch, TO_VICT);
        return -1;
    }

    while( eqList[i].eqId != 0 )
        if( GET_OBJ_VNUM(obj) == eqList[i].eqId )
            break;
        else
            i++;

    /* named item is not in list */
    if (eqList[i].eqId == 0) {
        act("$n tells you, 'That piece of equipment is useless for this "
                "procedure.'", FALSE, meta, obj, ch, TO_VICT);
        return -1;
    }

    /* ensure the player has enough qp */
    qp = eqList[i].eqRate / 10;
    if (GET_QP(ch) < qp) {
        act("$n tells you, 'You need more quest points.'", FALSE, meta,
                NULL, ch, TO_VICT);
        return -1;
    }

    return i;
}

int
metaYou( CharData *ch, ObjData  *obj )
{
    int cost = GET_LEVEL(ch)*25000;
 
    if(GET_AGE(ch) < 15){
        act("$n tells you, 'I'm afraid I cannot help someone of your youth.'", FALSE, meta, obj, ch, TO_VICT);
        return 0;
    }

    if( cost > GET_GOLD(ch) ){
        act("$n tells you, 'You can't afford it!'", FALSE, meta, 0, ch, TO_VICT);
        return( TRUE );
    }

    ch->player.time.birth += (2 * SECS_PER_MUD_YEAR);
    GET_GOLD(ch) -= cost;
    sendChar( ch, "An apparition of a fountain of great magic appears before you!\r\nYou feel the power of youth coursing through your veins!\r\n" );
    
    return 0;
}

int
metaAge( CharData *ch, ObjData  *obj )
{
    int cost = GET_LEVEL(ch)*25000;

    if(GET_AGE(ch) > 99){
        act("$n tells you, 'I'm afraid I cannot help someone of your age.'", FALSE, meta, obj, ch, TO_VICT);
        return 0;
    }

    if( cost > GET_GOLD(ch) ){
        act("$n tells you, 'You can't afford it!'", FALSE, meta, 0, ch, TO_VICT);
        return( TRUE );
    }

    ch->player.time.birth -= (2 * SECS_PER_MUD_YEAR);
    GET_GOLD(ch) -= cost;
    sendChar(ch, "An apparition of a fountain of great magic appears before you!\r\nYou feel the power of youth drain from your veins!\r\n" );

    return 0;
}

int
metaCha( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        { 14709,  50 }, /* Royal Robes              */
        { 12301,  85 }, /* Golden Crown - Arilan    */
        { 20223, 100 }, /* Diamond Pinkie Ring      */
        { 0, 0 }
    };
    int max_cha = race_stat_limits[(int)GET_RACE(ch)][CHARISMA_INDEX];
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    if( BOOST_CHA(ch) >= max_cha ){
        act("$n tells you, 'You are too &10puny&00 to become &08godlike&00, go away!'",
             FALSE, meta, obj, ch, TO_VICT);
        return(0);
    }

    BOOST_CHA(ch) += 1;

    infuse( ch, obj,
            "An apparition of the Goddess Venus touches your cheek.",
            "You feel attractive!" );

    return( validEq[eqIdx].eqRate );
}


int
metaCon( CharData *ch, ObjData *obj )
{
    infT validEq[] = {
        {  2749,   85 }, /* Ring of Healing          */
        {  5524,  100 }, /* Kraken                   */
        { 14754,   75 }, /* Xerxes Bloodstone        */
        { 0, 0 }
    };
    int max_con = race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX];
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    if( BOOST_CON(ch) >= max_con ){
        act("$n tells you, 'You are too &10puny&00 to become &08godlike&00, go away!'",
             FALSE, meta, obj, ch, TO_VICT);
        return(0);
    }

    BOOST_CON(ch) += 1;

    infuse( ch, obj,
            "An image of Morgion flashes and fades in your mind's eye.",
            "You feel healthy!" );

    return( validEq[eqIdx].eqRate );
}


int
metaDex( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        {  5803,   75 }, /* Eye of the Serpent       */
        {  5508,  100 }, /* Boots of Stealth         */
        { 13802,   95 }, /* Winged Helmet of Hermes  */
        { 15255,   85 }, /* Manx Dagger              */
        { 0, 0 }
    };
    int max_dex = race_stat_limits[(int)GET_RACE(ch)][DEXTERITY_INDEX];
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    if( BOOST_DEX(ch) >= max_dex ){
        act("$n tells you, 'You are too &10puny&00 to become &08godlike&00, go away!'",
             FALSE, meta, obj, ch, TO_VICT);
        return(0);
    }

    BOOST_DEX(ch) += 1;

    infuse( ch, obj,
            "An image of Hermes, darting around your head, appears.",
            "You feel nimble!" );

    return( validEq[eqIdx].eqRate );
}


int
metaHit( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        {  5112,  100 }, /* Ruby Ring                */
        { 15242,   95 }, /* Heart of Jubilex         */
        {  3817,   85 }, /* Green Amulet             */
        { 14746,   75 }, /* Ring of Fire             */
        { 0, 0 }
    };
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    GET_MAX_HIT(ch) += 5;
    GET_HIT(ch) = GET_MAX_HIT(ch);

    infuse( ch, obj,
            "An image of Xerxes the Mad flashes and fades in your mind's eye.",
            "You feel tough!" );

    return( validEq[eqIdx].eqRate );
}


int
metaInt( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        { 14755,   75 }, /* Xerxes Sapphire          */
        { 12834,   90 }, /* Sorcerers Cap            */
        { 20226,  100 }, /* Ancient Spellbook        */
        { 0, 0 }
    };
    int max_int = race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX];
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0)  return(0);

    if( BOOST_INT(ch) >= max_int ){
        act("$n tells you, 'You are too &10puny&00 to become &08godlike&00, go away!'",
             FALSE, meta, obj, ch, TO_VICT);
        return(0);
    }

    BOOST_INT(ch) += 1;

    infuse( ch, obj,
            "An apparition of the Grandmaster nods at you in approval.",
            "You feel smart!" );

    return( validEq[eqIdx].eqRate );
}

int
metaMan( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        {  2061,  100 }, /* Diamond Brooch           */
        { 21204,   95 }, /* Platinum Dragon Claw     */
        { 13808,   85 }, /* Coral Gauntlets          */
        { 11026,   75 }, /* Ring of Energy           */
        { 0, 0 }
    };
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    GET_MAX_MANA(ch) += 5;
    GET_MANA(ch) = GET_MAX_MANA(ch);

    infuse( ch, obj,
            "An image of the Shadow Tower flashes and fades in your mind's eye.",
            "You feel raw arcane power!" );

    return( validEq[eqIdx].eqRate );
}


int
metaStr( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        { 2711,    90 }, /* Bladed Feather Sargonas  */
        { 3802,   100 }, /* Seashell Armor           */
        { 8107,   100 }, /* Solamnic Plate           */
        { 14506,   75 }, /* Belt of Skulls           */
        { 15360,  100 }, /* Spectral Gauntlets       */
        { 21269,   85 }, /* Girdle of Titan Strength */
        { 0, 0 }
    };
    int max_str = race_stat_limits[(int)GET_RACE(ch)][STRENGTH_INDEX];
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    if( BOOST_STR(ch) < max_str ){
        BOOST_STR(ch) += 1;
    }
    else if( BOOST_ADD(ch) < 100 ){
        BOOST_ADD(ch) += 10;
        if( BOOST_ADD(ch) > 100 )
            BOOST_ADD(ch) = 100;
    }
    else {
        act("$n tells you, 'You are too &10puny&00 to become &08godlike&00, go away!'",
             FALSE, meta, obj, ch, TO_VICT);
        return(0);
    }
    infuse( ch, obj,
            "An apparition of Atlas supporting the world appears before you.",
            "You feel stronger!" );

    return( validEq[eqIdx].eqRate );
}

int
metaVig( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        {  2208,  100 }, /* Bat Ring                 */
        {  2327,   95 }, /* Boots of Striding        */
        { 12023,  100 }, /* Winged Sandals           */
        { 21220,   90 }, /* Swampboots               */
        { 0, 0 }
    };
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    GET_MAX_MOVE(ch) += 5;
    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    infuse( ch, obj,
            "An image of the Strider flashes and fades in your mind's eye.",
            "You feel vigorous!" );

    return( validEq[eqIdx].eqRate );
}


int
metaWis( CharData *ch, ObjData  *obj )
{
    infT validEq[] = {
        { 4316,    85 }, /* Arthur's Crown           */
        { 5510,   100 }, /* Tattered Cloak           */
        { 13811,   95 }, /* Ring of Wisdom           */
        { 15368,  100 }, /* Captains Jacket          */
        { 0, 0 }
    };
    int max_wis = race_stat_limits[(int)GET_RACE(ch)][WISDOM_INDEX];
    int eqIdx = okToInfuse( ch, obj, validEq );

    if( eqIdx < 0 ) return(0);

    if( BOOST_WIS(ch) >= max_wis ){
        act("$n tells you, 'You are too &10puny&00 to become &08godlike&00, go away!'",
             FALSE, meta, obj, ch, TO_VICT);
        return(0);
    }

    BOOST_WIS(ch) += 1;

    infuse( ch, obj,
            "An apparition of the Dali Lama smiles upon you.",
            "You feel wise!" );

    return( validEq[eqIdx].eqRate );
}

#define CHOICE_HIT 1
#define CHOICE_MANA 2
#define CHOICE_MOVE 3

int reroll(CharData *ch, int choice) {

    if(choice != CHOICE_HIT && choice != CHOICE_MANA && choice != CHOICE_MOVE)
    {
        mudlog( NRM, LVL_IMMORT, TRUE, "ERROR!  Bad choice fed to reroll function!  Aborting!\r\n");
        return -1;
    }

    short add_hp   = 0;
    short hp_bonus = 0;
    short add_mana = 0;
    short mana_bonus = 0;
    short add_move = 0;
    short move_bonus = 0;

        if (IS_REMORT(ch)) {
        switch (GET_CLASS(ch)) {
            case CLASS_WARRIOR:
                add_hp += 100;
                break;
            case CLASS_RANGER:
                add_hp += 70;
                add_mana += 10;
                add_move += 20;
                break;
            case CLASS_SOLAMNIC_KNIGHT:
            case CLASS_DEATH_KNIGHT:
                add_hp += 70;
                add_mana += 30;
                break;
            case CLASS_ASSASSIN:
            case CLASS_THIEF:
                add_hp += 60;
                add_move += 40;
                break;
            case CLASS_SHADOW_DANCER:
            case CLASS_SHOU_LIN:
                add_hp += 60;
                add_mana += 30;
                add_move += 10;
                break;
            case CLASS_MAGIC_USER:
            case CLASS_NECROMANCER:
                add_hp += 50;
                add_mana += 50;
                break;
            case CLASS_CLERIC:
                add_hp += 40;
                add_mana += 60;
                break;
        }
    }

        int i = 0;
    for(i = 0; i < 50; i++) {
        add_hp += con_app[(int)race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX]].hitp;
        hp_bonus += 2*con_app[(int)race_stat_limits[(int)GET_RACE(ch)][CONSTITUTION_INDEX]].extra;
        add_mana += int_app[(int)race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX]].manap;
        mana_bonus += 2*int_app[(int)race_stat_limits[(int)GET_RACE(ch)][INTELLIGENCE_INDEX]].extra;

        switch (GET_CLASS(ch)) {

            case CLASS_MAGIC_USER:
                add_hp  += 6;
                add_mana += 6;
                mana_bonus += 3;
                add_move += 2;
                break;

            case CLASS_CLERIC:
                add_hp  += 7;
                hp_bonus += 3;
                add_mana += 5;
                add_move += 2;
                break;

            case CLASS_THIEF:
                add_hp  += 10;
                add_mana = 0;
                mana_bonus = 0;
                add_move += 2;
                move_bonus += 3;
                break;

            case CLASS_WARRIOR:
                add_hp  += 13;
                add_mana = 0;
                mana_bonus = 0;
                add_move += 2;
                move_bonus += 3;
                break;

            case CLASS_RANGER:
                add_hp  += 12;
                hp_bonus += 3;
                add_mana += 2;
                add_move += 3;
                break;

            case CLASS_ASSASSIN:
                add_hp  += 10;
                add_mana = 0;
                mana_bonus = 0;
                add_move += 2;
                move_bonus += 3;
                break;

            case CLASS_SHOU_LIN:
                add_hp  += 10;
                add_mana += 2;
                add_move += 2;
                break;

            case CLASS_SOLAMNIC_KNIGHT:
            case CLASS_DEATH_KNIGHT:
                add_hp  += 12;
                hp_bonus += 3;
                add_mana += 2;
                mana_bonus += 3;
                add_move += 2;
                break;

            case CLASS_SHADOW_DANCER:
                add_hp  += 10;
                add_mana += 3;
                mana_bonus += 3;
                add_move += 2;
                break;

            case CLASS_NECROMANCER:
                add_hp  += 6;
                add_mana += 6;
                add_move += 2;
                break;

        }

        // drow get a little more mana : Sanji
        if(IS_DROW(ch)) add_mana += number(1,2);
    }

     // Add initial values
    add_hp += 10;
    add_mana += 100;
    add_move += 60;

    if(choice == CHOICE_HIT)
        return add_hp + hp_bonus/6;
    else if (choice == CHOICE_MOVE)
        return add_move + move_bonus/6;
    else if (choice == CHOICE_MANA)
        return add_mana + mana_bonus/6;

    mudlog( NRM, LVL_IMMORT, TRUE, "ERROR!  Choice no longer an available option at the end of the reroll helper function!\r\n");
        return -1;
}

int metaInformation(CharData *ch, ObjData  *obj) {
    if(GET_LEVEL(ch) < 50) {
        act("$n tells you, 'Come back when you have risen to your full potential!'", FALSE, meta, obj, ch, TO_VICT);
        return(FALSE);
    }

    if(notNaked(ch))
        return(0);

    act("$n touches your temples, drawing insight.  You feel informed!", FALSE, meta, obj, ch, TO_VICT);
    sendChar(ch, "Hit: %d, Power: %d, Vigor: %d.\r\n", reroll(ch, CHOICE_HIT), reroll(ch, CHOICE_MANA), reroll(ch, CHOICE_MOVE));

    return 100;
}

int metaReroll( CharData *ch, ObjData  *obj ) {

    if(GET_LEVEL(ch) < 50) {
        act("$n tells you, 'Come back when you have risen to your full potential!'", FALSE, meta, obj, ch, TO_VICT);
        return(FALSE);
    }

    if(notNaked(ch))
        return(0);

    sprintf(buf, "%s (%s, %s)rerolls. \r\nCurrent stats are %d hit, %d mana, %d move.",
        GET_NAME(ch), CLASS_ABBR(ch), RACE_ABBR(ch), GET_MAX_HIT(ch), GET_MAX_MANA(ch), GET_MAX_MOVE(ch));
    general_log(buf, "misc/rerolls");

    ch->points.max_hit  = reroll(ch, CHOICE_HIT);
    ch->points.max_move = reroll(ch, CHOICE_MOVE);
    ch->points.max_mana = reroll(ch, CHOICE_MANA);

    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    save_char(ch, NOWHERE);
    
    sprintf(buf, "New stats are %d hit, %d mana, %d move.\r\n",
        GET_MAX_HIT(ch), GET_MAX_MANA(ch), GET_MAX_MOVE(ch));
    general_log(buf, "misc/rerolls");

    act("You fall into a stupor.", FALSE, meta, obj, ch, TO_VICT);
    add_affect(ch, ch, SKILL_POULTICE, 0, 0, 0, 2 TICKS, AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);

    return 100;
}


/*
** Yea, yea, it's sort of backwards from the standard configuration
** but I'm tired of all of those stinkin function prototypes so I
** moved the control array down here. So sue me. Anyway, ever since
** I started writing everything in C++ in RL I've grown to dislike
** the switch/case/case/case ad nauseum style when I write code. This
** method may not be any better but *I* like it so pffffbbbtttttttt :p
*/
typedef struct {
  int  (*metaFunc)(CharData *, ObjData *);
  char  *name;             /* name the attr is purchased under     */
  int   cost;              /* cost PER LEVEL of purchaser          */
} metaEntry;

metaEntry metaActs[] = {
  { metaHun, "sate",   8},
  { metaThr, "soak",   8},
  { metaCon, "con",   10},
  { metaDex, "dex",   10},
  { metaStr, "str",   10},
  { metaInt, "int",   10},
  { metaWis, "wis",   10},
  { metaCha, "cha",   10},
  { metaYou, "youth",  0},
  { metaAge, "age",    0},
  { metaInformation, "information", 1},
  { metaReroll, "reroll", 1},
  { NULL,    "",       0}
};

SPECIAL( metaphysician ) {
    meta = me;

    if( !ch->desc || IS_NPC(ch) ) return FALSE;

    if( !CMD_IS("list" ) &&
        !CMD_IS("buy"  ) &&
        !CMD_IS("terms") ) return FALSE;

    if( CMD_IS( "list" )){
        act( "$n shows you a menu, 'I offer the following services:'", FALSE, meta, 0, ch, TO_VICT);
        sendChar(ch,
"+---------------+----------------------------------------+-----------------+\r\n"
"|   Service     |               Description              |       Rates     |\r\n"
"|               |                                        | Gold | Exp | QP |\r\n"
"| sate, soak    | Releases the recipient from their      |      |     |    |\r\n"
"|               |   mortal bond to hunger or thirst.     |   8  |  0  |  0 |\r\n"
"|               |                                        |      |     |    |\r\n"
"| cha, con, dex | Increases base character attribute     |      |     |    |\r\n"
"| int, str, wis |   by 1.                                |  10  | 10  | 10 |\r\n"
"|               |                                        |      |     |    |\r\n"
"| information   | Informs character of effects should    |      |     |    |\r\n"
"|               |   he or she reroll                     |   1  |  0  |  1 |\r\n"
"|               |                                        |      |     |    |\r\n"
"| reroll        | Gives the mortal the exact average     |      |     |    |\r\n"
"|               |   rolls for hitpoints, power, and      |   1  |  0  |  1 |\r\n"
"|               |   vigor.                               |      |     |    |\r\n"
"|               |                                        |      |     |    |\r\n"
"| youth         | Reduces the recipient's age by 2 years.|   0  |1.25*|  0 |\r\n"
"| age           | Adds two years to the recipient's age. |   0  |1.25*|  0 |\r\n"
"+---------------+----------------------------------------+------+-----+----+\r\n"
"| Make certain you read the '&08terms&00' of doing business in this shop         |\r\n"
"| before initiating a transaction. Also, try '&09help meta&00' for a brief       |\r\n"
"| explanation of how things work around here.                              |\r\n"
"+---------------+----------------------------------------+------+-----+----+\r\n"
"| * Price reduced for characters below level fifty.                        |\r\n"
"+--------------------------------------------------------------------------+\r\n");
        sendChar( ch, "\r\n" );
        return( TRUE );
    }

    if( CMD_IS( "terms" )){
        act( "$n shows you a contract that reads:", FALSE, meta, 0, ch, TO_VICT);
        sendChar( ch, "----------------------------------------------------------\r\n" );
        sendChar( ch, "* * *NO REFUNDS* * * ALL  SALES FINAL * * *NO REFUNDS* * *\r\n" );
        sendChar( ch, "----------------------------------------------------------\r\n" );
        sendChar( ch, "    When you purchase  an item from  this shop you will be\r\n" );
        sendChar( ch, "charged  the going  rate for the desired  item.  My prices\r\n" );
        sendChar( ch, "fluctuate  based on  the mood the gods are in  when I open\r\n" );
        sendChar( ch, "up  for business  everyday. If my  prices vary,  and trust\r\n" );
        sendChar( ch, "me, they will, and you feel the need to  hassle me for the\r\n" );
        sendChar( ch, "difference I  will be forced to contact the  High Assassin\r\n" );
        sendChar( ch, "to put  a contract on your life for being an insolent fool\r\n" );
        sendChar( ch, "who needs to be taught a lesson.                          \r\n" );
        sendChar( ch, "                                                          \r\n" );
        sendChar( ch, "    By purchasing  an item you AGREE to  the FULL TERMS of\r\n" );
        sendChar( ch, "this contract which means you  will not be suprised when I\r\n" );
        sendChar( ch, "deal with you in a severe fashion. I *DO* hope  I'm making\r\n" );
        sendChar( ch, "myself perfectly clear. Have a pleasant day. Drive through\r\n" );
        sendChar( ch, "please.                                                   \r\n" );
        sendChar( ch, "----------------------------------------------------------\r\n" );
        return( TRUE );
    }

    if( CMD_IS( "buy" )){
        metaEntry *ptr  = metaActs;
        int        rate = 0;
        struct     obj_data *metaObj;
        char metaCmd[80], objName[80];

        argument = one_argument( argument, metaCmd );
        /*
        ** First, let's locate a valid meta entry based on what the
        ** customer is attempting to buy.
        */
        while( ptr->metaFunc != NULL && strcmp(ptr->name, metaCmd )){
            ptr++;
        }

        if(ptr->metaFunc == NULL) {
            act("$n tells you, 'I have no idea what you are talking about.'", FALSE, meta, 0, ch, TO_VICT);
            return( TRUE );
        }

        if(GET_LEVEL(ch) < 30 && (ptr->metaFunc != metaYou && ptr->metaFunc != metaAge)) {
            sendChar( ch, "You must be at least level 30 to purchase this.\r\n" );
            return( TRUE );
        }

        if( ptr->cost > GET_GOLD(ch) ){
            act("$n tells you, 'You can't afford it!'", FALSE, meta, 0, ch, TO_VICT);
            return( TRUE );
        }

        one_argument( argument, objName );
        if( objName != NULL )
            metaObj = get_obj_in_list_vis( ch, objName, ch->carrying );

        // Remove all spell affects from player before beginning.
        act("$n touches your nose.  All your spells and effects are instantly removed.", FALSE, meta, 0, ch, TO_VICT);
        while (ch->affected)
            affect_remove(ch, ch->affected, TRUE);

        if(( rate = ptr->metaFunc( ch, metaObj ))){
            int  charged = ptr->cost*1000000 * (abs(rate)/100.0);
            mudlog( NRM, LVL_IMMORT, TRUE, "METAPHYS: %s paid %d for %s",
                               GET_NAME(ch), charged, metaCmd );
            GET_GOLD(ch) -= charged;
            if(rate > 0) GET_EXP(ch)  -= charged;
            if(rate > 0) GET_QP(ch) -= ptr->cost * rate / 100.0;
            save_char( ch, NOWHERE );
        }
        return( TRUE );
    }
    else
        return( FALSE );
}

