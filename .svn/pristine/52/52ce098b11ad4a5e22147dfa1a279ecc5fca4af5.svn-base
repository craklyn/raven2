#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "actions/act.clan.h"
#include "specials/seek.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/house.h"
#include "general/color.h"
#include "olc/olc.h"
#include "actions/fight.h"
#include "util/weather.h"
#include "olc/oedit.h"
#include "olc/qedit.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/combspec.h"
#include "specials/special.h"
#include "actions/outlaw.h"
#include "actions/quest.h"
#include "scripts/dg_scripts.h"
#include "specials/flag_game.h"
#include "magic/sing.h"
#include "util/utils.h"
#include "specials/muckle.h"

CharData *the_muckle = NULL;
int muckle_duration;
int muckle_pvpFactor;

void muckle_report();
char *muckle_scoreboard();
void muckle_over();
void muckle_sort();

char *muckle_scoreboard() {
    CharData *i;

    sprintf(buf, "&09 Muckle Scoreboard:&00\r\n\r\n");
    sprintf(buf, "%s Game length: &14%d:00 &00\r\n\r\n", buf, muckle_duration);
    sprintf(buf, "%s&14 Name           &07|&14 Time &07|&14 Muckle &00\r\n", buf);
    sprintf(buf, "%s&07----------------+------+--------&00\r\n", buf);

    for(i = character_list; i; i = i->next) {
        if(!IS_NPC(i) && i->muckleTime)
            sprintf(buf, "%s%15s &07|&14 %d:%02d &07|&14 %s&00\r\n", buf, GET_NAME(i),
                    i->muckleTime/60, i->muckleTime%60, i == the_muckle? " &08* ": "");
    }

    return buf;
}

void muckle_over(int awardQP) {
    CharData *i;
    int reward = 0;

    for(i = character_list; i; i = i->next) {
        sendChar(i, "The quest is over.\r\n");
        REMOVE_BIT_AR(PRF_FLAGS(i), PRF_QUEST);

        if(!awardQP)
            break;

        reward = i->muckleTime/60 + (i->muckleTime % 60 > 0 ? 1:0);
        if(i->muckleTime == muckle_duration * 60)
            reward += 2;

        if(!IS_NPC(i) && reward) {
            sendChar(i, "You have been awarded %d quest points for your effort.\r\n", reward);
            GET_QP(i) += reward;
            mudlog(BRF, LVL_IMMORT, TRUE, "%s has received %d quest points for %s efforts in muckle.",
                    GET_NAME(i), reward, HSHR(i));
        }
        
        i->muckleTime = 0;
    }

    CONFIG_QUEST_ACTIVE = 0;
}

void muckle_sort() {
    // TODO: Sort the muckle list for better displaying
}

void muckle_update() {
    if(!muckle_active)
        return;

    if(!CONFIG_QUEST_ACTIVE) {
        send_to_all("&12The game of muckle has ended.&00\r\n");
        quest_echo(muckle_scoreboard());
        muckle_over(FALSE);
        muckle_active = FALSE;
        the_muckle = NULL;
        muckle_pvpFactor = 1;
    }

    if(the_muckle) {
        if(IN_ARENA(the_muckle)) {
            (the_muckle->muckleTime)++;
            muckle_sort();
            if(!(the_muckle->muckleTime % 60))
                quest_echo(muckle_scoreboard()); 
            if(!(the_muckle->muckleTime % (60 * muckle_duration))) {
                sprintf(buf, "\r\n&14Congratulations to %s, the winner at muckle!&00\r\n", GET_NAME(the_muckle));
                quest_echo(buf);
                muckle_over(TRUE);
            }
        }
        else {
            sprintf(buf, "&08MUCKLE:&00 There is currently no muckle!&00\r\n");
            quest_echo(buf);
            the_muckle = NULL;
        }
    }

    
}
