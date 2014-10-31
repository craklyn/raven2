/* ************************************************************************
 *   File: comm.c                                        Part of CircleMUD *
 *  Usage: Communication, socket handling, main(), central game loop       *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 ************************************************************************ */

#define __COMM_C__

#include "general/conf.h"
#include "general/sysdep.h"

/* Begin conf.h dependent includes */

#if CIRCLE_GNU_LIBC_MEMORY_TRACK
#include <mcheck.h>
#endif

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#endif

/* end conf.h dependent includes */

/* Note, most includes for all platforms are in sysdep.h.  The list of
 * files that is included is controlled by conf.h for that platform. */

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "general/handler.h"
#include "specials/house.h"
#include "specials/scatter.h"
#include "specials/mail.h"
#include "general/color.h"
#include "olc/olc.h"
#include "olc/genolc.h"           /* for free_save_list */
#include "util/weather.h"
#include "scripts/dg_scripts.h"
#include "magic/fishing.h"
#include "specials/boards.h"
#include "actions/ban.h"
#include "actions/fight.h"
#include "magic/sing.h"
#include "magic/spells.h"
#include "specials/muckle.h"
#include "specials/scatter.h"     /* for scatter_pulse() */
#include "general/objsave.h"      /* for free_save_list */
#include "actions/act.h"          /* ACMDs located within the act*.c files */

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

/* externs */
extern int MAX_DESCRIPTORS_AVAILABLE;
//extern char last_command_entered[MAX_STRING_LENGTH + 100];

/* locally defined globals, used externally */
DescriptorData *descriptor_list = NULL; /* master desc list */
struct txt_block *bufpool = 0; /* pool of large output buffers */
int buf_largecount = 0; /* # of large buffers which exist */
int buf_overflows = 0; /* # of overflows of output */
int buf_switches = 0; /* # of switches from small to large buf */
int circle_shutdown = 0; /* clean shutdown */
int circle_reboot = 0; /* reboot the game after a shutdown */
int oracle_reboot = 0;
int oracle_counter = 0;
int muckle_active = 0;
int scheck = 0;        /* for syntax checking mode */
FILE *logfile = NULL;     /* Where to send the log messages. */
char reboot_reason[80] = " - Just a quicky reboot.";
int poll_running = 0;
char poll_question[255] = "";
char poll_answer1[255] = "";
char poll_answer2[255] = "";
char poll_answer3[255] = "";
char poll_answer4[255] = "";
char poll_answer5[255] = "";
int no_specials = 0; /* Suppress ass. of special routines */
int avail_descs = 0; /* max descriptors available */
int tics = 0; /* for extern checkpointing */
int dg_act_check; /* toggle for act_trigger*/
unsigned long dg_global_pulse = 0; /* number of pulses since game start */
ush_int port;
int raw_input_logging = 0;

/* functions in this file */
int get_from_q(struct txt_q * queue, char *dest, int *aliased);
static void init_game(ush_int local_port);
void signal_setup(void);
void game_loop(int mother_desc);
static socket_t init_socket(ush_int port);
static int new_descriptor(socket_t s);
int get_avail_descs(void);
int process_output(DescriptorData * t);
int process_input(DescriptorData * t);
static void free_bufpool (void);
struct timeval timediff(struct timeval * a, struct timeval * b);
void flush_queues(DescriptorData * d);
void nonblock(int s);
int perform_subst(DescriptorData * t, char *orig, char *subst);
int perform_alias(DescriptorData * d, char *orig);
void record_usage(void);
static char *make_prompt(struct descriptor_data *point);

static struct in_addr *get_bind_addr(void);
static int parse_ip(const char *addr, struct in_addr *inaddr);
static int set_sendbuf(socket_t s);
static void setup_log(const char *filename, int fd);
static int open_logfile(const char *filename, FILE *stderr_fp);

/* extern fcnts */
void boot_db(void);
void zone_update(void);
void affect_update(void); /* In spells.c */
void mobile_activity(void);
void string_add(DescriptorData * d, char *str);
void perform_violence(void);
void perform_songs(void);
void show_string(DescriptorData * d, char *input);
int isbanned(char *hostname);
void process_events(void);
void update_kill_counts(void); /* in fight.c */

/* externally defined functions, used locally */
#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif

/*  main game loop and related stuff */

/* *********************************************************************
 *  main game loop and related stuff                                    *
 ********************************************************************* */

int moon_main(int argc, char **argv)
{
    int pos = 1;
    const char *dir;

#ifdef MEMORY_DEBUG
    zmalloc_init();
#endif

#if CIRCLE_GNU_LIBC_MEMORY_TRACK
    mtrace(); /* This must come before any use of malloc(). */
#endif

    /* Load the game configuration. We must load BEFORE we use any of the
     * constants stored in constants.c.  Otherwise, there will be no variables
     * set to set the rest of the vars to, which will mean trouble --> Mythran */
  CONFIG_CONFFILE = NULL;
  while ((pos < argc) && (*(argv[pos]) == '-')) {
    if (*(argv[pos] + 1) == 'f') {
            if (*(argv[pos] + 2))
	CONFIG_CONFFILE = argv[pos] + 2;
            else if (++pos < argc)
	CONFIG_CONFFILE = argv[pos];
      else {
                puts("SYSERR: File name to read from expected after option -f.");
                exit(1);
            }
        }
        pos++;
    }
    pos = 1;

  if (!CONFIG_CONFFILE)
    CONFIG_CONFFILE = strdup(CONFIG_FILE);

  load_config();

  port = CONFIG_DFLT_PORT;
  dir = CONFIG_DFLT_DIR;

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
        case 'f':
            if (! *(argv[pos] + 2))
                ++pos;
            break;
    case 'o':
      if (*(argv[pos] + 2))
	CONFIG_LOGNAME = argv[pos] + 2;
      else if (++pos < argc)
	CONFIG_LOGNAME = argv[pos];
      else {
	puts("SYSERR: File name to log to expected after option -o.");
	exit(1);
      }
      break;
        case 'd':
            if (*(argv[pos] + 2))
                dir = argv[pos] + 2;
            else if (++pos < argc)
                dir = argv[pos];
      else {
                puts("SYSERR: Directory arg expected after option -d.");
                exit(1);
            }
            break;
        case 'm':
            mini_mud = 1;
            no_rent_check = 1;
            puts("Running in minimized mode & with no rent check.");
            break;
        case 'c':
            scheck = 1;
            puts("Syntax check mode enabled.");
            break;
        case 'q':
            no_rent_check = 1;
            puts("Quick boot mode -- rent check supressed.");
            break;
        case 'r':
            circle_restrict = 1;
            puts("Restricting game -- no new players allowed.");
            break;
        case 's':
            no_specials = 1;
            puts("Suppressing assignment of special routines.");
            break;
        case 'h':
            /* From: Anil Mahajan. Do NOT use -C, this is the copyover mode and
             * without the proper copyover.dat file, the game will go nuts! */
            printf("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n"
                   "  -c             Enable syntax check mode.\n"
                   "  -d <directory> Specify library directory (defaults to 'lib').\n"
                   "  -h             Print this command line argument help.\n"
                   "  -m             Start in mini-MUD mode.\n"
                   "  -f <file>      Use <file> for configuration.\n"
                   "  -o <file>      Write log to <file> instead of stderr.\n"
                   "  -q             Quick boot (doesn't scan rent for object limits)\n"
                   "  -r             Restrict MUD -- no new players allowed.\n"
                   "  -s             Suppress special procedure assignments.\n"
                   " Note:		These arguments are 'CaSe SeNsItIvE!!!'\n",
                   argv[0]
                   );
            exit(0);
        default:
            printf("SYSERR: Unknown option -%c in argument string.\n", *(argv[pos] + 1));
            break;
        }
        pos++;
    }

  if (pos < argc) {
    if (!isdigit(*argv[pos])) {
            printf("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n", argv[0]);
            exit(1);
    } else if ((port = atoi(argv[pos])) <= 1024) {
            printf("SYSERR: Illegal port number %d.\n", port);
            exit(1);
        }
    }

  /* All arguments have been parsed, try to open log file. */
  setup_log(CONFIG_LOGNAME, STDERR_FILENO);

  /* Moved here to distinguish command line options and to show up
   * in the log if stderr is redirected to a file. */
  mlog("Loading configuration.");
  mlog ("%s", circlemud_version);

  if (chdir(dir) < 0) {
        perror("SYSERR: Fatal error changing to data directory");
        exit(1);
    }
    mlog ("Using %s as data directory.", dir);

    mlog("Running game on port %d.", port);
    init_game(port);

     mlog ("Clearing game world.");
     destroy_db ();

  if (!scheck) {
    mlog("Clearing other memory.");
    free_bufpool (); /* comm.c */
    free_player_index (); /* db.c */
    free_messages (); /* fight.c */
    clear_free_list (); /* mail.c */
    free_text_files (); /* db.c */
    board_clear_all(); /* boards.c */
    free_help_table();      /* db.c */
    free_invalid_list(); /* ban.c */
    free_save_list();       /* genolc.c */
    free_strings(&config_info, OASIS_CFG); /* oasis_delete.c */
  }
    /* probably should free the entire config here.. */
    free(CONFIG_CONFFILE);

    mlog("Done.");

    #ifdef MEMORY_DEBUG
    zmalloc_check();
    #endif

    return (0);
}
/* Init sockets, run game, and cleanup sockets */
static void init_game(ush_int local_port)
{
    int mother_desc;

  /* We don't want to restart if we crash before we get up. */
  touch(KILLSCRIPT_FILE);

    srandom(time(0));

    mlog("Opening mother connection.");
    mother_desc = init_socket(local_port);

    avail_descs = get_avail_descs();

    boot_db();

#if defined(CIRCLE_UNIX) || defined(CIRCLE_MACINTOSH)
    mlog("Signal trapping.");
    signal_setup();
#endif

    boot_corpses();

     /* If we made it this far, we will be able to restart without problem. */
  remove(KILLSCRIPT_FILE);

    mlog("Entering game loop.");

    game_loop(mother_desc);

    Crash_save_all();
    House_save_all();

    mlog("Closing all sockets.");
    while (descriptor_list)
        close_socket(descriptor_list);

    CLOSE_SOCKET(mother_desc);
    fclose(player_fl);

    if (circle_reboot)
    {
        mlog("Rebooting.");
    exit(52);			/* what's so great about HHGTTG, anyhow? */
    }
    mlog("Normal termination of game.");
}

