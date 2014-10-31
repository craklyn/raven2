/* ************************************************************************
*   File: comm.h                                        Part of CircleMUD *
*  Usage: header file: prototypes of public communication functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#ifndef _COMM_H_
#define _COMM_H_

#define OPT_USEC 100000 /* 10 passes per second */
#define PASSES_PER_SEC (1000000 / OPT_USEC)
#define SEC             * PASSES_PER_SEC  
#define PULSE_ZONE      (10 SEC)
#define PULSE_MOBILE    (10 SEC)
#define PULSE_VIOLENCE  (2 SEC)
#define PULSE_AFFECT    (1 SEC) 
#define PULSE_WATER	(2 SEC)
#define PULSE_DISASTER  (6 SEC)
#define PULSE_SUFFER	(15 SEC)
#define PULSE_AUCTION   (25 SEC)
#define PULSE_HEALING   (15 SEC)
#define PULSE_SPAMMING  (5 SEC)
#define PULSE_MAIL      (500 SEC)
#define NUM_RESERVED_DESCS 8

extern void send_to_all(const char *messg, ...) __attribute__ ((format (printf, 1,
    2)));
extern void send_to_char(char *messg, CharData *ch);
extern void send_to_room(char *messg, int room);
extern void send_to_outdoor(char *messg);
extern void perform_to_all(char *messg, CharData *ch);
extern void close_socket(DescriptorData *d);
extern void auction_update();
extern void water_activity();
extern void disaster_activity();
extern void suffer_activity();
extern void perform_act( char     *orig, 
                         CharData *ch,
                         ObjData  *obj,
                         void     *vict_obj,
                         CharData *to );

extern void act( char     *str,
                 int       hide_invisible,
                 CharData *ch,
                 ObjData  *obj,
                 void     *vict_obj,
                 int      type);

/* Act type settings and flags */
#define TO_ROOM    1  /* act() type: to everyone in room, except ch. */
#define TO_VICT    2  /* act() type: to vict_obj. */
#define TO_NOTVICT 3  /* act() type: to everyone in room, not ch or vict_obj. */
#define TO_CHAR    4  /* act() type: to ch. */
#define TO_ACTSPAM 5
#define TO_SLEEP   128 /* act() flag: to char, even if sleeping */

#define USING_SMALL(d) ((d)->output == (d)->small_outbuf)
#define USING_LARGE(d)  (!USING_SMALL(d))

extern int write_to_descriptor(int desc, char *txt);
extern void write_to_q(char *txt, struct txt_q *queue, int aliased);
size_t write_to_output(DescriptorData *d, const char *txt, ...) __attribute__ ((format (printf, 2, 3)));
size_t vwrite_to_output(DescriptorData *d, const char *format, va_list args);
extern void page_string(DescriptorData *d, char *str, int keep_internal);
extern void setConnectState( DescriptorData *d, int newState );

typedef void sigfunc(int);

/* global buffering system - allow access to global variables within comm.c */
#ifndef __COMM_C__
extern DescriptorData *descriptor_list;
extern int circle_shutdown;
extern int circle_reboot;
extern int oracle_reboot;
extern int oracle_counter;
extern int raw_input_logging;
extern FILE *logfile;
extern int muckle_active;

#endif /* __COMM_C__ */

#endif /* _COMM_H_ */
