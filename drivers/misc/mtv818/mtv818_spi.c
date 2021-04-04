#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "src/raontv.h"
#include "src/raontv_internal.h"

#include "mtv818.h"
#include "mtv818_spi.h"
#include "mtv818_ioctl.h"


#ifdef RTV_IF_SPI
//struct spi_device *dmb_spi_ptr;

void mtv818_spi_read_burst(unsigned char reg, unsigned char *buf, int size)
{
	int ret;
	u8 out_buf[2], read_out_buf[2];
	struct spi_message msg;
	struct spi_transfer msg_xfer0 = {
		.tx_buf = out_buf,
		.len		= 2,
		.cs_change	= 1,
		.delay_usecs = 0,
	};
	
	struct spi_transfer msg_xfer1 = {
		.tx_buf = read_out_buf,
		.rx_buf = buf,
		.len		= size,
		.cs_change	= 0,
		.delay_usecs = 0,
	};

	spi_message_init(&msg);
	
	out_buf[0] = RAONTV_CHIP_ADDR;
	out_buf[1] = reg;	
	spi_message_add_tail(&msg_xfer0, &msg);

	read_out_buf[0] = RAONTV_CHIP_ADDR|0x1;
	spi_message_add_tail(&msg_xfer1, &msg);

#ifndef RTV_DUAL_CHIP_USED
	ret = spi_sync(mtv818_cb_ptr->spi_ptr, &msg);
	if (ret)
	{
		DMBERR("[mtv818_spi_read_burst]	mtv818_spi_write() error: %d\n", ret);	
	}
#else
	if(RaonTvChipIdx == 0) /* Master MTV */
	{
		ret = spi_sync(mtv818_cb_ptr->spi_ptr, &msg);
		if (ret)
		{
			DMBERR("[mtv818_spi_read_burst] MASTER mtv818_spi_write() error: %d\n", ret);	
		}
	}
	else
	{
		ret = spi_sync(mtv818_cb_ptr->spi_slave_ptr, &msg);
		if (ret)
		{
			DMBERR("[mtv818_spi_read_burst] SLAVE mtv818_spi_write() error: %d\n", ret);	
		}
	}
#endif
}

unsigned char mtv818_spi_read(unsigned char reg)
{
	int ret;
	u8 out_buf[2], read_out_buf[2];
	u8 in_buf[2];
	struct spi_message msg;
	struct spi_transfer msg_xfer0 = {
		.tx_buf = out_buf,
		.len		= 2,
		.cs_change	= 1,
		.delay_usecs = 0,
	};
	
	struct spi_transfer msg_xfer1 = {
		.tx_buf = read_out_buf,
		.rx_buf = in_buf,
		.len		= 2,
		.cs_change	= 0,
		.delay_usecs = 0,
	};

	spi_message_init(&msg);
	
	out_buf[0] = RAONTV_CHIP_ADDR;
	out_buf[1] = reg;	
	spi_message_add_tail(&msg_xfer0, &msg);

	read_out_buf[0] = RAONTV_CHIP_ADDR|0x1;
	spi_message_add_tail(&msg_xfer1, &msg);

#ifndef RTV_DUAL_CHIP_USED
	ret = spi_sync(mtv818_cb_ptr->spi_ptr, &msg);
	if (ret)
	{
		DMBERR("[mtv818_spi_read]  mtv818_spi_read() error: %d\n", ret);  
		return 0xFF;
	}
#else
	if(RaonTvChipIdx == 0) /* Master MTV */
	{
		ret = spi_sync(mtv818_cb_ptr->spi_ptr, &msg);
		if (ret)
		{
			DMBERR("[mtv818_spi_read] MASTER mtv818_spi_read() error: %d\n", ret);	
			return 0xFF;
		}
	}
	else
	{
		ret = spi_sync(mtv818_cb_ptr->spi_slave_ptr, &msg);
		if (ret)
		{
			DMBERR("[mtv818_spi_read] SLAVE mtv818_spi_read() error: %d\n", ret);	
			return 0xFF;
		}
	}
#endif

	return (in_buf[1]);
}


void mtv818_spi_write(unsigned char reg, unsigned char val)
{
	u8 out_buf[3];
	u8 in_buf[3];
	struct spi_message msg;
	struct spi_transfer msg_xfer = {
		.len		= 3,
		.cs_change	= 0,
		.delay_usecs = 0,
	};
	int ret;

	spi_message_init(&msg);

	out_buf[0] = RAONTV_CHIP_ADDR;
	out_buf[1] = reg;
	out_buf[2] = val;

	msg_xfer.tx_buf = out_buf;
	msg_xfer.rx_buf = in_buf;
	spi_message_add_tail(&msg_xfer, &msg);

#ifndef RTV_DUAL_CHIP_USED
	ret = spi_sync(mtv818_cb_ptr->spi_ptr, &msg);
	if (ret)
	{
		DMBERR("[mtv818_spi_write]  mtv818_spi_write() error: %d\n", ret);  
	}
#else
	if(RaonTvChipIdx == 0) /* Master MTV */
	{
		ret = spi_sync(mtv818_cb_ptr->spi_ptr, &msg);
		if (ret)
		{
			DMBERR("[mtv818_spi_write] MASTER mtv818_spi_write() error: %d\n", ret);	
		}
	}
	else
	{
		ret = spi_sync(mtv818_cb_ptr->spi_slave_ptr, &msg);
		if (ret)
		{
			DMBERR("[mtv818_spi_write] SLAVE mtv818_spi_write() error: %d\n", ret);	
		}
	}
#endif	
}


