/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/11 17:13:01 by jchene            #+#    #+#             */
/*   Updated: 2023/01/14 17:45:09 by jchene           ###   ########.fr       */
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

//Initialise le child avec les valeurs par defaut
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
	DATA->pipes[0][IN] = -1;
	DATA->pipes[0][OUT] = -1;
	DATA->pipes[1][IN] = -1;
	DATA->pipes[1][OUT] = -1;
	E_STATUS = SUCCESS;
	init_child(&DATA->childs[0]);
	init_child(&DATA->childs[1]);
}

//-----			-----//

//Free les childs
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
	while (i + pos + 1 < ARGC && strcmp(ARGV[i + pos + 1], ";") && strcmp(ARGV[i + pos + 1], "|")) //Compte le nombre d'arguments
	{
		i++;
	}
	ch->args = (char**)malloc(sizeof(char *) * (i + 1));
	if (!ch->args) //Si l'allocation a échouée
	{
		destructor(FAILURE); //Free les childs alloués et set le status a Failure
		return;
	}
	ch->cmd = ARGV[pos + 1];
	for (int j = 0; j < i; j++) //Remplis les argument du child
	{
		ch->args[j] = ARGV[pos + j + 1];
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
		init_child(&(DATA->childs[child_id]));	//Initialise le child avec les valeurs par defaut
		
		if (i > 0 && strcmp(ARGV[i - 1], "|") == 0)	//Check du fd d'entree si pipe
			DATA->childs[child_id].fds[IN] = DATA->pipes[1 - child_id][OUT];
		
		while (j < ARGC && strcmp(ARGV[j], ";") && strcmp(ARGV[j], "|")) //Check si pipe de sortie
		{
			j++;
		}
		if (j < ARGC && !strcmp(ARGV[i], "|")) //Si pipe de sortie crée le pipe et le branche
		{
			if (pipe(DATA->pipes[child_id]))
			{
				destructor(FAILURE);
				return;
			}
			DATA->childs[child_id].fds[OUT] = DATA->pipes[child_id][IN];
		}

		start_child(&(DATA->childs[child_id]), i); //Alloue et remplis le child avec la commande et les arguments
		if (E_STATUS == FAILURE) //Verifie si l'allocation s'est bien deroulée
			return;
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
