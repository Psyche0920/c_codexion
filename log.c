/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 16:18:11 by wehan             #+#    #+#             */
/*   Updated: 2026/05/19 00:48:51 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	safe_log(t_app *app, int id, char *msg)
{
	pthread_mutex_lock(&app->log_mutex);
	if (!is_stopped(app))
		printf("%ld %d %s\n", timestamp(app), id, msg);
	pthread_mutex_unlock(&app->log_mutex);
}

// 意思：

// stop == 0:
//     正常打印

// stop == 1:
//     普通状态不再打印

// msg == "burned out":
//     即使 stop，也允许打印
