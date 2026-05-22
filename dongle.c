/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 18:17:00 by wehan             #+#    #+#             */
/*   Updated: 2026/05/22 20:38:43 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_dongles(t_app *app)
{
	int	i;

	app->dongles = malloc(sizeof(t_dongle) * app->config.number_of_coders);
	if (!app->dongles)
		return (0);
	i = 0;
	while (i < app->config.number_of_coders)
	{
		pthread_mutex_init(&app->dongles[i].mutex, NULL);
		pthread_cond_init(&app->dongles[i].cond, NULL);
		app->dongles[i].available_at = 0;
		app->dongles[i].occupied = 0;
		app->dongles[i].size = 0;
		app->dongles[i].capacity = app->config.number_of_coders;
		app->dongles[i].heap = malloc(sizeof(t_request)
				* app->config.number_of_coders);
		if (!app->dongles[i].heap)
			return (0);
		i++;
	}
	return (1);
}

void	destroy_dongles(t_app *app)
{
	int	i;

	i = 0;
	while (i < app->config.number_of_coders)
	{
		free(app->dongles[i].heap);
		pthread_cond_destroy(&app->dongles[i].cond);
		pthread_mutex_destroy(&app->dongles[i].mutex);
		i++;
	}
	free(app->dongles);
}

int	take_dongles(t_coder *coder)
{
	int	first;
	int	second;

	get_order(coder, &first, &second);
	while (!is_stopped(coder->app))
	{
		if (request_dongle(coder, first, 2))
		{
			if (request_dongle(coder, second, 2))
				return (1);
			release_one_dongle(coder, first);
		}
		usleep(1000);
	}
	return (0);
}

void	release_one_dongle(t_coder *coder, int index)
{
	t_dongle	*dongle;

	dongle = &coder->app->dongles[index];
	pthread_mutex_lock(&dongle->mutex);
	dongle->available_at = get_time_ms() + coder->app->config.dongle_cooldown;
	dongle->occupied = 0;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongles(t_coder *coder)
{
	int	first;
	int	second;

	get_order(coder, &first, &second);
	release_one_dongle(coder, second);
	release_one_dongle(coder, first);
}
