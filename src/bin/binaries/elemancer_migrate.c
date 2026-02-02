/* ************************************************************************
 *  file:  elemancer_migrate.c                          Part of RavenMUD   *
 *  Usage: One-time migration to apply ITEM_ANTI_ELEMANCER to items that   *
 *         are already ITEM_ANTI_THIEF in player/locker files.             *
 ************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "../../lib/general/conf.h"
#include "../../lib/general/sysdep.h"
#include "../../lib/general/db.h"
#include "../../lib/general/structs.h"

// Redefine macros if missing to ensure standalone compilation
#ifndef IS_SET_AR
#define IS_SET_AR(var, bit)  ((var)[(bit) / 32] & (1 << ((bit) % 32)))
#endif
#ifndef SET_BIT_AR
#define SET_BIT_AR(var, bit) ((var)[(bit) / 32] |= (1 << ((bit) % 32)))
#endif

#ifndef ITEM_ANTI_THIEF
#define ITEM_ANTI_THIEF 14
#endif
#ifndef ITEM_ANTI_ELEMANCER
#define ITEM_ANTI_ELEMANCER 58
#endif

// Function to update a single item structure
int update_item(ObjFileElem *item) {
    // Rule: If Anti-Thief, make Anti-Elemancer
    if (IS_SET_AR(item->extra_flags, ITEM_ANTI_THIEF)) {
        if (!IS_SET_AR(item->extra_flags, ITEM_ANTI_ELEMANCER)) {
            SET_BIT_AR(item->extra_flags, ITEM_ANTI_ELEMANCER);
            return 1;
        }
    }
    return 0;
}

void process_list(glob_t *files, long offset) {
    int i;
    FILE *fl;
    ObjFileElem item;
    long pos;
    int count;

    for (i = 0; i < files->gl_pathc; i++) {
        fl = fopen(files->gl_pathv[i], "r+");
        if (!fl) continue;

        if (offset > 0)
            fseek(fl, offset, SEEK_SET);
            
        count = 0;

        while (1) {
            pos = ftell(fl);
            if (fread(&item, sizeof(ObjFileElem), 1, fl) != 1) break;

            if (update_item(&item)) {
                // Rewind to start of item
                fseek(fl, pos, SEEK_SET);
                // Write modified item
                fwrite(&item, sizeof(ObjFileElem), 1, fl);
                // Position is now at start of next item, correct for loop
                count++;
            }
        }
        
        if (count > 0) printf("Updated %d items in %s\n", count, files->gl_pathv[i]);
        fclose(fl);
    }
}

int main(int argc, char **argv) {
    glob_t files;

    printf("Starting Elemancer Item Migration...\n");

    // Search paths adapted for running from root or src/bin
    
    // Process Player Objects
    if (glob("sys/plrobjs/*/*.objs", 0, NULL, &files) == 0) {
        process_list(&files, sizeof(struct rent_info));
        globfree(&files);
    } else if (glob("../../../sys/plrobjs/*/*.objs", 0, NULL, &files) == 0) {
        process_list(&files, sizeof(struct rent_info));
        globfree(&files);
    } else {
        printf("No player objects found (checked sys/plrobjs and ../../../sys/plrobjs).\n");
    }

    // Process Lockers
    if (glob("sys/lockers/*.objs", 0, NULL, &files) == 0) {
        process_list(&files, 0);
        globfree(&files);
    } else if (glob("../../../sys/lockers/*.objs", 0, NULL, &files) == 0) {
        process_list(&files, 0);
        globfree(&files);
    } else {
        printf("No locker objects found (checked sys/lockers and ../../../sys/lockers).\n");
    }

    printf("Migration complete.\n");
    return 0;
}
