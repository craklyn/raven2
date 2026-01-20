
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"

int list_chores(CharFileU *data)
{
    int i;
    for (i = 0; i < MAX_CHORES; i++) {
        int z = data->player_specials_saved.chores[i];
        printf("Chore #%d\n", z);
    }
    return 1;
}

int toggle_task(CharFileU *data, int task)
{
    data->player_specials_saved.chores[task] =
            -data->player_specials_saved.chores[task];
    return 1;
}

void set_task(char *playername, int task)
{
    FILE *fl;
    CharFileU player;

    if (!(fl = fopen(SYS_PLRFILES, "r+"))) {
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
            if (player.level == 60) {
                printf("No.\n");
                fclose(fl);
                return;
            }
            toggle_task(&player, task);
            fseek(fl, -sizeof(player), SEEK_CUR);
            fwrite(&player, sizeof(player), 1, fl);
            fclose(fl);
            list_chores(&player);
            return;
	}
    }

    fclose(fl);
}

int main( int argc, char **argv )
{
    if (argc != 3) {
        printf("Usage: goals <who> <which>\n\n");
        printf("Toggles the completed status of a task.\n");
    } else {
        set_task(argv[1], atoi(argv[2]));
    }

    return (0);
}
