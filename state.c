/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/23 13:51:41 by wehan             #+#    #+#             */
/*   Updated: 2026/05/23 15:03:39 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	all_coders_done(t_app *app)
{
	int	i;

	pthread_mutex_lock(&app->state_mutex);
	i = 0;
	while (i < app->config.number_of_coders)
	{
		if (app->coders[i].compile_count
			< app->config.number_of_compiles_required)
		{
			pthread_mutex_unlock(&app->state_mutex);
			return (0);
		}
		i++;
	}
	pthread_mutex_unlock(&app->state_mutex);
	return (1);
}
