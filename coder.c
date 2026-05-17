/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 17:41:54 by wehan             #+#    #+#             */
/*   Updated: 2026/05/17 17:45:02 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	safe_log(coder->app, coder->id, "is refactoring");
	usleep(1000 * 1000);
	safe_log(coder->app, coder->id, "is debugging");
	return (NULL);	
}
