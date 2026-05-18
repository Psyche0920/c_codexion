/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 17:41:54 by wehan             #+#    #+#             */
/*   Updated: 2026/05/19 00:52:56 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!is_stopped(coder->app) && coder->compile_count < coder->app->config.number_of_compiles_required)
	{
		take_dongles(coder);
		if (is_stopped(coder->app))
		{
			release_dongles(coder);
			return (NULL);
		}
		update_compile_start(coder);
		safe_log(coder->app, coder->id, "is compiling");
		usleep(coder->app->config.time_to_compile * 1000);
		coder->compile_count++;
		release_dongles(coder);
		if (is_stopped(coder->app))
			return (NULL);
		safe_log(coder->app, coder->id, "is debugging");
		usleep(coder->app->config.time_to_debug * 1000);
		if (is_stopped(coder->app))
			return (NULL);
		safe_log(coder->app, coder->id, "is refactoring");
		usleep(coder->app->config.time_to_refactor * 1000);
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
		if(pthread_create(
			&app->coders[i].thread,
			NULL,
			coder_routine,
			&app->coders[i]) != 0)
			return (0);
		i++;
	}
	return (1);
}

void join_coders(t_app *app)
{
	int	i;

	i = 0;
	while(i < app->config.number_of_coders)
	{	
		pthread_join(app->coders[i].thread, NULL);
		i++;
	}
}

// 1. [coder_routine]根据 subject：

// If a coder did not start compiling within time_to_burnout milliseconds
// since the beginning of their last compile
// or the beginning of the simulation,
// they burn out.

// 真正意思是：

// burnout 只看：
// 当前时间 - 上一次开始 compile 的时间
// 完全不管：
// 当前正在 debug
// 当前正在 refactor
// 当前正在等待 mutex
// 当前正在 sleep

// 2. 每个 coder 不只 compile 一次
// 而是 compile N 次
// N = number_of_compiles_required

// 3. pthread_create(&thread, NULL, routine, arg);
// 第1个：thread 的地址，系统会把新线程信息写进去
// 第2个：线程属性，NULL = 默认
// 第3个：线程要执行的函数
// 第4个：传给线程函数的参数

// pthread_join(thread, NULL);
// 第1个：你要等待的 thread
// 第2个：接收 thread return value 的地址，NULL = 不关心返回值