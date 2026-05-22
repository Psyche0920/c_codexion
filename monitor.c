/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 20:41:26 by wehan             #+#    #+#             */
/*   Updated: 2026/05/22 20:41:34 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_stopped(t_app *app)
{
	int	stop;

	pthread_mutex_lock(&app->state_mutex);
	stop = app->stop;
	pthread_mutex_unlock(&app->state_mutex);
	return (stop);
}

void	set_stop(t_app *app)
{
	int	i;

	pthread_mutex_lock(&app->state_mutex);
	app->stop = 1;
	pthread_mutex_unlock(&app->state_mutex);
	i = 0;
	while (app->dongles && i < app->config.number_of_coders)
	{
		pthread_mutex_lock(&app->dongles[i].mutex);
		pthread_cond_broadcast(&app->dongles[i].cond);
		pthread_mutex_unlock(&app->dongles[i].mutex);
		i++;
	}
}

void	update_compile_start(t_coder *coder)
{
	pthread_mutex_lock(&coder->app->state_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->app->state_mutex);
}

static int	check_burnout(t_app *app, int i)
{
	long	now;
	long	last;

	pthread_mutex_lock(&app->state_mutex);
	now = get_time_ms();
	last = app->coders[i].last_compile_start;
	pthread_mutex_unlock(&app->state_mutex);
	if (now - last > app->config.time_to_burnout)
	{
		safe_log(app, app->coders[i].id, "burned out");
		set_stop(app);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_app	*app;
	int		i;

	app = (t_app *)arg;
	while (!is_stopped(app))
	{
		i = 0;
		while (i < app->config.number_of_coders)
		{
			if (check_burnout(app, i))
				return (NULL);
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
