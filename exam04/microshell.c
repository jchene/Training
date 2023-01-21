/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/11 17:13:01 by jchene            #+#    #+#             */
/*   Updated: 2023/01/21 18:41:55 by jchene           ###   ########.fr       */
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
#define INIT 0
#define END_OF_CMD 1
#define END_OF_PIPELINE 2
#define END_OF_LINE 3

//-----DATA-----//

#define IN 0
#define OUT 1
#define PIN 1
#define POUT 0

#define DATA data()
#define E_STATUS DATA->exit_status

typedef struct s_child
{
	char	*cmd;
	char	**args;
	int		ends[2];
}	t_child;

typedef struct s_data
{
	int		pipes[2][2];
	int		exit_status;
	t_child	childs[2];
}	t_data;

t_data *data(void)
{
	static t_data data;
	return (&data);
}
//-----			-----//

int	set_status(int status)
{
	E_STATUS = status;
	return(status);
}

void init_data()
{
	E_STATUS = INIT;
	for (int i = 0; i < 2; i++)
	{
		DATA->childs[i].args = NULL;
		for (int j = 0; j < 2; j++)
			DATA->pipes[i][j] = -1;
	}
}

void init_child(t_child *ch)
{
	ch->cmd = NULL;
	ch->args = NULL;
	ch->ends[IN] = -1;
	ch->ends[OUT] = -1;
}

void mWrite(int fd, char *str1, char *str2)
{
	int i = 0;
	while (str1[i])
		write(fd, &str1[i++], 1);
	i = 0;
	while (str2[i])
		write(fd, &str2[i++], 1);
	write(fd, "\n", 1);
}

void *cmalloc(size_t size)
{
	void *ptr = malloc(size);
	for (size_t i = 0; i < size; i++)
		((unsigned char *)ptr)[i] = '\0';
	return (ptr);
}

int cclose(int *fd)
{
	int ret = close(*fd);
	*fd = -1;
	return (ret);
}

int destructor(int status)
{
	for (int i = 0; i < 2; i++)
	{
		if (DATA->childs[i].args)
		{
			free(DATA->childs[i].args);
			DATA->childs[i].args = NULL;
		}
		for (int j = 0; j < 2; j++)
		{
			if (DATA->pipes[i][j] != -1)
				cclose(&DATA->pipes[i][j]);
		}
	}
	return (status);
}

int fill_child(t_child *ch, int child_id, int *pos, int argc, char**argv)
{
	int i = 0;

	if (*pos > 0 && !strcmp(argv[(*pos + 1) - 1], "|")) //Si pipe en entrée
		ch->ends[IN] = DATA->pipes[1 - child_id][POUT];
	else
		ch->ends[IN] = -1;
	while (i + *pos + 1 < argc && strcmp(argv[i + *pos + 1], ";") && strcmp(argv[i + *pos + 1], "|")) // Compte le nombre d'arguments
		i++;
	ch->args = (char **)cmalloc(sizeof(char *) * (i + 1)); //Alloue la tableau de pointeurs sur arguments
	if (!ch->args) // Si l'allocation a échouée
		return(set_status(FAILURE));
	for (int j = 0; j < i; j++) //Recupere les arguments
		ch->args[j] = argv[j + *pos + 1];
	ch->cmd = argv[*pos + 1]; //Recupere la commande
	
	if (i + *pos + 1 < argc && !strcmp(argv[i + *pos + 1], "|")) //Si pipe de sortie crée le pipe et le branche
	{
		if (pipe(DATA->pipes[child_id]) == -1)
			return(set_status(FAILURE));
		ch->ends[OUT] = DATA->pipes[child_id][PIN];
	}
	else
		ch->ends[OUT] = -1;
	if ((i + *pos + 1 >= argc))
		E_STATUS = END_OF_LINE;
	else if (!strcmp(argv[i + *pos + 1], ";"))
		E_STATUS = END_OF_PIPELINE;
	else if (!strcmp(argv[i + *pos + 1], "|"))
		E_STATUS = END_OF_CMD;
	if (i + *pos + 1 < argc)
		*pos += 2;
	*pos += i - 1;
	return (E_STATUS);
}

void execcd(t_child *ch)
{
	int nb_args = 0;
	if (!ch->args)
		return;
	while (ch->args[nb_args])
		nb_args++;
	if (nb_args != 2)
	{
		mWrite(STDERR_FILENO, "error: cd: bad arguments", "");
		return;
	}
	if (chdir(ch->args[1]) == -1)
		mWrite(STDERR_FILENO, "error: cd: cannot change path directory to ", ch->args[1]);
	return;
}

void launch_child(t_child *ch, int child_id, char **envp)
{
	if (!strcmp(ch->cmd, "cd"))
	{
		execcd(ch);
		return;
	}
	int pid = fork();
	if (pid == 0) //Si on est dans le child
	{
		for(int j = IN; j < OUT + 1; j++) //Relie les pipes si besoin
			if (ch->ends[j] != -1)
				dup2(ch->ends[j], j);
		execve(ch->cmd, ch->args, envp); //Execute la commande
		mWrite(STDERR_FILENO, "error: cannot execute ", ch->cmd);
		if (ch->ends[IN] != -1)
		{
			cclose(&ch->ends[IN]);
			close(STDIN_FILENO);
		}
		if (ch->ends[OUT] != -1)
		{
			cclose(&ch->ends[OUT]);
			close(STDOUT_FILENO);
		}
		destructor(E_STATUS);
		exit(strcmp(ch->cmd, "cd") ? EXIT_FAILURE : EXIT_SUCCESS);
	}
	else //Si on est dans le parent
	{
		if (ch->ends[OUT] != -1)
			cclose(&(DATA->pipes[child_id][PIN]));
		if (ch->ends[IN] != -1)
			cclose(&(DATA->pipes[1 - child_id][POUT]));
		if (ch->args)
		{
			free(ch->args);
			ch->args = NULL;
		}
	}
}

void process_args(int argc, char **argv, char **envp)
{
	int i = 0;
	int child_id = 0;
	unsigned int nb_process = 0;

	while (E_STATUS != END_OF_LINE)
	{
		init_child(&(DATA->childs[child_id]));	// Initialise le child avec les valeurs par defaut
		fill_child(&(DATA->childs[child_id]), child_id, &i, argc, argv);	// Alloue, remplis le child avec la commande et les arguments et branche les pipes
		if (E_STATUS == FAILURE)
			return;
		launch_child(&(DATA->childs[child_id]), child_id, envp);
		child_id = 1 - child_id;
		nb_process++;
		if (E_STATUS == END_OF_PIPELINE || E_STATUS == END_OF_LINE)
		{
			for (unsigned int j = 0; j < nb_process; j++)
				waitpid(-1, NULL, 0);
			nb_process = 0;
		}
	}
	return;
}

int main(int argc, char **argv, char **envp)
{
	if (argc < 2 || !*envp)
		return (-1);
	init_data();
	process_args(argc, argv, envp);
	if (E_STATUS == FAILURE)
		mWrite(STDERR_FILENO, "error: fatal", "");
	return (destructor(E_STATUS));
}