/* init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens. */
static socket_t init_socket(ush_int local_port)
{
  socket_t s;
    struct sockaddr_in sa;
  int opt;

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("SYSERR: Error creating socket");
    exit(1);
  }

#if defined(SO_REUSEADDR)
    opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0){
    perror("SYSERR: setsockopt REUSEADDR");
        exit(1);
    }
#endif

  set_sendbuf(s);

/* The GUSI sockets library is derived from BSD, so it defines SO_LINGER, even
 * though setsockopt() is unimplimented. (from Dean Takemori) */
#if defined(SO_LINGER)
    {
        struct linger ld;

        ld.l_onoff = 0;
        ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
      perror("SYSERR: setsockopt SO_LINGER");	/* Not fatal I suppose. */
        }
#endif

  /* Clear the structure */
  memset((char *)&sa, 0, sizeof(sa));

  sa.sin_family = AF_INET;
  sa.sin_port = htons(local_port);
  sa.sin_addr = *(get_bind_addr());

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        perror("SYSERR: bind");
        CLOSE_SOCKET(s);
        exit(1);
    }
    nonblock(s);
    listen(s, 5);
    return (s);
}

int get_avail_descs(void)
{
    int max_descs = 0;

    /*
     * First, we'll try using getrlimit/setrlimit.  This will probably work
     * on most systems.
     */
#if defined (RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#if !defined(RLIMIT_NOFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
    {
        struct rlimit limit;

        getrlimit(RLIMIT_NOFILE, &limit);
        if (limit.rlim_max == RLIM_INFINITY)
            max_descs = CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS;
        else
            max_descs = limit.rlim_max;

        limit.rlim_cur = max_descs;

        setrlimit(RLIMIT_NOFILE, &limit);
    }
#elif defined (OPEN_MAX)
    max_descs = OPEN_MAX; /* Uh oh.. rlimit didn't work, but we have
				 * OPEN_MAX */
#else
    /*
     * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
     * use the POSIX sysconf() function.  (See Stevens' _Advanced Programming
     * in the UNIX Environment_).
     */
    errno = 0;
    if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0)
    {
        if (errno == 0)
            max_descs = CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS;
        else
        {
            perror("Error calling sysconf");
            exit(1);
        }
    }
#endif

    max_descs = MIN(CONFIG_MAX_PLAYING, max_descs - NUM_RESERVED_DESCS);

    if (max_descs <= 0)
    {
        mlog("Non-positive max player limit!");
        exit(1);
    }
    mlog("Setting player limit to %d", max_descs);
    return max_descs;
}

void
oracle()
{
    if (oracle_reboot)
    {
        oracle_counter -= 1;
        switch (oracle_counter)
        {
        case -1 :
            send_to_all("------------------------------------------------------------------------\r\n");
            send_to_all("The Oracle Informs you: The reboot was &09cancelled&00 - logins are enabled.  \r\n");
            send_to_all("------------------------------------------------------------------------\r\n");
            circle_restrict = 0;
            oracle_reboot = 0;
            oracle_counter = 0;
            break;
        case 3:
            circle_restrict = LVL_CREATOR;
            send_to_all("------------------------------------------------------------------------\r\n");
            send_to_all("The Oracle Informs you: I have &11wizlocked&00 RavenMUD - logins are &08disabled.&00\r\n");
        case 60:
        case 45:
        case 30:
        case 20:
        case 15:
        case 10:
        case 5:
        case 4:
        case 2:
        case 1:
            send_to_all("------------------------------------------------------------------------\r\n");
            send_to_all( "The Oracle Informs you: RavenMUD will reboot in %d ticks.\r\n", oracle_counter);
            send_to_all( "            The Reason:\r\n%s\r\n", reboot_reason);
            send_to_all("------------------------------------------------------------------------\r\n");
            break;
        case 0:
            send_to_all("------------------------------------------------------------------------\r\n");
            send_to_all("The Oracle Informs you: RavenMUD is rebooting NOW!\r\n");
            send_to_all( "            The Reason:\r\n%s\r\n", reboot_reason);
            send_to_all("------------------------------------------------------------------------\r\n\n\n");
            circle_shutdown = 1;
            break;
        default:
            break;
        }
    }
}

void
poll_report ()
{
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  int e = 0;
  int totalvote = 0;

  DescriptorData *dsc;

  for (dsc = descriptor_list; dsc; dsc = dsc->next)
    {
      if (dsc->character && !IS_NPC (dsc->character))
        {
          if (GET_VOTE (dsc->character) == 0) continue;
          else if (GET_VOTE (dsc->character) == 1) a++;
          else if (GET_VOTE (dsc->character) == 2) b++;
          else if (GET_VOTE (dsc->character) == 3) c++;
          else if (GET_VOTE (dsc->character) == 4) d++;
          else if (GET_VOTE (dsc->character) == 5) e++;
          else
            mudlog (BRF, LVL_IMMORT, TRUE, "(POLL) In Poll_report(), %s voted %d.",
                    GET_NAME (dsc->character), GET_VOTE (dsc->character));
        }
    }

  totalvote = a + b + c + d + e;

  send_to_all ("------------------------------------------------------------------------\r\n");
  send_to_all ("The RavenMUD Pollster informs you:\r\n");
  send_to_all ("Poll results:\r\n");
  send_to_all ("------------------------------------------------------------------------\r\n");


  if (!totalvote)
    {
      send_to_all ("No votes were cast!\r\n");
      return;
    }

  if (poll_answer1[0] != '\0' && a)
    {
      send_to_all ("Option 1 received %d vote%s - %s\r\n", a, (a > 1) ? "s" : "", poll_answer1);
    }
  if (poll_answer2[0] != '\0' && b)
    {
      send_to_all ("Option 2 received %d vote%s - %s\r\n", b, (b > 1) ? "s" : "", poll_answer2);
    }
  if (poll_answer3[0] != '\0' && c)
    {
      send_to_all ("Option 3 received %d vote%s - %s\r\n", c, (c > 1) ? "s" : "", poll_answer3);
    }
  if (poll_answer4[0] != '\0' && d)
    {
      send_to_all ("Option 4 received %d vote%s - %s\r\n", d, (d > 1) ? "s" : "", poll_answer4);
    }
  if (poll_answer5[0] != '\0' && e)
    {
      send_to_all ("Option 5 received %d vote%s - %s\r\n", e, (e > 1) ? "s" : "", poll_answer5);
    }

  send_to_all ("------------------------------------------------------------------------\r\n");

}

