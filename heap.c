#include "codexion.h"

int heap_push(t_dongle *dongle, t_request request, int scheduler)
{
    if (dongle->size >= dongle->capacity)
        return (0);
    dongle->heap[dongle->size] = request;
    heap_up(dongle, dongle->size, scheduler);
    dongle->size++;
    return(1);
}

int heap_peek(t_dongle *dongle)
{
    if (dongle->size <= 0)
        return (-1);
    return (dongle->heap[0].coder_id);
}

int heap_remove(t_dongle *dongle, int coder_id, int scheduler)
{
    int i;
    int parent;

    i = 0;
    while(i < dongle->size)  // sorting through, arbitrary remove: coder got dongle, coder burnedout(edf reached)//monitor thread fdound coder dead, simulation stopped
    {
        if (dongle->heap[i].coder_id == coder_id)  // get the coder in heap
        {
            dongle->heap[i] = dongle->heap[dongle->size - 1]; //replace the removed one pos with the last ele
            dongle->size--; //effective range exclude the last ele, which will be released together with dongle in the end
            
            if (i > 0)
            {
                parent = (i - 1) / 2;
                if (request_less(dongle->heap[i], dongle->heap[parent], scheduler)) // 1, right, child < parent
                    heap_up(dongle, i, scheduler);
                else
                    heap_down(dongle, i, scheduler); // !!! only focus on the current, before already sorted
            }
            else
                heap_down(dongle, i, scheduler);
            return (1);
        }
        i++;
    }
    return (0);
}

// find the coder to be removed, last ele cover, reduce effective scope,
// if not on the top(in the middle), compare with the current(coz previous already sorted), head up or down;
// if on the top, head down;

// 1. queue is a rule 
// a. so binary (min) heap (priority) queue(partical searching, child-parent priority) 
// is also a queue, is the rule differing from 
// normal queue(whole searching, fifo, strict sequencing cannot swap), 
// which can also change into binary queue after adding "compare, swap, bubble up/down"

// b. on the other hand, talking of storage, we are using "heap memory" storage malloc, 
// coz consecutive memory storage more consecutive and faster.

// 2. new ele pushed and inserted at the end->request_less compared child-parent->if child smaller swap->finish partial sorting, peek top smallest

// 1. 为什么叫 binary heap？

// 其实：

// “binary”

// 指：

// binary tree structure

// 因为每个节点：

// 最多：

// 两个 children

// 例如：

//           A
//        /     \
//       B       C
//     /   \
//    D     E
// “heap”

// 不是 heap memory。

// 这里：

// heap

// 指：

// heap property

// 例如：

// min heap
// parent <= children
// max heap
// parent >= children
// 所以：
// binary heap

// 真正意思：

// “满足 heap property 的 binary tree”
// 为什么叫 heap？

// 历史命名。

// 意思接近：

// “堆起来”

// priority 元素：

// 堆在顶部
// 2. “binary 就够了，为什么还叫 heap？”

// 因为：

// binary tree ≠ binary heap
// 普通 binary tree

// 没有 priority rule。

// 例如：

//         A
//       /   \
//      Z     B

// 也可以。

// binary heap

// 必须：

// parent 比 children 更优先

// 所以：

// binary

// 描述：

// 树结构
// heap

// 描述：

// priority ordering rule
// 3. 你现在这句理解已经非常接近正确：

// 你写：

// binary heap queue
// (partial ordering, child-parent priority)
// 基本正确。

// 更准确：

// binary heap priority queue

// 因为：

// queue

// 表示：

// 等待结构
// priority queue

// 表示：

// priority-first rule
// binary heap

// 表示：

// 实现 priority queue 的数据结构
// 4. “normal queue can also become binary heap queue?”
// 对！！！

// 非常正确。🔥

// queue 本来只是：
// “等待元素集合”
// FIFO queue

// 规则：

// arrival order
// priority queue

// 规则：

// priority order
// 如果你：
// 增加 compare
// 增加 swap
// 增加 bubble up/down

// 于是：

// 它已经不再是 FIFO queue

// 而变成：

// priority queue
// 如果：

// priority queue 又采用：

// binary heap ordering

// 它就：

// 变成 binary heap priority queue
// 所以：
// queue 是规则层

// 不是实现层。

// 5. “queue is a rule for data structure”
// 完全正确。🔥

// queue / stack / priority queue：

// 都是：

// abstract data structure semantics

// 例如：

// 名字	规则
// stack	LIFO
// queue	FIFO
// priority queue	priority first
// 实现它们的方法：

// 可以：

// array
// linked list
// heap
// tree
// 6. “malloc heap memory because contiguous and faster?”
// 对！！！

// 这里你已经把：

// heap memory

// vs

// binary heap

// 区分开了。🔥

// 我们这里：
// malloc(sizeof(t_request) * capacity)

// 使用：

// heap memory（堆区）
// 为什么？

// 因为：

// dynamic size

// 运行时才知道：

// number_of_coders
// contiguous memory

// 得到：

// [request][request][request]

// 连续内存。

// 为什么连续快？
// CPU cache locality

// CPU 喜欢：

// 连续读取
// linked nodes：
// pointer jump jump jump

// 慢。

// heap array：
// 连续 memory

// cache friendly。

// 所以：

// binary heap 常常：

// 用 array 存储

// 不是 linked nodes。

// 最核心总结
// queue / priority queue

// 是：

// 行为规则
// binary heap

// 是：

// 实现 priority queue 的部分排序 binary tree
// malloc heap memory

// 是：

// 连续动态内存区域
// 三个 “heap/queue”

// 名字像。

// 但：

// 完全不同层面的概念。