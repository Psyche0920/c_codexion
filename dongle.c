#include "codexion.h"

int init_dongles(t_app *app)
{
    int i;

    app->dongles = malloc(sizeof(t_dongle) * app->config.number_of_coders);
    if(!app->dongles)
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
		app->dongles[i].heap = malloc(sizeof(t_request) * app->config.number_of_coders);
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
	while(i < app->config.number_of_coders)
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
	if (!request_dongle(coder, first))
		return(0);
	if(!request_dongle(coder, second))
	{
		release_one_dongle(coder, first);
		return (0);
	}
	return (1);
}

void	release_dongles(t_coder *coder)
{
	int	first;
	int second;

	get_order(coder, &first, &second);
	release_one_dongle(coder, second);
	release_one_dongle(coder, first);
}

// subject：
// After being released, a dongle is unavailable until its cooldown has passed.
// 也就是：从 release 的那一刻开始算 cooldown。不是从 compile 开始，也不是从 get_order 开始
// available_at = get_time_ms() + dongle_cooldown;

// 意思是：

// 现在放下 dongle
// 从现在开始冷却 cooldown ms
// 到 available_at 之后才能再次被拿

// release_dongles()
// available_at = get_time_ms() + dongle_cooldown;
// 从 release 那一刻开始计算 cooldown

//

// // // init_dongles：申请 number_of_coders 个 dongle。
// // 每个 dongle 初始化 mutex、condition variable、cooldown 时间、occupied 状态、heap size/capacity，
// 并 malloc 一块 request heap。

// // // destroy_dongles：每个 dongle 先 free 自己的 heap，
// 再 destroy cond 和 mutex，最后 free 整个 dongles 数组。

// // // take_dongles：先用 get_order 算出这个 coder 该拿哪两把 dongle，
// 然后按固定顺序请求两把，避免死锁。

// // // request_dongle：创建 request，放入当前 dongle 的 heap；
// // 然后等待，直到这个 dongle 没被占、cooldown 结束、并且 heap top 是自己。
// 满足后 remove 自己的 request，标记 occupied，打印拿到 dongle。

// // // release_one_dongle：释放时设置新的 available_at，把 occupied 设回 0，
// // 然后 pthread_cond_broadcast 唤醒等待这个 dongle 的 threads。