void
poll ()
{
  if (poll_running)
  {
      poll_running -= 1;
      switch (poll_running)
      {
          case 0:
              poll_report ();
              poll_clear ();
              break;
              
          case 1:
              send_to_all ("The poll is closing soon!\r\n");
          case 3:
              if (poll_question[0] == '\0' || poll_answer1[0] == '\0')
              {
                  poll_running = 0; // Let's not panic, but this is bad!
                  return;
              }
              send_to_all ("------------------------------------------------------------------------\r\n");
              send_to_all ("The RavenMUD Pollster asks you:\r\n%s\r\n", poll_question);
              
              send_to_all ("------------------------------------------------------------------------\r\n");
              send_to_all ("1)  %s\r\n", poll_answer1);
              
              if (poll_answer2[0] == '\0')
              {
                  send_to_all ("------------------------------------------------------------------------\r\n");
                  break;
              }
              send_to_all ("\r\n2)  %s\r\n", poll_answer2);
              
              if (poll_answer3[0] == '\0')
              {
                  send_to_all ("------------------------------------------------------------------------\r\n");
                  break;
              }
              send_to_all ("\r\n3)  %s\r\n", poll_answer3);
              
              if (poll_answer4[0] == '\0')
              {
                  send_to_all ("------------------------------------------------------------------------\r\n");
                  break;
              }
              send_to_all ("\r\n4)  %s\r\n", poll_answer4);
              
              if (poll_answer5[0] == '\0')
              {
                  send_to_all ("------------------------------------------------------------------------\r\n");
                  break;
              }
              send_to_all ("\r\n5)  %s\r\n", poll_answer5);
              send_to_all ("------------------------------------------------------------------------\r\n");
          default:
              break;
      }

      if(poll_running == 1 || poll_running == 3)
           send_to_all("Cast your &09vote&00 now!\r\n");
    }
}

/*
 ** Anti-spamming code
 */
void spamming_update(void)
{
    DescriptorData *i;

    /* every five seconds you get 3 public channel usages back */
  for (i = descriptor_list; i; i = i->next) {
        i->noisy -= 3;
        if (i->noisy < 0) i->noisy = 0;
        i->spammy -= 13;
        if (i->spammy < 0) i->spammy = 0;
    }
}

/*
 ** The Main Gaming Loop
 */
void
game_loop(int mother_desc)
{
    fd_set input_set, output_set, exc_set;
    struct timeval last_time, now, timespent, timeout, null_time, opt_time;
    char comm[MAX_STRING_LENGTH];
    DescriptorData *d, *next_d;
    int pulse = 0, maxdesc, aliased;
    int opt_skew = 0;
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    opt_time.tv_usec = OPT_USEC;
    opt_time.tv_sec = 0;
    gettimeofday(&last_time, (struct timezone *) 0);

    /*
     ** Mud Main Loop
     */
    while (!circle_shutdown)
    {
      /*
      ** Sleep in select until someone connects.
      */
        if (descriptor_list == NULL)
        {
            mlog("No connections.  Going to sleep.");
            FD_ZERO(&input_set);
            FD_SET(mother_desc, &input_set);
            if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0)
            {
                if (errno == EINTR)
                    mlog("Waking up to process signal.");
                else
                    perror("game_loop: select error");
            }
            else
                mlog("New connection.  Waking up.");
            gettimeofday(&last_time, (struct timezone *) 0);
        }
        /* Set up the input, output, and exception sets for select(). */
        FD_ZERO(&input_set);
        FD_ZERO(&output_set);
        FD_ZERO(&exc_set);
        FD_SET(mother_desc, &input_set);
        maxdesc = mother_desc;
        for (d = descriptor_list; d; d = d->next)
        {
#ifndef CIRCLE_WINDOWS
            if (d->descriptor > maxdesc)
                maxdesc = d->descriptor;
#endif
            FD_SET(d->descriptor, &input_set);
            FD_SET(d->descriptor, &output_set);
            FD_SET(d->descriptor, &exc_set);
        }

        do
        {
            errno = 0;
            /*
             ** Figure out how long to sleep.
             */
            gettimeofday(&now, (struct timezone *) 0);
            timespent = timediff(&now, &last_time);
            timeout = timediff(&opt_time, &timespent);
            timeout.tv_usec -= opt_skew;
            /*
             ** Make certain the values are reasonable.
             */
            if (timeout.tv_sec > 1) timeout.tv_sec = 0;
            if (timeout.tv_usec < 0) timeout.tv_usec = 100;
            /*
             ** Sleep until the next 0.1 second mark.
             */
            if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
                if (errno != EINTR)
                {
                    perror("Select sleep");
                    exit(1);
                }
        }
        while (errno);

        /*
         ** Record the time for the next pass.
         */
        gettimeofday(&last_time, (struct timezone *) 0);
        if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0)
        {
            perror("Select poll");
            return;
        }

        /*
         ** Check for a new connection
         */
        if (FD_ISSET(mother_desc, &input_set))
            new_descriptor(mother_desc);
        /*
         ** Close the connections with pending exceptions.
         */
        for (d = descriptor_list; d; d = next_d)
        {
            next_d = d->next;
            if (FD_ISSET(d->descriptor, &exc_set))
            {
                FD_CLR(d->descriptor, &input_set);
                FD_CLR(d->descriptor, &output_set);
                close_socket(d);
            }
        }

        /* Process descriptors with input pending */
        for (d = descriptor_list; d; d = next_d)
        {
            next_d = d->next;
            if (FD_ISSET(d->descriptor, &input_set))
                if (process_input(d) < 0)
                    close_socket(d);
        }
        /*
         ** Process the commands from the clients.
         */
        for (d = descriptor_list; d; d = next_d)
        {
            next_d = d->next;

            if (d->character != NULL)
            {
                if (d->connected == CON_PLAYING)
                {
                    STUN_DEC(d->character);
                    d->wait = d->character->player.stunned;
                    FLEEING_DEC(d->character);
                }
            }



            if ((--(d->wait) <= 0) && get_from_q(&d->input, comm, &aliased))
            {
                if (d->character)
                {
                    if (d->character->offbalance > 0) --(d->character->offbalance);
                    d->character->char_specials.timer = 0;
                    if (!d->connected && GET_WAS_IN(d->character) != NOWHERE)
                    {
                        if (d->character->in_room != NOWHERE)
                            char_from_room(d->character);
                        char_to_room(d->character, GET_WAS_IN(d->character));
                        GET_WAS_IN(d->character) = NOWHERE;
                        act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
                    }
                }
                d->wait = d->prompt_mode = 1;
                /* Checks below were reversed so you can use page_string */
                /* function in the editor. */
                if (d->showstr_point) /* reading something w/ pager */
                    show_string(d, comm);
                else if (d->str) /* writing boards, mail etc. */
                    string_add(d, comm);
                else if (d->connected != CON_PLAYING)
                {
                    nanny(d, comm);
                }
                else
                {
                    if (aliased)
                        d->prompt_mode = 0;
                    else
                    {
                        if (perform_alias(d, comm))
                        {
                            get_from_q(&d->input, comm, &aliased);
                        }
                    }

                    command_interpreter(d->character, comm);
                }
            }
        }
        /*
         ** Send out the queued output.
         */
        for (d = descriptor_list; d; d = next_d)
        {
            next_d = d->next;
            if (FD_ISSET(d->descriptor, &output_set) && *(d->output))
            {
                if (process_output(d) < 0)
                {
                    mudlog(NRM, LVL_IMMORT, TRUE, "CLOSING SOCKET");
                    close_socket(d);
                }
                else
                {
                    d->prompt_mode = 1;
                }
            }
        }

        /*
         ** Close connections that are in the CON_CLOSE state.
         */
        for (d = descriptor_list; d; d = next_d)
        {
            next_d = d->next;
            if (d->connected != CON_PLAYING)
            {
                /*
                 ** Auto timeout problem connections that
                 ** tend to linger.
                 */
                if ((pulse % PASSES_PER_SEC) == 0)
                {
                    d->dcTimer -= 1;
                    if (d->dcTimer < 0)
                    {
                        /* mudlog(NRM, LVL_IMMORT, TRUE, "AutoDisconnect Idle Port %d", d->desc_num ); */
                        SET_DCPENDING(d);
                    }
                    else if (d->dcTimer == 60)
                    {
                        /* mudlog(NRM, LVL_IMMORT, TRUE, "Descriptor %d will disconnect in 60 seconds.", d->desc_num ); */
                    }
                }
            }
            if (d->dcPending) close_socket(d);
      }

        /*
         ** Send an appropriate prompt to each player.
         */
        for (d = descriptor_list; d; d = d->next)
        {
            if (d->prompt_mode)
            {
	        write_to_descriptor(d->descriptor, make_prompt(d));
                d->prompt_mode = 0;
            }
        }
        /*
         ** That takes care of multiplexing the player I/O
         ** now do the real mud stuff.
         */
        pulse++;

      if (!(pulse % PULSE_ZONE)) zone_update();

        // This particular if statement SHOULD be called on a ten second
        // update rate. For some unknown reason the Openix OS is sliding
        // this out to as much as 15 seconds. In order to compensate we're
        // going to dynamically adjust the clock skew rate.
        //                                             Digger
        //
        if (!(pulse % PULSE_MOBILE))
        {
            static int last_pulse = 0;
            if (last_pulse != 0)
            {
                int pulse_skew = (now.tv_sec - last_pulse)SEC - PULSE_MOBILE;

                if ((pulse_skew > 2) && (opt_skew < 90000))
                {
                    opt_skew += 10000;
                }
                else if ((pulse_skew < -2) && (opt_skew > -90000))
                {
                    opt_skew -= 10000;
                }
            }
            last_pulse = now.tv_sec;

            mobile_activity();
            delete_extracted();
      }

        // Call this continously so mobs can do _stuff_ _better_!  Craklyn
        mobile_combat_moves();

        // Make the calls to the violence code.
        //
        if (!(pulse % PULSE_VIOLENCE))
        {
            perform_violence();
        }
        if ((PULSE_VIOLENCE / 2) == (pulse % (PULSE_VIOLENCE))) // Between combat rounds
        {
            perform_songs();
        }

        // Update players underwater.
        //
        if (!(pulse % PULSE_WATER)) water_activity(pulse);

        // Update disaster rooms.
        //
        if (!(pulse % PULSE_DISASTER)) disaster_activity(pulse);

        // Update suffer rooms.
        //
        if (!(pulse % PULSE_SUFFER)) suffer_activity(pulse);

        // Do pulse heals.
        //
        if (!(pulse % PULSE_HEALING))
        {
            pulse_heal();
            poll();
        }

        // Run the Auction
        //
        if (!(pulse % PULSE_AUCTION)) auction_update();

        // Decrease counters for public channel spamming
        //
        if (!(pulse % PULSE_SPAMMING)) spamming_update();

        // Check for posted parcels past their expiry time
        //
        if (!(pulse % PULSE_MAIL)) mail_update();

      // Decrease counters for public channel spamming
      //
      if( !(pulse % PULSE_SPAMMING) ) spamming_update();


        if( !(pulse % PULSE_AFFECT) ) {
            affect_update();
            regen_update();
            muckle_update();
        }


        if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC)))
        {
            weather_and_time(1);
            point_update();
            oracle();
            fflush(player_fl);
            scatter_pulse();
            fishoff_ticker();
            update_kill_counts();
        }

        // Periodically save everyone to disk every 60 seconds.
        //
        if (!(pulse % (60 * PASSES_PER_SEC)))
        {
            Crash_save_all();
        }

        // Save houses and record mud usage every 300 seconds (5minutes).
        //
        if (!(pulse % (300 * PASSES_PER_SEC)))
        {
#ifdef HOUSE_ENABLED
          House_save_all();
#endif
          record_usage();
      }

        if (!(pulse % (40 * PASSES_PER_SEC)))
        {
            check_fishing();
        }

      // Process scripts
      dg_global_pulse++;

      process_events();

      if (!(pulse % PULSE_DG_SCRIPT))
            script_trigger_check();

        // Reset the pulse every 30 minutes.
        //
        if (pulse >= (30 * 60 * PASSES_PER_SEC)) pulse = 0;

      tics++;
    }

    for (d = descriptor_list; d; d = next_d)
    {
        if (FD_ISSET(d->descriptor, &output_set) && *(d->output))
            process_output(d);
    }

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout);
}

