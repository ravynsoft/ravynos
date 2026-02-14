#include <darwintest.h>

#include <unistd.h>
#include <readpassphrase.h>

T_DECL(readpassphrase_stdin, "readpassphrase_stdin")
{
	int stdin_pipe[2] = { 0 };
	char pwd[] = "ishouldnotbedoingthis\n";
	char buff[128];

	T_ASSERT_POSIX_ZERO(pipe(stdin_pipe),
			"must be able to create a pipe");
	T_ASSERT_EQ(STDIN_FILENO, dup2(stdin_pipe[0], STDIN_FILENO),
			"must be able to re-register the read end of the pipe with STDIN_FILENO");
	T_ASSERT_EQ((ssize_t) sizeof(pwd), write(stdin_pipe[1], pwd, sizeof(pwd)),
			"must be able to write into the pipe");
	T_ASSERT_EQ((void *) buff, (void *) readpassphrase("", buff, sizeof(buff), RPP_STDIN),
			"readpassphrase must return its buffer argument on success");
	// readpassphrase eats newlines
	pwd[sizeof(pwd) - 2] = 0;
	T_ASSERT_EQ_STR(buff, pwd,
			"readpassphrase with RPP_STDIN must capture stdin");
}

