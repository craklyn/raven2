/* ************************************************************************
 *  File: weather.c                                      Part of CircleMUD *
 *  Functions that handle the in game progress of time and weather changes.*                       *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 *  Documentation Update ported from tbamud 3.58 on 03.07.09               *
 ************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/weather.h"

/* local file functions for weather.c */

static void another_hour(int mode);
static int current_sunrise(void);
static int current_sunset(void);

/** Call this function every mud hour to increment the gametime (by one hour)
 * and the weather patterns.
 * @param mode Really, this parameter has the effect of a boolean. In the
 * current incarnation of the function and utility functions, as long as mode
 * is non-zero, the gametime will increment one hour and the weather will be
 * changed.
 */
void weather_and_time(int mode) {
    another_hour(mode);
    if (mode)
        weather_change();
}

/** Increment the game time by one hour (no matter what) and display any time
 * dependent messages via send_to_outdoors() (if parameter is non-zero).
 * @param mode Really, this parameter has the effect of a boolean. If non-zero,
 * display day/night messages to all eligible players.
 */
static void another_hour(int mode)
{
    time_info.hours++;
    /*ho humm Weather messages by Isgrimner. Modified by Sanji*/
    if (mode) {
        if (time_info.hours == current_sunrise()) {
            weather_info.sunlight = SUN_RISE;
            send_to_outdoor("&03The large, burning sun rises in the sky.&00\r\n");
        } else if (time_info.hours == (current_sunrise() + 1)) {
            weather_info.sunlight = SUN_LIGHT;
            send_to_outdoor("&03The morn has begun.&00\r\n");
        } else if (time_info.hours == current_sunset()) {
            weather_info.sunlight = SUN_SET;
            send_to_outdoor("&10The sun slowly sinks into the west.&00\r\n");
        } else if (time_info.hours == (current_sunset() + 1)) {
            weather_info.sunlight = SUN_DARK;
            send_to_outdoor("&13The land is cloaked in darkness.&00\r\n");
        } else if (time_info.hours < current_sunset() &&
                time_info.hours > current_sunrise()) weather_info.sunlight = SUN_LIGHT;
        else weather_info.sunlight = SUN_DARK;
    }
    if (time_info.hours > 23) { /* Changed by HHS due to bug ??? */
        time_info.hours -= 24;
        time_info.day++;

        if (time_info.day > 34) {
            time_info.day = 0;
            time_info.month++;

            if (time_info.month > 15) {
                time_info.month = 0;
                time_info.year++;
            }
        }
    }
}

/** Controls the in game weather system. If the weather changes, an information
 * update is sent via send_to_outdoors().
 * @todo There are some hard coded values that could be extracted to make
 * customizing the weather patterns easier.
 */
void weather_change(void)
{
    int diff, change;
    if ((time_info.month >= 9) && (time_info.month <= 15))
        diff = (weather_info.pressure > 985 ? -2 : 2);
    else
        diff = (weather_info.pressure > 1015 ? -2 : 2);

    weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

    weather_info.change = MIN(weather_info.change, 12);
    weather_info.change = MAX(weather_info.change, -12);

    weather_info.pressure += weather_info.change;

    weather_info.pressure = MIN(weather_info.pressure, 1040);
    weather_info.pressure = MAX(weather_info.pressure, 960);

    change = 0;

    switch (weather_info.sky) {
        case SKY_CLOUDLESS:
            if (weather_info.pressure < 990)
                change = 1;
            else if (weather_info.pressure < 1010)
                if (dice(1, 4) == 1)
                    change = 1;
            break;
        case SKY_CLOUDY:
            if (weather_info.pressure < 970)
                change = 2;
            else if (weather_info.pressure < 990) {
                if (dice(1, 4) == 1)
                    change = 2;
                else
                    change = 0;
            } else if (weather_info.pressure > 1030)
                if (dice(1, 4) == 1)
                    change = 3;

            break;
        case SKY_RAINING:
            if (weather_info.pressure < 970) {
                if (dice(1, 4) == 1)
                    change = 4;
                else
                    change = 0;
            } else if (weather_info.pressure > 1030)
                change = 5;
            else if (weather_info.pressure > 1010)
                if (dice(1, 4) == 1)
                    change = 5;

            break;
        case SKY_LIGHTNING:
            if (weather_info.pressure > 1010)
                change = 6;
            else if (weather_info.pressure > 990)
                if (dice(1, 4) == 1)
                    change = 6;

            break;
        default:
            change = 0;
            weather_info.sky = SKY_CLOUDLESS;
            break;
    }

    switch (change) {
        case 0:
            break;
        case 1:
            send_to_outdoor("&06Clouds begin to blanket the sky.&00\r\n");
            weather_info.sky = SKY_CLOUDY;
            break;
        case 2:
            send_to_outdoor("&11Rain begins to fall in sheets.&00\r\n");
            weather_info.sky = SKY_RAINING;
            break;
        case 3:
            send_to_outdoor("&03The sun begins to break through the clouds.&00\r\n");
            weather_info.sky = SKY_CLOUDLESS;
            break;
        case 4:
            send_to_outdoor("&10Lightning races across the sky.&00\r\n");
            weather_info.sky = SKY_LIGHTNING;
            break;
        case 5:
            send_to_outdoor("&06The rain finally lets up.&00\r\n");
            weather_info.sky = SKY_CLOUDY;
            break;
        case 6:
            send_to_outdoor("&11The lightning storm has finally ended.&00\r\n");
            weather_info.sky = SKY_RAINING;
            break;
        default:
            break;
    }
}
// Sets MUD time and weather information in game loop, called in db.c.