/* ******************************************************************
 *  general utility stuff (for local use)                            *
 ****************************************************************** */

struct timeval timediff(struct timeval * a, struct timeval * b)
{
    struct timeval rslt, tmp;

    tmp = *a;

    if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
    {
        rslt.tv_usec += 1000000;
        --(tmp.tv_sec);
    }

    if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
    {
        rslt.tv_usec = 0;
        rslt.tv_sec = 0;
    }
    return rslt;
}

void
record_usage(void)
{
    int sockets_connected = 0, sockets_playing = 0;
    DescriptorData *d;

    for (d = descriptor_list; d; d = d->next)
    {
        sockets_connected++;
        if (!d->connected)
            sockets_playing++;
    }

    mlog("nusage: %-3d sockets connected, %-3d sockets playing",
            sockets_connected, sockets_playing);
#ifdef RUSAGE
    {
        struct rusage ru;

        getrusage(0, &ru);
        mlog("rusage: %d %d %d %d %d %d %d",
                ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss,
                ru.ru_ixrss, ru.ru_ismrss, ru.ru_idrss, ru.ru_isrss);
    }
#endif

}

/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(DescriptorData *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  write_to_output (d, "%s", off_string);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(DescriptorData *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) TELOPT_NAOFFD,
    (char) TELOPT_NAOCRD,
    (char) 0,
  };

  write_to_output (d, "%s", on_string);
}

#define COLOR_ON(ch) (!IS_NPC(ch) ? (PRF_FLAGGED((ch), PRF_COLOR_1) || PRF_FLAGGED((ch), PRF_COLOR_2) ? 1 : 0) : 0)

static char *
make_prompt (DescriptorData * d)
{
  static char prompt[MAX_PROMPT_LENGTH];

  /* Note, prompt is truncated at MAX_PROMPT_LENGTH chars (structs.h) */

  if (d->showstr_point)
    snprintf (prompt, sizeof (prompt),
              "\r\n*** Press return to continue, q to quit ***");

  else if (d->str)
    strcpy (prompt, "] "); /* strcpy: OK (for 'MAX_PROMPT_LENGTH >= 3') */
  else if (STATE (d) == CON_PLAYING && !IS_NPC (d->character))
    {
      int count;
      size_t len = 0;

      *prompt = '\0';
      CharData *ch = d->character;
      
      len = snprintf (prompt, sizeof (prompt), CCNRM(ch, C_NRM));

      if (GET_INVIS_LEV (ch) && len < sizeof (prompt))
        {
          count = snprintf (prompt + len, sizeof (prompt) - len, "i%d ", GET_INVIS_LEV (ch));
          if (count >= 0)
            len += count;
        }

      if (PRF_FLAGGED (ch, PRF_DISPHP) && len < sizeof (prompt))
        {
          count = snprintf (prompt + len, sizeof (prompt) - len, "%s%d%sH ",
                            colorRatio (ch, COLOR_RAW, C_CMP, GET_HIT (ch), GET_MAX_HIT (ch)),
                            GET_HIT (ch), CCNRM (ch, C_NRM));
          if (count >= 0)
            len += count;
        }

      if (PRF_FLAGGED (ch, PRF_DISPMANA) && len < sizeof (prompt))
        {
          count = snprintf (prompt + len, sizeof (prompt) - len, "%s%d%sM ",
                            colorRatio (ch, COLOR_RAW, C_CMP, GET_MANA (ch), GET_MAX_MANA (ch)),
                            GET_MANA (ch), CCNRM (ch, C_NRM));
          if (count >= 0)
            len += count;
        }

      if (PRF_FLAGGED (ch, PRF_DISPMOVE) && len < sizeof (prompt))
        {
          count = snprintf (prompt + len, sizeof (prompt) - len, "%s%d%sV ",
                            colorRatio (ch, COLOR_RAW, C_CMP, GET_MOVE (ch), GET_MAX_MOVE (ch)),
                            GET_MOVE (ch), CCNRM (ch, C_NRM));
          if (count >= 0)
            len += count;
        }
      if (PRF_FLAGGED (ch, PRF_DISPEXP) && len < sizeof (prompt))
        {
          float thisLvl = (titles[(int) GET_CLASS (ch)][GET_LEVEL (ch) + 0].exp);
          float nextLvl = (titles[(int) GET_CLASS (ch)][GET_LEVEL (ch) + 1].exp);
          float levelXP = GET_EXP (ch) - thisLvl;
          float totalXP = nextLvl - thisLvl;
          count = snprintf (prompt + len, sizeof (prompt) - len, "[%s%d%s] ",
                            CCBLU (ch, C_NRM), (int) (100.0 * (levelXP / totalXP)),
                            CCNRM (ch, C_NRM));
          if (count >= 0)
            len += count;
        }

      if ((PRF_FLAGGED (ch, PRF_SHOWTANK) && FIGHTING (ch)) && len < sizeof (prompt))
        {
          CharData *tank = FIGHTING (FIGHTING (ch));
          if (tank && tank != ch && tank->in_room == ch->in_room)
            {
              count = snprintf (prompt + len, sizeof (prompt) - len, "[%s %s%d%sH] ", 
              IS_NPC(tank) ? "Tank:" : PERS(tank, ch),
              colorRatio (ch, COLOR_RAW, C_CMP, GET_HIT (tank), GET_MAX_HIT (tank)),
              GET_HIT (tank), CCNRM (ch, C_NRM));

              if (count >= 0)
                len += count;
            }
        }
        if (IS_AFK(d->character) && len < sizeof (prompt)) {
            count = snprintf(prompt + len, sizeof (prompt) - len, "AFK ");
            if (count >= 0)
                len += count;
        }

      if (len < sizeof (prompt))
        strncat (prompt, "> ", sizeof (prompt) - len - 1); /* strncat: OK */
    }
  else if (STATE (d) == CON_PLAYING && IS_NPC (d->character))
    snprintf (prompt, sizeof (prompt), "%s> ", GET_NAME (d->character));
  else
    *prompt = '\0';

  return (prompt);
}

