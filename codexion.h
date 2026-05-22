/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:36:39 by wehan             #+#    #+#             */
/*   Updated: 2026/05/22 17:52:38 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <time.h>

typedef struct s_app t_app;

typedef struct s_config
{
	int		number_of_coders;
	long	time_to_burnout;
	long	time_to_compile;
	long	time_to_debug;
	long	time_to_refactor;
	int		number_of_compiles_required;
	long	dongle_cooldown;
	int		scheduler;
}	t_config;

typedef struct s_coder
{
	int			id;
	int			compile_count;
	pthread_t	thread;
	t_app		*app;
	long		last_compile_start;
}	t_coder;

typedef struct s_request
{
	int		coder_id;
	long	arrival_time;
	long	deadline;
}	t_request;

typedef struct s_dongle
{
	pthread_mutex_t mutex;
	pthread_cond_t	cond;
	long			available_at;
	int				occupied;
	t_request		*heap;  //
	int				size;  //actual
	int				capacity;  // max
}	t_dongle;

typedef struct s_app
{
	t_config		config;
	t_coder			*coders;
	t_dongle		*dongles;
	long			start_time;
	pthread_mutex_t	log_mutex;
	pthread_t		monitor;
	pthread_mutex_t	state_mutex;
	int				stop;
}	t_app;

int		parse_args(int argc, char **argv, t_config *config);
long	get_time_ms(void);
long	timestamp(t_app *app);
void	safe_log(t_app *app, int id, char *msg);

int		init_coders(t_app *app);
void	join_coders(t_app *app);
void	*coder_routine(void *arg);

int		init_dongles(t_app *app);
void	destroy_dongles(t_app *app);
int		take_dongles(t_coder *coder);
void	release_dongles(t_coder *coder);
void	get_order(t_coder *coder, int *first, int *second);
int		request_dongle(t_coder *coder, int index, long timeout_ms);  //最多等待这个 dongle 多少毫秒
void	release_one_dongle(t_coder *coder, int index);

void	*monitor_routine(void *arg);
int		is_stopped(t_app *app);
void	set_stop(t_app *app);
void	update_compile_start(t_coder *coder);

int		heap_push(t_dongle *dongle, t_request request, int scheduler);
int		heap_peek(t_dongle *dongle);
int		heap_remove(t_dongle *dongle, int coder_id, int scheduler);

int		request_less(t_request a, t_request b, int scheduler);
void	swap_request(t_request *a, t_request *b);
void	heap_up(t_dongle *dongle, int index, int scheduler);
void	heap_down(t_dongle *dongle, int index, int scheduler);


#endif
