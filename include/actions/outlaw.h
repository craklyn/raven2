#ifndef __OUTLAW_H__
#define __OUTLAW_H__

// ============================================================================ 
// Functions to respond to player actions.
// ============================================================================ 
void player_attack_victim( CharData *ch, CharData *vict);
void player_steal_victim( CharData *ch, CharData *victim);
void player_loot_corpse( CharData *ch, ObjData *corpse );

// ============================================================================ 
// Functions to set an "outlaw" type flag on a player.
// It's preferable to call one of the others(see above), if appropriate.
// ============================================================================ 
void set_killer_player( CharData *ch, const int duration);
void set_thief_player( CharData *ch, const int duration );
void set_hunted_player( CharData *ch, const int duration );

// ============================================================================ 
// functions to remove an "outlaw" type flag on a player.
// ============================================================================ 
void unset_killer_player( CharData *ch);
void unset_thief_player( CharData *ch);
void unset_hunted_player( CharData *ch);

// ============================================================================ 
// Constants controlling duration of flags(in ticks)
// ============================================================================ 
#define KILLER_TIME 125
#define THIEF_TIME  20
#define HUNTED_TIME 15

#endif /* __OUTLAW_H__ */
