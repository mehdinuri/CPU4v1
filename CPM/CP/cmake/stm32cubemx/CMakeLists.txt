cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	MBEDTLS_CONFIG_FILE=<mbedtls_config.h> 
	USE_PWR_LDO_SUPPLY 
	USE_HAL_DRIVER 
	STM32H743xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../MBEDTLS/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../USB_DEVICE/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../USB_DEVICE/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/system
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/RTOS2/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lan8742
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/netif/ppp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/include/mbedtls
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip/apps
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip/priv
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip/prot
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/netif
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix/net
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/stdc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/system/arch
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Core
    ${CMAKE_SOURCE_DIR}/App/Ports
    ${CMAKE_SOURCE_DIR}/App/Domain/Intersection
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Bootstrap
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Tasks
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Control/Legacy
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Persistence
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Update
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Time
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Comms
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/UI
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../MBEDTLS/App/mbedtls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../MBEDTLS/App/net_sockets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../MBEDTLS/Target/hardware_rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/lwip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/1201v0307.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/1202v0328.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/eth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/snmp_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/snmpv3_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/tc_kgm_v0.7.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/tcp_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/udp_probe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/Target/ethernetif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../USB_DEVICE/Target/usbd_conf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../USB_DEVICE/App/usb_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../USB_DEVICE/App/usbd_desc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../USB_DEVICE/App/usbd_custom_hid_if.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/freertos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/bdma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/fdcan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32h7xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32h7xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32h7xx_hal_timebase_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32h743xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32h7xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lan8742/lan8742.c
)

# Drivers Midllewares


set(USB_Device_Library_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c
)
set(FreeRTOS_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/Common/mpu_wrappers_v2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
)
set(LwIP_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/auth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ccp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/chap_ms.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/chap-md5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/chap-new.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/demand.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/eap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/eui64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/fsm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ipcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ipv6cp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/lcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/magic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/mppe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/multilink.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ppp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppcrypt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppoe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppol2tp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/upap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/vj.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/bridgeif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/bridgeif_fdb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ethernet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/lowpan6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/lowpan6_ble.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/lowpan6_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/slipif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/zepif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/api_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/api_msg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/err.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/if_api.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/netbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/netdb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/netifapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/sockets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/tcpip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/altcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/altcp_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/altcp_tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/dns.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/inet_chksum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/memp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/netif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/pbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/raw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/stats.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/sys.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/tcp_in.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/tcp_out.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/timeouts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/udp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/acd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/autoip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/dhcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/etharp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/icmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/igmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/ip4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_frag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/dhcp6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ethip6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/icmp6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/inet6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ip6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ip6_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ip6_frag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/mld6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/nd6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/system/OS/sys_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/mqtt/mqtt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmpv3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmpv3_mbedtls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_asn1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_icmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_interfaces.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_ip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_snmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_mib2_udp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_msg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_netconn.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_pbuf_stream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_raw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_scalar.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_snmpv2_framework.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_snmpv2_usm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_threadsync.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/snmp/snmp_traps.c
)
set(mbedTLS_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/aes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/asn1parse.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/asn1write.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/base64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/bignum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/cipher.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/cipher_wrap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/ctr_drbg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/dhm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/ecp_curves.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/entropy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/entropy_poll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/md.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/md5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/md_wrap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/memory_buffer_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/pem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/pkcs12.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/pk_wrap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/platform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/platform_util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/rsa_internal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/sha1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/sha256.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/ssl_ciphersuites.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/mbedTLS/library/version_features.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    USB_Device_Library	FreeRTOS	LwIP	mbedTLS	
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Create USB_Device_Library static library
add_library(USB_Device_Library OBJECT)
target_sources(USB_Device_Library PRIVATE ${USB_Device_Library_Src})
target_link_libraries(USB_Device_Library PUBLIC stm32cubemx)

# Create FreeRTOS static library
add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${FreeRTOS_Src})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)

# Create LwIP static library
add_library(LwIP OBJECT)
target_sources(LwIP PRIVATE ${LwIP_Src})
target_link_libraries(LwIP PUBLIC stm32cubemx)


# Create mbedTLS static library
add_library(mbedTLS OBJECT)
target_sources(mbedTLS PRIVATE ${mbedTLS_Src})
target_link_libraries(mbedTLS PUBLIC stm32cubemx)

# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
