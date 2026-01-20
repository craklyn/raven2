
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"
#include "specials/mail.h"

void usage(void)
{
    printf("Usage: mailfile <action>\n\n");
    printf("Where <action> is one of:\n");
    printf("  l[ist]\n");
    printf("    Show all mail currently in the system\n");
    printf("  p[ackages]\n");
    printf("    Show all packages currently in the system\n");
    printf("  s[ender]     <item #> <newfrom>\n");
    printf("    Change who sent a mail item\n");
    printf("  r[ecipient]  <item #> <newto>\n");
    printf("    Change who will receive a mail item\n");
    printf("  d[elete]     <item #>\n");
    printf("    Remove a mail item from the system\n");
    printf("  b[lat]\n");
    printf("    Blat out all non-zero rename slots from packages\n");
    exit(3);
}

PlayerIndexElement *player_table;
int top_of_p_table;

void index_players(void)
{
    struct char_file_u dummy;
    FILE *player_fl;
    long size, recs;
    int nr = -1, i;

    if (!(player_fl = fopen(SYS_PLRFILES, "r+b")))
    {
        printf("Can't open player file %s\n", SYS_PLRFILES);
        return;
    }
    fseek(player_fl, 0L, SEEK_END);
    size = ftell(player_fl);
    rewind(player_fl);
    recs = size / sizeof (struct char_file_u);
    printf("%lu players in database\n", recs);
    CREATE(player_table, struct player_index_element, recs);

    for (; !feof(player_fl);)
    {
        fread(&dummy, sizeof (struct char_file_u), 1, player_fl);
        if (!feof(player_fl))
        { /* new record */
            nr++;
            CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
            for (i = 0;
                    (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
            player_table[nr].id = dummy.char_specials_saved.idnum;
        }
    }

    top_of_p_table = nr;

    fclose(player_fl);
}

/*char *get_name_by_id(long id)
{ 
    int i;

    for (i = 0; i <= top_of_p_table; i++)
        if ((player_table + i)->id == id)
            return ((player_table + i)->name);

    return NULL;
}

long get_id_by_name(char *name)
{
    int i; 

    for (i = 0; i <= top_of_p_table; i++)
        if (!strcmp((player_table + i)->name, name))
            return ((player_table + i)->id);

    return -1;
}*/

void list_mail(int all)
{
    header_block_type block;
    int total = 0, i = 0;
    ObjFileElem store;
    FILE *mail_file;

    if (!(mail_file = fopen(MAIL_FILE, "r+")))
    {
        printf("Mail file '%s' not found.\n", MAIL_FILE);
        return;
    }

    while (fread(&block, sizeof (header_block_type), 1, mail_file))
    {
        switch (block.block_type)
        {
        case HEADER_BLOCK:
            if (all)
            {
                printf("#%03d: Mail from %s to %s\n",
                       i, get_name_by_id(block.header_data.from),
                       get_name_by_id(block.header_data.to));
                total++;
            }
            break;
        case PARCEL_BLOCK:
            memcpy(&store, block.txt, sizeof (store));
            printf("#%03d: Package from %s to %s, item #%d\n",
                   i, get_name_by_id(block.header_data.from),
                   get_name_by_id(block.header_data.to),
                   store.item_number);
            total++;
            break;
        }
        i++;
    }

    printf("%d items listed.\n", total);

    fclose(mail_file);
}

void blat_blat(void)
{
    header_block_type block;
    ObjFileElem store;
    FILE *mail_file;

    if (!(mail_file = fopen(MAIL_FILE, "r+b")))
    {
        printf("Mail file '%s' not found.\n", MAIL_FILE);
        return;
    }

    while (fread(&block, sizeof (header_block_type), 1, mail_file))
    {
        if (block.block_type == PARCEL_BLOCK)
        {
            memcpy(&store, block.txt, sizeof (store));
            if (store.rename_slot != 0)
            {
                printf("Package from %s to %s, item #%d, non-zero slot\n",
                       get_name_by_id(block.header_data.from),
                       get_name_by_id(block.header_data.to),
                       store.item_number);
                store.rename_slot = 0;
                memcpy(block.txt, &store, sizeof (store));
                fseek(mail_file, -sizeof (header_block_type), SEEK_CUR);
                fwrite(&block, sizeof (header_block_type), 1, mail_file);
            }
        }
    }

    fclose(mail_file);
}

int change_person(int mail, char *from, int sender)
{
    long id = get_id_by_name(from);
    header_block_type block;
    FILE *mail_file;

    if (sender == -1)
    {
        printf("Sender '%s' is unknown.\n", from);
        return (0);
    }

    if (!(mail_file = fopen(MAIL_FILE, "r+")))
    {
        printf("Mail file '%s' not found.\n", MAIL_FILE);
        return (0);
    }

    fseek(mail_file, mail * BLOCK_SIZE, SEEK_SET);
    if (ftell(mail_file) != mail * BLOCK_SIZE)
    {
        printf("Index is out of range.\n");
    }
    else
    {
        fread(&block, sizeof (block), 1, mail_file);
        if (block.block_type != HEADER_BLOCK &&
                block.block_type != PARCEL_BLOCK)
        {
            printf("That block is neither a header nor a parcel.\n");
        }
        else
        {
            if (sender)
            {
                printf("Original sender: %s\n",
                       get_name_by_id(block.header_data.from));
                printf("New sender: %s\n", from);
                block.header_data.from = id;
            }
            else
            {
                printf("Original recipient: %s\n",
                       get_name_by_id(block.header_data.to));
                printf("New recipient: %s\n", from);
                block.header_data.to = id;
            }
            fseek(mail_file, mail * BLOCK_SIZE, SEEK_SET);
            fwrite(&block, sizeof (block), 1, mail_file);
        }
    }

    fclose(mail_file);
    return (0);
}

int main(int argc, char **argv)
{
    char *endptr;
    int num;

    if (argc < 2 || argc > 4) usage();
    index_players();
    switch (*argv[1])
    {
    case 'l': // list all mails
        list_mail(1);
        break;
    case 'p': // list packages
        list_mail(0);
        break;
    case 's': // change sender
        if (argc != 4) usage();
        num = strtol(argv[2], &endptr, 0);
        if (*endptr != '\0') usage();
        change_person(num, argv[3], 1);
        break;
    case 'r': // change recipeint
        if (argc != 4) usage();
        num = strtol(argv[2], &endptr, 0);
        if (*endptr != '\0') usage();
        change_person(num, argv[3], 0);
        break;
    case 'd': // delete mail
        //if (argc != 3) usage();
        //num = strtol(argv[2], &endptr, 0);
        //if (*endptr != '\0') usage();
        //delete_mail(num);
        break;
    case 'b':
        blat_blat();
        break;
    default:
        usage();
    }
    return 0;
}