void reset_time(void) {
    long beginning_of_time = 650315000;
    struct time_info_data mud_time_passed(time_t t2, time_t t1);

    time_info = mud_time_passed(time(0), beginning_of_time);

    if (time_info.hours <= current_sunrise())
        weather_info.sunlight = SUN_DARK;
    else if (time_info.hours == current_sunrise())
        weather_info.sunlight = SUN_RISE;
    else if (time_info.hours <= current_sunset())
        weather_info.sunlight = SUN_LIGHT;
    else if (time_info.hours == current_sunset())
        weather_info.sunlight = SUN_SET;
    else
        weather_info.sunlight = SUN_DARK;

    mlog("   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
            time_info.day, time_info.month, time_info.year);

    weather_info.pressure = 960;
    if ((time_info.month >= 7) && (time_info.month <= 12))
        weather_info.pressure += dice(1, 50);
    else
        weather_info.pressure += dice(1, 80);

    weather_info.change = 0;

    if (weather_info.pressure <= 980)
        weather_info.sky = SKY_LIGHTNING;
    else if (weather_info.pressure <= 1000)
        weather_info.sky = SKY_RAINING;
    else if (weather_info.pressure <= 1020)
        weather_info.sky = SKY_CLOUDY;
    else
        weather_info.sky = SKY_CLOUDLESS;
}

// this is a sorta sine wave light thingy.  Modify at your risk.

static int current_sunrise(void) {
    if ((time_info.month == 0) || (time_info.month == 15)) return 8;
    else if ((time_info.month == 1) || (time_info.month == 2)) return 7;
    else if ((time_info.month == 3) || (time_info.month == 4)) return 6;
    else if ((time_info.month == 5) || (time_info.month == 6)) return 5;
    else if ((time_info.month == 7) || (time_info.month == 8)) return 4;
    else if ((time_info.month == 9) || (time_info.month == 10)) return 5;
    else if ((time_info.month == 11) || (time_info.month == 12)) return 6;
    else if ((time_info.month == 13) || (time_info.month == 14)) return 7;
    else return 0;
}

static int current_sunset(void) {
    if ((time_info.month == 0) || (time_info.month == 1)) return 17;
    else if ((time_info.month == 2) || (time_info.month == 3)) return 18;
    else if ((time_info.month == 4) || (time_info.month == 5)) return 19;
    else if ((time_info.month == 6) || (time_info.month == 7)) return 20;
    else if ((time_info.month == 8) || (time_info.month == 9)) return 19;
    else if ((time_info.month == 10) || (time_info.month == 11)) return 18;
    else if ((time_info.month == 12) || (time_info.month == 13)) return 17;
    else if ((time_info.month == 14) || (time_info.month == 15)) return 16;
    else return 1;
}

