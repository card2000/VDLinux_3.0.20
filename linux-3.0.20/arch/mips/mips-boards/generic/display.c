/*
 * Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999,2000 MIPS Technologies, Inc.  All rights reserved.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Display routines for display messages in MIPS boards ascii display.
 */

#include <linux/compiler.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <asm/mips-boards/generic.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

//extern const char display_string[];
//static unsigned int display_count;
//static unsigned int max_display_count;

void sys_set_pnl_init_status(int status);
int sys_get_pnl_init_status(void);

#if 1
// Added by Mstar
// Add a variable to set panel init status for fast logo function.
static int sys_pnl_init_status = 0;

void sys_set_pnl_init_status(int status)
{
    sys_pnl_init_status = status;
    return;
}

int sys_get_pnl_init_status(void)
{
    return sys_pnl_init_status;
}
#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wredundant-decls"
EXPORT_SYMBOL(sys_set_pnl_init_status);
EXPORT_SYMBOL(sys_get_pnl_init_status);
#pragma GCC diagnostic pop   // require GCC 4.6
#endif

#if 0
void mips_display_message(const char *str)
{
#if 0
	static unsigned int __iomem *display = NULL;
	int i;

	if (unlikely(display == NULL))
		display = ioremap(ASCII_DISPLAY_POS_BASE, 16*sizeof(int));

	for (i = 0; i <= 14; i=i+2) {
	         if (*str)
		         __raw_writel(*str++, display + i);
		 else
		         __raw_writel(' ', display + i);
	}
#endif
}

//static void scroll_display_message(unsigned long data);
//static DEFINE_TIMER(mips_scroll_timer, scroll_display_message, HZ, 0);

//static void scroll_display_message(unsigned long data)
void scroll_display_message(unsigned long data)
{
#if 0
	mips_display_message(&display_string[display_count++]);
	if (display_count == max_display_count)
		display_count = 0;

	mod_timer(&mips_scroll_timer, jiffies + HZ);
#endif
}

void mips_scroll_message(void)
{
#if 0
	del_timer_sync(&mips_scroll_timer);
	max_display_count = strlen(display_string) + 1 - 8;
	mod_timer(&mips_scroll_timer, jiffies + 1);
#endif
}
#endif
