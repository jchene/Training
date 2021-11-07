/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/06 22:14:13 by jchene            #+#    #+#             */
/*   Updated: 2021/11/07 01:49:55 by jchene           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./get_next_line.h"

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	if (!str)
		return (i);
	while (str[i])
		i++;
	return (i);
}

void	ft_bzero(void *b, size_t n)
{
	unsigned char	*ptr;

	ptr = (unsigned char *)b;
	while (n-- > 0)
		*(ptr++) = 0;
}

void	ft_append(char *dst, char *src)
{
	int	i;
	int	j;

	if (!src)
		return ;
	i = 0;
	j = 0;
	while (dst[i] != '\0')
		i++;
	while (src[j])
	{
		dst[i] = src[j];
		i++;
		j++;
	}
	dst[i] = '\0';
	return ;
}

char	*ft_strjoin(char *line, char *buffer)
{
	char			*array;
	unsigned int	size;

	if (!line && !buffer)
		return (NULL);
	size = ft_strlen(line) + ft_strlen(buffer);
	array = malloc(sizeof(char) * (size + 1));
	if (!array)
		return (NULL);
	ft_bzero(array, size + 1);
	ft_append(array, line);
	ft_append(array, buffer);
	free(line);
	return (array);
}

char	*get_line(char *remains)
{
	int		i;
	char	*line;

	i = 0;
	while (remains[i] && remains[i] != '\n')
		i++;
	line = malloc(sizeof(char) * (i + 1));
	if (!line)
		return (NULL);
	i = 0;
	while (remains[i] && remains[i] != '\n')
	{
		line[i] = remains[i];
		i++;
	}
	line[i] = '\0';
	return (line);
}

char	*cut_line(char *remains)
{
	int		i;
	int		size;
	char	*new_remains;

	i = 0;
	while (remains[i] && remains[i] != '\n')
		i++;
	if (!remains[i])
	{
		free(remains);
		return (NULL);
	}
	size = ft_strlen(remains);
	new_remains = malloc(sizeof(char) * (size - i + 1));
	if (!new_remains)
		return (NULL);
	ft_bzero(new_remains, size - i + 1);
	ft_append(new_remains, &remains[i + 1]);
	new_remains[size - i] = '\0';
	free(remains);
	remains = NULL;
	return (new_remains);
}

int	get_next_line(char **line)
{
	int			fd;
	int			readed;
	char		buffer[BUFFER_SIZE + 1];
	static char	*remains;

	fd = 0;
	readed = 1;
	buffer[0] = 0;
	while (buffer[0] != '\n' && readed != 0)
	{
		readed = read(fd, buffer, BUFFER_SIZE);
		if (readed == -1)
			return (-1);
		buffer[readed] = '\0';
		remains = ft_strjoin(remains, buffer);
	}
	*line = get_line(remains);
	remains = cut_line(remains);
	if (readed == 0)
		return (0);
	else
		return (1);
}
