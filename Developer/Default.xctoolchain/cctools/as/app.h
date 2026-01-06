#import <stdio.h>

extern FILE *scrub_file;
extern char *scrub_string;
extern char *scrub_last_string;

extern void do_scrub_begin(
    void);
extern int do_scrub_next_char(
    FILE *fp);
extern int do_scrub_next_char_from_string(void);

/*
 * typedefs and routines to save scrub context so .include can make recursive
 * calls to the sanitising routines.
 */
typedef struct scrub_context_data {
    FILE *last_scrub_file;
    int last_state;
    int last_old_state;
    char *last_out_string;
    char last_out_buf[20];
    int last_add_newlines;
} scrub_context_data;

extern void save_scrub_context(
    scrub_context_data *save_buffer_ptr);
extern void restore_scrub_context(
    scrub_context_data *save_buffer_ptr);