void
write_to_q(char *txt, struct txt_q * queue, int aliased)
{
    /*struct txt_block *new;*/
    TxtBlock *new;

    CREATE(new, struct txt_block, 1);
    CREATE(new->text, char, strlen(txt) + 1);
    strcpy(new->text, txt);
    new->aliased = aliased;

    /* queue empty? */
    if (!queue->head)
    {
        new->next = NULL;
        queue->head = queue->tail = new;
    }

    else
    {
        queue->tail->next = new;
        queue->tail = new;
        new->next = NULL;
    }
}

int
get_from_q(struct txt_q * queue, char *dest, int *aliased)
{
    struct txt_block *tmp;

    /* queue empty? */
    if (!queue->head)
        return 0;

    tmp = queue->head;
    strcpy(dest, queue->head->text);
    *aliased = queue->head->aliased;
    queue->head = queue->head->next;

    free(tmp->text);
    free(tmp);

    return 1;
}

/* Empty the queues before closing connection */
void
flush_queues(DescriptorData * d)
{
    int dummy;

    if (d->large_outbuf)
    {
        d->large_outbuf->next = bufpool;
        bufpool = d->large_outbuf;
    }
    while (get_from_q(&d->input, buf2, &dummy));
}

/* Add a new string to a player's output queue. For outside use. */
size_t
write_to_output (DescriptorData *t, const char *txt, ...)
{
  va_list args;
  size_t left;

  va_start (args, txt);
  left = vwrite_to_output (t, txt, args);
  va_end (args);

  return left;
    }

/* Add a new string to a player's output queue. */
size_t vwrite_to_output(struct descriptor_data *t, const char *format, va_list args)
{
  const char *text_overflow = "\r\nOVERFLOW\r\n";
  static char txt[MAX_STRING_LENGTH];
  size_t wantsize;
  int size;

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufspace == 0)
    return (0);

  wantsize = size = vsnprintf(txt, sizeof(txt), format, args);
  /* If exceeding the size of the buffer, truncate it for the overflow message */
  if (size < 0 || wantsize >= sizeof(txt)) {
    size = sizeof(txt) - 1;
    strcpy(txt + size - strlen(text_overflow), text_overflow);	/* strcpy: OK */
  }

  /* If the text is too big to fit into even a large buffer, truncate
   * the new text to make it fit.  (This will switch to the overflow
   * state automatically because t->bufspace will end up 0.) */
  if (size + t->bufptr + 1 > LARGE_BUFSIZE) {
    size = LARGE_BUFSIZE - t->bufptr - 1;
    txt[size] = '\0';
    buf_overflows++;
  }

  /* If we have enough space, just write to buffer and that's it! If the
   * text just barely fits, then it's switched to a large buffer instead. */
  if (t->bufspace > size) {
    strcpy(t->output + t->bufptr, txt);	/* strcpy: OK (size checked above) */
    t->bufspace -= size;
    t->bufptr += size;
    return (t->bufspace);
  }

  buf_switches++;

  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {			/* else create a new one */
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output);	/* strcpy: OK (size checked previously) */
  t->output = t->large_outbuf->text;	/* make big buffer primary */
  strcat(t->output, txt);	/* strcat: OK (size checked) */

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;

  return (t->bufspace);
}

static void
free_bufpool (void)
{
  struct txt_block *tmp;

  while (bufpool)
    {
      tmp = bufpool->next;
      if (bufpool->text)
        free (bufpool->text);
      free (bufpool);
      bufpool = tmp;
    }
}

/* ******************************************************************
 *  socket handling                                                  *
 ****************************************************************** */

/* get_bind_addr: Return a struct in_addr that should be used in our
 * call to bind().  If the user has specified a desired binding
 * address, we try to bind to it; otherwise, we bind to INADDR_ANY.
 * Note that inet_aton() is preferred over inet_addr() so we use it if
 * we can.  If neither is available, we always bind to INADDR_ANY. */
static struct in_addr *get_bind_addr()
{
  static struct in_addr bind_addr;

  /* Clear the structure */
  memset((char *) &bind_addr, 0, sizeof(bind_addr));

  /* If DLFT_IP is unspecified, use INADDR_ANY */
  if (CONFIG_DFLT_IP == NULL) {
    bind_addr.s_addr = htonl(INADDR_ANY);
  } else {
    /* If the parsing fails, use INADDR_ANY */
    if (!parse_ip(CONFIG_DFLT_IP, &bind_addr)) {
      mlog("SYSERR: DFLT_IP of %s appears to be an invalid IP address",
          CONFIG_DFLT_IP);
      bind_addr.s_addr = htonl(INADDR_ANY);
    }
  }

  /* Put the address that we've finally decided on into the logs */
  if (bind_addr.s_addr == htonl(INADDR_ANY))
    mlog("Binding to all IP interfaces on this host.");
  else
    mlog("Binding only to IP address %s", inet_ntoa(bind_addr));

  return (&bind_addr);
}

#ifdef HAVE_INET_ATON
/* inet_aton's interface is the same as parse_ip's: 0 on failure, non-0 if
 * successful. */
static int parse_ip(const char *addr, struct in_addr *inaddr)
{
  return (inet_aton(addr, inaddr));
}

#elif HAVE_INET_ADDR

/* inet_addr has a different interface, so we emulate inet_aton's */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  long ip;

  if ((ip = inet_addr(addr)) == -1) {
    return (0);
  } else {
    inaddr->s_addr = (unsigned long) ip;
    return (1);
  }
}

#else
/* If you have neither function - sorry, you can't do specific binding. */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  mlog("SYSERR: warning: you're trying to set DFLT_IP but your system has no "
      "functions to parse IP addresses (how bizarre!)");
  return (0);
}
#endif /* INET_ATON and INET_ADDR */

/* Sets the kernel's send buffer size for the descriptor */
static int set_sendbuf(socket_t s)
{
#if defined(SO_SNDBUF)
  int opt = MAX_SOCK_BUF;

  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt SNDBUF");
    return (-1);
  }
#endif

  return (0);
}

