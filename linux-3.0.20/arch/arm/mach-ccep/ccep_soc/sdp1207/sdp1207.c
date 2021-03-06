/*
 * sdp1207.c
 *
 * Copyright (C) 2011 Samsung Electronics.co
 * Sola lee <sssol.lee@samsung.com>
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/sn1202033.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/hardware/cache-l2x0.h>

#include <asm/io.h>
#include <asm/irq.h>

#include <mach/hardware.h>
#include <mach/ccep_soc.h>

#include <plat/sdp_spi.h> // use for struct sdp_spi_info and sdp_spi_chip_ops by drain.lee
#include <linux/spi/spi.h>

#include <plat/sdp_mmc.h>

extern unsigned int sdp_revision_id;

static struct map_desc sdp1207_io_desc[] __initdata = {
/* ------------------------------------------- */
/* ------ special function register ---------- */
/* ------------------------------------------- */
// 0x1000_0000 ~ 0x100F_FFFF, 1Mib
 {
	.virtual = VA_IO_BASE0,
	.pfn     = __phys_to_pfn(PA_IO_BASE0),
	.length  = (1 << 20),
	.type    = MT_DEVICE
 },
// 0x3010_0000 ~ 0x301F_FFFF, 1Mib
 {
	.virtual = VA_IO_BASE0 + 0x100000,
	.pfn     = __phys_to_pfn(0x30000000 + 0x100000),
	.length  = (1 << 20),
	.type    = MT_DEVICE
 },
// 0x30B0_0000 ~ 0x30BF_FFFF, 1Mib
 {
	.virtual = VA_IO_BASE0 + 0xB00000,
	.pfn     = __phys_to_pfn(0x30000000 + 0xB00000),
	.length  = (1 << 20),
	.type    = MT_DEVICE
 },
};

static struct resource sdp_uart0_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE,
		.end	= PA_UART_BASE + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC
		.start 	= IRQ_UART0,
		.end	= IRQ_UART0,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource sdp_uart1_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE + 0x40,
		.end	= PA_UART_BASE + 0x40 + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC
		.start 	= IRQ_UART1,
		.end	= IRQ_UART1,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource sdp_uart2_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE + 0x80,
		.end	= PA_UART_BASE + 0x80 + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC
		.start 	= IRQ_UART2,
		.end	= IRQ_UART2,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource sdp_uart3_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE + 0xC0,
		.end	= PA_UART_BASE + 0xC0 + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC
		.start 	= IRQ_UART3,
		.end	= IRQ_UART3,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start 	= PA_UDMA_RX_BASE,
		.end	= PA_UDMA_RX_BASE + 0x40,
		.flags	= IORESOURCE_MEM,
		.name	= "udma_rx",
	},
	[3] = {
		.start 	= PA_UDMA_TX_BASE,
		.end	= PA_UDMA_TX_BASE + 0x40,
		.flags	= IORESOURCE_MEM,
		.name	= "udma_tx",
	},
	[4] = {
		.start 	= IRQ_UDMA_RX,
		.end	= IRQ_UDMA_RX,
		.flags	= IORESOURCE_IRQ,
		.name	= "udma_rx",
	},
	[5] = {
		.start 	= IRQ_UDMA_TX,
		.end	= IRQ_UDMA_TX,
		.flags	= IORESOURCE_IRQ,
		.name	= "udma_tx",
	},
};



