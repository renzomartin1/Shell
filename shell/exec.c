#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// aplico VAR=valor al entorno del hijo antes del exec
        //  y se llama despues del fork para no contaminar shell padre
	char key[ARGSIZE], value[ARGSIZE];
	for (int i = 0; i < eargc; i++) {
		int idx = block_contains(eargv[i], '=');
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);
		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd = open(file,flags,0664);
        if(fd < 0){
                perror("open error");
                _exit(1);
        }
        return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		e = (struct execcmd *) cmd;
		if (e->argv[0] == NULL) {
			_exit(0);
		}
		set_environ_vars(e->eargv, e->eargc); // las variables temporarias 
		execvp(e->argv[0], e->argv);
		perror("exec failed ");
		_exit(1);

	case BACK: {
		// runs a command in background
		//
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here

		 struct execcmd *r = (struct execcmd *) cmd;
                int fd;


                if(strlen(r->out_file) > 0){
                        fd = open_redir_fd(r->out_file, O_CREAT | O_WRONLY | O_TRUNC);
                        dup2(fd,STDOUT_FILENO);
                        close(fd);
                }


                if(strlen(r->in_file) > 0){
                        fd = open_redir_fd(r->in_file, O_RDONLY);
                        dup2(fd,STDIN_FILENO);
                        close(fd);
                }


                if(strlen(r->err_file) > 0){
                        if(strcmp(r->err_file,"&1") == 0){
                                dup2(STDOUT_FILENO,STDERR_FILENO);
                        }
                        else{
                                fd = open_redir_fd(r->err_file, O_CREAT | O_WRONLY | O_TRUNC);
                                dup2(fd,STDERR_FILENO);
                                close(fd);
                        }
                }


                execvp(r->argv[0], r->argv);
                perror("execvp error");

                //printf("Redirections are not yet implemented\n");
                _exit(-1);
                break;
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here

                struct pipecmd *p = (struct pipecmd *) cmd;

                int fd[2];

                if(pipe(fd) < 0){
                        perror("pipe error");
                        _exit(1);
                }

                pid_t left = fork();
                if(left == 0){
                        dup2(fd[1],STDOUT_FILENO);
                        close(fd[0]);
                        close(fd[1]);

                        exec_cmd(p->leftcmd);
                        _exit(1);
                }

                pid_t right = fork();
                if(right == 0){
                        dup2(fd[0],STDIN_FILENO);
                        close(fd[1]);
                        close(fd[0]);

                        exec_cmd(p->rightcmd);
                        _exit(1);
                }

                close(fd[0]);
                close(fd[1]);

                waitpid(left, NULL,0);
                waitpid(right,NULL,0);
                //printf("Pipes are not yet implemented\n");

                // free the memory allocated
                // for the pipe tree structure
                //free_command(parsed_pipe);

                break;


		}
	}
}
