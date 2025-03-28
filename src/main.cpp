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
#include "CommunicationAPI.h"

//--------------SETUP FUNCTIONS DECLARATION-------------------
void setup_routine(); // Setups the hardware and software of the system

//--------------LOOP FUNCTIONS DECLARATION--------------------
void loop_background_task(); // Code to be executed in the background task
void loop_critical_task();   // Code to be executed in real time in the critical task

//--------------USER VARIABLES DECLARATIONS-------------------
bool pwm_enable = false; // Activer la PWM une fois souhaité
float32_t dutyCycle = 0.1;
volatile float32_t VI2Low = 0;
volatile float32_t II2Low = 0;
volatile float32_t IIHigh = 0;
volatile float32_t VIHigh = 0;
volatile float32_t VI1Low = 0;
volatile float32_t II1Low = 0;
volatile uint32_t encoderValue = 0;
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

    // Setup de l'ADC
    twist.setAllAdcDecim(1);
    twist.setAllDeadTime(200, 200);
    data.enableTwistDefaultChannels();
    twist.setAllTriggerValue(0.5);

    // Rapport cyclique initiale
    twist.setAllDutyCycle(dutyCycle); // TODO à déplacer

    // Activation des broches C0 à C3 & A1 et A0 pour les prises de mesure
    data.enableAcquisition(1, 24); // C0; VI2Low
    data.enableAcquisition(1, 25); // C1; II2Low
    data.enableAcquisition(1, 26); // C2; IIHigh
    data.enableAcquisition(1, 27); // C3, VIHigh
    data.enableAcquisition(1, 29); // A0; VI1Low
    data.enableAcquisition(1, 30); // A1; II1Low

    // Setup de l'encodeur
    spin.timer.startLogTimer4IncrementalEncoder();

    // ------------------------------ TASKS ------------------------------- à faire à la FIN DU SETUP
    // 
    // Créer les taches
    uint32_t background_task_number = task.createBackground(loop_background_task);
    task.createCritical(loop_critical_task, 500); // en micro secondes
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
    // Aquisition et affichage des mesures de Courant/Tension
    data.triggerAcquisition(1);
    VI2Low = data.getLatest(1, 24);
    II2Low = data.getLatest(1, 25);
    IIHigh = data.getLatest(1, 26);
    VIHigh = data.getLatest(1, 27);
    VI1Low = data.getLatest(1, 29);
    II1Low = data.getLatest(1, 30);
    printk("VI2Low;\%f;II2Low;\%f;IIHigh;\%f;VIHigh;\%f;VI1Low;\%f;II1Low;\%f\n"
        , VI2Low, II2Low, IIHigh, VIHigh, VI1Low, II1Low);

    // Récupération et affichage de la valeur de l'encodeur
    //printk("Encodeur=\%d \n", encoderValue);
    
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
    // Allumage de la PWM
    if (!pwm_enable)
    {
        pwm_enable = true;
        twist.startAll();

    }
    
    // Test de la variation de rapport cyclyque (le passage de 0.9 à 0.1 est quelque peu violent)
    /*if (dutyCycle <= 0.9)
    {
        dutyCycle = dutyCycle + 0.00001;
    }

    if (dutyCycle > 0.9)
    {
        dutyCycle = 0.1;
    }*/

    encoderValue = spin.timer.getTimer4IncrementalEncoderValue();

    // étape finale de loop_critical_task(), mise à jour du rapport cyclique
    twist.setAllDutyCycle(dutyCycle);
}

/**
 * Fonction main générique / ne pas modifier
 */
int main(void)
{
    setup_routine();

    return 0;
}