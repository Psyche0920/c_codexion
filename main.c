/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 13:25:24 by wehan             #+#    #+#             */
/*   Updated: 2026/05/22 19:56:11 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	init_app_state(t_app *app)
{
	app->start_time = get_time_ms();
	app->coders = NULL;
	app->dongles = NULL;
	app->stop = 0;
	pthread_mutex_init(&app->state_mutex, NULL);
	pthread_mutex_init(&app->log_mutex, NULL);
}

void	join_coders(t_app *app)
{
	int	i;

	i = 0;
	while (i < app->config.number_of_coders)
	{
		pthread_join(app->coders[i].thread, NULL);
		i++;
	}
}

static void	destroy_app(t_app *app)
{
	destroy_dongles(app);
	pthread_mutex_destroy(&app->log_mutex);
	pthread_mutex_destroy(&app->state_mutex);
	free(app->coders);
}

static int	start_app(t_app *app)
{
	if (!init_dongles(app) || !init_coders(app))
	{
		pthread_mutex_destroy(&app->log_mutex);
		free(app->coders);
		free(app->dongles);
		return (0);
	}
	pthread_create(&app->monitor, NULL, monitor_routine, app);
	return (1);
}

int	main(int argc, char **argv)
{
	t_app	app;

	if (!parse_args(argc, argv, &app.config))
		return (1);
	init_app_state(&app);
	if (!start_app(&app))
		return (1);
	join_coders(&app);
	set_stop(&app);
	pthread_join(app.monitor, NULL);
	destroy_app(&app);
	return (0);
}
