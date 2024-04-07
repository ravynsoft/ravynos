/* amigaos.c uses only amigaos APIs,
 * as opposed to amigaio.c which mixes amigaos and perl APIs */

#include <string.h>

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__CLIB2__)
#  include <dos.h>
#endif
#if defined(__NEWLIB__)
#  include <amiga_platform.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#undef WORD
#define WORD int16

#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigaos.h"

struct UtilityIFace *IUtility = NULL;

/***************************************************************************/

struct Interface *OpenInterface(CONST_STRPTR libname, uint32 libver)
{
        struct Library *base = IExec->OpenLibrary(libname, libver);
        struct Interface *iface = IExec->GetInterface(base, "main", 1, NULL);
        if (iface == NULL)
        {
                // We should probably post some kind of error message here.

                IExec->CloseLibrary(base);
        }

        return iface;
}

/***************************************************************************/

void CloseInterface(struct Interface *iface)
{
        if (iface != NULL)
        {
                struct Library *base = iface->Data.LibBase;
                IExec->DropInterface(iface);
                IExec->CloseLibrary(base);
        }
}

BOOL __unlink_retries = FALSE;

void ___makeenviron() __attribute__((constructor));
void ___freeenviron() __attribute__((destructor));

void ___openinterfaces() __attribute__((constructor));
void ___closeinterfaces() __attribute__((destructor));

void ___openinterfaces()
{
        if (!IDOS)
                IDOS = (struct DOSIFace *)OpenInterface("dos.library", 53);
        if (!IUtility)
                IUtility =
                    (struct UtilityIFace *)OpenInterface("utility.library", 53);
}

void ___closeinterfaces()
{
        CloseInterface((struct Interface *)IDOS);
        CloseInterface((struct Interface *)IUtility);
}
int VARARGS68K araddebug(UBYTE *fmt, ...);
int VARARGS68K adebug(UBYTE *fmt, ...);

#define __USE_RUNCOMMAND__

char **myenviron = NULL;
char **origenviron = NULL;

static void createvars(char **envp);

struct args
{
        BPTR seglist;
        int stack;
        char *command;
        int length;
        int result;
        char **envp;
};

int __myrc(__attribute__((unused))char *arg)
{
        struct Task *thisTask = IExec->FindTask(0);
        struct args *myargs = (struct args *)thisTask->tc_UserData;
        if (myargs->envp)
                createvars(myargs->envp);
        // adebug("%s %ld %s \n",__FUNCTION__,__LINE__,myargs->command);
        myargs->result = IDOS->RunCommand(myargs->seglist, myargs->stack,
                                          myargs->command, myargs->length);
        return 0;
}

int32 myruncommand(
    BPTR seglist, int stack, char *command, int length, char **envp)
{
        struct args myargs;
        struct Task *thisTask = IExec->FindTask(0);
        struct Process *proc;

        // adebug("%s %ld  %s\n",__FUNCTION__,__LINE__,command?command:"NULL");

        myargs.seglist = seglist;
        myargs.stack = stack;
        myargs.command = command;
        myargs.length = length;
        myargs.result = -1;
        myargs.envp = envp;

        if ((proc = IDOS->CreateNewProcTags(
                        NP_Entry, __myrc, NP_Child, TRUE, NP_Input, IDOS->Input(),
                        NP_Output, IDOS->Output(), NP_Error, IDOS->ErrorOutput(),
                        NP_CloseInput, FALSE, NP_CloseOutput, FALSE, NP_CloseError,
                        FALSE, NP_CopyVars, FALSE,

                        //           NP_StackSize,           ((struct Process
                        //           *)myargs.parent)->pr_StackSize,
                        NP_Cli, TRUE, NP_UserData, (int)&myargs,
                        NP_NotifyOnDeathSigTask, thisTask, TAG_DONE)))

        {
                IExec->Wait(SIGF_CHILD);
        }
        return myargs.result;
}

