/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --cmse"
#endif

#include "fsl_device_registers.h"
#include "arm_cmse.h"
#include "board.h"
#include "veneer_table.h"
#include "tzm_config.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CODE_START_NS              0x00010000

#define DEMO_SECURE_SW1_GPIO            SECGPIO
#define DEMO_SECURE_SW1_GPIO_PORT       0U
#define DEMO_SECURE_SW1_GPIO_PIN        5U

#define DEMO_SW1_GPIO                   BOARD_SW1_GPIO
#define DEMO_SW1_GPIO_PORT              BOARD_SW1_GPIO_PORT
#define DEMO_SW1_GPIO_PIN               BOARD_SW1_GPIO_PIN

#define DEMO_SW2_GPIO                   BOARD_SW2_GPIO
#define DEMO_SW2_GPIO_PORT              BOARD_SW2_GPIO_PORT
#define DEMO_SW2_GPIO_PIN               BOARD_SW2_GPIO_PIN

#define DEMO_BLUE_LED_PIN_SEC_MASK      AHB_SECURE_CTRL_SEC_GPIO_MASK0_PIO0_PIN5_SEC_MASK_MASK

#define DEMO_SECURE_GPIO_CLOCK         kCLOCK_Gpio_Sec
#define DEMO_SECURE_GPIO_RST           kGPIOSEC_RST_SHIFT_RSTn

#define DEMO_SYSTICK_CLK_FREQ           CLOCK_GetFreq(kCLOCK_Systick0)
#define NON_SECURE_START          DEMO_CODE_START_NS

/* typedef for non-secure callback functions */
typedef void (*funcptr_ns) (void) __attribute__((cmse_nonsecure_call));
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void) 
{
/* The TrustZone should be configured as early as possible after RESET.
 * Therefore it is called from SystemInit() during startup. The SystemInitHook() weak function 
 * overloading is used for this purpose.
*/
    BOARD_InitTrustZone();
}

/*!
 * @brief SysTick Handler
 */
void SysTick_Handler(void)
{
    /* Control GPIO MASK based on S2 button press */
    if (GPIO_PinRead(DEMO_SW2_GPIO, DEMO_SW2_GPIO_PORT, DEMO_SW2_GPIO_PIN))
    {
        AHB_SECURE_CTRL->SEC_GPIO_MASK0 |= DEMO_BLUE_LED_PIN_SEC_MASK;
    }
    else
    {
        AHB_SECURE_CTRL->SEC_GPIO_MASK0 &= ~DEMO_BLUE_LED_PIN_SEC_MASK;
    }
    /* Control green LED based on S1 button press */
    if (GPIO_PinRead(DEMO_SECURE_SW1_GPIO, DEMO_SECURE_SW1_GPIO_PORT, DEMO_SECURE_SW1_GPIO_PIN))
    {
        LED_GREEN_OFF();
    }
    else
    {
        LED_GREEN_ON();
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    funcptr_ns ResetHandler_ns;

    /* Board pin init */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();

    /* Initialize secure GPIO for button S1. */
    CLOCK_EnableClock(DEMO_SECURE_GPIO_CLOCK);
    RESET_PeripheralReset(DEMO_SECURE_GPIO_RST);
    GPIO_PinInit(DEMO_SECURE_SW1_GPIO, DEMO_SECURE_SW1_GPIO_PORT, DEMO_SECURE_SW1_GPIO_PIN,
                 &(gpio_pin_config_t){kGPIO_DigitalInput, 0});

    /* Initialize GPIO ports for buttons S1, S2 and RGB LED*/
    GPIO_PortInit(DEMO_SW1_GPIO, DEMO_SW1_GPIO_PORT);
    GPIO_PortInit(DEMO_SW2_GPIO, DEMO_SW2_GPIO_PORT);
    GPIO_PortInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT);
    GPIO_PortInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT);
    
    /* Initialize GPIO pin for button S1. */
    GPIO_PinInit(DEMO_SW1_GPIO, DEMO_SW1_GPIO_PORT, DEMO_SW1_GPIO_PIN, \
                 &(gpio_pin_config_t){kGPIO_DigitalInput, 0});
    
    /* Initialize GPIO pin for button S2. */
    GPIO_PinInit(DEMO_SW2_GPIO, DEMO_SW2_GPIO_PORT, DEMO_SW2_GPIO_PIN, \
                 &(gpio_pin_config_t){kGPIO_DigitalInput, 0});
    
    /* Initialize GPIO PINS for RGB LED*/
    LED_BLUE_INIT(0x1U);
    LED_GREEN_INIT(0x1U);

    /* Set systick reload value to generate 5ms interrupt */
    SysTick_Config(USEC_TO_COUNT(5000U, DEMO_SYSTICK_CLK_FREQ));

    /* Set non-secure main stack (MSP_NS) */
    __TZ_set_MSP_NS(*((uint32_t *)(NON_SECURE_START)));
 
    /* Set non-secure vector table */
    SCB_NS->VTOR = NON_SECURE_START;
    
    /* Get non-secure reset handler */
    ResetHandler_ns = (funcptr_ns)(*((uint32_t *)((NON_SECURE_START) + 4U)));
     
    /* Call non-secure application - jump to normal world */
    /*  */
    ResetHandler_ns();    
    while (1)
    {
        /* This point should never be reached */
    }
}
