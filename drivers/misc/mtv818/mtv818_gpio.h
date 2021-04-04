#ifndef __MTV818_GPIO_H__
#define __MTV818_GPIO_H__


#ifdef __cplusplus 
extern "C"{ 
#endif  

#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>

#if defined(CONFIG_ARCH_S5PV310)// for Hardkernel
#define PORT_CFG_OUTPUT                 1
#define MTV_PWR_EN                      S5PV310_GPD0(3)         
#define MTV_PWR_EN_CFG_VAL              PORT_CFG_OUTPUT
#define RAONTV_IRQ_INT                  gpio_to_irq(S5PV310_GPX0(2))

#elif defined(CONFIG_ARCH_S5PV210)//for MV210
#define S5PV210_GPD0_PWM_TOUT_0         (0x1 << 0)
#define MTV_PWR_EN                      S5PV210_GPD0(0) 
#define MTV_PWR_EN_CFG_VAL              S5PV210_GPD0_PWM_TOUT_0 
#define RAONTV_IRQ_INT                  IRQ_EINT6

#else
#error "code not present"
#endif

#ifdef __cplusplus 
} 
#endif 

#endif /* __MTV818_GPIO_H__*/