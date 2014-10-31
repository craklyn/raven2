
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"

int numeric_arg(char *arg, int *result, int min, int max)
{
    long val;
    char *end;

    val = strtol(arg, &end, 0);
    if (end == arg || *end != '\0') {
        printf("'%s' is not a number!\n", arg);
        return 0;
    }

    if (val < min || val > max) {
        printf("'%ld' is out of range %d - %d\n", val, min, max);
        return 0;
    }

    *result = val;
    return 1;
}

int set_level(CharFileU *data, char *arg, int opt)
{
    int level;

    if (!numeric_arg(arg, &level, 1, 60)) return 0;
    data->level = level;
    return 1;
}

int set_clan(CharFileU *data, char *arg, int opt)
{
    int clan;

    if (!numeric_arg(arg, &clan, 0, 50)) return 0;
    data->player_specials_saved.clan_id = clan;
    return 1;
}

int set_clanlvl(CharFileU *data, char *arg, int opt)
{
    int level;

    if (!numeric_arg(arg, &level, 0, 6)) return 0;
    data->player_specials_saved.clan_rank = level;
    return 1;
}

int set_arenarank(CharFileU *data, char *arg, int opt)
{
    int level;

    if (!numeric_arg(arg, &level, -999, 1000)) return 0;
    data->player_specials_saved.arenarank = level;
    return 1;
}

#define MISC 0
#define BINARY 1
#define NUMBER 2

typedef int (*set_func)(CharFileU *data, char *arg, int opt);

struct set_struct {
    char *cmd;
    char type;
    set_func func;
    int opt;
} fields[] = {
    { "level",     NUMBER, set_level,     0 },
    { "clan",      NUMBER, set_clan,      0 },
    { "clanlvl",   NUMBER, set_clanlvl,   0 },
    { "arenarank", NUMBER, set_arenarank, 0 },
    { NULL, 0, NULL, 0 },
};

void forall(set_func func, char *arg, int opt)
{
    FILE *fl;
    CharFileU player;

    if (!(fl = fopen("players", "r+"))) {
        perror("Can't open 'players'");
        return;
    }

    for (;;) {
        fread( &player, sizeof(player), 1, fl );

        if (feof(fl)) {
            printf("Done\n");
            fclose(fl);
            return;
        }

        if (func(&player, arg, opt)) {
            fseek(fl, -sizeof(player), SEEK_CUR);
            fwrite(&player, sizeof(player), 1, fl);
        }
    }
}

void enact (char *playername, set_func func, char *arg, int opt)
{
    FILE *fl;
    CharFileU player;

    if (!(fl = fopen("players", "r+"))) {
        perror("Can't open 'players'");
        return;
    }

    for (;;) {
        fread( &player, sizeof(player), 1, fl );

        if (feof(fl)) {
            printf("Player not found\n");
            fclose(fl);
            return;
        }

        if (strcasecmp(playername, player.name) == 0) {
            if (func(&player, arg, opt)) {
                fseek(fl, -sizeof(player), SEEK_CUR);
                fwrite(&player, sizeof(player), 1, fl);
            }
            fclose(fl);
            return;
	}
    }

    fclose(fl);
}

int main( int argc, char **argv )
{
    int i;
    CharFileU *entry;

    if (argc != 4) {
        printf("Usage: setpfile <who> <what> <value>\n\n");
        printf("Sets a value in the playerfile for the given player.\n");
        printf("Legal values to set are:\n");
    }

    for (i = 0; fields[i].cmd; i++) {
        if (strcmp(argv[2], fields[i].cmd) == 0) {
            if (strcmp(argv[1], "-all") == 0) {
                forall(fields[i].func, argv[3], fields[i].opt);
            } else {
                enact(argv[1], fields[i].func, argv[3], fields[i].opt);
            }
            exit(0);
        }
    }

}
