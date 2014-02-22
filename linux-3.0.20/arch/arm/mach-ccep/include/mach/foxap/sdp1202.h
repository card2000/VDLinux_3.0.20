/*
 * sdp1202fpga.h : defined SoC SFRs
 *
 * Copyright (C) 2012 Samsung Electronics co.
 * Seungjun Heo <seungjun.heo@samsung.com>
 *
 */

#ifndef _SDP1202_H_
#define _SDP1202_H_

#define VA_IO_BASE0           (0xFE000000)
#define PA_IO_BASE0           (0x10000000)
#define DIFF_IO_BASE0	      (VA_IO_BASE0 - PA_IO_BASE0)

#define PA_MMC_BASE           (0x10000000)
#define VA_MMC_BASE           (PA_MMC_BASE + DIFF_IO_BASE0)

/* Ethernet Controller Register */
#define PA_GMAC_BASE           (0x100D0000)
#define VA_GMAC_BASE           (PA_GMAC_BASE + DIFF_IO_BASE0)
#define SDP_GMAC_BUS		   (64)/* Bus bits */

/* Power Management unit & PLL Register */
#define PA_PMU_BASE           (0x10090800)
#define VA_PMU_BASE           (PA_PMU_BASE + DIFF_IO_BASE0)

/* Pad Control Register */
#define PA_PADCTRL_BASE       (0x10090C00)
#define VA_PADCTRL_BASE       (PA_PADCTRL_BASE + DIFF_IO_BASE0)

/* Timer Register */
#define PA_TIMER_BASE         (0x10090400)
#define VA_TIMER_BASE         (PA_TIMER_BASE + DIFF_IO_BASE0)

/* UART Register */
#define PA_UART_BASE          (0x10090A00)
#define VA_UART_BASE          (PA_UART_BASE + DIFF_IO_BASE0)

/* Interrupt controller register */
#define PA_INTC_BASE          (0x10090F00)
#define VA_INTC_BASE          (PA_INTC_BASE + DIFF_IO_BASE0)

/* Watchdog Register */
#define PA_WDT_BASE           (0x10090600)
#define VA_WDT_BASE           (PA_WDT_BASE + DIFF_IO_BASE0)

/* GZIP Decomprocessor register */
#define PA_UNZIP_BASE	(0x10350000)
#define VA_UNZIP_BASE	(PA_UNZIP_BASE + DIFF_IO_BASE0)

/* PL310 L2 cache controller */
#define PA_L2C_BASE		(0x10b50000)
#define VA_L2C_BASE		(PA_L2C_BASE + DIFF_IO_BASE0)

/* Core PMU Register */
#define PA_COREPMU_BASE	(0x10b70000)
#define VA_COREPMU_BASE	(PA_COREPMU_BASE + DIFF_IO_BASE0)

/* DMA330 controller register AMS */
#define PA_AMS_DMA330_0_BASE         (0x100A0000)
#define VA_AMS_DMA330_0_BASE         (PA_AMS_DMA330_0_BASE + DIFF_IO_BASE0)

#define PA_AMS_DMA330_1_BASE         (0x100A4000)
#define VA_AMS_DMA330_1_BASE         (PA_AMS_DMA330_1_BASE + DIFF_IO_BASE0)

/* SPI Register */
#define PA_SPI_BASE          (0x10090200)
#define VA_SPI_BASE          (PA_SPI_BASE + DIFF_IO_BASE0)

/* TSC Register */
#define PA_TSC_BASE			(0x10090CDC)
#define VA_TSC_BASE			(PA_TSC_BASE + DIFF_IO_BASE0)

#if 0
/* Core Base Register */
#define PA_CORE_BASE	(0x10b40000)
#define PA_CORE(x)		(PA_CORE_BASE + (x))
#endif

/* SCU Register */
#define PA_SCU_BASE	0
#define VA_SCU_BASE	0