static int new_descriptor(socket_t s)
{
    socket_t desc;
    int sockets_connected = 0;
    socklen_t i;
    static int last_desc = 0; /* last descriptor number */
    DescriptorData *newd;
    struct sockaddr_in peer;
    struct hostent *from;

    /* accept the new connection */
  i = sizeof(peer);
  if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET) {
    perror("SYSERR: accept");
    return (-1);
    }
    /* keep it from blocking */
    nonblock(desc);

  /* set the send buffer size */
  if (set_sendbuf(desc) < 0) {
    CLOSE_SOCKET(desc);
    return (0);
  }

    /* make sure we have room for it */
    for (newd = descriptor_list; newd; newd = newd->next)
        sockets_connected++;

  if (sockets_connected >= CONFIG_MAX_PLAYING) {
    write_to_descriptor(desc, "Sorry, the game is full right now... please try again later!\r\n");
    CLOSE_SOCKET(desc);
    return (0);
    }
    /* create a new descriptor */
    CREATE(newd, DescriptorData, 1);

  /* find the sitename */
  if (CONFIG_NS_IS_SLOW ||
      !(from = gethostbyaddr((char *) &peer.sin_addr,
		             sizeof(peer.sin_addr), AF_INET))) {

    /* resolution failed */
    if (!CONFIG_NS_IS_SLOW)
      perror("SYSERR: gethostbyaddr");

    /* find the numeric site address */
    strncpy(newd->host, (char *)inet_ntoa(peer.sin_addr), HOST_LENGTH);	/* strncpy: OK (n->host:HOST_LENGTH+1) */
        *(newd->host + HOST_LENGTH) = '\0';
  } else {
    strncpy(newd->host, from->h_name, HOST_LENGTH);	/* strncpy: OK (n->host:HOST_LENGTH+1) */
    *(newd->host + HOST_LENGTH) = '\0';
    }

    /* determine if the site is banned */
  if (isbanned(newd->host) == BAN_ALL) {
    CLOSE_SOCKET(desc);
    mudlog(CMP, LVL_GOD, TRUE, "Connection attempt denied from [%s]", newd->host);
        free(newd);
    return (0);
    }

    /* initialize descriptor data */
    newd->descriptor = desc;
    setConnectState(newd, CON_GET_NAME);
    newd->pos = -1;
    newd->wait = 1;
    newd->inbuf[0] = '\0';
    newd->output = newd->small_outbuf;
    newd->bufspace = SMALL_BUFSIZE - 1;
    newd->next = descriptor_list;
    newd->login_time = time(0);

    if (++last_desc == 1000)
        last_desc = 1;
    newd->desc_num = last_desc;

    /* prepend to list */
    descriptor_list = newd;

    write_to_output (newd, "%s", CONFIG_GREETINGS);

    return (0);
}

/* Send all of the output that we've accumulated for a player out to the
 * player's descriptor. 32 byte GARBAGE_SPACE in MAX_SOCK_BUF used for:
 *	 2 bytes: prepended \r\n
 *	14 bytes: overflow message
 *	 2 bytes: extra \r\n for non-comapct
 *      14 bytes: unused */
int process_output(DescriptorData * t)
{
    static char i[MAX_SOCK_BUF], *osb = i + 2;
    static int result;

    /* we may need this \r\n for later -- see below */
    strcpy(i, "\r\n"); /* strcpy: OK (for 'MAX_SOCK_BUF >= 3') */

    /* now, append the 'real' output */
    strcpy(osb, t->output); /* strcpy: OK (t->output:LARGE_BUFSIZE < osb:MAX_SOCK_BUF-2) */

    /* if we're in the overflow state, notify the user */
  if (t->bufptr < 0)
        strcat(i, "**OVERFLOW**\r\n"); /* strcpy: OK (osb:MAX_SOCK_BUF-2 reserves space) */

    /* add the extra CRLF if the person isn't in compact mode */
    if (!t->connected && t->character && !IS_NPC(t->character) && !PRF_FLAGGED(t->character, PRF_COMPACT))
        strcat(osb, "\r\n"); /* strcpy: OK (osb:MAX_SOCK_BUF-2 reserves space) */

    procColor(i, (clr(t->character, C_CMP)));
  
     /* now, send the output.  If this is an 'interruption', use the prepended
     * CRLF, otherwise send the straight output sans CRLF. */

    if (!t->prompt_mode) /* && !t->connected) */
        result = write_to_descriptor(t->descriptor, i);
    else
        result = write_to_descriptor(t->descriptor, osb);

    /* handle snooping: prepend "% " and send to snooper */
    if (t->snoop_by)
      write_to_output (t->snoop_by, "%% %*s%%%%", result, t->output);

    /*
     * if we were using a large buffer, put the large buffer on the buffer pool
     * and switch back to the small one
     */
    if (t->large_outbuf)
    {
        t->large_outbuf->next = bufpool;
        bufpool = t->large_outbuf;
        t->large_outbuf = NULL;
        t->output = t->small_outbuf;
    }
    /* reset total bufspace back to that of a small buffer */
    t->bufspace = SMALL_BUFSIZE - 1;
    t->bufptr = 0;
    *(t->output) = '\0';

    return result;
}

int write_to_descriptor(int desc, char *txt)
{
    int total, bytes_written;

    total = strlen(txt);

    do
    {
        if ((bytes_written = write(desc, txt, total)) < 0)
        {
#ifdef EWOULDBLOCK
            if (errno == EWOULDBLOCK)
                errno = EAGAIN;
#endif
            if (errno == EAGAIN)
            {
                static int eagainCnt = 0;
                if (++eagainCnt == 10)
                {
                    mlog("process_output: socket write would block repeated 10 times.");
                    eagainCnt = 0;
                }
            }
            else
                perror("Write to socket");
            return -1;
        }
        else
        {
            txt += bytes_written;
            total -= bytes_written;
        }
    }
    while (total > 0);

    return 0;
}

/*
 ** ASSUMPTION: There will be no newlines in the raw input buffer
 ** when this function is called.  We must maintain that before
 ** returning.
 */
