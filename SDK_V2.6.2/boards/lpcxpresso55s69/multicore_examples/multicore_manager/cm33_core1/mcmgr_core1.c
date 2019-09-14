/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "mcmgr.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_INIT() GPIO_PinInit(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, &led_config);
#define LED_TOGGLE() GPIO_PortToggle(GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
#define APP_READY_EVENT_DATA (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Function to create delay for Led blink.
 */
void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    MCMGR_EarlyInit();
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t startupData, i;
    mcmgr_status_t status;

    /* After 100 cycles, an exception will occur */
    uint32_t timeToException = 100;

    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };

    /* Initialize MCMGR, install generic event handlers */
    MCMGR_Init();

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);

    /* Make a noticable delay after the reset */
    /* Use startup parameter from the master core... */
    for (i = 0; i < startupData; i++)
        delay();

    /* Init board hardware.*/
    /* enable clock for GPIO */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    BOARD_InitPins_Core1();
    /* Configure LED */
    LED_INIT();

    /* Signal the other core we are ready by triggering the event and passing the APP_READY_EVENT_DATA */
    MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, APP_READY_EVENT_DATA);

    while (1)
    {
        delay();
        LED_TOGGLE();

        if (--timeToException == 0)
        {
            /* Trigger an exception here! Try to write to an invalid address! */
            *((uint32_t *)0xFFFFFFFF) = timeToException;
            /* mcmgr.c contains an exception handler,
               which will signal the exception to the other core */
        }
    }
}
