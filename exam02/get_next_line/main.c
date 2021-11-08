/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jchene <jchene@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/08 01:52:57 by jchene            #+#    #+#             */
/*   Updated: 2021/11/08 02:49:59 by jchene           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./get_next_line.h"
# include <stdio.h>

int	main(void)
{
	char	*line;

	while (get_next_line(&line) > 0)
	{
		printf("%s\n", line);
		free(line);
		line = NULL;
	}
	if (line)
		free(line);
	return (0);
}
