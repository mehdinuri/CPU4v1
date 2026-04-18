/**
 ******************************************************************************
 * @file    Platform/STM32/Bootstrap/main_ssm.h
 * @brief   Composition root for the SSM firmware.
 *          Call MainApplication_Init() once, after peripheral init, before
 *          osKernelStart().
 ******************************************************************************
 */

#ifndef PLATFORM_STM32_BOOTSTRAP_MAIN_SSM_H
#define PLATFORM_STM32_BOOTSTRAP_MAIN_SSM_H

#ifdef __cplusplus
extern "C" {
#endif

void MainApplication_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_STM32_BOOTSTRAP_MAIN_SSM_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