char *mystrdup(const char *s)
{
        char *result = NULL;
        size_t size;

        size = strlen(s) + 1;

        if ((result = (char *)IExec->AllocVecTags(size, TAG_DONE)))
        {
                memmove(result, s, size);
        }
        return result;
}

unsigned int pipenum = 0;

int pipe(int filedes[2])
{
        char pipe_name[1024];

//   adebug("%s %ld \n",__FUNCTION__,__LINE__);
#ifdef USE_TEMPFILES
        sprintf(pipe_name, "/T/%x.%08lx", pipenum++, IUtility->GetUniqueID());
#else
        sprintf(pipe_name, "/PIPE/%x%08lx/4096/0", pipenum++,
                IUtility->GetUniqueID());
#endif

        /*      printf("pipe: %s \n", pipe_name);*/

        filedes[1] = open(pipe_name, O_WRONLY | O_CREAT);
        filedes[0] = open(pipe_name, O_RDONLY);
        if (filedes[0] == -1 || filedes[1] == -1)
        {
                if (filedes[0] != -1)
                        close(filedes[0]);
                if (filedes[1] != -1)
                        close(filedes[1]);
                return -1;
        }
        /*      printf("filedes %d %d\n", filedes[0],
         * filedes[1]);fflush(stdout);*/

        return 0;
}

int fork(void)
{
        fprintf(stderr, "Can not bloody fork\n");
        errno = ENOMEM;
        return -1;
}

int wait(__attribute__((unused))int *status)
{
        fprintf(stderr, "No wait try waitpid instead\n");
        errno = ECHILD;
        return -1;
}

char *convert_path_a2u(const char *filename)
{
        struct NameTranslationInfo nti;

        if (!filename)
        {
                return 0;
        }

        __translate_amiga_to_unix_path_name(&filename, &nti);

        return mystrdup(filename);
}
char *convert_path_u2a(const char *filename)
{
        struct NameTranslationInfo nti;

        if (!filename)
        {
                return 0;
        }

        if (strcmp(filename, "/dev/tty") == 0)
        {
                return mystrdup("CONSOLE:");
                ;
        }

        __translate_unix_to_amiga_path_name(&filename, &nti);

        return mystrdup(filename);
}

struct SignalSemaphore environ_sema;
struct SignalSemaphore popen_sema;


void amigaos4_init_environ_sema()
{
        IExec->InitSemaphore(&environ_sema);
        IExec->InitSemaphore(&popen_sema);
}

void amigaos4_obtain_environ()
{
        IExec->ObtainSemaphore(&environ_sema);
}

void amigaos4_release_environ()
{
        IExec->ReleaseSemaphore(&environ_sema);
}

static void createvars(char **envp)
{
        if (envp)
        {
                /* Set a local var to indicate to any subsequent sh that it is
                * not
                * the top level shell and so should only inherit local amigaos
                * vars */
                IDOS->SetVar("ABCSH_IMPORT_LOCAL", "TRUE", 5, GVF_LOCAL_ONLY);

                amigaos4_obtain_environ();

                envp = myenviron;

                while ((envp != NULL) && (*envp != NULL))
                {
                        int len;
                        char *var;
                        char *val;
                        if ((len = strlen(*envp)))
                        {
                                if ((var = (char *)IExec->AllocVecTags(len + 1, AVT_ClearWithValue,0,TAG_DONE)))
                                {
                                        strcpy(var, *envp);

                                        val = strchr(var, '=');
                                        if (val)
                                        {
                                                *val++ = '\0';
                                                if (*val)
                                                {
                                                        IDOS->SetVar(
                                                            var, val,
                                                            strlen(val) + 1,
                                                            GVF_LOCAL_ONLY);
                                                }
                                        }
                                        IExec->FreeVec(var);
                                }
                        }
                        envp++;
                }
                amigaos4_release_environ();
        }
}

struct command_data
{
        STRPTR args;
        BPTR seglist;
        struct Task *parent;
};


