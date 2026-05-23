/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 17:41:54 by wehan             #+#    #+#             */
/*   Updated: 2026/05/23 13:57:45 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	*single_coder_routine(t_coder *coder)
{
	pthread_mutex_lock(&coder->app->dongles[0].mutex);
	safe_log(coder->app, coder->id, "has taken a dongle");
	while (!is_stopped(coder->app))
		usleep(1000);
	pthread_mutex_unlock(&coder->app->dongles[0].mutex);
	return (NULL);
}

static int	run_compile(t_coder *coder)
{
	if (!take_dongles(coder))
		return (0);
	if (is_stopped(coder->app))
	{
		release_dongles(coder);
		return (0);
	}
	update_compile_start(coder);
	safe_log(coder->app, coder->id, "is compiling");
	usleep(coder->app->config.time_to_compile * 1000);
	pthread_mutex_lock(&coder->app->state_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->app->state_mutex);
	release_dongles(coder);
	if (all_coders_done(coder->app))
		set_stop(coder->app);
	return (!is_stopped(coder->app));
}
// if nonstop0, return 1

static int	run_rest(t_coder *coder)
{
	safe_log(coder->app, coder->id, "is debugging");
	usleep(coder->app->config.time_to_debug * 1000);
	if (is_stopped(coder->app))
		return (0);
	safe_log(coder->app, coder->id, "is refactoring");
	usleep(coder->app->config.time_to_refactor * 1000);
	return (!is_stopped(coder->app));
}
// if nonstop0, return1 

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->app->config.number_of_coders == 1)
		return (single_coder_routine(coder));
	while (!is_stopped(coder->app)
		&& coder->compile_count
		< coder->app->config.number_of_compiles_required)
	{
		if (!run_compile(coder))
			return (NULL);
		if (!run_rest(coder))
			return (NULL);
	}
	return (NULL);
}

int	init_coders(t_app *app)
{
	int	i;

	app->coders = malloc(sizeof(t_coder) * app->config.number_of_coders);
	if (!app->coders)
		return (0);
	i = 0;
	while (i < app->config.number_of_coders)
	{
		app->coders[i].id = i + 1;
		app->coders[i].compile_count = 0;
		app->coders[i].last_compile_start = get_time_ms();
		app->coders[i].app = app;
		if (pthread_create(&app->coders[i].thread, NULL, coder_routine,
				&app->coders[i]) != 0)
			return (0);
		i++;
	}
	return (1);
}
