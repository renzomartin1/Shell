#include "runcmd.h"

int status = 0;
struct cmd *parsed_pipe;
pid_t last_pid = 0;
static int is_background = 0;


// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;


	// if the "enter" key is pressed
	// just print the prompt again
	if (cmd[0] == END_STRING)
		return 0;

	// "history" built-in call
	if (history(cmd)) {
		status = 0;  // los built-ins no pasan por waitpid, hay que actualizar status a mano
		return 0;
	}

	// "cd" built-in call
	if (cd(cmd)) {
		status = 0;
		return 0;
	}

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" built-in call
	if (pwd(cmd)) {
		status = 0;
		return 0;
	}

	// parses the command line
	parsed = parse_line(cmd);

	is_background = (parsed->type == BACK);

	// forks and run the command
	if ((p = fork()) == 0) {
		if (parsed->type != BACK)
			setpgid(0, 0);
		if (parsed->type == PIPE)
			parsed_pipe = parsed;
		exec_cmd(parsed);
		_exit(1);
	}


	if (parsed->type != BACK)
		setpgid(p, p);

	parsed->pid = p;
	last_pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is
	//		going to be run in the 'back'
	// - print info about it with
	// 	'print_back_info()'
	//
	if (parsed->type == BACK) {
		struct backcmd *b = (struct backcmd *) parsed;
		b->c->pid = p;
		print_back_info(b->c);
		free_command(parsed);
		return 0;
	}


	// waits for the process to finish
	waitpid(p, &status, 0);

	print_status_info(parsed);

	free_command(parsed);

	return 0;
}