/* Core Power Register */
#define PA_CORE_POWER_BASE	0x10b70000
#define VA_CORE_POWER_BASE	(PA_CORE_POWER_BASE + DIFF_IO_BASE0)
#define V_CORE_POWER_VALUE	0xFFFFF

#define PA_CORE_MISC_BASE	0x10B70400
#define VA_CORE_MISC_BASE	(PA_CORE_MISC_BASE + DIFF_IO_BASE0)

/* GIC Register */
#ifdef CONFIG_USE_EXT_GIC
#define PA_GIC_BASE		(0x10b80000)
#define VA_GIC_BASE		(PA_GIC_BASE + DIFF_IO_BASE0)
#define VA_GIC_CPU_BASE	(VA_GIC_BASE + 0x2000)
#define VA_GIC_DIST_BASE	(VA_GIC_BASE + 0x1000)
#else
#define PA_GIC_BASE		(0x30b40000)
#define VA_GIC_BASE		(PA_GIC_BASE + DIFF_IO_BASE0)
#define VA_GIC_CPU_BASE	(VA_GIC_BASE + 0x100)
#define VA_GIC_DIST_BASE	(VA_GIC_BASE + 0x1000)
#endif

/* ARM TWD Register */
//#define PA_TWD_BASE	PA_CORE(0x600)
//#define VA_TWD_BASE	(PA_TWD_BASE + DIFF_IO_BASE0)

/* PCI PHY */

/* USB EHCI0 host controller register */
#define PA_EHCI0_BASE		(0x100E0000)
#define VA_EHCI0_BASE		(PA_EHCI0_BASE + DIFF_IO_BASE0)

/* USB EHCI1 host controller register */
#define PA_EHCI1_BASE		(0x100F0000)
#define VA_EHCI1_BASE		(PA_EHCI1_BASE + DIFF_IO_BASE0)

/* USB OHCI0 host controller register */
#define PA_OHCI0_BASE		(0x100E8000)
#define VA_OHCI0_BASE		(PA_OHCI0_BASE + DIFF_IO_BASE0)

/* USB OHCI1 host controller register */
#define PA_OHCI1_BASE		(0x100F8000)
#define VA_OHCI1_BASE		(PA_OHCI1_BASE + DIFF_IO_BASE0)

/* USB XHCI0 host controller register */
#define PA_XHCI0_BASE		(0x100C0000)
//#define VA_XHCI0_BASE		(PA_XHCI0_BASE + DIFF_IO_BASE0)

/* USB OTG controller register */
#define PA_OTG_BASE			(0x10020000)
#define VA_OTG_BASE			(PA_OTG_BASE + DIFF_IO_BASE0)

#define	PA_EBUS_BASE		(0x10408000)
#define VA_EBUS_BASE		(PA_EBUS_BASE + DIFF_IO_BASE0)

/* end of SFR_BASE */

/* SMC Register */
#define PA_SMC_BASE	(0x10078000)
#define VA_SMC_BASE	(PA_SMC_BASE + DIFF_IO_BASE0)

#define VA_SMC(offset)  (*(volatile unsigned *)(VA_SMC_BASE+(offset)))

#define VA_SMC_BANK(bank, offset)  (*(volatile unsigned *)(VA_SMC_BASE+(bank)+(offset)))

#define O_SMC_BANK0		0x48
#define O_SMC_BANK1		0x24
#define O_SMC_BANK2		0x00
#define O_SMC_BANK3		0x6c

#define VA_SMC_BANK0(offset)  	VA_SMC_BANK(O_SMC_BANK0, offset)
#define VA_SMC_BANK1(offset)  	VA_SMC_BANK(O_SMC_BANK1, offset)
#define VA_SMC_BANK2(offset)  	VA_SMC_BANK(O_SMC_BANK2, offset)
#define VA_SMC_BANK3(offset)  	VA_SMC_BANK(O_SMC_BANK3, offset)

