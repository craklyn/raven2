
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"

int main(void)
{
  FILE *pfile = fopen("etc/players", "r+b");

  if (!pfile) {
    fprintf(stderr, "Can't open player file!\n");
    exit(3);
  }

  while (!feof(pfile)) {
    struct char_file_u ch;

    if (fread(&ch, sizeof(ch), 1, pfile) != 1) break;
    if (ch.player_specials_saved.clan_id == 15) {
      printf("%s on channel #%d.\n", ch.name,
          ch.player_specials_saved.clan_id);
      ch.player_specials_saved.clan_id = 0;
      ch.player_specials_saved.clan_rank = 0;
      fseek(pfile, -sizeof(ch), SEEK_CUR);
      fwrite(&ch, sizeof(ch), 1, pfile);
    }
  }
  fclose(pfile);
  return 0;
}
