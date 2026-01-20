/* ************************************************************************
*  File: chores.c                                       Part of  RavenMUD *
*  Author: Imhotep of RavenMUD						  *
*  Usage: Random tasks for characters to achieve                          *
*                                                                         *
*  RavenMUD is derived from CircleMUD, so the CircleMUD license applies.  *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "general/class.h"        /* for IS_AFFECTED see note in class.c */

#define CHORE_VISIT     0
#define CHORE_KILL      1
#define CHORE_QUEST     2

typedef struct player_chore {
    unsigned char type;
    int vnum;
} PlayerChore;

static PlayerChore chores[] = {
	
    {CHORE_QUEST, 18004}, //Mayor:Kill barb warrior
    {CHORE_QUEST, 18008}, //Mayor:Kill Rendor
    {CHORE_QUEST, 18016}, //Mayor:Kill quickling
    {CHORE_QUEST, 18019}, //Mayor:Kill Macsen
    {CHORE_QUEST, 18020}, //Mayor:Kill Monkey 
    {CHORE_QUEST, 18026}, //Mayor:Kill Dervel 
    {CHORE_QUEST, 18029}, //Mayor:Kill Kuo-toan Priestess
    {CHORE_QUEST, 18034}, //Mayor:Kill Madam Zash
    {CHORE_QUEST, 18039}, //Mayor:kill follower mad
    {CHORE_QUEST, 55103}, //Primrose:yellow dress
    {CHORE_QUEST, 55104}, //Faerie:ogre feather
    {CHORE_QUEST,  7401}, //Ivan:Cheese stick
    {CHORE_QUEST,  7405}, //Ivan: wood elf
    {CHORE_QUEST,  7408}, //Ivan:whale boots
    {CHORE_QUEST,  7409}, //Ivan:Picnic Lunch
    {CHORE_QUEST,  7412}, //Jarl:Sword
    {CHORE_QUEST,  7414}, //Alchem:Lizard tongue
    {CHORE_QUEST,  7421}, //Rhyvis:slab of beef
    {CHORE_QUEST,  7422}, //Rhyvis:Seaweed
    {CHORE_QUEST,  7425}, //Rhyvis:Leafy Green
    {CHORE_QUEST, 27124}, //wizen: Kill a medusa
    {CHORE_QUEST, 27129}, //wizen: Return the shield of ancient harmony.
    {CHORE_QUEST, 27118}, //wizen: Kill Ssirak and return the tome.
    {CHORE_QUEST, 27105}, //wizen: Slay Flidalis and return her armor and head.
    {CHORE_QUEST, 27122}, //wizen: Kill Evergreen and get ring of autumn leaves
    {CHORE_QUEST, 27133}, //wizen: Kill a Tempest in Soth's Playground.
    {CHORE_QUEST, 27111}, //wizen: Slay a shady character and return his ranseur dagger.
    {CHORE_QUEST, 27113}, //wizen: Slay the minotaur god and return his claws.
    {CHORE_QUEST, 27132}, //wizen: Kill the Horrifying ghoul on the Ebony queen.
    {CHORE_QUEST, 27110}, //wizen: Slay the mother phoenix and return the egg.
    {CHORE_QUEST, 18154}, //Ruby: DragonQ:Grand Bronze Dragon
    {CHORE_QUEST, 18153}, //Ruby: DragonQ:Great Dragon Wyvn
    {CHORE_QUEST, 18156}, //elite royal guardsman: Green Dragon
    {CHORE_KILL,    903}, //River Island of Minos, Forest Minotaur
    {CHORE_KILL,   1108}, //Thieves' Den, leader of the thieves
    {CHORE_KILL,   1319}, //Three of Swords, Gabadiel
    {CHORE_KILL,   1414}, //In the Air, Kitten 
    {CHORE_KILL,   1506}, //Kingdom of Juargan, Grimdale
    {CHORE_KILL,   1504}, //Kingdom of Juargan, Juargan
    {CHORE_KILL,   1724}, //Land of Fire Newts, lumbering Neanderthal
    {CHORE_KILL,   1806}, //Haven, snow queen
    {CHORE_KILL,   1928}, //Garamond, guardian 
    {CHORE_KILL,   2003}, //Draconia, powerful mage 
    {CHORE_KILL,   2215}, //Gnomes, fire bat
    {CHORE_KILL,   2311}, //Mahn-Tor, Ogre Chief
    {CHORE_KILL,   2323}, //Mahn-Tor, Minotaur Butler
    {CHORE_KILL,   2329}, //Mahn-Tor, Sumaron
    {CHORE_KILL,   2417}, //Shire, dwarven prince
    {CHORE_KILL,   2513}, //Dragonia, golden dragon
    {CHORE_KILL,   2525}, //Dragonia, Bishop dragon
    {CHORE_KILL,   2614}, //elven valley, antelope
    {CHORE_KILL,   2813}, //Asgard, Battle Master 
    {CHORE_KILL,   2825}, //Asgard, Goddess Freya
    {CHORE_KILL,   3143}, //S Midgaard, Baron of Midgaard
    {CHORE_KILL,   3315}, //N Midgaard, Terwin
    {CHORE_KILL,   3414}, //Tunnel Maze, mad etcher  
    {CHORE_KILL,   3804}, //Isle of Tuatha, Mananan mac Lir
    {CHORE_KILL,   3903}, //Mystic Sea, Obsidian Guardian
    {CHORE_KILL,   4101}, //Moria, troll 
    {CHORE_KILL,   4317}, //castle of swords, snap-dragon
    {CHORE_KILL,   4310}, //castle of swords, third knight of sword
    {CHORE_KILL,   4660}, //Underworld, human rogue
    {CHORE_KILL,   5010}, //Eastern Desert, dracolich
    {CHORE_KILL,   5109}, //City of Drow, yochlol
    {CHORE_KILL,   5210}, //City of Thalos, Behern
    {CHORE_KILL,   5307}, //Enfans, guard dog
    {CHORE_KILL,   5511}, //Swamp of thanos, Queen of Widows
    {CHORE_KILL,   5528}, //Swamp of Thanos, Lich of Thanos
    {CHORE_KILL,   5813}, //Pyramid of Serp, Jailor
    {CHORE_KILL,   5906}, //lair of Aspis, aspis cow
    {CHORE_KILL,   6115}, //Haondor(dark), Shargugh
    {CHORE_KILL,   6248}, //Isle of Pirates, Alindra
    {CHORE_KILL,   6231}, //Isle of Pirates, Mizzenmast
    {CHORE_KILL,   6217}, //Isle of Pirates, Smallbody
    {CHORE_KILL,   6315}, //Arachnos, Ki-Rin
    {CHORE_KILL,   6502}, //Dwarven Caverns, mirror image
    {CHORE_KILL,   6604}, //Temple of time, Second Priest
    {CHORE_KILL,   7748}, //Cloudy Mountain, Bazarnog
    {CHORE_KILL,   7754}, //Cloudy Mountain, Commander Gauzrah
    {CHORE_KILL,   7710}, //Cloudy Mountain, winged dragon
    {CHORE_KILL,   7901}, //Ebony Queen, Captain Gravic 
    {CHORE_KILL,   8006}, //Soths, Warden 
    {CHORE_KILL,   8117}, //Solamnic Knights, Great Silver Wyrm
    {CHORE_KILL,   8202}, //Silver Lady, Zeigan
    {CHORE_KILL,   8301}, //Xanth, Gap Dragon
    {CHORE_KILL,   8427}, //Myth Tyr, pile of bones
    {CHORE_KILL,   8401}, //Myth Tyr, Garckin
    {CHORE_KILL,   8706}, //Urkiish Und, Magistrate
    {CHORE_KILL,   9110}, //Volcano, fiend 
    {CHORE_KILL,   9601}, //Drekken Noir, Midnight Guard
    {CHORE_KILL,   9712}, //Valley of Dawn, Emperor 
    {CHORE_KILL,   9822}, //Hamlet of Dusk, Kennel Owner
    {CHORE_KILL,  10029}, //LupusVille (Day), bellringer
    {CHORE_KILL,  10115}, //LupusVille (Night), Cat Lord
    {CHORE_KILL,  11207}, //Valley Mad Mage, Troll Mage
    {CHORE_KILL,  11506}, //Shaden, Baron
    {CHORE_KILL,  12108}, //Irridenta, mama Griffon 
    {CHORE_KILL,  12308}, //Duke Arilan, Jeeves 
    {CHORE_KILL,  12501}, //Freehold Thieves, Honorable Man
    {CHORE_KILL,  12609}, //Blackhand, witch 
    {CHORE_KILL,  12719}, //Michaeline Keep, Sephriana 
    {CHORE_KILL,  12805}, //Dragonor Keep, Flower 
    {CHORE_KILL,  13049}, //HighTower, Necromancer
    {CHORE_KILL,  13280}, //Astral Ext, githyanki warlock
    {CHORE_KILL,  13529}, //New Thalos, Aziz
    {CHORE_KILL,  13806}, //Olympus, Ares
    {CHORE_KILL,  13916}, //Warcamp, Baltac
    {CHORE_KILL,  14002}, //Trial of the Initiate, Ludwig
    {CHORE_KILL,  15106}, //Dark Friends, Guild Clerk 
    {CHORE_KILL,  15257}, //Dreamworld, Medusa 
    {CHORE_KILL,  15323}, //Lord Vrolock, Carnifern
    {CHORE_KILL,  15336}, //Lord Vrolock, Lizard Man
    {CHORE_KILL,  15421}, //BlackMarket, L'orin
    {CHORE_KILL,  15604}, //Tombs o Tarin, Silver Dragon
    {CHORE_KILL,  15722}, //Legend, yellowjacket
    {CHORE_KILL,  16103}, //Isle of Myst, lightening
    {CHORE_KILL,  16215}, //Isle of Myst, shrieker 
    {CHORE_KILL,  17015}, //Deep Moria, mine leader 
    {CHORE_KILL,  17207}, //Farm, Scarecrow
	{CHORE_KILL,  17417}, //Orchard, faerie warrior
	{CHORE_KILL,  18335}, //Knights of Krynn, Vampiress
	{CHORE_KILL,  18398}, //Knights of Krynn, Dark Daughter
	{CHORE_KILL,  18804}, //Dwarven Village, Daphne
	{CHORE_KILL,  18902}, //Lost Temple, Statue 
	{CHORE_KILL,  19029}, //Emerald Forest, marmalade cat
	{CHORE_KILL,  19109}, //Elemental Canyon, Fire Ruler
	{CHORE_KILL,  19203}, //Rats Lair, Sammy
	{CHORE_KILL,  20216}, //Blackheart Keep, Karik
	{CHORE_KILL,  20507}, //Kenku Village, Kenku King
	{CHORE_KILL,  20605}, //Under Dark, Huge Beholder
	{CHORE_KILL,  20672}, //Under Dark, Shillikif
	{CHORE_KILL,  20699}, //Under Dark, Ass Guildmaster
	{CHORE_KILL,  20710}, //Under Dark, Lord Rygar
	{CHORE_KILL,  21223}, //Forgotten City, Balor
	{CHORE_KILL,  21324}, //Forgotten Forest, Ariel
	{CHORE_KILL,  21521}, //UnderDark Ext, Vampire Mage
	{CHORE_KILL,  21840}, //Siege on Malfor's, Black Demon
	{CHORE_KILL,  22502}, //Portals of time, Keeper Fire
	{CHORE_KILL,  22932}, //Freeport, Lord Mourning
	{CHORE_KILL,  23002}, //Perceptory, Caritas
	{CHORE_KILL,  23418}, //New Camelot, Mordred
	{CHORE_KILL,  23445}, //New Camelot, Saxon Chief
	{CHORE_KILL,  23500}, //Ocean, Kraken 
	{CHORE_KILL,  23662}, //Dead Man's, Keclutrac
	{CHORE_KILL,  23805}, //Plains Shierre, peacock 
	{CHORE_KILL,  24033}, //Les Foret, Chopper 
	{CHORE_KILL,  24716}, //Usurpers, Badwick
	{CHORE_KILL,  27122}, //Meth-Mertul, Terelat Officer
	{CHORE_KILL,  27201}, //Land of Jermlaine, Jerm Brat
	{CHORE_KILL,  27600}, //Everwar, Unger Troll 
	{CHORE_KILL,  30408}, //Fairhaven, Warden 
	{CHORE_KILL,  30806}, //Fair Sewers, Crayfish 
        {CHORE_KILL,  31027}, //DT Dungeon, elven thief
	{CHORE_KILL,  31307}, //Rabbit Hole, Mother rabbit
	{CHORE_KILL,  31811}, //Blight, Aes Sedai 
	{CHORE_KILL,  32306}, //Village Turra, Gnome leader
	{CHORE_KILL,  32416}, //Dungeons Denegorth, Monk Amal
	{CHORE_KILL,  32605}, //Forest Turra, elven chief
	{CHORE_KILL,  33006}, //River Dangers, tiny alligator
	{CHORE_KILL,  34007}, //Xak Tsaroth, Phudge
	{CHORE_KILL,  34110}, //Crystal Palace, Kallist
	{CHORE_KILL,  39730}, //Atlantis, Leviathan
	{CHORE_KILL,  39887}, //drakyri Isle, frail man
	{CHORE_KILL,  40122}, //Steeding, Carn ape 
	{CHORE_KILL,  43406}, //SKG, lich 
};

