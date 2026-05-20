#include "codexion.h"

int  request_less(t_request a, t_request b, int scheduler)
{
    if (scheduler == 0)
        return (a.arrival_time < b.arrival_time);
    return (a.deadline < b.deadline);

}

void swap_request(t_request *a, t_request *b)
{
    t_request   tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

void heap_up(t_dongle *dongle, int index, int scheduler)
{
    int parent;

    while (index > 0)
    {
        parent = (index - 1) / 2;
        if (request_less(dongle->heap[index], dongle->heap[parent], scheduler))
        {
            swap_request(&dongle->heap[index], &dongle->heap[parent]);
            index = parent;
        }
        else
            break ; // return 0, if child > parent, break
        swap_request(&dongle->heap[index], &dongle->heap[parent]);
        index = parent; //move up
    }
}

void heap_down(t_dongle *dongle, int index, int scheduler)
{
    int left;
    int right;
    int best;

    while(1)
    {
        left = index * 2 + 1;
        right = index *2 + 2;
        best = index;
        if (left < dongle->size && request_less(dongle->heap[left], dongle->heap[best], scheduler))
            best = left;
        if (right < dongle->size && request_less(dongle->heap[right], dongle->heap[best], scheduler))
            best = right;
        if (best == index)
            break ;
        swap_request(&dongle->heap[index], &dongle->heap[best]);
            index = best;
    }
}