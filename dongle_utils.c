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

static t_request make_request(t_coder *coder)
{
	t_request	request;

	request.coder_id = coder->id;
	request.arrival_time = get_time_ms();
	request.deadline = coder->last_compile_start + coder->app->config.time_to_burnout;
	return (request);
}

static int can_take_dongle(t_coder *coder, t_dongle *dongle)
{
	if (dongle->occupied)
		return (0);
	if (get_time_ms() < dongle->available_at)
		return (0);
	if (heap_peek(dongle) != coder->id)
		return (0);
	return (1);
}

int request_dongle(t_coder *coder, int index, long timeout_ms)
{
	t_dongle		*dongle;
	t_request		request;
	struct timeval	tv;
	struct timespec	ts;

	dongle = &coder->app->dongles[index];
	request = make_request(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_push(dongle, request, coder->app->config.scheduler);
	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = (tv.tv_usec + timeout_ms * 1000) * 1000; // trans into nsec
	while(ts.tv_nsec >= 1000000000) // if nsec>=1sec, make it into sec
	{
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}
	while(!is_stopped(coder->app) && !can_take_dongle(coder, dongle))  
	// end condi-stop & can take dongle->take dongle// oder if 0 (notstop, cannot take dongle->wait)
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts); // accept only abst/struct timespec: s+ns
		//sleep+wakable+timeout autowake
	if (is_stopped(coder->app)) // if 1(program stop)
	{
		heap_remove(dongle, coder->id, coder->app->config.scheduler);  // remove its request
		pthread_mutex_unlock(&dongle->mutex);  //unlock
		return (0);  // quit
	}
	heap_remove(dongle, coder->id, coder->app->config.scheduler);  // can take dongle
	dongle->occupied = 1;
	safe_log(coder->app, coder->id, "has taken a dongle");
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

void release_one_dongle(t_coder *coder, int index)
{
	t_dongle	*dongle;

	dongle = &coder->app->dongles[index];
	pthread_mutex_lock(&dongle->mutex);
	dongle->available_at = get_time_ms() + coder->app->config.dongle_cooldown;
	dongle->occupied = 0;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}