int myexecvp(bool isperlthread, const char *filename, char *argv[])
{
        //	adebug("%s %ld
        //%s\n",__FUNCTION__,__LINE__,filename?filename:"NULL");
        /* if there's a slash or a colon consider filename a path and skip
         * search */
        int res;
        char *name = NULL;
        char *pathpart = NULL;
        if ((strchr(filename, '/') == NULL) && (strchr(filename, ':') == NULL))
        {
                const char *path;
                const char *p;
                size_t len;
                struct stat st;

                if (!(path = getenv("PATH")))
                {
                        path = ".:/bin:/usr/bin:/c";
                }

                len = strlen(filename) + 1;
                name = (char *)IExec->AllocVecTags(strlen(path) + len, AVT_ClearWithValue,0,AVT_Type,MEMF_SHARED,TAG_DONE);
                pathpart = (char *)IExec->AllocVecTags(strlen(path) + 1, AVT_ClearWithValue,0,AVT_Type,MEMF_SHARED,TAG_DONE);
                p = path;
                do
                {
                        path = p;

                        if (!(p = strchr(path, ':')))
                        {
                                p = strchr(path, '\0');
                        }

                        memcpy(pathpart, path, p - path);
                        pathpart[p - path] = '\0';
                        if (!(strlen(pathpart) == 0))
                        {
                                sprintf(name, "%s/%s", pathpart, filename);
                        }
                        else
                                sprintf(name, "%s", filename);

                        if ((stat(name, &st) == 0) && (S_ISREG(st.st_mode)))
                        {
                                /* we stated it and it's a regular file */
                                /* let's boogie! */
                                filename = name;
                                break;
                        }

                }
                while (*p++ != '\0');
        }

        res = myexecve(isperlthread, filename, argv, myenviron);

        if(name)
        {
                IExec->FreeVec((APTR)name);
                name = NULL;
        }
        if(pathpart)
        {
                IExec->FreeVec((APTR)pathpart);
                pathpart = NULL;
        }
        return res;
}

int myexecv(bool isperlthread, const char *path, char *argv[])
{
        return myexecve(isperlthread, path, argv, myenviron);
}

int myexecl(bool isperlthread, const char *path, ...)
{
        va_list va;
        char *argv[1024]; /* 1024 enough? let's hope so! */
        int i = 0;
        // adebug("%s %ld\n",__FUNCTION__,__LINE__);

        va_start(va, path);
        i = 0;

        do
        {
                argv[i] = va_arg(va, char *);
        }
        while (argv[i++] != NULL);

        va_end(va);
        return myexecve(isperlthread, path, argv, myenviron);
}

int pause(void)
{
        fprintf(stderr, "Pause not implemented\n");

        errno = EINTR;
        return -1;
}

uint32 size_env(struct Hook *hook, __attribute__((unused))APTR userdata, struct ScanVarsMsg *message)
{
        if (strlen(message->sv_GDir) <= 4)
        {
                hook->h_Data = (APTR)(((uint32)hook->h_Data) + 1);
        }
        return 0;
}

uint32 copy_env(struct Hook *hook, __attribute__((unused))APTR userdata, struct ScanVarsMsg *message)
{
        if (strlen(message->sv_GDir) <= 4)
        {
                char **env = (char **)hook->h_Data;
                uint32 size =
                    strlen(message->sv_Name) + 1 + message->sv_VarLen + 1 + 1;
                char *buffer = (char *)IExec->AllocVecTags((uint32)size,AVT_ClearWithValue,0,TAG_DONE);


                snprintf(buffer, size - 1, "%s=%s", message->sv_Name,
                         message->sv_Var);

                *env = buffer;
                env++;
                hook->h_Data = env;
        }
        return 0;
}