int
process_input(DescriptorData *t)
{
    int space_left, failed_subst;
    char *ptr, *write_point, *nl_pos = NULL;
    char tmp[MAX_INPUT_LENGTH + 8];

    // First, find the point where we left off reading data.
    //
    int bufLen = strlen(t->inbuf);
    int maxRead;
    char* readPtr = t->inbuf + bufLen;
    int bytesIn = 0;

    do
    {
        maxRead = MAX_INPUT_LENGTH - bufLen - 1;
        if (maxRead <= 0)
        {
            mlog("TRUNC: Line from %s was truncated.",
                    t->character ? GET_NAME(t->character) : t->host);
            mudlog(NRM, LVL_IMMORT, TRUE, "LOCKUP attempt? Keep an eye on %s",
                    t->character ? GET_NAME(t->character) : t->host);
            if (t->character) WAIT_STATE(t->character, (PULSE_VIOLENCE * 10));
            return (-1);
        }
        bytesIn = read(t->descriptor, readPtr, maxRead);

        if (bytesIn < 0)
        {
#ifdef EWOULDBLOCK
            if (errno == EWOULDBLOCK) errno = EAGAIN;
#endif
            if (errno != EAGAIN)
            {
                static int errcnt = 0;
                if (errcnt++ % 50 == 0)
                    mudlog(NRM, LVL_IMMORT, TRUE, "Errno %d, losing connection [%d]", errno, errcnt);
                return -1; /* some error condition was encountered on read */
            }
            else return 0; /* the read would have blocked: no data there */
        }
        else if (bytesIn == 0)
        {
            static int errcnt = 0;
            if (errcnt++ % 50 == 0)
                mlog("EOF on socket read (connection broken by peer)");
            return -1;
        }

        // At this point, we know we have data on the socket so terminate
        // it and find the newline character in the string.
        //
        *(readPtr + bytesIn) = '\0';
        for (ptr = readPtr; *ptr && !nl_pos; ptr++)
            if (ISNEWL(*ptr)) nl_pos = ptr;

        readPtr += bytesIn;
        bufLen += bytesIn;

        if ((nl_pos == NULL) && (bytesIn == 1)) return 0;

    }
    while (nl_pos == NULL);

    if ((raw_input_logging ||
            (t->character != NULL && PLR_FLAGGED(t->character, PLR_RAWLOG))) &&
            bytesIn > 2)
    {
        static char rawOut[132];
        int rawLen = (bytesIn < 100 ? bytesIn : 100);
        char *rawName = (t->character == NULL ? t->host : GET_NAME(t->character));

        memset(rawOut, 0, sizeof ( rawOut));
        memcpy(rawOut, t->inbuf, rawLen - 1);
        mlog("RAW:%3d [%s] [%s]", bytesIn, rawName, rawOut);
    }

    // At this point we have at least one newline in the string; now we
    // can copy the formatted data to a new array for further processing.
    //
    readPtr = t->inbuf;

    while (nl_pos != NULL)
    {
        write_point = tmp;
        space_left = MAX_INPUT_LENGTH - 1;

        for (ptr = readPtr; (space_left > 0) && (ptr < nl_pos); ptr++)
        {
            if (*ptr == '\b') /* handle backspacing */
            {
                if (write_point > tmp)
                {
                    if (*(--write_point) == '$')
                    {
                        write_point--;
                        space_left += 2;
                    }
                    else
                    {
                        space_left++;
                    }
                }
            }
            else if (isascii(*ptr) && isprint(*ptr))
            {
                if ((*(write_point++) = *ptr) == '$') /* copy one character */
                {
                    *(write_point++) = '$'; /* if it's a $, double it */
                    space_left -= 2;
                }
                else
                {
                    space_left--;
                }
            }
        }
        *write_point = '\0';

        if ((space_left <= 0) && (ptr < nl_pos))
        {
            mlog("TRUNC: Line from %s was truncated.",
                    t->character ? GET_NAME(t->character) : t->host);
            mudlog(NRM, LVL_IMMORT, TRUE, "LOCKUP attempt? Keep an eye on %s",
                    t->character ? GET_NAME(t->character) : t->host);
            if (t->character) WAIT_STATE(t->character, (PULSE_VIOLENCE * 10));
            return (-1);
        }

        if (t->snoop_by)

            write_to_output (t->snoop_by, "%% Snp[%s]\r\n", tmp);
        failed_subst = 0;

        if (*tmp == '!')
        {
            strcpy(tmp, t->last_input);
        }
        else if (*tmp == '^')
        {
            if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
                strcpy(t->last_input, tmp);
        }
        else
        {
            strcpy(t->last_input, tmp);
        }

        if (!failed_subst)
        {
            write_to_q(tmp, &t->input, 0);
        }

        // Find the end of this line
        //
        while (ISNEWL(*nl_pos)) nl_pos++;

        // See if there's another newline in the input buffer
        //
        readPtr = ptr = nl_pos;
        for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
        {
            if (ISNEWL(*ptr)) nl_pos = ptr;
        }
    }
    /*
     ** Move the rest of the buffer up to the beginning
     ** of the input buffer for the next pass.
     */
#if 0
    write_point = t->inbuf;
    while (*readPtr) *(write_point++) = *(readPtr++);
    *write_point = '\0';
#else
    {
        int moveBytes = strlen(readPtr) + 1;
        if (moveBytes >= MAX_INPUT_LENGTH)
        {
            mudlog(NRM, LVL_IMMORT, TRUE, "WARNING: Possible input stream corruption.");
            moveBytes = MAX_INPUT_LENGTH;
        }
        memmove(t->inbuf, readPtr, moveBytes);
    }
#endif
    return 1;
}

/* perform substitution for the '^..^' csh-esque syntax */
int perform_subst(DescriptorData * t, char *orig, char *subst)
{
    char new[MAX_INPUT_LENGTH + 5];

    char *first, *second, *strpos;

    first = subst + 1;
    if (!(second = strchr(first, '^')))
    {
      write_to_output (t, "Invalid substitution.\r\n");
        return 1;
    }
    *(second++) = '\0';

    if (!(strpos = strstr(orig, first)))
    {
      write_to_output (t, "Invalid substitution.\r\n");
        return 1;
    }
    strncpy(new, orig, (strpos - orig));
    new[(strpos - orig)] = '\0';
    strcat(new, second);
    if (((strpos - orig) + strlen(first)) < strlen(orig))
        strcat(new, strpos + strlen(first));
    strcpy(subst, new);

    return 0;
}

void close_socket(DescriptorData *d)
{
    DescriptorData *temp;

    close(d->descriptor);
    flush_queues(d);
    /*
     ** Forget snooping
     */
    if (d->snooping) d->snooping->snoop_by = NULL;

    if (d->snoop_by)
    {
      write_to_output (d->snoop_by, "Your victim is no longer among us.\r\n");
        d->snoop_by->snooping = NULL;
    }
  /*. Kill any OLC stuff .*/
  switch (d->connected) {
    case CON_OEDIT:
    case CON_REDIT:
    case CON_ZEDIT:
    case CON_MEDIT:
    case CON_SEDIT:
    case CON_QEDIT:
    case CON_TEDIT:
    case CON_TRIGEDIT:
    case CON_CEDIT:  
    case CON_HEDIT:
        cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
        break;
    }

    if (d->character)
    {
        if (d->connected == CON_PLAYING)
        {
            save_char(d->character, NOWHERE);
            act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
            mudlog( NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, "Closing link to: %s.", GET_NAME(d->character));
            d->character->desc = NULL;
        }
        else
        {
            free_char(d->character);
        }
    }

    REMOVE_FROM_LIST(d, descriptor_list, next);

    if (d->showstr_head) free(d->showstr_head);

    free(d);
}

/*
 * I have removed all the #ifdefs and whatnot from here - nonblock(),
 * process_input(), and process_output() all now universally use the POSIX
 * discipline for nonblocking IO.  -JE
 * Used to be O_NONBLOCK
 * BWRS uses FIONBIO
 * 2.2 used FNDELAY
 */
void nonblock(int s)
{
    if (fcntl(s, F_SETFL, FNDELAY) < 0)
    {
        perror("Fatal error executing nonblock (comm.c)");
        exit(1);
    }
}

/* ******************************************************************
 *  signal-handling functions (formerly signals.c)                   *
 ****************************************************************** */


void checkpointing()
{
  if (!tics) {
        mlog("SYSERR: CHECKPOINT shutdown: tics not updated");
        abort();
  } else
        tics = 0;
}

void reread_wizlists()
{
    mudlog(CMP, LVL_IMMORT, FALSE, "Rereading wizlists.");
    reboot_wizlists();
}

void unrestrict_game()
{
    extern struct ban_list_element *ban_list;
    extern int num_invalid;

    mudlog(BRF, LVL_IMMORT, TRUE, "Received SIGUSR2 - completely unrestricting game (emergent)");
    ban_list = NULL;
    circle_restrict = 0;
    num_invalid = 0;
}

void hupsig()
{
    mlog("Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
    exit(0); /* perhaps something more elegant should
				 * substituted */
}

/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted because BSD systems do not restart
 * select(), even if SA_RESTART is used.
 */
sigfunc *my_signal(int signo, sigfunc * func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif

    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;

    return oact.sa_handler;
}

void signal_setup(void)
{
    struct itimerval itime;
    struct timeval interval;

    /* user signal 1: reread wizlists.  Used by autowiz system. */
    my_signal(SIGUSR1, reread_wizlists);

    /*
     * user signal 2: unrestrict game.  Used for emergencies if you lock
     * yourself out of the MUD somehow.  (Duh...)
     */
    my_signal(SIGUSR2, unrestrict_game);

    /*
     * set up the deadlock-protection so that the MUD aborts itself if it gets
     * caught in an infinite loop for more than 3 minutes
     */
    interval.tv_sec = 180;
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, NULL);
    my_signal(SIGVTALRM, checkpointing);

    /* just to be on the safe side: */
    my_signal(SIGHUP, hupsig);
    my_signal(SIGINT, hupsig);
    my_signal(SIGTERM, hupsig);
    my_signal(SIGPIPE, SIG_IGN);
    my_signal(SIGALRM, SIG_IGN);
    //my_signal(SIGSEGV, hupsig2);

    /* Strange, we been getting SIGWINCH and this has been causing us to die.. ignore it */
    my_signal(SIGWINCH, SIG_IGN);
}

/* ****************************************************************
 *       Public routines for system-to-player-communication        *
 *******************************************************************/



void
send_to_char(char *messg, CharData * ch)
{
    if (ch && ch->desc && messg) write_to_output(ch->desc, "%s", messg);
}

void send_to_all(const char *messg, ...)
{
  struct descriptor_data *i;
  va_list args;

  if (messg == NULL)
    return;

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING)
      continue;

    va_start(args, messg);
    vwrite_to_output(i, messg, args);
    va_end(args);
  }
}

