/*
 * Copyright (c) 2021-2024 LAAS-CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

/**
 * @brief  This file it the main entry point of the
 *         OwnTech Power API. Please check the OwnTech
 *         documentation for detailed information on
 *         how to use Power API: https://docs.owntech.org/
 *
 * @author Clément Foucher <clement.foucher@laas.fr>
 * @author Luiz Villa <luiz.villa@laas.fr>
 */

//--------------OWNTECH APIs----------------------------------
#include "DataAPI.h"
#include "TaskAPI.h"
#include "TwistAPI.h"
#include "SpinAPI.h"

//--------------SETUP FUNCTIONS DECLARATION-------------------
void setup_routine(); // Setups the hardware and software of the system

//--------------LOOP FUNCTIONS DECLARATION--------------------
void loop_background_task(); // Code to be executed in the background task
void loop_critical_task();   // Code to be executed in real time in the critical task

//--------------USER VARIABLES DECLARATIONS-------------------
bool pwm_enable = false;
//float32_t meas_data;

//--------------SETUP FUNCTIONS-------------------------------

/**
 * This is the setup routine.
 * It is used to call functions that will initialize your spin, twist, data and/or tasks.
 * In this example, we setup the version of the spin board and a background task.
 * The critical task is defined but not started.
 */
void setup_routine()
{
    // Setup the hardware first
    spin.version.setBoardVersion(SPIN_v_1_0);
    twist.setVersion(shield_TWIST_V1_3);

    twist.initAllBuck(VOLTAGE_MODE);
    //twist.initLegBuck(LEG1, VOLTAGE_MODE);
    //twist.initLegBoost(LEG2);
    twist.setAllAdcDecim(1);
    twist.setAllDeadTime(200, 200);
    twist.setLegPhaseShift(LEG2, 180);
    //twist.setAllPhaseShift(180);
    data.enableTwistDefaultChannels();
    twist.setAllDutyCycle(0.9);
    //twist.startAll();

    // ------------------------------ TASKS -------------------------------
    uint32_t background_task_number = task.createBackground(loop_background_task);
    task.createCritical(loop_critical_task, 500); // Uncomment if you use the critical task
    // Finally, start tasks
    task.startBackground(background_task_number);
    task.startCritical(); // Uncomment if you use the critical task
}

//--------------LOOP FUNCTIONS--------------------------------

/**
 * This is the code loop of the background task
 * It is executed every second as defined by its "suspend task" in its last line.
 * You can use it to execute slow code such as state-machines.
 */
void loop_background_task()
{
    // Task content
    spin.led.toggle();

    // Pause between two runs of the task
    task.suspendBackgroundMs(1000);
}

/**
 * This is the code loop of the critical task
 * It is executed every 500 micro-seconds defined in the setup_software function.
 * You can use it to execute an ultra-fast code with the highest priority which cannot be interruped.
 */
void loop_critical_task()
{
    if (!pwm_enable)
    {
        pwm_enable = true;
        twist.startAll();
    }
}

/**
 * This is the main function of this example
 * This function is generic and does not need editing.
 */
int main(void)
{
    setup_routine();

    return 0;
}