void ___makeenviron()
{
        struct Hook *hook = (struct Hook *)IExec->AllocSysObjectTags(ASOT_HOOK,TAG_DONE);

        if(hook)
        {
                char varbuf[8];
                uint32 flags = 0;

                struct DOSIFace *myIDOS =
                    (struct DOSIFace *)OpenInterface("dos.library", 53);
                if (myIDOS)
                {
                        uint32 size = 0;
                        if (myIDOS->GetVar("ABCSH_IMPORT_LOCAL", varbuf, 8,
                                           GVF_LOCAL_ONLY) > 0)
                        {
                                flags = GVF_LOCAL_ONLY;
                        }
                        else
                        {
                                flags = GVF_GLOBAL_ONLY;
                        }

                        hook->h_Entry = size_env;
                        hook->h_Data = 0;

                        myIDOS->ScanVars(hook, flags, 0);
                        size  = ((uint32)hook->h_Data) + 1;

                        myenviron = (char **)IExec->AllocVecTags(size *
                                    sizeof(char **),
                                    AVT_ClearWithValue,0,TAG_DONE);
                        origenviron = myenviron;
                        if (!myenviron)
                        {
                                IExec->FreeSysObject(ASOT_HOOK,hook);
                                CloseInterface((struct Interface *)myIDOS);
                                return;
                        }
                        hook->h_Entry = copy_env;
                        hook->h_Data = myenviron;

                        myIDOS->ScanVars(hook, flags, 0);
                        IExec->FreeSysObject(ASOT_HOOK,hook);
                        CloseInterface((struct Interface *)myIDOS);
                }
        }
}

void ___freeenviron()
{
        char **i;
        /* perl might change environ, it puts it back except for ctrl-c */
        /* so restore our own copy here */
        struct DOSIFace *myIDOS =
            (struct DOSIFace *)OpenInterface("dos.library", 53);
        if (myIDOS)
        {
                myenviron = origenviron;

                if (myenviron)
                {
                        for (i = myenviron; *i != NULL; i++)
                        {
                                IExec->FreeVec(*i);
                        }
                        IExec->FreeVec(myenviron);
                        myenviron = NULL;
                }
                CloseInterface((struct Interface *)myIDOS);
        }
}


/* Work around for clib2 fstat */
#ifndef S_IFCHR
#define S_IFCHR 0x0020000
#endif

#define SET_FLAG(u, v) ((void)((u) |= (v)))

int afstat(int fd, struct stat *statb)
{
        int result;
        BPTR fh;
        int mode;
        BOOL input;
        /* In the first instance pass it to fstat */
        // adebug("fd %ld ad %ld\n",fd,amigaos_get_file(fd));

        if ((result = fstat(fd, statb) >= 0))
                return result;

        /* Now we've got a file descriptor but we failed to stat it */
        /* Could be a nil: or could be a std#? */

        /* if get_default_file fails we had a dud fd so return failure */
#if !defined(__CLIB2__)

        fh = amigaos_get_file(fd);

        /* if nil: return failure*/
        if (fh == 0)
                return -1;

        /* Now compare with our process Input() Output() etc */
        /* if these were regular files sockets or pipes we had already
         * succeeded */
        /* so we can guess they a character special console.... I hope */

        struct ExamineData *data;
        char name[120];
        name[0] = '\0';

        data = IDOS->ExamineObjectTags(EX_FileHandleInput, fh, TAG_END);
        if (data != NULL)
        {

                IUtility->Strlcpy(name, data->Name, sizeof(name));

                IDOS->FreeDosObject(DOS_EXAMINEDATA, data);
        }

        // adebug("ad %ld '%s'\n",amigaos_get_file(fd),name);
        mode = S_IFCHR;

        if (fh == IDOS->Input())
        {
                input = TRUE;
                SET_FLAG(mode, S_IRUSR);
                SET_FLAG(mode, S_IRGRP);
                SET_FLAG(mode, S_IROTH);
        }
        else if (fh == IDOS->Output() || fh == IDOS->ErrorOutput())
        {
                input = FALSE;
                SET_FLAG(mode, S_IWUSR);
                SET_FLAG(mode, S_IWGRP);
                SET_FLAG(mode, S_IWOTH);
        }
        else
        {
                /* we got a filehandle not handle by fstat or the above */
                /* most likely it's NIL: but lets check */
                struct ExamineData *exd = NULL;
                if ((exd = IDOS->ExamineObjectTags(EX_FileHandleInput, fh,
                                                   TAG_DONE)))
                {
                        BOOL isnil = FALSE;
                        if (exd->Type ==
                                (20060920)) // Ugh yes I know nasty.....
                        {
                                isnil = TRUE;
                        }
                        IDOS->FreeDosObject(DOS_EXAMINEDATA, exd);
                        if (isnil)
                        {
                                /* yep we got NIL: */
                                SET_FLAG(mode, S_IRUSR);
                                SET_FLAG(mode, S_IRGRP);
                                SET_FLAG(mode, S_IROTH);
                                SET_FLAG(mode, S_IWUSR);
                                SET_FLAG(mode, S_IWGRP);
                                SET_FLAG(mode, S_IWOTH);
                        }
                        else
                        {
                                IExec->DebugPrintF(
                                    "unhandled filehandle in afstat()\n");
                                return -1;
                        }
                }
        }

        memzero(statb, sizeof(statb));

        statb->st_mode = mode;

#endif
        return 0;
}