void send_to_outdoor(char *messg)
{
    DescriptorData *i;

    if (!messg || !*messg)
        return;

    for (i = descriptor_list; i; i = i->next)
        if (!i->connected && i->character && AWAKE(i->character) &&
                OUTSIDE(i->character))
            write_to_output(i, "%s", messg);
}

void send_to_room(char *messg, int room)
{
    CharData *i;
    int notsleep = room < 0;

    if (notsleep) room = -room;
    if (messg)
        for (i = world[room].people; i; i = i->next_in_room)
            if (i->desc && (!notsleep || GET_POS(i) > POS_SLEEPING))
        write_to_output (i->desc, "%s", messg);
}

/*
 ** higher-level communication: the act() function
 */
void
perform_act(char *orig,
            CharData *ch,
            ObjData *obj,
            void *vict_obj,
            CharData * to)
{
    register char *i, *buf;
    static char lbuf[MAX_STRING_LENGTH];
    char *colorHighlight = NULL;
    CharData * dg_victim = NULL;
    ObjData * dg_target = NULL;
    char *dg_arg = NULL;

    buf = lbuf;

    for (;;)
    {
        if (*orig == '$')
        {
            switch (*(++orig))
            {
            case 'a': i = SANA(obj);
                break;
            case 'A':
            {
                i = SANA((ObjData *) vict_obj);
            }
                break;
            case 'e': i = HSSH(ch);
                break;
            case 'E':
            {
                i = HSSH((CharData *) vict_obj);
                dg_victim = (CharData *) vict_obj;
            }
                break;
            case 'F': i = fname((char *) vict_obj);
                break;
            case 'm': i = HMHR(ch);
                break;
            case 'M':
            {
                i = HMHR((CharData *) vict_obj);
                dg_victim = (CharData *) vict_obj;
            }
                break;
            case 'n':
            {
                i = PERS(ch, to);
                dg_victim = (CharData *) vict_obj;
            }
                break;
            case 'o': i = OBJN(obj, to);
                break;
            case 'O':
            {
                i = OBJN((ObjData *) vict_obj, to);
                dg_victim = (CharData *) vict_obj;
            }
                break;
            case 'p': i = OBJS(obj, to);
                break;
            case 'P':
            {
                i = OBJS((ObjData *) vict_obj, to);
                dg_victim = (CharData *) vict_obj;
            }
                break;
            case 's': i = HSHR(ch);
                break;
            case 'S':
            {
                i = HSHR((CharData *) vict_obj);
                dg_victim = (CharData *) vict_obj;
            }
                break;
            case 'T':
            {
                i = (char *) vict_obj;
                dg_arg = (char *) vict_obj;
            }
                break;
            case 'Z': i = "\012\015";
                break;
            case '$': i = "$";
                break;
            case 'N':
            {
                CharData *ch = vict_obj;
                i = PERS(ch, to);
                colorHighlight = colorRatio(ch, COLOR_COOKED, C_CMP, GET_HIT(ch), GET_MAX_HIT(ch));
                dg_victim = (CharData *) vict_obj;
            }
                break;
            default:
                mlog("SYSERR: Illegal $-code to act():");
                strcpy(buf1, "SYSERR: ");
                strcat(buf1, orig);
                mlog(buf1);
                break;
            }

            if (colorHighlight != NULL)
            {
                char *col = colorHighlight;
                while ((*buf = *(col++))) buf++;
                colorHighlight = KNRM;
            }

            while ((*buf = *(i++))) buf++;

            if (colorHighlight != NULL)
            {
                char *col = KNRM;
                while ((*buf = *(col++))) buf++;
            }

            orig++;
        }
        else if (!(*(buf++) = *(orig++)))
            break;
    }

    *(--buf) = '\r';
    *(++buf) = '\n';
    *(++buf) = '\0';

    if (to->desc)
    write_to_output (to->desc, "%s", CAP (lbuf));

    if ((IS_NPC(to) && dg_act_check) && (to != ch))
        act_mtrigger(to, lbuf, ch, dg_victim, obj, dg_target, dg_arg);

}

void
act(char *str,
    int hide_invisible,
    CharData *ch,
    ObjData *obj,
    void *vict_obj,
    int type)
{
    static int to_sleeping;
    CharData *to;

    if (!str || !*str) return;

    if (!(dg_act_check = !(type & DG_NO_TRIG)))
        type &= ~DG_NO_TRIG;

    /*
     * Warning: the following TO_SLEEP code is a hack.
     *
     * I wanted to be able to tell act to deliver a message regardless of sleep
     * without adding an additional argument.  TO_SLEEP is 128 (a single bit
     * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
     * command.  It's not legal to combine TO_x's with each other otherwise.
     */

    /* check if TO_SLEEP is there, and remove it if it is. */
    if ((to_sleeping = (type & TO_SLEEP)))
        type &= ~TO_SLEEP;

    if (type == TO_CHAR)
    {
        if (ch && SENDOK(ch))
            perform_act(str, ch, obj, vict_obj, ch);
        return;
    }

    if (type == TO_VICT)
    {
        if ((to = (CharData *) vict_obj) && SENDOK(to))
            perform_act(str, ch, obj, vict_obj, to);
        return;
    }
    /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

    if (ch && ch->in_room != NOWHERE)
        to = world[ch->in_room].people;
    else if (obj && obj->in_room != NOWHERE)
        to = world[obj->in_room].people;
  else {
#if 0
        mlog("SYSERR: no valid target to act()!");
#endif
        return;
    }

    /* This is the most god awful if statement I've ever seen. Vex. */
    /* Added a check for "to" at start since there was crashes occuring when */
    /* people where zapped by eq upon entering the game. */
    for (; to; to = to->next_in_room)
        if (to && SENDOK(to) && (to != ch) &&
                !(hide_invisible && ch && !CAN_SEE(to, ch)) &&
                (type == TO_ROOM || (to != vict_obj)) &&
                ((type != TO_ACTSPAM) || !PRF_FLAGGED(to, PRF_NOSPAM)))
            perform_act(str, ch, obj, vict_obj, to);
}

/* Prefer the file over the descriptor. */
static void setup_log(const char *filename, int fd)
{
  FILE *s_fp;

#if defined(__MWERKS__) || defined(__GNUC__)
  s_fp = stderr;
#else
  if ((s_fp = fdopen(STDERR_FILENO, "w")) == NULL) {
    puts("SYSERR: Error opening stderr, trying stdout.");

    if ((s_fp = fdopen(STDOUT_FILENO, "w")) == NULL) {
      puts("SYSERR: Error opening stdout, trying a file.");

      /* If we don't have a file, try a default. */
      if (filename == NULL || *filename == '\0')
        filename = "log/syslog";
    }
  }
#endif

  if (filename == NULL || *filename == '\0') {
    /* No filename, set us up with the descriptor we just opened. */
    logfile = s_fp;
    puts("Using file descriptor for logging.");
    return;
  }

  /* We honor the default filename first. */
  if (open_logfile(filename, s_fp))
    return;

  /* Well, that failed but we want it logged to a file so try a default. */
  if (open_logfile("log/syslog", s_fp))
    return;

  /* Ok, one last shot at a file. */
  if (open_logfile("syslog", s_fp))
    return;

  /* Erp, that didn't work either, just die. */
  puts("SYSERR: Couldn't open anything to log to, giving up.");
  exit(1);
}

static int open_logfile(const char *filename, FILE *stderr_fp)
{
  if (stderr_fp)	/* freopen() the descriptor. */
    logfile = freopen(filename, "w", stderr_fp);
  else
    logfile = fopen(filename, "w");

  if (logfile) {
    printf("Using log file '%s'%s.\n",
		filename, stderr_fp ? " with redirection" : "");
    return (TRUE);
  }

  printf("SYSERR: Error opening file '%s': %s\n", filename, strerror(errno));
  return (FALSE);
}

void
setConnectState(DescriptorData *d, int newState)
{
    d->connected = newState;
    switch (newState)
    {
    case CON_PLAYING:
        d->dcTimer = 300;
        break;
    case CON_CLOSE:
        d->dcTimer = 75;
        break;
    case CON_OEDIT:
    case CON_REDIT:
    case CON_ZEDIT:
    case CON_MEDIT:
    case CON_SEDIT:
    case CON_QEDIT:
    case CON_TEDIT:
    case CON_QABILS:
    case CON_CEDIT:
    case CON_HEDIT:
        d->dcTimer = 600;
        break;
    default:
        d->dcTimer = 180;
        break;
    }
}

