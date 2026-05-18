#include "codexion.h"

int init_dongles(t_app *app)
{
    int i;

    app->dongles = malloc(sizeof(pthread_mutex_t) * app->config.number_of_coders);
    if(!app->dongles)
        return (0);
    i = 0;
    while (i < app->config.number_of_coders)
    {
        pthread_mutex_init(&app->dongles[i], NULL);
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
		pthread_mutex_destroy(&app->dongles[i]);
		i++;
	}
	free(app->dongles);
}

static void	get_order(t_coder *coder, int *first, int *second)
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

void	take_dongles(t_coder *coder)
{
	int	first;
	int	second;

	get_order(coder, &first, &second);
	pthread_mutex_lock(&coder->app->dongles[first]);
	safe_log(coder->app, coder->id, "has taken a dongle");
	pthread_mutex_lock(&coder->app->dongles[second]);
	safe_log(coder->app, coder->id, "has taken a dongle");
}

void	release_dongles(t_coder *coder)
{
	int	first;
	int second;

	get_order(coder, &first, &second);
	pthread_mutex_unlock(&coder->app->dongles[second]);
	pthread_mutex_unlock(&coder->app->dongles[first]);
}

// 1. unlock 顺序可以改吗？

// 现在：

// pthread_mutex_unlock(&coder->app->dongles[second]);
// pthread_mutex_unlock(&coder->app->dongles[first]);

// 这是反向释放。

// 通常习惯：

// lock:   first -> second
// unlock: second -> first

// 这叫 LIFO，像栈一样，比较清晰。

// 2. 
// There are as many dongles as coders.
// dongle 数量 = coder 数量

// 3. 
// dongles = 多个资源锁
// log_mutex = 一个打印锁


// ✗ ./codexion 4 800 200 300 500 3 5 fifo
// 0 1 has taken a dongle
// 0 2 has taken a dongle
// 0 3 has taken a dongle
// 0 3 has taken a dongle
// 0 3 is compiling
// 200 3 is debugging
// 200 2 has taken a dongle
// 200 2 is compiling
// 400 2 is debugging
// 401 1 has taken a dongle
// 401 1 is compiling
// 500 3 is refactoring
// 601 1 is debugging
// 601 4 has taken a dongle
// 601 4 has taken a dongle
// 601 4 is compiling
// 701 2 is refactoring
// 801 4 is debugging
// 901 1 is refactoring
// 1102 4 is refactoring

// mutex_lock 的行为是：

// 如果 mutex 空闲
// 立刻成功
// 继续执行
// 如果 mutex 已被占
// 线程阻塞等待
// small forever left