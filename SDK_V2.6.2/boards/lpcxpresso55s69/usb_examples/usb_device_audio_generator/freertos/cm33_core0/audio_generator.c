/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_audio_config.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "audio_generator.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#include <stdio.h>
#include <stdlib.h>

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "pin_mux.h"
#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
extern void USB_PrepareData(void);
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
extern void Board_DMIC_DMA_Init(void);
#endif
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
extern void SCTIMER_CaptureInit(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Audio data information */
extern uint8_t s_wavBuff[];

extern usb_device_class_struct_t g_UsbDeviceAudioClass;

/* Default value of audio generator device struct */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_audio_generator_struct_t s_audioGenerator = {
    .deviceHandle = NULL,
    .audioHandle = (class_handle_t)NULL,
    .copyProtect = 0x01U,
    .curMute = 0x01U,
    .curVolume = {0x00U, 0x80U},
    .minVolume = {0x00U, 0x80U},
    .maxVolume = {0xFFU, 0x7FU},
    .resVolume = {0x01U, 0x00U},
    .curBass = 0x00U,
    .minBass = 0x80U,
    .maxBass = 0x7FU,
    .resBass = 0x01U,
    .curMid = 0x00U,
    .minMid = 0x80U,
    .maxMid = 0x7FU,
    .resMid = 0x01U,
    .curTreble = 0x01U,
    .minTreble = 0x80U,
    .maxTreble = 0x7FU,
    .resTreble = 0x01U,
    .curAutomaticGain = 0x01U,
    .curDelay = {0x00U, 0x40U},
    .minDelay = {0x00U, 0x00U},
    .maxDelay = {0xFFU, 0xFFU},
    .resDelay = {0x00U, 0x01U},
    .curLoudness = 0x01U,
    .curSamplingFrequency = {0x00U, 0x00U, 0x01U},
    .minSamplingFrequency = {0x00U, 0x00U, 0x01U},
    .maxSamplingFrequency = {0x00U, 0x00U, 0x01U},
    .resSamplingFrequency = {0x00U, 0x00U, 0x01U},
    .speed = USB_SPEED_FULL,
    .attach = 0U,
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    .generatorIntervalCount = 0,
    .audioPllTicksPrev = 0,
    .audioPllTicksDiff = 0,
    .audioPllTicksEma = AUDIO_PLL_USB1_SOF_INTERVAL_COUNT,
    .audioPllTickEmaFrac = 0,
    .audioPllStep = AUDIO_PLL_FRACTIONAL_CHANGE_STEP,
#endif
};

/* USB device class information */
static usb_device_class_config_struct_t s_audioConfig[1] = {{
    USB_DeviceAudioCallback, (class_handle_t)NULL, &g_UsbDeviceAudioClass,
}};

/* USB device class configuration information */
static usb_device_class_config_list_struct_t s_audioConfigList = {
    s_audioConfig, USB_DeviceCallback, 1U,
};

/*******************************************************************************
* Code
******************************************************************************/
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(s_audioGenerator.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(s_audioGenerator.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif

#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUnused, 0U);
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, NULL);
#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
}
#endif
/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle           The Audio class handle.
 * @param event            The Audio class event type.
 * @param param            The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error = kStatus_USB_Success;

    switch (event)
    {
        case USB_DEVICE_AUDIO_GET_CUR_MUTE_CONTROL:
            request->buffer = &s_audioGenerator.curMute;
            request->length = sizeof(s_audioGenerator.curMute);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.curVolume;
            request->length = sizeof(s_audioGenerator.curVolume);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_BASS_CONTROL:
            request->buffer = &s_audioGenerator.curBass;
            request->length = sizeof(s_audioGenerator.curBass);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_MID_CONTROL:
            request->buffer = &s_audioGenerator.curMid;
            request->length = sizeof(s_audioGenerator.curMid);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.curTreble;
            request->length = sizeof(s_audioGenerator.curTreble);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &s_audioGenerator.curAutomaticGain;
            request->length = sizeof(s_audioGenerator.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_DELAY_CONTROL:
            request->buffer = s_audioGenerator.curDelay;
            request->length = sizeof(s_audioGenerator.curDelay);
            break;
        case USB_DEVICE_AUDIO_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.curSamplingFrequency;
            request->length = sizeof(s_audioGenerator.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.minVolume;
            request->length = sizeof(s_audioGenerator.minVolume);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_BASS_CONTROL:
            request->buffer = &s_audioGenerator.minBass;
            request->length = sizeof(s_audioGenerator.minBass);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_MID_CONTROL:
            request->buffer = &s_audioGenerator.minMid;
            request->length = sizeof(s_audioGenerator.minMid);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.minTreble;
            request->length = sizeof(s_audioGenerator.minTreble);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_DELAY_CONTROL:
            request->buffer = s_audioGenerator.minDelay;
            request->length = sizeof(s_audioGenerator.minDelay);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.minSamplingFrequency;
            request->length = sizeof(s_audioGenerator.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.maxVolume;
            request->length = sizeof(s_audioGenerator.maxVolume);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_BASS_CONTROL:
            request->buffer = &s_audioGenerator.maxBass;
            request->length = sizeof(s_audioGenerator.maxBass);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_MID_CONTROL:
            request->buffer = &s_audioGenerator.maxMid;
            request->length = sizeof(s_audioGenerator.maxMid);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.maxTreble;
            request->length = sizeof(s_audioGenerator.maxTreble);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_DELAY_CONTROL:
            request->buffer = s_audioGenerator.maxDelay;
            request->length = sizeof(s_audioGenerator.maxDelay);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.maxSamplingFrequency;
            request->length = sizeof(s_audioGenerator.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_GET_RES_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.resVolume;
            request->length = sizeof(s_audioGenerator.resVolume);
            break;
        case USB_DEVICE_AUDIO_GET_RES_BASS_CONTROL:
            request->buffer = &s_audioGenerator.resBass;
            request->length = sizeof(s_audioGenerator.resBass);
            break;
        case USB_DEVICE_AUDIO_GET_RES_MID_CONTROL:
            request->buffer = &s_audioGenerator.resMid;
            request->length = sizeof(s_audioGenerator.resMid);
            break;
        case USB_DEVICE_AUDIO_GET_RES_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.resTreble;
            request->length = sizeof(s_audioGenerator.resTreble);
            break;
        case USB_DEVICE_AUDIO_GET_RES_DELAY_CONTROL:
            request->buffer = s_audioGenerator.resDelay;
            request->length = sizeof(s_audioGenerator.resDelay);
            break;
        case USB_DEVICE_AUDIO_GET_RES_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.resSamplingFrequency;
            request->length = sizeof(s_audioGenerator.resSamplingFrequency);
            break;

        case USB_DEVICE_AUDIO_SET_CUR_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curVolume;
            }
            else
            {
                uint16_t volume = (uint16_t)((uint16_t)s_audioGenerator.curVolume[1] << 8U);
                volume |= (uint8_t)(s_audioGenerator.curVolume[0]);
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_MUTE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curMute;
            }
            else
            {
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curBass;
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curMid;
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curTreble;
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curAutomaticGain;
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curDelay;
            }
            break;
        case USB_DEVICE_AUDIO_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curSamplingFrequency;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.minVolume;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.minBass;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.minMid;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.minTreble;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.minDelay;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.minSamplingFrequency;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.maxVolume;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.maxBass;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.maxMid;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.maxTreble;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.maxDelay;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.maxSamplingFrequency;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.resVolume;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.resBass;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.resMid;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.resTreble;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.resDelay;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.resSamplingFrequency;
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle            The Audio class handle.
 * @param event             The Audio class event type.
 * @param param             The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)param;

    switch (event)
    {
        case kUSB_DeviceAudioEventStreamSendResponse:
            if ((s_audioGenerator.attach) &&
                (ep_cb_param->length == ((USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                      FS_ISO_IN_ENDP_PACKET_SIZE)))
            {
                USB_PrepareData();
                error = USB_DeviceAudioSend(handle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                            (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                         FS_ISO_IN_ENDP_PACKET_SIZE);
            }
            break;

        default:
            if (param && (event > 0xFFU))
            {
                error = USB_DeviceAudioRequest(handle, event, param);
            }
            break;
    }
    return error;
}

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle           The USB device handle.
 * @param event            The USB device event type.
 * @param param            The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint8_t *temp8 = (uint8_t *)param;
    uint16_t *temp16 = (uint16_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            /* The device BUS reset signal detected */
            s_audioGenerator.attach = 0U;
            s_audioGenerator.currentConfiguration = 0U;
            error = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &s_audioGenerator.speed))
            {
                USB_DeviceSetSpeed(handle, s_audioGenerator.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                s_audioGenerator.attach = 0U;
                s_audioGenerator.currentConfiguration = 0U;
            }
            else if (USB_AUDIO_GENERATOR_CONFIGURE_INDEX == (*temp8))
            {
                /* Set the configuration request */
                s_audioGenerator.attach = 1U;
                s_audioGenerator.currentConfiguration = *temp8;
            }
            else
            {
                error = kStatus_USB_InvalidRequest;
            }
            break;
        case kUSB_DeviceEventSetInterface:

            if (s_audioGenerator.attach)
            {
                /* Set alternateSetting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                if (s_audioGenerator.currentInterfaceAlternateSetting[interface] != alternateSetting)
                {
                    s_audioGenerator.currentInterfaceAlternateSetting[interface] = alternateSetting;
                    if (alternateSetting == 1U)
                    {
                        USB_PrepareData();
                        USB_DeviceAudioSend(s_audioGenerator.audioHandle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                            (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                         FS_ISO_IN_ENDP_PACKET_SIZE);
                    }
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get the current configuration request */
                *temp8 = s_audioGenerator.currentConfiguration;
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_AUDIO_GENERATOR_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | s_audioGenerator.currentInterfaceAlternateSetting[interface];
                    error = kStatus_USB_Success;
                }
                else
                {
                    error = kStatus_USB_InvalidRequest;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get the device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get the configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get the string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }

    return error;
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    SCTIMER_CaptureInit();
#endif

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &s_audioConfigList, &s_audioGenerator.deviceHandle))
    {
        usb_echo("USB device failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device audio generator demo\r\n");
        s_audioGenerator.audioHandle = s_audioConfigList.config->classHandle;
    }

    USB_DeviceIsrEnable();

    USB_DeviceRun(s_audioGenerator.deviceHandle);
}

#if USB_DEVICE_CONFIG_USE_TASK
void USBDeviceTask(void *handle)
{
    while (1U)
    {
        USB_DeviceTaskFn(handle);
    }
}
#endif

/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
void APPTask(void *handle)
{
    USB_DeviceApplicationInit();

#if USB_DEVICE_CONFIG_USE_TASK
    if (s_audioGenerator.deviceHandle)
    {
        if (xTaskCreate(USBDeviceTask,                     /* pointer to the task */
                        "usb device task",                 /* task name for kernel awareness debugging */
                        5000L / sizeof(portSTACK_TYPE),    /* task stack size */
                        s_audioGenerator.deviceHandle,     /* optional task startup argument */
                        5,                                 /* initial priority */
                        &s_audioGenerator.deviceTaskHandle /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("usb device task create failed!\r\n");
            return;
        }
    }
#endif

    while (1)
    {
    }
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);

#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /* Put PHY powerdown under software control */
    *((uint32_t *)(USBHSH_BASE + 0x50)) = USBHSH_PORTMODE_SW_PDCOM_MASK;
    /* According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    CLOCK_DisableClock(kCLOCK_Usbh1);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif

#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
    Board_DMIC_DMA_Init();
#endif

    if (xTaskCreate(APPTask,                                /* pointer to the task */
                    "app task",                             /* task name for kernel awareness debugging */
                    5000L / sizeof(portSTACK_TYPE),         /* task stack size */
                    &s_audioGenerator,                      /* optional task startup argument */
                    4,                                      /* initial priority */
                    &s_audioGenerator.applicationTaskHandle /* optional task handle to create */
                    ) != pdPASS)
    {
        usb_echo("app task create failed!\r\n");
#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
        return 1U;
#else
        return;
#endif
    }
    vTaskStartScheduler();

#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
    return 1U;
#endif
}