#define CHORE_COUNT (sizeof(chores)/sizeof(PlayerChore))

#define CHORE(ch, i) (ch->player_specials->saved.chores[i])

/* assign a random set of chores to a player */
void chore_initialise(CharData *ch)
{
    int i, j, c;

    for (i = 0; i < MAX_CHORES; i++) {
        do {
            c = number(1, CHORE_COUNT) - 1;
            for (j = 0; j < i; j++) if (CHORE(ch, j) == c) c = -1;
        } while (c == -1);
        CHORE(ch, i) = c;
    }
    ch->player_specials->saved.chore_count = 0;
}

static void check_one_kill(CharData *ch, int vnum)
{
    int i;

    /* mobs need not apply */
    if (IS_NPC(ch)) return;

    if(!PRF_FLAGGED(ch, PRF_BRIEF))
        sendChar(ch, "&05This kill counts toward status.&00\r\n");

    for (i = 0; i < MAX_CHORES; i++) {
        if (CHORE(ch, i) >= 0 && chores[CHORE(ch, i)].type == CHORE_KILL &&
                chores[CHORE(ch, i)].vnum == vnum) {
            CHORE(ch, i) = -CHORE(ch, i);
        }
    }
}

void chore_check_kill(CharData *ch, int vnum)
{
    FollowType *fol;

    // If ch is null, bail
    if (!ch)
        return;

    /* make sure the char gets it */
    check_one_kill(ch, vnum);

    /* if not grouped, go away */
    // This code is disabled because mobiles can be unofficially
    // grouped but contribute...
    //if (!IS_AFFECTED(ch, AFF_GROUP)) return;

    /* For every grouped follower in the same room, check it also */
    for (fol = ch->followers; fol; fol = fol->next) {
        if (IS_AFFECTED(fol->follower, AFF_GROUP) &&
                fol->follower->in_room == ch->in_room)
            check_one_kill(fol->follower, vnum);
    }

        /* If the char has a master in the same room, switch to them */
    if(ch->master && ch->master->in_room == ch->in_room) {
        ch = ch->master;
        chore_check_kill(ch, vnum);
    }
    else if(ch->master)
        ch = ch->master;

}

