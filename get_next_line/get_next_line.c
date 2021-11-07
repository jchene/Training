/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/07 14:16:51 by jchene            #+#    #+#             */
/*   Updated: 2021/11/07 15:13:51 by jchene           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./get_next_line.h"

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

void	ft_bzero(void *p, unsigned int n)
{
	unsigned char	*ptr;

	ptr = (unsigned char *)p;
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
	while (dst[i] != '\0')
		i++;
	j = 0;
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
	int		size;
	char	*new_line;

	if (!line && !buffer)
		return (NULL);
	size = ft_strlen(line) + ft_strlen(buffer);
	new_line = malloc(sizeof(char) * (size + 1));
	if (!new_line)
		return (NULL);
	ft_bzero(new_line, size + 1);
	ft_append(new_line, line);
	ft_append(new_line, buffer);
	free(line);
	line = NULL;
	return (new_line);
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
		remains = NULL;
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
	static char	*remains;
	char		buffer[BUFFER_SIZE + 1];
	int			readed;
	int			fd;

	fd = 0;
	readed = 1;
	ft_bzero(buffer, 2);
	while (buffer[0] != '\n' && readed > 0)
	{
		readed = read(fd, buffer, BUFFER_SIZE);
		if (readed == -1)
			return (-1);
		buffer[readed] = '\0';
		remains = ft_strjoin(remains, buffer);
	}
	*line = get_line(remains);
	remains = cut_line(remains);
	return (readed == 0 ? 0 : 1);
}
