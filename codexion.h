/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wehan <wehan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:36:39 by wehan             #+#    #+#             */
/*   Updated: 2026/05/17 16:57:05 by wehan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

typedef struct s_config
{
	int		number_of_coders;
	long	time_to_burnout;
	long	time_to_compile;
	long	time_to_debug;
	long	time_to_refactor;
	int		number_of_compiles_required;
	long	dongle_cooldown;
	int		scheduler;
}	t_config;

typedef struct s_app
{
	t_config		config;
	long			start_time;
	pthread_mutex_t	log_mutex;
}	t_app;

int		parse_args(int argc, char **argv, t_config *config);
long	get_time_ms(void);
long	timestamp(t_app *app);
void	safe_log(t_app *app, int id, char *msg);

#endif
