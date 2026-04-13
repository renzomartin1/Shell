#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

void
sigchild_handler(int signum)
{
	pid_t pid;
	int status;

	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		char str[BUFLEN] = { 0 };

		snprintf(str, sizeof(str), "==> terminado: PID=%d\n", pid);
		write(STDOUT_FILENO, str, strlen(str));
	}
}


static void
init_signals()
{
	stack_t sig_stack;

	memset(&sig_stack, 0, sizeof(sig_stack));

	sig_stack.ss_sp = malloc(SIGSTKSZ);
	if (sig_stack.ss_sp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	sig_stack.ss_size = SIGSTKSZ;
	sig_stack.ss_flags = 0;

	if (sigaltstack(&sig_stack, NULL) == -1) {
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}


	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigchild_handler;
	sa.sa_flags = SA_RESTART | SA_ONSTACK;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction");
	}
}

int
main(void)
{
	init_shell();
	init_signals();
	run_shell();

	return 0;
}