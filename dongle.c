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
	while (1)
	{
		pthread_mutex_lock(&coder->app->dongles[first].mutex);
		if (get_time_ms() >= coder->app->dongles[first].available_at)
			break ;
		pthread_mutex_unlock(&coder->app->dongles[first].mutex);
		usleep(1000);
	}
	safe_log(coder->app, coder->id, "has taken a dongle");
		while (1)
	{
		pthread_mutex_lock(&coder->app->dongles[second].mutex);
		if (get_time_ms() >= coder->app->dongles[second].available_at)
			break ; // if end cooldown, keep the lock, otherwise unlock
		pthread_mutex_unlock(&coder->app->dongles[second].mutex);
		usleep(1000);
	}
	safe_log(coder->app, coder->id, "has taken a dongle");
}

void	release_dongles(t_coder *coder)
{
	int	first;
	int second;

	get_order(coder, &first, &second);
	coder->app->dongles[second].available_at = 
	get_time_ms() + coder->app->config.dongle_cooldown;
	pthread_mutex_unlock(&coder->app->dongles[second].mutex); // ava_t = releas_t + cldown_t
	coder->app->dongles[first].available_at = 
	get_time_ms() + coder->app->config.dongle_cooldown;
	pthread_mutex_unlock(&coder->app->dongles[first].mutex);
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


// 2. void	take_dongles(t_coder *coder)
// 逻辑是：
// lock mutex
// ↓
// 检查 cooldown 是否结束
// ↓
// 如果没结束:
//     unlock
//     sleep 1ms
//     再来尝试

// 如果结束:
//     保持 lock
//     真正拿到 dongle


// 3. busy waiting
// while (get_time_ms() < available_at)
// 	;

// CPU：
// 一直运行这个线程
// 每秒检查几亿次
// 占满 CPU。

// sleep waiting
// while (get_time_ms() < available_at)
// 	usleep(1000);

// CPU：
// 线程睡眠
// 1ms 后再唤醒检查
// 几乎不占 CPU。
// 对，usleep(10) 太短了。

// 注意单位：
// usleep(10);
// 不是 10ms，而是 10 microseconds = 0.01ms。
// 所以它会变成：
// 每 0.01ms 醒一次
// 每 1ms 醒 100 次
// 每 1 秒醒 100,000 次
// 这会增加 CPU 调度负担。

// 而：
// usleep(1000);
// 是：
// 1000 microseconds = 1ms
// 每 1ms 醒一次
// 每 1 秒醒 1000 次
// 对这个项目已经够精确了。
