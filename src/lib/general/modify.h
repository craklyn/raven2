#ifndef __MODIFY_H__
#define __MODIFY_H__
/* ============================================================================ 
Header file for modify. Not much here atm.
Vex.
============================================================================ */
void show_string(struct descriptor_data *d, char *input);
void smash_tilde(char *str);
void send_editor_help(struct descriptor_data *d);
/* Following function prototypes moved here from comm.h */
void  string_write(struct descriptor_data *d, char **txt, size_t len, long mailto, void *data);
void  string_add(struct descriptor_data *d, char *str);
void page_string(DescriptorData * d, char *str, int keep_internal);
/* ============================================================================
Misc editor defines.
============================================================================ */
/* action modes for parse_action */
#define PARSE_FORMAT           0
#define PARSE_REPLACE          1
#define PARSE_HELP             2
#define PARSE_DELETE           3
#define PARSE_INSERT           4
#define PARSE_LIST_NORM        5
#define PARSE_LIST_NUM         6
#define PARSE_EDIT             7

/* Defines for the action variable. */
#define STRINGADD_OK		0	/* Just keep adding text.	     */
#define STRINGADD_SAVE		1	/* Save current text.		     */
#define STRINGADD_ABORT		2	/* Abort edit, restore old text.     */
#define STRINGADD_ACTION	4	/* Editor action, don't append \r\n. */

/* format modes for format_text */
#define FORMAT_INDENT (1 << 0)

#define SEND_RULER(d) \
  write_to_output( d, "&08+&00---------" \
                      "&09+&00---------" \
                      "&09+&00---------" \
                      "&09+&00---------" \
                      "&09+&00---------" \
                      "&09+&00---------" \
                      "&09+&00---------" \
                      "&09+&10---------" \
                      "&08+&00\n" );

#define PAGE_LENGTH 22
#define PAGE_WIDTH  80

#endif /* _MODIFY_H_*/
