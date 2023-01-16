/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/11 17:13:01 by jchene            #+#    #+#             */
/*   Updated: 2023/01/16 19:49:41 by jchene           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//-----HEADERS-----//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//-----STATUS CODES-----//

#define FAILURE -1
#define CD_BAD_ARG -2
#define FAILED_EXEC -3
#define INIT 0
#define END_OF_CMD 1
#define END_OF_LINE 2

//-----DATA-----//

#define IN 0
#define OUT 1

#define DATA data()
#define ARGC DATA->argc
#define ARGV DATA->argv
#define ENVP DATA->envp
#define E_STATUS DATA->exit_status

typedef struct s_child
{
	int		child_pid;	
	char	*cmd;
	char	**args;
	int		active_pipe[2];
}	t_child;

typedef struct s_data
{
	char	**argv;
	char	**envp;
	int		argc;
	int		pipes[2][2];
	int		exit_status;
	int		std_fds[2];
	t_child	childs[2];
}	t_data;

t_data *data(void)
{
	static t_data data;
	return (&data);
}
//-----			-----//

const char	*get_status(int status)
{
	if (status == FAILURE)
		return ("FAILURE");
	else if (status == CD_BAD_ARG)
		return ("CD_BAD_ARG");
	else if (status == FAILED_EXEC)
		return ("FAILED EXEC");
	else if (status == INIT)
		return ("INIT");
	else if (status == END_OF_CMD)
		return ("END_OF_CMD");
	else if (status == END_OF_LINE)
		return ("END_OF_LINE");
	return ("");
}

// Set le status
int	set_status(int status)
{
	fprintf(stderr, "Setting status to: %s\n", get_status(status));
	E_STATUS = status;
	return(status);
}

// Initialise le child avec les valeurs par defaut
void init_child(t_child *ch)
{
	ch->child_pid = -1;
	ch->cmd = NULL;
	ch->args = NULL;
	ch->active_pipe[IN] = 0;
	ch->active_pipe[OUT] = 0;
}

int init_data(int argc, char **argv, char **envp)
{
	ARGC = argc;
	ARGV = argv;
	ENVP = envp;
	fprintf(stderr, "ARGC: %d\n", argc);
	E_STATUS = INIT;
	init_child(&DATA->childs[0]);
	init_child(&DATA->childs[1]);
	if (pipe(DATA->pipes[0]) == -1)
		return (set_status(FAILURE));
	fprintf(stderr, "First pipe: in: %d out: %d\n", DATA->pipes[0][IN], DATA->pipes[0][OUT]);
	if (pipe(DATA->pipes[1]) == -1)
		return (set_status(FAILURE));
	fprintf(stderr, "Second pipe: in: %d out: %d\n", DATA->pipes[1][IN], DATA->pipes[1][OUT]);
	DATA->std_fds[IN] = dup(STDIN_FILENO);
	if (DATA->std_fds[IN] == -1)
		return (set_status(FAILURE));
	fprintf(stderr, "Saved STDIN\n");
	DATA->std_fds[OUT] = dup(STDOUT_FILENO);
	if (DATA->std_fds[OUT] == -1)
		return (set_status(FAILURE));
	fprintf(stderr, "Saved STDOUT\n");
	return (E_STATUS);
}

// Free les childs et set le status a status
int destructor(int status)
{
	fprintf(stderr, "Destroying with status: %d\n", status);
	if (DATA->childs[0].args)
	{
		free(DATA->childs[0].args);
		DATA->childs[0].args = NULL;
	}
	if (DATA->childs[1].args)
	{
		free(DATA->childs[1].args);
		DATA->childs[1].args = NULL;
	}
	close(DATA->std_fds[IN]);
	close(DATA->std_fds[OUT]);
	return (status);
}

int fill_child(t_child *ch, int child_id, int *pos)
{
	int i = 0;

	fprintf(stderr, "Filling child: %d at pos %d\n", child_id, *pos);
	if (*pos > 0 && !strcmp(ARGV[(*pos + 1) - 1], "|")) //Si pipe en entrée
	{
		ch->active_pipe[IN] = 1;
		if (dup2(DATA->pipes[1 - child_id][OUT], STDIN_FILENO) == -1) //Branche l'input du child sur le pipe de numero inverse au child
			return (set_status(FAILURE));
		fprintf(stderr, "		Input connected %d\n", DATA->pipes[1 - child_id][OUT]);
	}
	while (i + *pos + 2 < ARGC && strcmp(ARGV[i + *pos + 2], ";") && strcmp(ARGV[i + *pos + 2], "|")) // Compte le nombre d'arguments
	{
		i++;
	}
	fprintf(stderr, "Counted %d args\n", i);
	ch->args = (char **)malloc(sizeof(char *) * (i + 1)); //Alloue la tableau de pointeurs sur arguments
	if (!ch->args) // Si l'allocation a échouée
		return(set_status(FAILURE));	
	fprintf(stderr, "Malloc ok\nI:%d\n", i);
	i = 0;
	ch->cmd = ARGV[*pos + 1];	// Recupere la commande
	while (i + *pos + 2 < ARGC && strcmp(ARGV[i + *pos + 2], ";") && strcmp(ARGV[i + *pos + 2], "|")) // Recupere les arguments
	{
		ch->args[i] = ARGV[i + *pos + 2];
		i++;
	}
	fprintf(stderr, "Command: %s\nArg1: %s\nI: %d\n", ch->cmd, ch->args[0], i);
	if (i + *pos + 2 < ARGC && (!strcmp(ARGV[i + *pos + 2], "|") || !strcmp(ARGV[i + *pos + 2], ";"))) //So on arrive a la fin de la commande on set le status
		set_status(END_OF_CMD);

	if (i + *pos + 2 < ARGC && !strcmp(ARGV[i + *pos + 2], "|")) //Si pipe de sortie crée le pipe et le branche
	{
		ch->active_pipe[OUT] = 1;
		fprintf(stderr, "kjwebcwbekwbekjbwckjbckjwbcwejkbcwekjcb\n");
		if (dup2(DATA->pipes[child_id][IN], STDOUT_FILENO) == -1) //Branche l'input du child sur le pipe de numero inverse au child
			return (set_status(FAILURE));
		fprintf(stderr, "		Output connected: %d\n", DATA->pipes[child_id][IN]);
	}
	if (i + *pos + 2 >= ARGC) //Si on est arrivé a la fin de ligne set le status
		set_status(END_OF_LINE);
	if (E_STATUS == END_OF_CMD)
		fprintf(stderr, "End of command\n");
	else if (E_STATUS == END_OF_LINE)
		fprintf(stderr, "End of line\n");
	*pos += i;
	if (E_STATUS == END_OF_CMD)
		*pos += 2;
	fprintf(stderr, "Pos now at: %d\n", *pos);
	return (E_STATUS);
}

int launch_child(t_child *ch, int child_id, int pos)
{
	int i = 0;
	ch->child_pid = fork();
	if (ch->child_pid == 0)
	{
		fprintf(stderr, "pid child: %d\n", getpid());
		execve(ch->cmd, ch->args, ENVP);
		exit(0);
	}
	else
	{
		fprintf(stderr, "pid parent: %d\n", getpid());
		if (ch->active_pipe[IN])
		{
			close(DATA->pipes[1 - child_id][IN]);
			fprintf(stderr, "closed: %d\n", DATA->pipes[1 - child_id][IN]);
			close(DATA->pipes[1 - child_id][OUT]);
			fprintf(stderr, "closed: %d\n", DATA->pipes[1 - child_id][OUT]);
			while (i + pos + 2 < ARGC && strcmp(ARGV[i + pos + 2], "|") && strcmp(ARGV[i + pos + 2], ";"))
				i++;
			fprintf(stderr, "iiiiiiii: %d\n", i);
			if (i + pos + 2 < ARGC && !strcmp(ARGV[i + pos + 2], "|"))
			{
				pipe(DATA->pipes[1 - child_id]);
				fprintf(stderr, "New pipe: in: %d out: %d\n", DATA->pipes[1 - child_id][IN], DATA->pipes[1 - child_id][OUT]);
			}
		}
		dup2(DATA->std_fds[IN], STDIN_FILENO);
		dup2(DATA->std_fds[IN], STDIN_FILENO);
		fprintf(stderr, "reset to std fds\n");
	}
	return (0);
}

int process_args()
{
	int i = 0;
	int child_id = 0;

	while (E_STATUS != END_OF_LINE)
	{
		fprintf(stderr, "----------------------------\n");
		init_child(&(DATA->childs[child_id]));	// Initialise le child avec les valeurs par defaut
		fill_child(&(DATA->childs[child_id]), child_id, &i);	// Alloue, remplis le child avec la commande et les arguments et branche les pipes
		if (E_STATUS == FAILURE)
			return (destructor(FAILURE));
		launch_child(&(DATA->childs[child_id]), child_id, i);
		child_id = 1 - child_id;
	}
	return (E_STATUS);
}

int main(int argc, char **argv, char **envp)
{
	if (argc < 2 || !*envp)
		return (-1);
	init_data(argc, argv, envp);
	if (E_STATUS == FAILURE)
		return (destructor(FAILURE));
	process_args();
	return (destructor(E_STATUS));
}