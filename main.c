/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 13:25:24 by wehan             #+#    #+#             */
/*   Updated: 2026/05/17 17:21:43 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_app	app;

	if (!parse_args(argc, argv, &app.config))
		return (1);
	app.start_time = get_time_ms();
	pthread_mutex_init(&app.log_mutex, NULL);
	safe_log(&app, 1, "has taken a dongle");
	usleep(1000);
	safe_log(&app, 1, "has taken a dongle");
	usleep(1000);
	safe_log(&app, 1, "is compiling");
	pthread_mutex_destroy(&app.log_mutex);
	return (0);
}
