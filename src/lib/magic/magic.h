#ifndef __MAGIC_H__
#define __MAGIC_H__
/* ============================================================================ 
magic.h
Header file for magic functions. Written by Vex of RavenMUD for RavenMUD.
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
int   magic_savingthrow( CharData *caster, CharData *victim, int type );
int   affect_type_is_a_protection(int type);
void  project_sound(CharData *ch, char *from_dir_sound, char *other_sound);
int   debuff_protections (CharData *ch, CharData * victim);
void  affect_update (void);

#define REMOVE_WEAK_PROTECTION(ch) \
	affect_from_char( (ch), SPELL_ARMOR ); 

#define REMOVE_STRONG_PROTECTION(ch) \
	affect_from_char( (ch), SPELL_AWAKEN);           \
        affect_from_char( (ch), SPELL_BLUR ); 		 \
	affect_from_char( (ch), SPELL_HOLY_ARMOR );	 \
	affect_from_char( (ch), SPELL_BARKSKIN );	 \
	affect_from_char( (ch), SPELL_ARMOR_OF_CHAOS );

#define EXIT_IF_STRONGLY_PROTECTED(ch) \
	if ( affected_by_spell( (ch), SPELL_HOLY_ARMOR ) || \
		affected_by_spell( (ch), SPELL_BLUR ) ||   \
		affected_by_spell( (ch), SPELL_ARMOR_OF_CHAOS ) || \
		affected_by_spell( (ch), SPELL_BARKSKIN) || \
                affected_by_spell( (ch), SPELL_AWAKEN)) \
{send_to_char("You are already protected by a strong protection spell.\r\n", ch); return;}

#define TIMER_GATE      0
#define TIMER_ELEMENTAL 1
#define TIMER_MONSTER   2
#define TIMER_WILD      0
#define TIMER_ANIMATE   0
#define TIMER_MAXTIMER  3



#endif
