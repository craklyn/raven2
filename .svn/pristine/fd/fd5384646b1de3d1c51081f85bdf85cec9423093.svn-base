
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"

int main(void)
{
  FILE *pfile = fopen("etc/players", "rb");
  int sum1 = 0, sum25 = 0, sum40 = 0, sum45 = 0;
  int cnt1 = 0, cnt25 = 0, cnt40 = 0, cnt45 = 0;

  if (!pfile) {
    fprintf(stderr, "Can't open player file!\n");
    exit(3);
  }

  while (!feof(pfile)) {
    struct char_file_u ch;

    if (fread(&ch, sizeof(ch), 1, pfile) != 1) break;
    printf("%s has %d quest points.\n", ch.name,
        ch.player_specials_saved.quest_pts);
    if (ch.level > 50) continue;
    if (ch.level >= 1) sum1 += ch.player_specials_saved.quest_pts, cnt1++;
    if (ch.level >= 25) sum25 += ch.player_specials_saved.quest_pts, cnt25++;
    if (ch.level >= 40) sum40 += ch.player_specials_saved.quest_pts, cnt40++;
    if (ch.level >= 45) sum45 += ch.player_specials_saved.quest_pts, cnt45++;
  }
  fclose(pfile);
  if (cnt1 > 0)
      printf("Average qps for players >= level 1: %d\n", sum1/cnt1);
  if (cnt25 > 0)
      printf("Average qps for players >= level 25: %d\n", sum25/cnt25);
  if (cnt40 > 0)
      printf("Average qps for players >= level 40: %d\n", sum40/cnt40);
  if (cnt45 > 0)
      printf("Average qps for players >= level 45: %d\n", sum45/cnt45);
  return 0;
}
