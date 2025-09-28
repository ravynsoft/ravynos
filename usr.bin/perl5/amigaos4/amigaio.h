#ifndef _AMIGAIO_H
#define _AMIGAIO_H

#ifndef H_PERL
#include "../perl.h"
#endif

struct StdioStore
{
        /* astdin...astderr are the amigaos file descriptors */
        long astdin;
        long astdout;
        long astderr;
        /* oldstdin...oldstderr are the amigados file handles */
        long oldstdin;
        long oldstdout;
        long oldstderr;
};

typedef struct StdioStore StdioStore;

/* get the amigaos file descriptors */
void amigaos_stdio_get(pTHX_ StdioStore *store);

/* save the amigados file handles (calls amigaos_stdio_get) */
void amigaos_stdio_save(pTHX_ StdioStore *store);

/* restore the amigados file handles stored with amigaos_stdio_save */
void amigaos_stdio_restore(pTHX_ const StdioStore *store);

/* everything the child needs from the parent is in UserData,
 * then pass it through task->tc_UserData or as arg to new pthread */
struct UserData
{
        struct Task *parent;
        I32 did_pipes;
        int pp;
        SV **sp;
        SV **mark;
        PerlInterpreter *my_perl;
};

void amigaos_fork_set_userdata(
    pTHX_ struct UserData *userdata, I32 did_pipes, int pp, SV **sp, SV **mark);

void *amigaos_system_child(void *userdata);

void amigaos_post_exec(int fd, int do_report);

Pid_t amigaos_fork();
Pid_t amigaos_waitpid(pTHX_ int optype, Pid_t pid, void *argflags);

#endif