static int mtv818_spi_probe(struct spi_device *spi)
{
	int ret;
	
	DMBMSG("[mtv818_spi_probe] ENTERED!!!!!!!!!!!!!!\n");
	
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	
	ret = spi_setup(spi);
	if (ret < 0)
	       return ret;

	mtv818_cb_ptr->spi_ptr = spi;

/*
  #ifdef RTV_DUAL_CHIP_USED
       RTV_MASTER_CHIP_SEL;
  #endif
	rtvOEM_PowerOn(1);
	DMBMSG("Check MTV818 0x00 = 0x%02x \n", RTV_REG_GET(0x00));
	rtvOEM_PowerOn(0);  
*/
	return 0;
}


#ifdef CONFIG_PM
static int mtv818_spi_suspend(struct spi_device *spi, pm_message_t msg)
{
	switch( mtv818_cb_ptr->tv_mode )
	{
#ifdef RTV_TDMB_ENABLE	
		case DMB_TV_MODE_TDMB: rtvTDMB_StandbyMode(1);	break;
#endif			

#ifdef RTV_FM_ENABLE 
		case DMB_TV_MODE_FM: rtvFM_StandbyMode(1); break;
#endif			

		default:	return -ENOTTY;			
	}

	return 0;
}

static int mtv818_spi_resume(struct spi_device *spi)
{
	switch( mtv818_cb_ptr->tv_mode )
	{
#ifdef RTV_TDMB_ENABLE	
		case DMB_TV_MODE_TDMB: rtvTDMB_StandbyMode(0);	break;
#endif			

#ifdef RTV_FM_ENABLE			
		case DMB_TV_MODE_FM: rtvFM_StandbyMode(0); break;
#endif

		default:	return -ENOTTY;			
	}

	return 0;
}
#endif

static int mtv818_spi_remove(struct spi_device *spi)
{
	return 0;
}



#ifdef RTV_DUAL_CHIP_USED
static int mtv818_slave_spi_probe(struct spi_device *spi)
{
	int ret;
	
	DMBMSG("[mtv818_spi_probe] SLAVE ENTERED!!!!!!!!!!!!!!\n");
	
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	
	ret = spi_setup(spi);
	if (ret < 0)
	       return ret;

	mtv818_cb_ptr->spi_slave_ptr = spi;

#if 0
	  RTV_SLAVE_CHIP_SEL;
	  rtvOEM_PowerOn(1);
	  DMBMSG("SLAVE MTV818 0x00 = 0x%02x \n", RTV_REG_GET(0x00));
	  rtvOEM_PowerOn(0);
#endif

	return 0;
}

static int mtv818_slave_spi_remove(struct spi_device *spi)
{
	return 0;
}
#endif /* #ifdef RTV_DUAL_CHIP_USED */



static struct spi_driver mtv818_spi_driver = {
	.driver = {
		.name		= RAONTV_DEV_NAME,
		.owner		= THIS_MODULE,
	},

	.probe    = mtv818_spi_probe,
#ifdef CONFIG_PM	
	.suspend	= mtv818_spi_suspend,
	.resume 	= mtv818_spi_resume,
#else
	.suspend	= NULL,
	.resume 	= NULL,
#endif
	.remove	= __devexit_p(mtv818_spi_remove),
};


#ifdef RTV_DUAL_CHIP_USED
static struct spi_driver mtv818_slave_spi_driver = {
	.driver = {
		.name		= "mtv818_slave",
		.owner		= THIS_MODULE,
	},

	.probe    = mtv818_slave_spi_probe,
#ifdef CONFIG_PM	
	.suspend	= NULL,
	.resume 	= NULL,
#else
	.suspend	= NULL,
	.resume 	= NULL,
#endif
	.remove	= __devexit_p(mtv818_slave_spi_remove),
};
#endif

int mtv818_spi_init(void)
{
	int ret;

	if((ret=spi_register_driver(&mtv818_spi_driver)) != 0)
	{
		DMBERR("[mtv818_spi_init] Master register error\n");
		return ret;
	}

#ifdef RTV_DUAL_CHIP_USED
	if((ret=spi_register_driver(&mtv818_slave_spi_driver)) != 0)
	{
		DMBERR("[mtv818_spi_init] Slave register error\n");
		return ret;
	}
#endif

	return ret;
}

void mtv818_spi_exit(void)
{
	spi_unregister_driver(&mtv818_spi_driver);

#ifdef RTV_DUAL_CHIP_USED
	spi_unregister_driver(&mtv818_slave_spi_driver);
#endif
}


#endif /* #ifdef RTV_IF_SPI */

