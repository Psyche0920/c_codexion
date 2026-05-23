/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 18:03:11 by wehan             #+#    #+#             */
/*   Updated: 2026/05/23 15:02:46 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	get_order(t_coder *coder, int *first, int *second)
{
	int	left;
	int	right;
	int	n;

	n = coder->app->config.number_of_coders;
	left = coder->id - 1;
	right = coder->id % n;
	if (left < right)
	{
		*first = left;
		*second = right;
	}
	else
	{
		*first = right;
		*second = left;
	}
}

static t_request	make_request(t_coder *coder)
{
	t_request	request;

	request.coder_id = coder->id;
	request.arrival_time = get_time_ms();
	request.deadline = coder->last_compile_start
		+ coder->app->config.time_to_burnout;
	return (request);
}

static int	can_take_dongle(t_coder *coder, t_dongle *dongle)
{
	if (dongle->occupied)
		return (0);
	if (get_time_ms() < dongle->available_at)
		return (0);
	if (heap_peek(dongle) != coder->id)
		return (0);
	return (1);
}

static void	make_timeout(struct timespec *ts, long timeout_ms)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	ts->tv_sec = tv.tv_sec;
	ts->tv_nsec = (tv.tv_usec + timeout_ms * 1000) * 1000;
	while (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}

int	request_dongle(t_coder *coder, int index, long timeout_ms)
{
	t_dongle		*dongle;
	struct timespec	ts;
	int				ret;

	dongle = &coder->app->dongles[index];
	pthread_mutex_lock(&dongle->mutex);
	heap_push(dongle, make_request(coder), coder->app->config.scheduler);
	make_timeout(&ts, timeout_ms);
	ret = 0;
	while (!is_stopped(coder->app)
		&& !can_take_dongle(coder, dongle) && ret == 0)
		ret = pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	if (is_stopped(coder->app))
	{
		heap_remove(dongle, coder->id, coder->app->config.scheduler);
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	heap_remove(dongle, coder->id, coder->app->config.scheduler);
	dongle->occupied = 1;
	safe_log(coder->app, coder->id, "has taken a dongle");
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}
