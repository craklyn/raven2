/* ************************************************************************
 *  file:  dumptally.c                                   Part of RavenMUD  *
 *  Usage: Dump tally digs through the character files and writes name of  *
 *  character and tally total to a new file. This script is used in        *
 *  conjunction with the in game 'tally' command.                          *
 *                                                                         *
 ************************************************************************* */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"

typedef struct tallyrec
{
    long id;
    short c;
} tally;

int
main(int argc, char **argv)
{
    tally rec;
    FILE *f;

    build_player_index();

    if (argc != 2)
    {
        printf("Usage: %s <tallyfile>\n", argv[0]);
        exit(3);
    }

    f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Cannot open %s: %s\n", argv[1], strerror(errno));
        exit(3);
    }

    while (fread(&rec, sizeof (rec), 1, f) == 1)
    {
        printf("%4d %s\n", rec.c, get_name_by_id(rec.id));
    }

    fclose(f);
      return (0);
}
