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
float32_t dutyCycle = 0.1;
//float32_t meas_data;

//--------------SETUP FUNCTIONS-------------------------------

/**
 * Routine de setup, 
 * It is used to call functions that will initialize your spin, twist, data and/or tasks.
 */
void setup_routine()
{
    // Setup du hardware en premier
    spin.version.setBoardVersion(SPIN_v_1_0);
    twist.setVersion(shield_TWIST_V1_3);

    // ----------------------------- User Setup ---------------
    // Setup du hacheur 4Q
    twist.initLegBuck(LEG1);
    twist.initLegBoost(LEG2);
    twist.setAllAdcDecim(1);
    twist.setAllDeadTime(200, 200);
    data.enableTwistDefaultChannels();
    twist.setAllDutyCycle(dutyCycle); // TODO à déplacer
    twist.setAllTriggerValue(0.5);

    // ------------------------------ TASKS -------------------------------
    // à faire à la fin du setup
    uint32_t background_task_number = task.createBackground(loop_background_task);
    task.createCritical(loop_critical_task, 500);
    // Démarrer les taches
    task.startBackground(background_task_number);
    task.startCritical();
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
    /* Test du changement de rapport cyclyque (le passage de 0.9 à 0.1 est quelque peu violent)
    if (dutyCycle <= 0.9)
    {
        dutyCycle = dutyCycle + 0.00001;
    }

    if (dutyCycle > 0.9)
    {
        dutyCycle = 0.1;
    }
    */

    twist.setAllDutyCycle(dutyCycle);
    data.getLatest();

}

/**
 * Fonction main générique / ne pas modifier
 */
int main(void)
{
    setup_routine();

    return 0;
}
