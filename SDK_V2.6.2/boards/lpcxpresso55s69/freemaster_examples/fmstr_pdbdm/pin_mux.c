/*
 * Copyright 2019 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v5.0
processor: LPC55S69
package_id: LPC55S69JBD100
mcu_data: ksdk2_0
processor_version: 5.0.0
board: LPCXpresso54608
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: cm33_core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '92', peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI_DATA, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI_DATA/SD1_D2/CTIMER2_MAT3/SCT0_OUT8/CMP0_OUT/PLU_OUT2/SECURE_GPIO0_29,
    mode: inactive, slew_rate: standard, invert: disabled, open_drain: disabled}
  - {pin_num: '94', peripheral: FLEXCOMM0, signal: TXD_SCL_MISO_WS, pin_signal: PIO0_30/FC0_TXD_SCL_MISO_WS/SD1_D3/CTIMER0_MAT0/SCT0_OUT9/SECURE_GPIO0_30, mode: inactive,
    slew_rate: standard, invert: disabled, open_drain: disabled}
  - {pin_num: '21', peripheral: SWD, signal: SWO, pin_signal: PIO0_10/ADC0_1/FC6_SCK/CT_INP10/CTIMER2_MAT0/FC1_TXD_SCL_MISO_WS/SCT0_OUT2/SWO/SECURE_GPIO0_10, mode: inactive,
    slew_rate: standard, invert: disabled, open_drain: disabled, asw: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Cortex-M33 (Core #0) */
void BOARD_InitPins(void)
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);

    IOCON->PIO[0][10] =
        ((IOCON->PIO[0][10] &
          /* Mask bits to zero which are setting */
          (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_MODE_MASK | IOCON_PIO_SLEW_MASK | IOCON_PIO_INVERT_MASK | IOCON_PIO_OD_MASK | IOCON_PIO_ASW_MASK)))

         /* Selects pin function.
          * : PORT010 (pin 21) is configured as SWO. */
         | IOCON_PIO_FUNC(PIO0_10_FUNC_ALT6)

         /* Selects function mode (on-chip pull-up/pull-down resistor control).
          * : Inactive.
          * Inactive (no pull-down/pull-up resistor enabled). */
         | IOCON_PIO_MODE(PIO0_10_MODE_INACTIVE)

         /* Driver slew rate.
          * : Standard mode, output slew rate control is enabled.
          * More outputs can be switched simultaneously. */
         | IOCON_PIO_SLEW(PIO0_10_SLEW_STANDARD)

         /* Input polarity.
          * : Disabled.
          * Input function is not inverted. */
         | IOCON_PIO_INVERT(PIO0_10_INVERT_DISABLED)

         /* Controls open-drain mode.
          * : Normal.
          * Normal push-pull output. */
         | IOCON_PIO_OD(PIO0_10_OD_NORMAL)

         /* Analog switch input control.
          * Usable only if DIGIMODE = 0b0: Analog switch is open. */
         | IOCON_PIO_ASW(PIO0_10_ASW_DISABLE));

    IOCON->PIO[0][29] =
        ((IOCON->PIO[0][29] &
          /* Mask bits to zero which are setting */
          (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_MODE_MASK | IOCON_PIO_SLEW_MASK | IOCON_PIO_INVERT_MASK | IOCON_PIO_OD_MASK)))

         /* Selects pin function.
          * : PORT029 (pin 92) is configured as FC0_RXD_SDA_MOSI_DATA. */
         | IOCON_PIO_FUNC(PIO0_29_FUNC_ALT1)

         /* Selects function mode (on-chip pull-up/pull-down resistor control).
          * : Inactive.
          * Inactive (no pull-down/pull-up resistor enabled). */
         | IOCON_PIO_MODE(PIO0_29_MODE_INACTIVE)

         /* Driver slew rate.
          * : Standard mode, output slew rate control is enabled.
          * More outputs can be switched simultaneously. */
         | IOCON_PIO_SLEW(PIO0_29_SLEW_STANDARD)

         /* Input polarity.
          * : Disabled.
          * Input function is not inverted. */
         | IOCON_PIO_INVERT(PIO0_29_INVERT_DISABLED)

         /* Controls open-drain mode.
          * : Normal.
          * Normal push-pull output. */
         | IOCON_PIO_OD(PIO0_29_OD_NORMAL));

    IOCON->PIO[0][30] =
        ((IOCON->PIO[0][30] &
          /* Mask bits to zero which are setting */
          (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_MODE_MASK | IOCON_PIO_SLEW_MASK | IOCON_PIO_INVERT_MASK | IOCON_PIO_OD_MASK)))

         /* Selects pin function.
          * : PORT030 (pin 94) is configured as FC0_TXD_SCL_MISO_WS. */
         | IOCON_PIO_FUNC(PIO0_30_FUNC_ALT1)

         /* Selects function mode (on-chip pull-up/pull-down resistor control).
          * : Inactive.
          * Inactive (no pull-down/pull-up resistor enabled). */
         | IOCON_PIO_MODE(PIO0_30_MODE_INACTIVE)

         /* Driver slew rate.
          * : Standard mode, output slew rate control is enabled.
          * More outputs can be switched simultaneously. */
         | IOCON_PIO_SLEW(PIO0_30_SLEW_STANDARD)

         /* Input polarity.
          * : Disabled.
          * Input function is not inverted. */
         | IOCON_PIO_INVERT(PIO0_30_INVERT_DISABLED)

         /* Controls open-drain mode.
          * : Normal.
          * Normal push-pull output. */
         | IOCON_PIO_OD(PIO0_30_OD_NORMAL));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/