void chore_check_quest(CharData *ch, int vnum)
{
    int i;

    /* A completed chore is a negative one */
    for (i = 0; i < MAX_CHORES; i++) {
        // Somewhere along the line, we started letting chore 0 be allowed.  This
        // makes it impossible to mark the chore as completed (-0).  To work
        // around this, we're just going to change it to chore = 1.
        if(CHORE(ch, i) == 0) {
            mudlog(NRM, LVL_IMPL, TRUE, "Fixing player %s's chores to be completable!", GET_NAME(ch));
            CHORE(ch, i) = 8;
        }

        if (CHORE(ch, i) >= 0 && chores[CHORE(ch, i)].type == CHORE_QUEST &&
                chores[CHORE(ch, i)].vnum == vnum) {
            CHORE(ch, i) = -CHORE(ch, i);
        }
    }
}

ACMD(do_showchores)
{
    CharData *victim;
    int i, c;
    one_argument(argument, buf);

    if (!*buf) {
        sendChar(ch, "Show whose goals?\r\n");
        return;
    }

    if ((victim = get_char_vis(ch, buf, 1)) == NULL) {
        sendChar(ch, "Who is that?\r\n");
        return;
    }

    if (IS_NPC(victim)) {
        sendChar(ch, "Goal #1: Become a real person.\r\n");
        return;
    }

    sendChar(ch, "Goals for %s:\r\n", GET_NAME(victim));
    for (i = 0; i < MAX_CHORES; i++) {
        c = CHORE(victim, i);
        sendChar(ch, "#%2d %c ", i, c < 0 ? '*' : ' ');
        if (c < 0) c = -c;
        if (c >= CHORE_COUNT) {
            sendChar(ch, "Invalid chore %d!\r\n", c);
        } else if (chores[c].type == CHORE_VISIT) {
            sendChar(ch, "Visit room #%d\r\n", chores[c].vnum);
        } else if (chores[c].type == CHORE_KILL) {
            sendChar(ch, "Kill mob #%d, ", chores[c].vnum);
            c = real_mobile(chores[c].vnum);
            if (c == -1) sendChar(ch, "&08unknown!&00\r\n");
            else sendChar(ch, "%s\r\n", mob_proto[c].player.short_descr);
        } else if (chores[c].type == CHORE_QUEST) {
            sendChar(ch, "Complete quest #%d", chores[c].vnum);
            c = real_quest(chores[c].vnum);
            if (c == -1) sendChar(ch, ", &08unknown!&00\r\n");
            else sendChar(ch, "\r\n");
        }
    }
}