#define O_SMC_IDCYR		(0x00)
#define O_SMC_WST1		(0x04)
#define O_SMC_WST2		(0x08)
#define O_SMC_WSTOEN		(0x0C)
#define O_SMC_WSTWEN		(0x10)
#define O_SMC_CR		(0x14)
#define O_SMC_SR		(0x18)
#define O_SMC_CIWRCON		(0x1C)
#define O_SMC_CIRDCON		(0x20)

#define O_SMC_WST1_BK0		0x148
#define O_SMC_WST3_BK1		0x144
#define O_SMC_WST3_BK2		0x140
#define O_SMC_WST3_BK3		0x14c

#define R_SMC_WST3_BK0		VA_SMC(O_SMC_WST3_BK0)
#define R_SMC_WST3_BK1		VA_SMC(O_SMC_WST3_BK1)
#define R_SMC_WST3_BK2		VA_SMC(O_SMC_WST3_BK2)
#define R_SMC_WST3_BK3		VA_SMC(O_SMC_WST3_BK3)

/* bank 0 */
#define R_SMC_IDCY_BK0		VA_SMC_BANK0(O_SMC_SMBIDCYR)
#define R_SMC_WST1_BK0		VA_SMC_BANK0(O_SMC_SMBWST1R)
#define R_SMC_WST2_BK0		VA_SMC_BANK0(O_SMC_SMBWST2R)
#define R_SMC_WSTOEN_BK0	VA_SMC_BANK0(O_SMC_SMBWSTOENR)
#define R_SMC_WSTWEN_BK0	VA_SMC_BANK0(O_SMC_SMBWSTWENR)
#define R_SMC_CR_BK0		VA_SMC_BANK0(O_SMC_SMBCR)
#define R_SMC_SR_BK0		VA_SMC_BANK0(O_SMC_SMBSR)
#define R_SMC_CIWRCON_BK0	VA_SMC_BANK0(O_SMC_CIWRCON)
#define R_SMC_CIRDCON_BK0	VA_SMC_BANK0(O_SMC_CIRDCON)

/* bank 1 */
#define R_SMC_IDCY_BK1		VA_SMC_BANK1(O_SMC_SMBIDCYR)
#define R_SMC_WST1_BK1		VA_SMC_BANK1(O_SMC_SMBWST1R)
#define R_SMC_WST2_BK1		VA_SMC_BANK1(O_SMC_SMBWST2R)
#define R_SMC_WSTOEN_BK1	VA_SMC_BANK1(O_SMC_SMBWSTOENR)
#define R_SMC_WSTWEN_BK1	VA_SMC_BANK1(O_SMC_SMBWSTWENR)
#define R_SMC_CR_BK1		VA_SMC_BANK1(O_SMC_SMBCR)
#define R_SMC_SR_BK1		VA_SMC_BANK1(O_SMC_SMBSR)
#define R_SMC_CIWRCON_BK1	VA_SMC_BANK1(O_SMC_CIWRCON)
#define R_SMC_CIRDCON_BK1	VA_SMC_BANK1(O_SMC_CIRDCON)

/* bank 2 */
#define R_SMC_IDCY_BK2		VA_SMC_BANK2(O_SMC_SMBIDCYR)
#define R_SMC_WST1_BK2		VA_SMC_BANK2(O_SMC_SMBWST1R)
#define R_SMC_WST2_BK2		VA_SMC_BANK2(O_SMC_SMBWST2R)
#define R_SMC_WSTOEN_BK2	VA_SMC_BANK2(O_SMC_SMBWSTOENR)
#define R_SMC_WSTWEN_BK2	VA_SMC_BANK2(O_SMC_SMBWSTWENR)
#define R_SMC_CR_BK2		VA_SMC_BANK2(O_SMC_SMBCR)
#define R_SMC_SR_BK2		VA_SMC_BANK2(O_SMC_SMBSR)
#define R_SMC_CIWRCON_BK2	VA_SMC_BANK2(O_SMC_CIWRCON)
#define R_SMC_CIRDCON_BK2	VA_SMC_BANK2(O_SMC_CIRDCON)

