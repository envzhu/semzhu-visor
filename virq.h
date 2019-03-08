#ifndef _VIRQ_H_INCLUDED_
#define _VIRQ_H_INCLUDED_

#include "typedef.h"
#include "virt_mmio.h"
#include "vm.h"


/* GPU interrupts */
#define GPU_INTERRUPT_TIMER0            0
#define GPU_INTERRUPT_TIMER1            1
#define GPU_INTERRUPT_TIMER2            2
#define GPU_INTERRUPT_TIMER3            3
#define GPU_INTERRUPT_CODEC0            4
#define GPU_INTERRUPT_CODEC1            5
#define GPU_INTERRUPT_CODEC2            6
#define GPU_INTERRUPT_JPEG              7
#define GPU_INTERRUPT_ISP               8
#define GPU_INTERRUPT_USB               9
#define GPU_INTERRUPT_3D                10
#define GPU_INTERRUPT_TRANSPOSER        11
#define GPU_INTERRUPT_MULTICORESYNC0    12
#define GPU_INTERRUPT_MULTICORESYNC1    13
#define GPU_INTERRUPT_MULTICORESYNC2    14
#define GPU_INTERRUPT_MULTICORESYNC3    15
#define GPU_INTERRUPT_DMA0              16
#define GPU_INTERRUPT_DMA1              17
#define GPU_INTERRUPT_DMA2              18
#define GPU_INTERRUPT_DMA3              19
#define GPU_INTERRUPT_DMA4              20
#define GPU_INTERRUPT_DMA5              21
#define GPU_INTERRUPT_DMA6              22
#define GPU_INTERRUPT_DMA7              23
#define GPU_INTERRUPT_DMA8              24
#define GPU_INTERRUPT_DMA9              25
#define GPU_INTERRUPT_DMA10             26
#define GPU_INTERRUPT_DMA11             27
#define GPU_INTERRUPT_DMA12             28
#define GPU_INTERRUPT_AUX               29
#define GPU_INTERRUPT_ARM               30
#define GPU_INTERRUPT_VPUDMA            31
#define GPU_INTERRUPT_HOSTPORT          32
#define GPU_INTERRUPT_VIDEOSCALER       33
#define GPU_INTERRUPT_CCP2TX            34
#define GPU_INTERRUPT_SDC               35
#define GPU_INTERRUPT_DSI0              36
#define GPU_INTERRUPT_AVE               37
#define GPU_INTERRUPT_CAM0              38
#define GPU_INTERRUPT_CAM1              39
#define GPU_INTERRUPT_HDMI0             40
#define GPU_INTERRUPT_HDMI1             41
#define GPU_INTERRUPT_PIXELVALVE1       42
#define GPU_INTERRUPT_I2CSPISLV         43
#define GPU_INTERRUPT_DSI1              44
#define GPU_INTERRUPT_PWA0              45
#define GPU_INTERRUPT_PWA1              46
#define GPU_INTERRUPT_CPR               47
#define GPU_INTERRUPT_SMI               48
#define GPU_INTERRUPT_GPIO0             49
#define GPU_INTERRUPT_GPIO1             50
#define GPU_INTERRUPT_GPIO2             51
#define GPU_INTERRUPT_GPIO3             52
#define GPU_INTERRUPT_I2C               53
#define GPU_INTERRUPT_SPI               54
#define GPU_INTERRUPT_I2SPCM            55
#define GPU_INTERRUPT_SDIO              56
#define GPU_INTERRUPT_UART              57
#define GPU_INTERRUPT_SLIMBUS           58
#define GPU_INTERRUPT_VEC               59
#define GPU_INTERRUPT_CPG               60
#define GPU_INTERRUPT_RNG               61
#define GPU_INTERRUPT_ARASANSDIO        62
#define GPU_INTERRUPT_AVSPMON           63
#define GPU_INTERRUPT_NUM               64

