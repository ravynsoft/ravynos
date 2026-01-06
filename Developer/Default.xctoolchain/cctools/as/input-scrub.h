extern int doing_include;

extern char *physical_input_file;
extern char *logical_input_file;
extern char *layout_file;

/* 1-origin line number in a source file. */
typedef unsigned int line_numberT;

extern line_numberT physical_input_line;
extern line_numberT logical_input_line;
extern line_numberT layout_line;
/*
 * Supplies sanitised buffers to read.c.
 * Also understands printing line-number part of error messages.
 */

/* Line number things. */
extern int seen_at_least_1_file(
    void);
extern void bump_line_counters(
    void);
extern void new_logical_line(
    char *fname,
    int line_number);
extern void as_where(
    void);
extern void as_file_and_line(
    char **file_ret,
    unsigned int *line_ret);
extern void as_where_ProjectBuilder(
    char **fileName,
    char **directory,
    int *line);
extern void as_perror(
    char *gripe,
    char *filename);

/* Sanitising things. */
extern void input_scrub_begin(
    void);
extern void input_scrub_end(
    void);
extern char *input_scrub_new_file(
    char *filename);
extern char *input_scrub_next_buffer(
    char **bufp);
extern char *find_an_include_file(
    char *no_path_name);
extern void read_an_include_file(
    char *no_path_name);