#undef CHORE
#define CHORE(ch, i) (ch->player_specials_saved.chores[i])
void chore_update(CharFileU *ch)
{
    int i, c = 0;

    for (i = 0; i < MAX_CHORES; i++) {
        if (CHORE(ch, i) < 0) c++;
    }

    ch->player_specials_saved.chore_count = c;
}

/* This function type, the kind that is no longer called, with no comments
 * why it's not longer called nor removed from the source, are scattered all over
 * this file. I've grouped them and commented them out for reference purposes.
 *
// assign a random set of chores to a player
void chore_reinitialise(CharData *ch)
{
    int i, j, c;

    for (i = 0; i < MAX_CHORES; i++) {

        if( CHORE(ch, i) < 0)
            continue;

        do {
            c = number(1, CHORE_COUNT) - 1;
            for (j = 0; j < i; j++) if (CHORE(ch, j) == c) c = -1;
        } while (c == -1);
        CHORE(ch, i) = c;
    }

}
End of comment chore_reinitialize */

/* Another Chore un-re-un-Init function with no notes and no calls in the code.
void chore_pfile_reinit(CharFileU *ch)
{
    int i, j, c;

    for (i = 0; i < MAX_CHORES; i++) {
        for (j = i; j < MAX_CHORES; j++) if (CHORE(ch, j) == CHORE(ch, i)) {
        }
    }
    ch->player_specials_saved.chore_count = 0;
}
End of comment chore_pfile_reinit */

/*

//  Reset chores is a function to re-scramble everyone's chores.  
//  It was last used when Eff got a copy of the code.  It can be
//  called in the boot_db function after boot db is finished.
void reset_chores() {

int i;
int player_i;
CharData *cbuf;
struct char_file_u tmp_store;


  for (i = 0; i <= top_of_p_table; i++) {
    CREATE(cbuf, CharData, 1);
    clear_char(cbuf);
    if ((player_i = load_char( (player_table + i)->name , &tmp_store)) > -1)
      store_to_char(&tmp_store, cbuf);

    chore_reinitialise(cbuf);

    char_to_store(cbuf, &tmp_store);
    fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
    fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
    free_char(cbuf);
  }
}

End of comment reset_chores */
