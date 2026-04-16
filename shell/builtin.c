#include "builtin.h"
#include <unistd.h>

// parses an argument of the command stream input
static char *
get_command_argument(char *buf, int idx)
{
	char *tok;
	int i;

	tok = (char *) calloc(ARGSIZE, sizeof(char));
	i = 0;

	while (buf[idx] != SPACE && buf[idx] != END_STRING) {
		tok[i] = buf[idx];
		i++;
		idx++;
	}

	return tok;
}

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	char *token = get_command_argument(cmd, 0);
	if (strcmp(token, "exit") == 0) {
		free(token);
		return true;
	}
	free(token);
	return false;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char *token = get_command_argument(cmd, 0);
	if (strcmp(token, "cd") == 0) {
		if (strlen(cmd) > 3) {
			char *dir = get_command_argument(cmd, 3);
			if (chdir(dir) < 0) {
				char errmsg[ARGSIZE];
				snprintf(errmsg,
				         sizeof(errmsg),
				         "cannot cd to %s ",
				         dir);
				perror(errmsg);
			}
			free(dir);
		} else {
			if (chdir(getenv("HOME")) < 0) {
				char errmsg[ARGSIZE];
				snprintf(errmsg,
				         sizeof(errmsg),
				         "cannot cd to %s ",
				         getenv("HOME"));
				perror(errmsg);
			}
		}

		char *cwd = getcwd(NULL, 0);
		if (cwd != NULL) {
			snprintf(prompt, sizeof(prompt), "(%s)", cwd);
		}
		free(cwd);
		free(token);
		return true;
	}

	free(token);
	return false;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	char *token = get_command_argument(cmd, 0);
	if (strcmp(token, "pwd") == 0) {
		char *cwd = getcwd(NULL, 0);
		if (cwd != NULL) {
			printf("%s\n", cwd);
			free(cwd);
		} else {
			perror("getcwd() error");
		}
		free(token);
		return true;
	}
	free(token);
	return false;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	char *token = get_command_argument(cmd, 0);
	if (strcmp(token, "history") == 0) {
		free(token);
		return true;
	}
	free(token);
	return false;
}
