/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 13:25:24 by wehan             #+#    #+#             */
/*   Updated: 2026/05/22 17:48:19 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_app	app;

	if (!parse_args(argc, argv, &app.config))
		return (1);
	app.start_time = get_time_ms();
	app.coders = NULL;
	app.dongles = NULL;
	app.stop = 0;
	pthread_mutex_init(&app.state_mutex, NULL);
	pthread_mutex_init(&app.log_mutex, NULL);
	if (!init_dongles(&app) || !init_coders(&app))
	{
		pthread_mutex_destroy(&app.log_mutex);
		free(app.coders);
		free(app.dongles);
		return (1);
	}
	pthread_create(&app.monitor, NULL, monitor_routine, &app);
	join_coders(&app);
	set_stop(&app);
	pthread_join(app.monitor, NULL);
	destroy_dongles(&app);
	pthread_mutex_destroy(&app.log_mutex);
	pthread_mutex_destroy(&app.state_mutex);
	free(app.coders);
	return (0);
}
