// 1. 
// if (scheduler == FIFO)
// 	比较 arrival_time;
// else if (scheduler == EDF)
// 	比较 deadline;

// 2. philosophers/codexion 为什么要求 heap?

// 因为它真正模拟：
// OS scheduler / real-time scheduling

// 真实 OS：
// earliest deadline first EDF
// 本来就是：
// priority scheduling problem
// 不是普通 queue problem。

// 最核心一句
// queue 只能自然处理“先来先服务” FIFO
// heap 可以处理“按优先级服务” 
// 所以 EDF 必须用：
// priority queue / heap