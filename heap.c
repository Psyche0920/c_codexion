#include "codexion.h"

static int  request_less(t_request a, t_request b, int scheduler)
{
    if (scheduler == 0)
        return (a.arrival_time < b.arrival_time);
    return (a.deadline < b.deadline);

}

static void swap_request(t_request *a, t_request *b)
{
    t_request   tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heap_up(t_dongle *dongle, int index, int scheduler)
{
    int parent;

    while (index > 0)
    {
        parent = (index - 1) / 2;
        if (!request_less(dongle->heap[index], dongle->heap[parent], scheduler))
            break ;
        swap_request(&dongle->heap[index], &dongle->heap[parent]);
        index = parent;
    }
}

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

// queue only FIFO 严格顺序 cannot A 和 C 交换