/* bank 3 */
#define R_SMC_IDCY_BK3		VA_SMC_BANK3(O_SMC_SMBIDCYR)
#define R_SMC_WST1_BK3		VA_SMC_BANK3(O_SMC_SMBWST1R)
#define R_SMC_WST2_BK3		VA_SMC_BANK3(O_SMC_SMBWST2R)
#define R_SMC_WSTOEN_BK3	VA_SMC_BANK3(O_SMC_SMBWSTOENR)
#define R_SMC_WSTWEN_BK3	VA_SMC_BANK3(O_SMC_SMBWSTWENR)
#define R_SMC_CR_BK3		VA_SMC_BANK3(O_SMC_SMBCR)
#define R_SMC_SR_BK3		VA_SMC_BANK3(O_SMC_SMBSR)
#define R_SMC_CIWRCON_BK3	VA_SMC_BANK3(O_SMC_CIWRCON)
#define R_SMC_CIRDCON_BK3	VA_SMC_BANK3(O_SMC_CIRDCON)

#define R_SMC_SMBEWS		VA_SMC(0x120)
#define R_SMC_CI_RESET		VA_SMC(0x128)
#define R_SMC_CI_ADDRSEL	VA_SMC(0x12c)
#define R_SMC_CI_CNTL		VA_SMC(0x130)
#define R_SMC_CI_REGADDR	VA_SMC(0x134)
#define R_SMC_PERIPHID0		VA_SMC(0xFE0)
#define R_SMC_PERIPHID1		VA_SMC(0xFE4)
#define R_SMC_PERIPHID2		VA_SMC(0xFE8)
#define R_SMC_PERIPHID3		VA_SMC(0xFEC)
#define R_SMC_PCELLID0		VA_SMC(0xFF0)
#define R_SMC_PCELLID1		VA_SMC(0xFF4)
#define R_SMC_PCELLID2		VA_SMC(0xFF8)
#define R_SMC_PCELLID3		VA_SMC(0xFFC)
#define R_SMC_CLKSTOP		VA_SMC(0x1e8)
#define R_SMC_SYNCEN		VA_SMC(0x1ec)

/* clock & power management */
#define VA_PMU(offset)    (*(volatile unsigned *)(VA_PMU_BASE+(offset)))

#define O_PMU_CPU_PMS_CON		(0x0)
#define O_PMU_BUS_PMS_CON		(0x0C)
#define O_PMU_DDR_PMS_CON		(0x14)
#define O_PMU_DDR_K_CON			(0x34)

/* define for 'C' */
#define R_PMU_CPU_PMS_CON	VA_PMU(O_PMU_CPU_PMS_CON)
#define R_PMU_BUS_PMS_CON	VA_PMU(O_PMU_BUS_PMS_CON)
#define R_PMU_DDR_PMS_CON	VA_PMU(O_PMU_DDR_PMS_CON)
#define R_PMU_DDR_K_CON		VA_PMU(O_PMU_DDR_K_CON)

#define PMU_PLL_P_VALUE(x)	((x >> 20) & 0x3F)
#define PMU_PLL_M_VALUE(x)	((x >> 8) & 0x3FF)
#define PMU_PLL_S_VALUE(x)	(x & 0x7)
#define PMU_PLL_K_VALUE(x)	(x & 0xFFFF)

#define GET_PLL_M(x)		PMU_PLL_M_VALUE(x)
#define GET_PLL_P(x)		PMU_PLL_P_VALUE(x)
#define GET_PLL_S(x)		PMU_PLL_S_VALUE(x)

#define REQ_FCLK	1		/* CPU Clock */
#define REQ_DCLK	2		/* DDR Clock */
#define REQ_BUSCLK	3		/* BUS Clock */
#define REQ_HCLK	4		/* AHB Clock */
#define REQ_PCLK	5		/* APB Clock */


#endif

