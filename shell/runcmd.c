#include "runcmd.h"

int status = 0;
pid_t last_pid = 0;  // PID del ultimo proceso lanzado en background (para $!)
struct cmd *parsed_pipe;

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

	// forks and run the command
	if ((p = fork()) == 0) {
		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		setpgid(0, 0);
		if (parsed->type == PIPE)
			parsed_pipe = parsed;
		exec_cmd(parsed);
	}

	setpgid(p, p);
	// stores the pid of the process
	parsed->pid = p;

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