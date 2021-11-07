/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inter.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/07 15:31:36 by jchene            #+#    #+#             */
/*   Updated: 2021/11/07 15:58:50 by jchene           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_strlen(char *str)
{
	int i;

	if (!str)
		return (0);
	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_bzero(void *p, unsigned int n)
{
	unsigned char	*ptr;

	ptr = (unsigned char *)p;
	while (n-- > 0)
		*(ptr++) = '\0';
}

int	main(int argc, char **argv)
{
	int		i;
	int		j;
	int		k;
	char	seen[ft_strlen(argv[1]) + 1];

	if (argc != 3)
	{
		write(1, "\n", 1);
		return (-1);
	}
	i = 0;
	ft_bzero(seen, ft_strlen(argv[1]) + 1);
	while (argv[1][i])
	{
		j = 0;
		while (argv[2][j])
		{
			if (argv[1][i] == argv[2][j])
			{
				k = 0;
				while (seen[k] && seen[k] != argv[1][i])
					k++;
				if (seen[k])
					break ;
				else
				{
					write(1, &argv[1][i], 1);
					seen[ft_strlen(seen)] = argv[1][i];
					break ;
				}
			}
			j++;
		}
		i++;
	}
	write(1, "\n", 1);
	return (0);
}
