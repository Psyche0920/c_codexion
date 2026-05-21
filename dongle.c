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
	while(!is_stopped(coder->app)) // if nonstop
	{
		if(request_dongle(coder, first, 2)) //!!!try get first，wait max 2ms。
		{
			if (request_dongle(coder, second, 2))
			return (1);
			release_one_dongle(coder, first);
		}
		usleep(1000);  //wait 1ms and retry, avoid cpu turning immer
	}
	return (0);  // if stop, quit with failure
}
//尝试拿第一把
// 如果第一把成功，再短暂尝试第二把
// 如果第二把失败，立刻释放第一把
// 然后重新开始

void	release_dongles(t_coder *coder)
{
	int	first;
	int second;

	get_order(coder, &first, &second);
	release_one_dongle(coder, second);
	release_one_dongle(coder, first);
}
// Subject 要求 EDF 下 feasible 参数不能 starvation。

// Next Step

// 把逻辑改成：

// 拿第一把
// 尝试拿第二把
// 如果第二把拿不到：
//     释放第一把
//     稍等
//     重新尝试

// 也就是修：

// take_dongles()

// 不要让 coder hold one dongle while waiting forever。