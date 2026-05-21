#include "codexion.h"

int	is_stopped(t_app *app)
{
    int stop;

    pthread_mutex_lock(&app->state_mutex);
    stop = app->stop;
    pthread_mutex_unlock(&app->state_mutex);
    return (stop);
}

void	set_stop(t_app *app)
{
	int	i;

	pthread_mutex_lock(&app->state_mutex);
	app->stop = 1;
	pthread_mutex_unlock(&app->state_mutex);
	i = 0;
	while(app->dongles && i < app->config.number_of_coders)
	{
		pthread_mutex_lock(&app->dongles[i].mutex);
		pthread_cond_broadcast(&app->dongles[i].cond);  // fix the bug, everycoder should be broadcasted
		pthread_mutex_unlock(&app->dongles[i].mutex);
		i++;
	}
}

void	update_compile_start(t_coder *coder)
{
	pthread_mutex_lock(&coder->app->state_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->app->state_mutex);
}

static int	check_burnout(t_app *app, int i)
{
	long 	now;
	long	last;

	pthread_mutex_lock(&app->state_mutex);
	now = get_time_ms();
	last = app->coders[i].last_compile_start;
	pthread_mutex_unlock(&app->state_mutex);
	if (now - last > app->config.time_to_burnout)
	{
		safe_log(app, app->coders[i].id, "burned out");
		set_stop(app);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_app	*app;
	int		i;

	app = (t_app *)arg;
	while (!is_stopped(app))
	{
		i = 0;
		while(i < app->config.number_of_coders)
		{
			if (check_burnout(app, i))
				return (NULL);
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}

// 1. 为什么读 stop 也要 lock？
// 因为 stop 是 shared variable。
// 一个 thread 写：
// app->stop = 1;
// 另一个 thread 同时读：
// if (app->stop)
// 这就是 data race。
// 你说“读不占资源”，这个想法在单线程里没问题，但多线程里：
// 读和写同一个变量同时发生，也是不安全的
// 所以 state_mutex 保护的是：
// shared state 的一致性
// 不是保护 CPU，也不是保护 dongle。


// 2. state_mutex 只代表 stop 0/1 吗？
// 不是。
// state_mutex 不是变量本身。
// 它是一把锁，保护这些 shared state：
// app->stop
// coder->last_compile_start
// 以后可能还有 compile_count / finished_count
// 所以：
// pthread_mutex_lock(&app->state_mutex);
// coder->last_compile_start = get_time_ms();
// pthread_mutex_unlock(&app->state_mutex);
// 意思是：
// 我正在安全修改 shared state，别人先不要读/写


// 3. monitor set_stop:
// 有人 burned out，异常结束。
// main set_stop:
// 所有 coder 正常完成，通知 monitor 停止。

// 4. monitor 和 coder 一样也是 thread？
// 对，完全正确。
// 你现在有两类 thread：
// coder threads   = 工人线程，负责 compile/debug/refactor
// monitor thread = 监督线程，负责检查谁 burned out


// 5. subject 要求 10ms，我们写 1ms
// 是的，这是故意的。
// usleep(1000);
// 意思是：
// monitor 每 1ms 检查一次
// subject 要求：
// burned out 后 10ms 内打印
// 我们每 1ms 检查一次，比 10ms 更严格，更安全。
// 但注意：OS 调度有误差，所以实际可能不是精确 1ms，但通常够用。