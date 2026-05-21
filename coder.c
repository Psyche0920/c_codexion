/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 17:41:54 by wehan             #+#    #+#             */
/*   Updated: 2026/05/21 21:23:43 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	
	coder = (t_coder *)arg;
	if (coder->app->config.number_of_coders == 1)
	{
		pthread_mutex_lock(&coder->app->dongles[0].mutex);
		safe_log(coder->app, coder->id, "has taken a dongle");
		while(!is_stopped(coder->app))
			usleep(1000);  // avoid busy waiting: cpu immer check if stop
		pthread_mutex_unlock(&coder->app->dongles[0].mutex);
		return (NULL);
	}
	while (!is_stopped(coder->app) && coder->compile_count < coder->app->config.number_of_compiles_required)
	{
		if(!take_dongles(coder))
			return (NULL);
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

// Step 9：处理 1 个 coder 特殊情况

// 因为 subject 说：

// If there is only one coder, there should be only one dongle on the table.

// 1 个 coder 只有 1 个 dongle，不能拿两个，所以它应该：

// 拿到一个 dongle
// 然后等到 burned out

// 1.  为什么同一个 coder 不能拿同一个 dongle 两次？

// 因为 subject 的逻辑是：
// compile 需要两个 dongles
// one in each hand

// 也就是说：
// 左手一个 dongle
// 右手一个 dongle
// 不是：
// 同一个 dongle 算两个

// subject 还说：如果只有一个 coder，桌上只有一个 dongle。
// 所以 1 coder 时：
// 只有 1 个 dongle
// compile 需要 2 个 dongles
// => 永远不能 compile
// => 最后 burned out

// 你的原代码在 n == 1 时：
// left = coder->id - 1;  // 0
// right = coder->id % n; // 1 % 1 = 0

// 所以：
// left = 0
// right = 0

// 它不是拿两个 dongles，而是想拿：
// dongle[0] + dongle[0]
// 这在逻辑上就是错的。

// 而且在 pthread 层面：
// pthread_mutex_lock(&dongles[0]);
// pthread_mutex_lock(&dongles[0]);
// 第二次不会成功。
// 它会卡住，因为普通 mutex 不允许同一个 thread 重复 lock。

// 所以：
// 逻辑上：同一个 dongle 不能当成两个 dongles
// 技术上：同一个 mutex 重复 lock 会 self-deadlock
// 因此必须 special case。

// 2. 正确顺序是：
// coder:
// 拿第 1 个 dongle
// 不尝试拿不存在的第 2 个 dongle
// while (!stop) 等待

// monitor:
// 发现超时 burnout
// 打印 "burned out"
// set_stop = 1

// coder:
// while 结束
// unlock 第 1 个 dongle
// return NULL

// main:
// !!!pthread_join 成功(not unlock, get stuck in the threads burnout is not success)
// 程序结束