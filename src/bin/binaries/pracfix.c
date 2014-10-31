
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "magic/spells.h"
#include "util/utils.h"

void usage(void)
{
    printf("Usage: pracfix <action>\n\n");
    printf("Where <action> is one of:\n");
    printf("  a[ssassins]\n");
    printf("    Convert blindness/paralyze and trap skills to 5 pracs\n");
    printf("  n[ecromancers]\n");
    printf("    Refund necromancers with the spell 'slow' 5 pracs\n");
    exit(3);
}

const int die_skills[] = {
    SPELL_BLINDNESS,
    SPELL_PARALYZE,
    SKILL_AMBUSH,
    SKILL_TRAP,
    SKILL_SEARCH_TRAP,
    -1
};

void fix_assassin(struct char_file_u *ch)
{
    int i;

    for (i = 0; die_skills[i] != -1; i++) {
	int now = ch->player_specials_saved.skills[die_skills[i]];
	if (now > 50) now = 50;
	now /= 10;
	ch->player_specials_saved.spells_to_learn += now;
	ch->player_specials_saved.skills[die_skills[i]] = 0;
    }
}

void fix_necromancer(struct char_file_u *ch)
{
    int now = ch->player_specials_saved.skills[SPELL_SLOW];
    if (now > 50) now = 50; now /= 10;
    ch->player_specials_saved.spells_to_learn += now;
    ch->player_specials_saved.skills[SPELL_SLOW] = 0;
}

void necromancer_hack(void)
{
    struct char_file_u ch;
    FILE *f = fopen("etc/players", "r+b");

    if (!f) {
	fprintf(stderr, "Can't open file etc/players\n");
	return;
    }

    while (!feof(f)) {
	fread(&ch, sizeof(ch), 1, f);
	if (ch.class == CLASS_NECROMANCER) {
	    printf("Aha!  %s is a necromancer!  Fixing them up.\n", ch.name);
	    fix_necromancer(&ch);
	    fseek(f, -sizeof(ch), SEEK_CUR);
	    fwrite(&ch, sizeof(ch), 1, f);
	}
    }

    fclose(f);
}

void assassin_hack(void)
{
    struct char_file_u ch;
    FILE *f = fopen("etc/players", "r+b");

    if (!f) {
	fprintf(stderr, "Can't open file etc/players\n");
	return;
    }

    while (!feof(f)) {
	fread(&ch, sizeof(ch), 1, f);
	if (ch.class == CLASS_ASSASSIN) {
	    printf("Aha!  %s is an assassin!  Fixing them up.\n", ch.name);
	    fix_assassin(&ch);
	    fseek(f, -sizeof(ch), SEEK_CUR);
	    fwrite(&ch, sizeof(ch), 1, f);
	}
    }

    fclose(f);
}

int main(int argc, char **argv)
{
    char *endptr;
    int vnum;

    if (argc < 2 || argc > 3) usage();
    switch (*argv[1]) {
	case 'a':
	    assassin_hack();
	    break;
        case 'n':
            necromancer_hack();
            break;
	default:
	    usage();
    }
    return 0;
}

