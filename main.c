/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 13:25:24 by wehan             #+#    #+#             */
/*   Updated: 2026/05/18 22:54:11 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_app	app;

	if (!parse_args(argc, argv, &app.config))
		return (1);
	app.start_time = get_time_ms();
	app.coders = NULL;
	app.dongles = NULL;
	pthread_mutex_init(&app.log_mutex, NULL);
	if (!init_dongles(&app) || !init_coders(&app))
	{
		pthread_mutex_destroy(&app.log_mutex);
		free(app.coders);
		free(app.dongles);
		return (1);
	}
	join_coders(&app);
	destroy_dongles(&app);
	pthread_mutex_destroy(&app.log_mutex);
	free(app.coders);
	return (0);
}


// ✅ 参数解析 parse_args
// ✅ 时间戳 gettimeofday
// ✅ safe_log + log_mutex
// ✅ 创建 coder threads
// ✅ pthread_join 等待线程
// ✅ app / coder 共享结构
// ✅ dongle mutex 数组
// ✅ 每个 coder 拿两个 dongles
// ✅ fixed locking order 防基础 deadlock
// ✅ compile / debug / refactor 基础流程

// 1. free vs destroy
//!!! free
// 释放：
// malloc 的内存
// !!!pthread_mutex_destroy
// 释放：
// mutex 内部的 OS resource
// !!!为什么有时既要 destroy 又要 free
// app->dongles = malloc(sizeof(pthread_mutex_t) * n); 这里：malloc 了 mutex 数组内存
// 然后：pthread_mutex_init(&app->dongles[i], NULL);这里：初始化每把 mutex

// 所以：现在有：
// A. malloc memory 需要：free(app->dongles);
// B. mutex internal resource 需要：pthread_mutex_destroy(&app->dongles[i]);

// malloc 出来的东西 => free
// pthread_mutex_init 过的 mutex => pthread_mutex_destroy
// malloc 出来的 mutex 数组 => destroy 每个 mutex + free 数组

// sequence: malloc->init_mutex->destroy_mutex->free

// dongles needs pointer (dynmaic only knows when parsing)
// while log_mutex for printing save its space withins struct


// 2. mutex functions:
// Function	Meaning
// pthread_mutex_init	初始化 mutex
// pthread_mutex_lock	拿锁
// pthread_mutex_unlock	解锁
// pthread_mutex_destroy	销毁 mutex


// 3. thread functions
// Function	Meaning
// pthread_create	创建线程
// pthread_join	等待线程结束
// pthread_detach	分离线程（自动 cleanup）
// pthread_exit	当前线程退出

// 4. time _SC_THREAD_SAFE_FUNCTIONS
// Function	Meaning
// gettimeofday	获取当前时间
// usleep	暂停线程

// 5. malloc and free