/* EHCI host controller */
static struct resource sdp_ehci0_resource[] = {
        [0] = {
                .start  = PA_EHCI0_BASE,
                .end    = PA_EHCI0_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_EHCI0,
                .end    = IRQ_EHCI0,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct resource sdp_ehci1_resource[] = {
        [0] = {
                .start  = PA_EHCI1_BASE,
                .end    = PA_EHCI1_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_EHCI1,
                .end    = IRQ_EHCI1,
                .flags  = IORESOURCE_IRQ,
        },
};

/* USB 2.0 companion OHCI */
static struct resource sdp_ohci0_resource[] = {
        [0] = {
                .start  = PA_OHCI0_BASE,
                .end    = PA_OHCI0_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_OHCI0,
                .end    = IRQ_OHCI0,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct resource sdp_ohci1_resource[] = {
        [0] = {
                .start  = PA_OHCI1_BASE,
                .end    = PA_OHCI1_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_OHCI1,
                .end    = IRQ_OHCI1,
                .flags  = IORESOURCE_IRQ,
        },
};
#if 0
/* xHCI host controller */
static struct resource sdp_xhci0_resource[] = {
        [0] = {
                .start  = PA_XHCI0_BASE,
                .end    = PA_XHCI0_BASE + 0xC700,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_USB3,
                .end    = IRQ_USB3,
                .flags  = IORESOURCE_IRQ,
        },
};
#endif

static u64 sdp_uart3_dmamask = (u32)0xFFFFFFFFUL;

static struct platform_device sdp_uart0 = {
	.name		= "sdp-uart",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(sdp_uart0_resource),
	.resource	= sdp_uart0_resource,
};

static struct platform_device sdp_uart1 = {
	.name		= "sdp-uart",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(sdp_uart1_resource),
	.resource	= sdp_uart1_resource,
};

static struct platform_device sdp_uart2 = {
	.name		= "sdp-uart",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(sdp_uart2_resource),
	.resource	= sdp_uart2_resource,
};

static struct platform_device sdp_uart3 = {
	.name		= "sdp-uart",
	.id		= 3,
    .dev = {
		.dma_mask               = &sdp_uart3_dmamask,
		.coherent_dma_mask      = 0xFFFFFFFFUL,
    },
	.num_resources	= ARRAY_SIZE(sdp_uart3_resource),
	.resource	= sdp_uart3_resource,
};

/* USB Host controllers */
static u64 sdp_ehci0_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_ehci1_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_ohci0_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_ohci1_dmamask = (u32)0xFFFFFFFFUL;
//static u64 sdp_xhci0_dmamask = (u32)0xFFFFFFFFUL;

static struct platform_device sdp_ehci0 = {
        .name           = "ehci-sdp",
        .id             = 0,
        .dev = {
                .dma_mask               = &sdp_ehci0_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ehci0_resource),
        .resource       = sdp_ehci0_resource,
};

static struct platform_device sdp_ehci1 = {
        .name           = "ehci-sdp",
        .id             = 1,
        .dev = {
                .dma_mask               = &sdp_ehci1_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ehci1_resource),
        .resource       = sdp_ehci1_resource,
};

static struct platform_device sdp_ohci0 = {
        .name           = "ohci-sdp",
        .id             = 0,
        .dev = {
                .dma_mask               = &sdp_ohci0_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ohci0_resource),
        .resource       = sdp_ohci0_resource,
};

static struct platform_device sdp_ohci1 = {
        .name           = "ohci-sdp",
        .id             = 1,
        .dev = {
                .dma_mask               = &sdp_ohci1_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ohci1_resource),
        .resource       = sdp_ohci1_resource,
};
#if 0
static struct platform_device sdp_xhci0 = {
        .name           = "xhci-sdp",
        .id             = 0,
        .dev = {
                .dma_mask               = &sdp_xhci0_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_xhci0_resource),
        .resource       = sdp_xhci0_resource,
};
#endif

// add sdpGmac platform device by tukho.kim 20091205
#include <plat/sdp_gmac_reg.h>

static int sdp1207_gmac_init(void)
{
	/* SRAM select*/
	writel(1, VA_IO_BASE0+0x00090CC0);
	return  0;
}

static struct sdp_gmac_plat sdp_gmac_platdata =  {
	.init = sdp1207_gmac_init,/* board specipic init */
	.phy_mask = 0x0,/*bit filed 1 is masked */
	.napi_weight = 64,/* netif_napi_add() weight value. 1 ~ max */
};

// sdpGmac resource
static struct resource sdpGmac_resource[] = {
        [0] = {
                .start  = PA_GMAC_BASE + SDP_GMAC_BASE,
                .end    = PA_GMAC_BASE + SDP_GMAC_BASE + sizeof(SDP_GMAC_T),
                .flags  = IORESOURCE_MEM,
        },

        [1] = {
                .start  = PA_GMAC_BASE + SDP_GMAC_MMC_BASE,
                .end    = PA_GMAC_BASE + SDP_GMAC_MMC_BASE + sizeof(SDP_GMAC_MMC_T),
                .flags  = IORESOURCE_MEM,
        },

        [2] = {
                .start  = PA_GMAC_BASE + SDP_GMAC_TIME_STAMP_BASE,
                .end    = PA_GMAC_BASE + SDP_GMAC_TIME_STAMP_BASE + sizeof(SDP_GMAC_TIME_STAMP_T),
                .flags  = IORESOURCE_MEM,
        },

        [3] = {
                .start  = PA_GMAC_BASE + SDP_GMAC_MAC_2ND_BLOCK_BASE,
                .end    = PA_GMAC_BASE + SDP_GMAC_MAC_2ND_BLOCK_BASE
                                + sizeof(SDP_GMAC_MAC_2ND_BLOCK_T),  // 128KByte
                .flags  = IORESOURCE_MEM,
        },

        [4] = {
                .start  = PA_GMAC_BASE + SDP_GMAC_DMA_BASE,
                .end    = PA_GMAC_BASE + SDP_GMAC_DMA_BASE + sizeof(SDP_GMAC_DMA_T),  // 128KByte
                .flags  = IORESOURCE_MEM,
        },

        [5] = {
                .start  = IRQ_GMAC,
                .end    = IRQ_GMAC,
                .flags  = IORESOURCE_IRQ,
        },
};

static u64 sdp_mac_dmamask = DMA_BIT_MASK(32);

static struct platform_device sdpGmac_devs = {
        .name           = ETHER_NAME,
        .id             = 0,
        .dev		= {
			.dma_mask = &sdp_mac_dmamask,
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.platform_data = &sdp_gmac_platdata,
		},
        .num_resources  = ARRAY_SIZE(sdpGmac_resource),
        .resource       = sdpGmac_resource,
};

// add sdpGmac platform device by tukho.kim 20091205 end
//
// mmc
static int sdp1207_mmch_init(SDP_MMCH_T *p_sdp_mmch)
{
	if(p_sdp_mmch->pm_is_valid_clk_delay == false)
	{
		/* save clk delay */
		p_sdp_mmch->pm_clk_delay[0] = *(volatile u32*)(VA_IO_BASE0 + 0x000908B4);
		p_sdp_mmch->pm_is_valid_clk_delay = true;
	}
	else
	{
		/* restore clk delay */
		*(volatile u32*)(VA_IO_BASE0 + 0x000908B4) = p_sdp_mmch->pm_clk_delay[0];
	}
	return 0;
}

static struct sdp_mmch_plat sdp_mmc_platdata = {
	.processor_clk = 100000000,
	.min_clk = 400000,
	.max_clk = 50000000,
	.caps =  (MMC_CAP_8_BIT_DATA | MMC_CAP_MMC_HIGHSPEED \
			| MMC_CAP_NONREMOVABLE | MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50),
	.init = sdp1207_mmch_init,
	.fifo_depth = 128,
//	.get_ro,
//	.get_cd,
//	.get_ocr,
};


static struct resource sdp_mmc_resource[] = {
        [0] = {
                .start  = PA_MMC_BASE,
                .end    = PA_MMC_BASE + 0x200,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_SDMMC,
                .end    = IRQ_SDMMC,
                .flags  = IORESOURCE_IRQ,
        },
};

static u64 sdp_mmc_dmamask = DMA_BIT_MASK(32);

static struct platform_device sdp_mmc = {
	.name		= "sdp-mmc",
	.id		= 0,
	.dev		= {
		.dma_mask = &sdp_mmc_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &sdp_mmc_platdata,
	},
	.num_resources	= ARRAY_SIZE(sdp_mmc_resource),
	.resource	= sdp_mmc_resource,
};

/* for sdp-smc device */
static struct resource sdp_smc_resource[] = {
	[0] = {		/* smcdma register */
		.start	= PA_SMCDMA_BASE,
		.end	= PA_SMCDMA_BASE + 0x1f,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {		/* smc bank 0 */
		.start	= 0,
		.end	= 0x1ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static u64 sdp_smc_dmamask = 0xffffffffUL;
static struct platform_device sdp_smc = {
	.name		= "sdp-smc",
	.id		= 0,
	.dev = {
		.dma_mask = &sdp_smc_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE (sdp_smc_resource),
	.resource	= sdp_smc_resource,
};

/* SATA0 */
static struct resource sdp_sata0_resource[] = {
        [0] = {
                .start  = PA_SATA0_BASE,
                .end    = PA_SATA0_BASE + 0x200,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_SATA0,
                .end    = IRQ_SATA0,
                .flags  = IORESOURCE_IRQ,
        },
};



/* SPI master controller */
#define OFF_MSPI_TX_CTRL		0x68// set 0x1000 for sel SPI
#define MSPI_TX_CTRL_SELSPI		(0x01 << 12)
#define REG_PAD_CTRL_10			VA_PADCTRL_BASE+0x28/* Pad select for SPI */
#define PAD_CTRL_10_SPI_SEL		(0x1<<2)
static int sdp1207_spi_init(void)
{
	void __iomem *base = ioremap(PA_SPI_BASE, 0x100);

	/* No MSPI */
	writel(readl(base+OFF_MSPI_TX_CTRL)|MSPI_TX_CTRL_SELSPI, base+OFF_MSPI_TX_CTRL);

	/* Pad SPI Selection */
	writel(readl(REG_PAD_CTRL_10)|PAD_CTRL_10_SPI_SEL, REG_PAD_CTRL_10);

	iounmap(base);
	return 0;
}

static struct resource sdp_spi_resource[] =
{
	[0] = {
			.start = PA_SPI_BASE,
			.end = PA_SPI_BASE + 0x18 - 1,
			.flags = IORESOURCE_MEM,
		},
	[1] = {
			.start = IRQ_SPI0,
			.end = IRQ_SPI0,
			.flags = IORESOURCE_IRQ,
		},
};

/* SPI Master Controller drain.lee */
static struct sdp_spi_info sdp_spi_master_data = {
	.num_chipselect = CONFIG_SDP_SPI_NUM_OF_CS,
	.init = sdp1207_spi_init,
};

static struct platform_device sdp_spi = {
	.name		= "sdp-spi",
	.id		= 0,
	.dev		= {
		.platform_data = &sdp_spi_master_data,
	},
	.num_resources	= ARRAY_SIZE(sdp_spi_resource),
	.resource	= sdp_spi_resource,
};


/* SATA1 */
static struct resource sdp_sata1_resource[] = {
        [0] = {
                .start  = PA_SATA1_BASE,
                .end    = PA_SATA1_BASE + 0x200,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_SATA1,
                .end    = IRQ_SATA1,
                .flags  = IORESOURCE_IRQ,
        },
};

static u64 sdp_sata0_dmamask = 0xFFFFFFFFUL;
static u64 sdp_sata1_dmamask = 0xFFFFFFFFUL;

static struct platform_device sdp_sata0 = {
        .name   =       "ahci-sata0",
        .id             =       0,
        .dev = {
        	.dma_mask               = &sdp_sata0_dmamask,
        	.coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_sata0_resource),
        .resource       =       sdp_sata0_resource,
};

static struct platform_device sdp_sata1 = {
        .name   =       "ahci-sata1",
        .id             =       0,
    	.dev = {
        	.dma_mask               = &sdp_sata1_dmamask,
            .coherent_dma_mask      = 0xFFFFFFFFUL,
    	},
        .num_resources  = ARRAY_SIZE(sdp_sata1_resource),
        .resource       =       sdp_sata1_resource,
};

/* regulator */
/* sn1202033 */
static struct regulator_consumer_supply sn1202033_supply[] = {
	{
		.supply = "VDD_CORE",
		.dev_name = NULL,
	},
	{
		.supply = "VDD_CPU",
		.dev_name = NULL,
	},
};

static struct regulator_init_data sn1202033_data[] = {
	[0] = {
		.constraints	= {
			.name		= "VDD_CORE range",
			.min_uV		= 680000,
			.max_uV		= 1950000,
			.always_on	= 1,
			.boot_on	= 1,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &sn1202033_supply[0],
	},
	[1] = {
		.constraints	= {
			.name		= "VDD_CPU range",
			.min_uV		= 680000,
			.max_uV		= 1950000,
			.always_on	= 1,
			.boot_on	= 1,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		},
		.num_consumer_supplies = 1,
		.consumer_supplies = &sn1202033_supply[1],
	},
};

static struct sn1202033_platform_data sn1202033_regulator[] = {
	[0] = {
		.i2c_addr	= 0xC0,
		.i2c_port	= 3,
		.def_volt	= 1100000,
		.init_data	= &sn1202033_data[0],
	},
	[1] = {
		.i2c_addr	= 0xC0,
		.i2c_port	= 3,
		.def_volt	= 1200000,
		.init_data	= &sn1202033_data[1],
	},	
};

static struct platform_device sdp_pmic_sn1202033[] = {
	[0] = {
		.name	= "sdp_sn1202033",
		.id		= 0,
		.dev	= {
			.platform_data = &sn1202033_regulator[0],
		},
	},
	[1] = {
		.name	= "sdp_sn1202033",
		.id		= 1,
		.dev	= {
			.platform_data = &sn1202033_regulator[1],
		},
	},
};
/* end Regulator */

static struct platform_device* sdp1207_init_devs[] __initdata = {
	&sdp_uart0,
	&sdp_uart1,
	&sdp_uart2,
	&sdp_uart3,
	&sdp_ehci0,
	&sdp_ohci0,
	&sdp_ehci1,
	&sdp_ohci1,
//	&sdp_xhci0,
// add sdpGmac platform device by tukho.kim 20091205
	&sdpGmac_devs,
	&sdp_mmc,
	&sdp_sata0,
	&sdp_sata1,
	&sdp_smc,
	&sdp_spi,
#ifdef CONFIG_ARCH_SDP1207_EVAL	
	&sdp_pmic_sn1202033[0],
	&sdp_pmic_sn1202033[1],
#endif
};



void __init sdp1207_iomap_init (void)
{
	iotable_init(sdp1207_io_desc, ARRAY_SIZE(sdp1207_io_desc));
}

static int sdp1207_get_revision_id(void)
{
#if 0
	void __iomem *base = (void __iomem*) (0x30091000 + DIFF_IO_BASE0);
	unsigned int rev_data;
	int rev_id;

	writel(0x1F, base + 0x704);
	while(readl(base + 0x700) != 0);
	while(readl(base + 0x700) != 0);
	readl(base + 0x708);
	rev_data = readl(base + 0x708);
	rev_data = (rev_data & 0xFC00) >> 10;
	printk("SDP Get Revision ID : %X ", rev_data);
	switch(rev_data)
	{
		case 0x31:		//ES 1
			rev_id = 1;
			break;
		case 0x22:
			rev_id = 2;
			break;
		case 0:			//ES 0
		default:
			rev_id = 0;
	}
	if((sdp_revision_id == 1) && (rev_id == 0))
		sdp_revision_id = 1;
	else
		sdp_revision_id = rev_id;

	printk("version ES%d\n", sdp_revision_id);
	return sdp_revision_id;
#endif
	return 0;
}

int sdp_get_revision_id(void)
{
	return (int) sdp_revision_id;
}

#if defined(CONFIG_CACHE_L2X0)
#define L2C310_CR	0x100	/* control register (L2X0_CTRL) */
#define L2C310_TRLCR	0x108	/* tag RAM latency control */
#define L2C310_DRLCR	0x10C	/* data RAM latency control */
#define L2C310_PCR	0Xf60	/* prefetch control */

#define L2C310_PCR_DLF	(1<<30)	/* double linefill enable */
#define L2C310_PCR_IPF	(1<<29)	/* instruction prefetch enable */
#define L2C310_PCR_DPF	(1<<28)	/* data prefetch enable */
#define L2C310_PCR_DROP	(1<<24)	/* prefetch drop enable */
#define L2C310_PCR_IDLF	(1<<23)	/* incr double linefill enable */

#define L2C310_AUX_BRESP	(1<<30) /* early BRESP enable */
#define L2C310_AUX_IPF	(1<<29)	/* inst prefetch enable */
#define L2C310_AUX_DPF	(1<<28)	/* data prefetch enable */
#define L2C310_AUX_RR	(1<<25) /* cache replacement policy */

static void __init sdp1207_l2c_init(void)
{
	void __iomem *base = (void __iomem*)VA_L2C_BASE;
	u32 v;

	/* prefetch control */
	v = readl (base + L2C310_PCR);
	v |= L2C310_PCR_IPF | L2C310_PCR_DPF;
	v |= L2C310_PCR_DROP | L2C310_PCR_IDLF;
#ifndef CONFIG_SDP_ARM_PL310_ERRATA_752271
	v |= L2C310_PCR_DLF;
#endif

	v &= ~0x1f;
	printk (KERN_CRIT "prefetch=0x%08x\n", v);
	writel (v, base + L2C310_PCR);

	/* tag RAM latency (Setup:1,Write:2,Read:2) */
	v = readl(base + L2C310_TRLCR);
	v &= (u32) ~(0x777);
	writel (v, base + L2C310_TRLCR);

	/* data RAM latency (Setup:1,Write:3,Read:2) */
	v = readl(base + L2C310_DRLCR);
	v &= (u32) ~(0x777);
	writel (v, base + L2C310_DRLCR);

	/* Data/Inst prefetch enable, random replacement */
	l2x0_init (base, 0, (u32) ~(L2C310_AUX_RR));
}
#else	/* CONFIG_SDP1105_L2C310 */
static void __init sdp1207_l2c_init(void) {}
#endif

static void sdp1207_patch(void)
{
	volatile unsigned int *v = (volatile unsigned int *) (VA_SCU_BASE + 0x30);

	*v |= 0x1;		//ARM_ERRATA_764369 SCU migratory bit set
}

void __init sdp1207_init(void)
{
#ifdef CONFIG_SDP_ARM_A9_ERRATA_764369
	sdp1207_patch();
#endif

	sdp1207_get_revision_id();
	platform_add_devices(sdp1207_init_devs, ARRAY_SIZE(sdp1207_init_devs));

}

early_initcall(sdp1207_l2c_init);

