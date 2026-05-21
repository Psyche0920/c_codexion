/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:36:39 by wehan             #+#    #+#             */
/*   Updated: 2026/05/21 21:27:00 by wehan            ###   ########.fr       */
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
int		request_dongle(t_coder *coder, int index);
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

// 1. 你现在：

// pthread_mutex_t mutex;
// pthread_cond_t  cond;

// 意思其实是：

// 这个 dongle:
// - 有自己的锁
// - 还有自己的等待队列通知机制

// 这已经非常接近真正的：

// FIFO / EDF scheduler

// 1)pthread_mutex_t: 
// mutex object（互斥锁对象）
// 进入 critical section
// 同一时间只能一个 thread 进入

// 2)condition variable（条件变量）
// 线程等待/唤醒机制
// 不满足条件时睡觉
// 条件满足时被唤醒
// 没有 cond
// 你现在：
// while (cooldown_not_finished)
// 	usleep(1000);
// 叫：
// polling
// 轮询
// 线程：
// 睡
// 醒
// 检查
// 睡
// 醒
// 检查
// 有 cond

// 线程可以：
// pthread_cond_wait(...)
// 意思：
// 我现在不满足条件
// 我彻底睡眠
// 直到别人通知我
// 然后 release_dongles：
// pthread_cond_signal(...)
// 或者：
// pthread_cond_broadcast(...)
// 意思：
// dongle 可用了
// 大家醒来重新竞争