BPTR amigaos_get_file(int fd)
{
        BPTR fh = (BPTR)NULL;
        if (!(fh = _get_osfhandle(fd)))
        {
                switch (fd)
                {
                case 0:
                        fh = IDOS->Input();
                        break;
                case 1:
                        fh = IDOS->Output();
                        break;
                case 2:
                        fh = IDOS->ErrorOutput();
                        break;
                default:
                        break;
                }
        }
        return fh;
}

/*########################################################################*/

#define LOCK_START 0xFFFFFFFFFFFFFFFELL
#define LOCK_LENGTH 1LL

// No wait forever option so lets wait for a loooong time.
#define TIMEOUT 0x7FFFFFFF

int amigaos_flock(int fd, int oper)
{
        BPTR fh;
        int32 success = -1;

        if (!(fh = amigaos_get_file(fd)))
        {
                errno = EBADF;
                return -1;
        }

        switch (oper)
        {
        case LOCK_SH:
        {
                if (IDOS->LockRecord(fh, LOCK_START, LOCK_LENGTH,
                                     REC_SHARED | RECF_DOS_METHOD_ONLY,
                                     TIMEOUT))
                {
                        success = 0;
                }
                break;
        }
        case LOCK_EX:
        {
                if (IDOS->LockRecord(fh, LOCK_START, LOCK_LENGTH,
                                     REC_EXCLUSIVE | RECF_DOS_METHOD_ONLY,
                                     TIMEOUT))
                {
                        success = 0;
                }
                break;
        }
        case LOCK_SH | LOCK_NB:
        {
                if (IDOS->LockRecord(fh, LOCK_START, LOCK_LENGTH,
                                     REC_SHARED_IMMED | RECF_DOS_METHOD_ONLY,
                                     TIMEOUT))
                {
                        success = 0;
                }
                else
                {
                        errno = EWOULDBLOCK;
                }
                break;
        }
        case LOCK_EX | LOCK_NB:
        {
                if (IDOS->LockRecord(fh, LOCK_START, LOCK_LENGTH,
                                     REC_EXCLUSIVE_IMMED | RECF_DOS_METHOD_ONLY,
                                     TIMEOUT))
                {
                        success = 0;
                }
                else
                {
                        errno = EWOULDBLOCK;
                }
                break;
        }
        case LOCK_UN:
        {
                if (IDOS->UnLockRecord(fh, LOCK_START, LOCK_LENGTH))
                {
                        success = 0;
                }
                break;
        }
        default:
        {
                errno = EINVAL;
                return -1;
        }
        }
        return success;
}
