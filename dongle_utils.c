#include "codexion.h"


void	get_order(t_coder *coder, int *first, int *second)
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

static t_request make_request(t_coder *coder)
{
	t_request	request;

	request.coder_id = coder->id;
	request.arrival_time = get_time_ms();
	request.deadline = coder->last_compile_start + coder->app->config.time_to_burnout;
	return (request);
}

static int can_take_dongle(t_coder *coder, t_dongle *dongle)
{
	if (dongle->occupied)
		return (0);
	if (get_time_ms() < dongle->available_at)
		return (0);
	if (heap_peek(dongle) != coder->id)
		return (0);
	return (1);
}

int request_dongle(t_coder *coder, int index)
{
	t_dongle	*dongle;
	t_request	request;

	dongle = &coder->app->dongles[index];
	request = make_request(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_push(dongle, request, coder->app->config.scheduler);
	while(!is_stopped(coder->app) && !can_take_dongle(coder, dongle))  
	// end condi-stop & can take dongle->take dongle// oder if 0 (notstop, cannot take dongle->wait)
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	if (is_stopped(coder->app)) // if 1(program stop)
	{
		heap_remove(dongle, coder->id, coder->app->config.scheduler);  // remove its request
		pthread_mutex_unlock(&dongle->mutex);  //unlock
		return (0);  // quit
	}
	heap_remove(dongle, coder->id, coder->app->config.scheduler);  // can take dongle
	dongle->occupied = 1;
	safe_log(coder->app, coder->id, "has taken a dongle");
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

void release_one_dongle(t_coder *coder, int index)
{
	t_dongle	*dongle;

	dongle = &coder->app->dongles[index];
	pthread_mutex_lock(&dongle->mutex);
	dongle->available_at = get_time_ms() + coder->app->config.dongle_cooldown;
	dongle->occupied = 0;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

// 1. 为什么 arrival_time = get_time_ms()？

// 因为 FIFO 要看：

// 谁先请求 dongle

// 所以 coder 发出请求那一刻，要记录当前时间：

// request.arrival_time = get_time_ms();

// FIFO 比较的就是：

// arrival_time 小的人优先
// 2. deadline 应该是什么？

// Subject 里 EDF 的 deadline 是：

// last_compile_start + time_to_burnout

// 意思是：

// 这个 coder 最晚必须在什么时候再次开始 compile

// last_compile_start 来自：

// update_compile_start(coder);

// 每次 coder 开始 compile 前更新。

// 所以：

// request.deadline = coder->last_compile_start
// 	+ coder->app->config.time_to_burnout;

// 例如：

// last_compile_start = 1000
// time_to_burnout = 800
// deadline = 1800

// 意思：

// 这个 coder 1800ms 之后还没重新开始 compile，就 burned out
// 3. pthread_cond 是什么？

// pthread_cond_t 是：

// 条件变量

// 它常常和 mutex 一起用。

// 作用：

// 线程睡觉等待某个条件变化

// 不是一直 while 空转浪费 CPU。

// pthread_cond_wait(&cond, &mutex)

// 意思：

// 当前 thread 睡觉
// 并且自动释放 mutex
// 等别人 broadcast/signal 后醒来
// 醒来时重新拿回 mutex

// 很重要：它会 释放 mutex，否则别人永远进不来修改 dongle 状态。

// pthread_cond_broadcast(&cond)

// 意思：

// 通知所有正在等这个 cond 的 threads：
// 条件可能变了，醒来重新检查

// 比如 release dongle 后：

// pthread_cond_broadcast(&dongle->cond);

// 告诉等待的人：

// dongle 可能空了 / cooldown 变了 / priority 可能轮到你了
// 4. if (!is_stopped(coder->app))

// 对。

// is_stopped(app)

// 返回：

// 1 = stopped
// 0 = not stopped

// 所以：

// !is_stopped(app)

// 意思：

// not stopped，程序还没停止
// 5. 这个 while 什么时候 break？
// while (!is_stopped(coder->app)
// 	&& !can_take_dongle(coder, dongle))
// 	pthread_cond_wait(&dongle->cond, &dongle->mutex);

// 意思是：

// 只要：
// 程序没停止
// 并且
// 我还不能拿这个 dongle

// 那我就睡觉等待

// 它会退出 while 的两种情况：

// 情况 A：程序 stop 了
// is_stopped == 1

// !is_stopped == 0，while 结束。

// 情况 B：我可以拿 dongle 了
// can_take_dongle == 1

// !can_take_dongle == 0，while 结束。

// 6. 什么叫可以拿 dongle？
// can_take_dongle()

// 里面检查三件事：

// 1. dongle 没被占用 occupied == 0
// 2. cooldown 已结束 get_time_ms() >= available_at
// 3. heap top 是自己 heap_peek(dongle) == coder->id

// 三件都满足，才能拿。

// 7. 为什么要 while，不是 if？

// 因为 pthread_cond_wait 醒来后，不代表一定轮到你。

// 它只是说：

// 条件可能变了

// 醒来后必须重新检查。

// 比如 broadcast 唤醒了 5 个 coder，但只有 heap top 那个能拿。

// 所以必须：

// wake up -> check again -> 不满足继续睡
// 8. occupied = 1 是什么意思？

// 对，意思是：

// 这个 dongle 当前被占用了

// 当 coder 成功拿到 dongle：

// dongle->occupied = 1;

// 释放时：

// dongle->occupied = 0;

// 所以：

// occupied = 1  不能被别人拿
// occupied = 0  空闲，但还要看 cooldown 和 heap priority