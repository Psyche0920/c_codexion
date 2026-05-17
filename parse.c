/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 14:39:20 by wehan             #+#    #+#             */
/*   Updated: 2026/05/17 14:57:45 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_number(char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static long	ft_atol(char *str)
{
	long	result;
	int		i;

	result = 0;
	i = 0;
	while (str[i])
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result);
}

static int	set_scheduler(char *str, t_config *config)
{
	if (strcmp(str, "fifo") == 0)
	{
		config->scheduler = 0;
		return (1);
	}
	if (strcmp(str, "edf") == 0)
	{
		config->scheduler = 1;
		return (1);
	}
	return (0);
}

static int	fill_config(char **argv, t_config *config)
{
	config->number_of_coders = (int)ft_atol(argv[1]);
	config->time_to_burnout = ft_atol(argv[2]);
	config->time_to_compile = ft_atol(argv[3]);
	config->time_to_debug = ft_atol(argv[4]);
	config->time_to_refactor = ft_atol(argv[5]);
	config->number_of_compiles_required = (int)ft_atol(argv[6]);
	config->dongle_cooldown = ft_atol(argv[7]);
	return (set_scheduler(argv[8], config));
}

int	parse_args(int argc, char **argv, t_config *config)
{
	int	i;

	if (argc != 9)
		return (fprintf(stderr, "Error: wrong number of arguments\n"), 0);
	i = 1;
	while (i <= 7)
	{
		if (!is_number(argv[i]))
		{
			return (fprintf(stderr, "Error: invalid numeric argument\n"), 0);
		}
		i++;
	}
	if (!fill_config(argv, config))
		return (fprintf(stderr, "Error: scheduler\n"), 0);
	if (config->number_of_coders <= 0 || config->time_to_burnout <= 0
		|| config->time_to_compile <= 0 || config->time_to_debug <= 0
		|| config->time_to_refactor <= 0 || config->dongle_cooldown < 0
		|| config->number_of_compiles_required <= 0)
		return (fprintf(stderr, "Error: invalid argument value\n"), 0);
	return (1);
}
