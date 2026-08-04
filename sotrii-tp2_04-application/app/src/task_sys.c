/*
 * Copyright (c) 2026 Sebastian Bedin <sebabedin@gmail.com> &
 * 					  Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Sebastian Bedin <sebabedin@gmail.com> &
 * 			 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"
#include "cmsis_os.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_sys_attribute.h"
#include "task_sys_interface.h"
#include "task_led_attribute.h"
#include "task_led_interface.h"
#include "task_led.h"

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI	0ul

#define DEL_SYS_MIN			(pdMS_TO_TICKS(50ul))
#define DEL_SYS_BLINK		(pdMS_TO_TICKS(500ul))

#define TASK_SYS_DEL_ZERO	(pdMS_TO_TICKS(0ul))
#define TASK_SYS_DEL_MAX	DEL_SYS_MIN

#define BTN_OP_TIME_THRESHOLD (pdMS_TO_TICKS(2000ul))

#define G_TASK_SEND_LED_AO_RUNTIME_US_INI	0ul
/********************** internal data declaration ****************************/
sys_sc_t sys_sc = {ST_SYS_INIT, {EV_SYS_OFF, ZERO}, ZERO, EV_SYS_NONE, ZERO, ZERO};
sys_ao_t sys_ao = {NULL, "Queue SYS AO", NULL, "Task SYS AO"};

/********************** internal functions declaration ***********************/
void task_sys_statechart(h_sys_t *h_sys_);

/********************** internal data definition *****************************/
static btn_id_t btn_active = -1;
/********************** external data declaration ****************************/
uint32_t g_task_sys_cnt;
uint32_t g_task_send_led_ao_runtime_us;

h_sys_t h_sys = {&sys_sc, &sys_ao};

TickType_t btn_op_time[BTN_QTY] = {
	pdMS_TO_TICKS(5000ul),
	pdMS_TO_TICKS(10000ul)
};

/********************** external functions definition ************************/
/* Task thread */
void task_sys(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_sys_cnt = G_TASK_SYS_CNT_INI;
	g_task_send_led_ao_runtime_us = G_TASK_SEND_LED_AO_RUNTIME_US_INI;
	h_sys_t *p_h_sys = (h_sys_t *)parameters;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
    {
		/* Update Task Counter */
		g_task_sys_cnt++;

		/* Get Events to excite Statechart */
		if (pdFAIL == xQueueReceive(p_h_sys->sys_ao->h_queue, (void *)&p_h_sys->sys_sc->ev_in, (TickType_t)ZERO))
		{
			p_h_sys->sys_sc->ev_in.ev = EV_SYS_NONE;
		}

		/* Run Statechart */
    	task_sys_statechart(p_h_sys);

    	/* We want this task to execute every 50 milliseconds. */
		vTaskDelay(TASK_SYS_DEL_MAX);
	}
}

void task_sys_statechart(h_sys_t *h_sys_)
{
	switch (h_sys_->sys_sc->state)
	{
		case ST_SYS_INIT:
			h_sys_->sys_sc->ev_out = EV_SYS_ON;

			/* Measure the WCT of send_led_ao which is the interface of the led active object*/
			cycle_counter_init();
			send_led_ao(&h_led[LED_A], (void *)&h_sys_->sys_sc->ev_out);
			g_task_send_led_ao_runtime_us = cycle_counter_get_time_us();

			send_led_ao(&h_led[LED_B], (void *)&h_sys_->sys_sc->ev_out);

			h_sys_->sys_sc->ev_out = EV_SYS_OFF;
			send_led_ao(&h_led[LED_C], (void *)&h_sys_->sys_sc->ev_out);

			h_sys_->sys_sc->state = ST_SYS_IDLE;

			break;

		case ST_SYS_IDLE:

			if (EV_SYS_OFF == h_sys_->sys_sc->ev_in.ev) {
				btn_active = h_sys_->sys_sc->ev_in.id;

				if (h_sys_->sys_sc->ev_in.tick > BTN_OP_TIME_THRESHOLD) {
					btn_op_time[btn_active] = h_sys_->sys_sc->ev_in.tick;
				}

				h_sys_->sys_sc->state = ST_SYS_ACTIVE;
				h_sys_->sys_sc->tick = ZERO;

				h_sys_->sys_sc->ev_out = EV_SYS_BLINK;
				send_led_ao(&h_led[btn_active], (void *)&h_sys_->sys_sc->ev_out);

				h_sys_->sys_sc->ev_out = EV_SYS_ON;
				send_led_ao(&h_led[LED_C], (void *)&h_sys_->sys_sc->ev_out);
			} else {
				h_sys_->sys_sc->tick += DEL_SYS_MIN;
			}

			break;

		case ST_SYS_ACTIVE:
			if (h_sys_->sys_sc->tick > btn_op_time[btn_active]) {
				h_sys_->sys_sc->state = ST_SYS_INIT;
				h_sys_->sys_sc->tick = ZERO;
			} else {
				h_sys_->sys_sc->tick += DEL_SYS_MIN;
			}

			break;
	}
}

/********************** end of file ******************************************/
