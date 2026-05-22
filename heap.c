/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 18:05:49 by wehan             #+#    #+#             */
/*   Updated: 2026/05/22 19:33:04 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_push(t_dongle *dongle, t_request request, int scheduler)
{
	if (dongle->size >= dongle->capacity)
		return (0);
	dongle->heap[dongle->size] = request;
	heap_up(dongle, dongle->size, scheduler);
	dongle->size++;
	return (1);
}

int	heap_peek(t_dongle *dongle)
{
	if (dongle->size <= 0)
		return (-1);
	return (dongle->heap[0].coder_id);
}

static void	recorder_after_remove(t_dongle *dongle, int i, int scheduler)
{
	int	parent;

	if (i > 0)
	{
		parent = (i - 1) / 2;
		if (request_less(dongle->heap[i], dongle->heap[parent], scheduler))
			heap_up(dongle, i, scheduler);
		else
			heap_down(dongle, i, scheduler);
	}
	else
		heap_down(dongle, i, scheduler);
}

int	heap_remove(t_dongle *dongle, int coder_id, int scheduler)
{
	int	i;

	i = 0;
	while (i < dongle->size)
	{
		if (dongle->heap[i].coder_id == coder_id)
		{
			dongle->heap[i] = dongle->heap[dongle->size - 1];
			dongle->size--;
			recorder_after_remove(dongle, i, scheduler);
			return (1);
		}
		i++;
	}
	return (0);
}

// find the coder to be removed, last ele cover, reduce effective scope,
// if not on the top(in the middle), 
// compare with the current(coz previous already sorted), head up or down;
// if on the top, head down;
