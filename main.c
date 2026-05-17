/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 13:25:24 by wehan             #+#    #+#             */
/*   Updated: 2026/05/17 13:46:08 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;

	if (!parse_args(argc, argv, &config))
		return (1);
	printf("number_of_coders: %d\n", config.number_of_coders);
	printf("time_to_burnout: %ld\n", config.time_to_burnout);
	printf("time_to_compile: %ld\n", config.time_to_compile);
	printf("time_to_debug: %ld\n", config.time_to_debug);
	printf("time_to_refactor: %ld\n", config.time_to_refactor);
	printf("number_of_compiles_required: %d\n",
		config.number_of_compiles_required);
	printf("dongle_cooldown: %ld\n", config.dongle_cooldown);
	if (config.scheduler == 0)
		printf("scheduler: fifo\n");
	else
		printf("scheduler: edf\n");
	return (0);
}
