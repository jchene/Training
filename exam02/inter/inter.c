/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inter.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/08 01:36:45 by jchene            #+#    #+#             */
/*   Updated: 2021/11/08 01:46:02 by jchene           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_strlen(char *str)
{
	int	i;

	if (!str)
		return (0);
	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_bzero(void *p, unsigned int size)
{
	unsigned char	*ptr;

	ptr = (unsigned char *)p;
	while (size-- > 0)
		*(ptr++) = 0;
}

int	is_charset(char *charset, char c)
{
	int	i;

	if (!charset)
		return (0);
	i = 0;
	while (charset[i])
	{
		if (charset[i] == c)
			return (1);
		i++;
	}
	return (0);
}

int	main(int argc, char **argv)
{
	int		i;
	int		k;
	char	seen[ft_strlen(argv[1]) + ft_strlen(argv[2]) + 1];

	if (argc != 3)
	{
		write(1, "\n", 1);
		return (-1);
	}
	ft_bzero(seen, ft_strlen(argv[1]) + ft_strlen(argv[2]) + 1);
	while (argv[1][i])
	{
		if (is_charset(argv[2], argv[1][i]))
		{
			if (!is_charset(seen, argv[1][i]))
			{
				write(1, &argv[1][i], 1);
				seen[ft_strlen(seen)] = argv[1][i];
			}
		}
		i++;
	}
	write(1, "\n", 1);
	return (0);
}