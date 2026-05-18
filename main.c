/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 13:25:24 by wehan             #+#    #+#             */
/*   Updated: 2026/05/18 21:18:06 by wehan            ###   ########.fr       */
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
	pthread_mutex_init(&app.log_mutex, NULL);
	if (!init_coders(&app))
	{
		pthread_mutex_destroy(&app.log_mutex);
		free(app.coders);
		return (1);
	}
	join_coders(&app);
	pthread_mutex_destroy(&app.log_mutex);
	return (0);
}

// 1. 关系图
    //             t_app
    //     ┌─────────────────┐
    //     │ config          │
    //     │ log_mutex       │
    //     │ start_time      │
    //     │ coders[]        │
    //     └─────────────────┘
    //             ↑
    //             │
    //     ┌───────┴───────┐
    //     │               │
    // coder1          coder2
    //     │               │
    //     └── app ptr ────┘


// pthread_mutex_init(&mutex, NULL);
// 第1个：mutex 地址
// 第2个：mutex 属性，NULL = 默认
// 它的作用：
// 初始化一把 mutex，让它可以被 lock/unlock 使用

// 3. philosophers/codexion 真正流程
// !!!main thread
// parse args
// init mutexes
// create coder threads
// join coder threads
// destroy mutexes
// free memory


// !!!coder thread
// take dongles
// compile
// release dongles
// debug
// refactor
// repeat


// !!!pthread_create
// 开始一个新的执行流
// !!!pthread_join
// 等待执行流结束
// !!!mutex
// 保证同一时刻只有一个 thread 能访问共享资源