#define VIRT_INTR_TIMER0            (1 << GPU_INTERRUPT_TIMER0)
#define VIRT_INTR_TIMER1            (1 << GPU_INTERRUPT_TIMER1)
#define VIRT_INTR_TIMER2            (1 << GPU_INTERRUPT_TIMER2)
#define VIRT_INTR_TIMER3            (1 << GPU_INTERRUPT_TIMER3)
#define VIRT_INTR_CODEC0            (1 << GPU_INTERRUPT_CODEC0)
#define VIRT_INTR_CODEC1            (1 << GPU_INTERRUPT_CODEC1)
#define VIRT_INTR_CODEC2            (1 << GPU_INTERRUPT_CODEC2)
#define VIRT_INTR_JPEG              (1 << GPU_INTERRUPT_JPRG)
#define VIRT_INTR_ISP               (1 << GPU_INTERRUPT_ISP)
#define VIRT_INTR_USB               (1 << GPU_INTERRUPT_USB)
#define VIRT_INTR_3D                (1 << GPU_INTERRUPT_3D)
#define VIRT_INTR_TRANSPOSER        (1 << GPU_INTERRUPT_TRANSPOSER)
#define VIRT_INTR_MULTICORESYNC0    (1 << GPU_INTERRUPT_MULTICORESYNC0)
#define VIRT_INTR_MULTICORESYNC1    (1 << GPU_INTERRUPT_MULTICORESYNC1)
#define VIRT_INTR_MULTICORESYNC2    (1 << GPU_INTERRUPT_MULTICORESYNC2
#define VIRT_INTR_MULTICORESYNC3    (1 << GPU_INTERRUPT_MULTICORESYNC3)
#define VIRT_INTR_DMA0              (1 << GPU_INTERRUPT_DMA0)
#define VIRT_INTR_DMA1              (1 << GPU_INTERRUPT_DMA1)
#define VIRT_INTR_DMA2              (1 << GPU_INTERRUPT_DMA2)
#define VIRT_INTR_DMA3              (1 << GPU_INTERRUPT_DMA3)
#define VIRT_INTR_DMA4              (1 << GPU_INTERRUPT_DMA4)
#define VIRT_INTR_DMA5              (1 << GPU_INTERRUPT_DMA5)
#define VIRT_INTR_DMA6              (1 << GPU_INTERRUPT_DMA6)
#define VIRT_INTR_DMA7              (1 << GPU_INTERRUPT_DMA7)
#define VIRT_INTR_DMA8              (1 << GPU_INTERRUPT_DMA8)
#define VIRT_INTR_DMA9              (1 << GPU_INTERRUPT_DMA9)
#define VIRT_INTR_DMA10             (1 << GPU_INTERRUPT_DMA10)
#define VIRT_INTR_DMA11             (1 << GPU_INTERRUPT_DMA11)
#define VIRT_INTR_DMA12             (1 << GPU_INTERRUPT_DMA12)
#define VIRT_INTR_AUX               (1 << GPU_INTERRUPT_AUX)
#define VIRT_INTR_ARM               (1 << GPU_INTERRUPT_ARM)
#define VIRT_INTR_VPUDMA            (1 << GPU_INTERRUPT_VPUDMA)
#define VIRT_INTR_HOSTPORT          (1 << GPU_INTERRUPT_HOSTPORT)
#define VIRT_INTR_VIDEOSCALER       (1 << GPU_INTERRUPT_VIDEOSCALER)
#define VIRT_INTR_CCP2TX            (1 << GPU_INTERRUPT_CCP2TX)
#define VIRT_INTR_SDC               (1 << GPU_INTERRUPT_SDC)
#define VIRT_INTR_DSI0              (1 << GPU_INTERRUPT_DSI0)
#define VIRT_INTR_AVE               (1 << GPU_INTERRUPT_AVE)
#define VIRT_INTR_CAM0              (1 << GPU_INTERRUPT_CAN0)
#define VIRT_INTR_CAM1              (1 << GPU_INTERRUPT_CAN1)
#define VIRT_INTR_HDMI0             (1 << GPU_INTERRUPT_HDMI0)
#define VIRT_INTR_HDMI1             (1 << GPU_INTERRUPT_HDMI1)
#define VIRT_INTR_PIXELVALVE1       (1 << GPU_INTERRUPT_PIXELVALVE1)
#define VIRT_INTR_I2CSPISLV         (1 << GPU_INTERRUPT_I2CSPISLV)
#define VIRT_INTR_DSI1              (1 << GPU_INTERRUPT_DSI1)
#define VIRT_INTR_PWA0              (1 << GPU_INTERRUPT_PWA0)
#define VIRT_INTR_PWA1              (1 << GPU_INTERRUPT_PWA1)
#define VIRT_INTR_CPR               (1 << GPU_INTERRUPT_CPR)
#define VIRT_INTR_SMI               (1 << GPU_INTERRUPT_SMI)
#define VIRT_INTR_GPIO0             (1 << GPU_INTERRUPT_GPIO0)
#define VIRT_INTR_GPIO1             (1 << GPU_INTERRUPT_GPIO1)
#define VIRT_INTR_GPIO2             (1 << GPU_INTERRUPT_GPIO2)
#define VIRT_INTR_GPIO3             (1 << GPU_INTERRUPT_GPIO3)
#define VIRT_INTR_I2C               (1 << GPU_INTERRUPT_I2C)
#define VIRT_INTR_SPI               (1 << GPU_INTERRUPT_SPI)
#define VIRT_INTR_I2SPCM            (1 << GPU_INTERRUPT_I2SPCM)
#define VIRT_INTR_SDIO              (1 << GPU_INTERRUPT_SDIO)
#define VIRT_INTR_UART              (1 << GPU_INTERRUPT_UART)
#define VIRT_INTR_SLIMBUS           (1 << GPU_INTERRUPT_SLIMBUS)
#define VIRT_INTR_VEC               (1 << GPU_INTERRUPT_VEC)
#define VIRT_INTR_CPG               (1 << GPU_INTERRUPT_CPG)
#define VIRT_INTR_RNG               (1 << GPU_INTERRUPT_RNG)
#define VIRT_INTR_ARASANSDIO        (1 << GPU_INTERRUPT_ARASANSDIO)
#define VIRT_INTR_AVSPMON           (1 << GPU_INTERRUPT_AVSPMON)


uint32_t hyp_timer_irq_is_pending(uint32_t cpu_id);
uint32_t hyp_timer_fiq_is_pending(uint32_t cpu_id);

void virt_device_intr_init(void);
int  virt_device_intr_set(vm_t *vm);
void virt_device_intr_release(vm_t *vm);

void virt_intr_handler(vcpu_t *cur_vcpu);
void virt_fiq_handler(vcpu_t *cur_vcpu);

#endif
