/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/11 17:13:01 by jchene            #+#    #+#             */
/*   Updated: 2023/01/12 20:24:52 by jchene           ###   ########.fr       */
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

//-----ERROR CODES-----//

#define FAILURE -1
#define CD_BAD_ARG -2
#define FAILED_EXEC -3
#define SUCCESS 0

//-----GLOBAL DATA-----//

#define IN 0
#define OUT 1

#define DATA data()
#define ARGC data()->argc
#define ARGV data()->argv
#define ENVP data()->envp
#define E_STATUS data()->exit_status

typedef struct s_child
{
	int		fds[2];
	char	*cmd;
	char	**args;
}	child;

typedef struct s_data
{
	int		argc;
	char	**argv;
	char	**envp;
	int		pipes[2][2];
	int		exit_status;
	child	childs[2];
}	t_data;

t_data	*data(void)
{
	static t_data	data;
	return (&data);
}

void init_child(child *ch)
{
	ch->fds[IN] = -1;
	ch->fds[OUT] = -1;
	ch->cmd = NULL;
	ch->args = NULL;
}

void	init_data(int argc, char **argv, char **envp)
{
	ARGC = argc;
	ARGV = argv;
	ENVP = envp;
	PIP1_RD = -1;
	PIP1_WR = -1;
	PIP2_RD = -1;
	PIP2_WR = -1;
	E_STATUS = SUCCESS;
	init_child(&DATA->childs[0]);
	init_child(&DATA->childs[1]);
}

//-----			-----//

int destructor(int status)
{
	if (DATA->childs[0].args)
	{
		free(DATA->childs[0].args);
		DATA->childs[0].args = NULL;
	}
	if (DATA->childs[1].args)
	{
		free(DATA->childs[1].args);
		DATA->childs[0].args = NULL;
	}
	E_STATUS = status;
	return (status);
}

void	start_child(child *ch, int pos)
{
	int i = 0;
	while (i + pos < ARGC)
	{
		if (strcmp(ARGV[i + pos], ";") == 0)
			break;
		i++;
	}
	ch->args = (char**)malloc(sizeof(char *) * i + 1);
	if (!ch->args)
	{
		destructor(FAILURE);
		return;
	}
	for (int j = 0; j < i; j++)
	{
		ch->args[j] = NULL;
	}
}

void process_args()
{
	int	i = 0;
	int j = 0;
	int	child_id = 0;

	while (1)
	{
		j = 0;
		init_child(&(DATA->childs[child_id]));
		start_child(&(DATA->childs[child_id]), i);
		if (E_STATUS == FAILURE)
			return;
		if (i > 0 && strcmp(ARGV[i - 1], "|") == 0)
			DATA->childs[child_id].fds[IN] = DATA->pipes[1 - child_id][OUT];
		if (i == 0 || strcmp(ARGV[i - 1], ";") == 0)
			DATA->childs[child_id].cmd = ARGV[i];
		else
		{
			while (i < ARGC && strcmp(ARGV[i], ";") && strcmp(ARGV[i], "|"))
				DATA->childs[child_id].args[j++] = ARGV[i++];
		}
		if (i < ARGC && strcmp(ARGV[i], "|") == 0)
		{
			if (pipe(DATA->pipes[child_id]))
			{
				destructor(FAILURE);
				return;
			}
		}
	}
}

int	main(int argc, char **argv, char **envp)
{
	if (argc < 2 || !*envp)
		return (-1);
	init_data(argc, argv, envp);
	process_args();
	if (E_STATUS == FAILURE)
		write(2, "error: fatal\n", 13);
	return (destructor(E_STATUS));
}
