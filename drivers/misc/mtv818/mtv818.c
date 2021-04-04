/*
 * mtv818.c
 *
 * RAONTECH MTV818 driver.
 *
 * (c) COPYRIGHT 2010 RAONTECH, Inc. ALL RIGHTS RESERVED.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "mtv818.h"
#include "mtv818_ioctl.h"
#include "mtv818_gpio.h"
  
#include "./src/raontv.h"
#include "./src/raontv_internal.h"

#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_SPI_SLAVE)	
	#include "mtv818_i2c.h"
#elif defined(RTV_IF_SPI)
	#include "mtv818_spi.h"
#endif

extern irqreturn_t mtv818_isr(int irq, void *param);

struct mtv818_cb *mtv818_cb_ptr = NULL;
extern int mtv818_isr_thread(void *data);


static const U8 mtv_reg_page_addr[] = 
{
	0x07/*HOST*/, 0x0F/*RF*/, 0x04/*COMM*/, 0x09/*DD*/,
	0x0B/*MSC0*/, 0x0C/*MSC1*/
};

#if defined(RTV_IF_SPI)

	#define _DEBUG_TSP_POOL
	#ifdef _DEBUG_TSP_POOL
		static unsigned int max_used_tsp_cnt;
	#endif

static MTV818_TSP_QUEUE_INFO mtv818_tsp_pool;
static MTV818_TSP_QUEUE_INFO mtv818_tsp_queue;


unsigned int mtv818_get_total_tsp(void)
{
	unsigned int total_tsp_bytes = mtv818_tsp_queue.cnt * MTV_TS_THRESHOLD_SIZE;
	
	if(mtv818_cb_ptr->prev_tsp != NULL)
		total_tsp_bytes += mtv818_cb_ptr->prev_tsp->len;

	return total_tsp_bytes;
}

void mtv818_reset_tsp(void)
{
	MTV818_TS_PKT_INFO *tsp;
	
  	DMBMSG("[mtv818_reset_tsp] Max used TSP count: %d\n", max_used_tsp_cnt);

  	if(mtv818_cb_ptr->prev_tsp != NULL)
	{
		mtv818_free_tsp(mtv818_cb_ptr->prev_tsp);
		mtv818_cb_ptr->prev_tsp = NULL; 
	}
	while ((tsp=mtv818_get_tsp()) != NULL) 
	{
		mtv818_free_tsp(tsp);
	}

	max_used_tsp_cnt = 0;

	DMBMSG("[mtv818_reset_tsp] Pool TSP count: %d\n", mtv818_tsp_pool.cnt);
}

// Dequeue a ts packet from ts data queue.
MTV818_TS_PKT_INFO * mtv818_get_tsp(void)
{
	MTV818_TS_PKT_INFO *tsp = NULL;
	struct list_head *head_ptr = &mtv818_tsp_queue.head;
		
	if(mtv818_tsp_queue.cnt != 0) //if(!list_empty(head_ptr))
	{
		spin_lock(&mtv818_tsp_queue.lock);
		
		tsp = list_first_entry(head_ptr, MTV818_TS_PKT_INFO, link);
		list_del(&tsp->link);
		mtv818_tsp_queue.cnt--;

		spin_unlock(&mtv818_tsp_queue.lock);
	}
	
	return tsp;
}

// Enqueue a ts packet into ts data queue.
void mtv818_put_tsp(MTV818_TS_PKT_INFO *tsp)
{
	spin_lock(&mtv818_tsp_queue.lock);

	list_add_tail(&tsp->link, &mtv818_tsp_queue.head);
	mtv818_tsp_queue.cnt++;

	spin_unlock(&mtv818_tsp_queue.lock);
}


void mtv818_init_tsp_queue(void)
{
	mtv818_cb_ptr->prev_tsp = NULL;
	
	spin_lock_init(&mtv818_tsp_queue.lock);
	INIT_LIST_HEAD(&mtv818_tsp_queue.head);
	mtv818_tsp_queue.cnt = 0;
}


void mtv818_free_tsp(MTV818_TS_PKT_INFO *tsp)
{	
	spin_lock(&mtv818_tsp_pool.lock);

	tsp->len = 0;
	list_add_tail(&tsp->link, &mtv818_tsp_pool.head);
	mtv818_tsp_pool.cnt++;

	spin_unlock(&mtv818_tsp_pool.lock);
}


MTV818_TS_PKT_INFO *mtv818_alloc_tsp(void)
{	
	MTV818_TS_PKT_INFO *tsp = NULL;
	struct list_head *head_ptr = &mtv818_tsp_pool.head;
		
	if(mtv818_tsp_pool.cnt != 0) //if(!list_empty(head_ptr))
	{
		spin_lock(&mtv818_tsp_pool.lock);
		
		tsp = list_first_entry(head_ptr, MTV818_TS_PKT_INFO, link);
		list_del(&tsp->link);
		mtv818_tsp_pool.cnt--;
#ifdef _DEBUG_TSP_POOL
		max_used_tsp_cnt = MAX(max_used_tsp_cnt, MAX_NUM_TS_PKT_BUF-mtv818_tsp_pool.cnt);
#endif	
		spin_unlock(&mtv818_tsp_pool.lock);
	}
	
	return tsp;
	
}


static int mtv818_delete_tsp_pool(void)
{
	struct list_head *head_ptr = &mtv818_tsp_pool.head;
	MTV818_TS_PKT_INFO *tsp;

	while ((tsp=mtv818_get_tsp()) != NULL) 
	{
		kfree(tsp);
	}

	while (!list_empty(head_ptr)) 
	{		
		tsp = list_entry(head_ptr->next, MTV818_TS_PKT_INFO, link);
		list_del(&tsp->link);		
		kfree(tsp);		
	}

	return 0;
}


static int mtv818_create_tsp_pool(void)
{
	unsigned int i;
	MTV818_TS_PKT_INFO *tsp;

	spin_lock_init(&mtv818_tsp_pool.lock);
	INIT_LIST_HEAD(&mtv818_tsp_pool.head);

	mtv818_tsp_pool.cnt = 0;
#ifdef _DEBUG_TSP_POOL
	max_used_tsp_cnt = 0;
#endif	

	for(i=0; i<MAX_NUM_TS_PKT_BUF; i++)
	{
		tsp = (MTV818_TS_PKT_INFO *)kmalloc(sizeof(MTV818_TS_PKT_INFO), GFP_DMA); 
		if(tsp == NULL)
		{
			mtv818_delete_tsp_pool();
			DMBERR("[mtv818_create_tsp_pool] %d TSP allocation failed!\n", i);
			return -ENOMEM;
		}

		tsp->len = 0;
		list_add_tail(&tsp->link, &mtv818_tsp_pool.head);
		mtv818_tsp_pool.cnt++;
	}	

 	return 0;
}

static void mtv818_exit_read_function(void)
{
#ifndef MTV818_NON_BLOCKING_READ_MODE
  	/* Stop read() function. User function may blocked at read(). */
	mtv818_cb_ptr->stop = 1;
	wake_up(&mtv818_cb_ptr->read_wq);

	/* Wait fo read() to free a previous tsp. */
	wait_for_completion(&mtv818_cb_ptr->read_exit);
#endif
}

static void mtv818_start_read_function(void)
{
#ifndef MTV818_NON_BLOCKING_READ_MODE
	mtv818_cb_ptr->stop = 0;
	init_completion(&mtv818_cb_ptr->read_exit);	
#endif
}
#endif


static int mtv818_configure_gpio(void)
{
	if(gpio_request(MTV_PWR_EN, "MTV_PWR_EN"))		
		DMBMSG("MTV_PWR_EN Port request error!!!\n");
	else	
	{
		// MTV_EN
		s3c_gpio_cfgpin(MTV_PWR_EN, S3C_GPIO_SFN(MTV_PWR_EN_CFG_VAL));
		s3c_gpio_setpull(MTV_PWR_EN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(MTV_PWR_EN, 0); // power down
		gpio_free(MTV_PWR_EN);
	}

	return 0;
}

#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
static int mtv818_isr_thread_run(void)
{	
	if(mtv818_cb_ptr->isr_thread_cb != NULL)
		return 0;

	//mtv818_cb_ptr->stop = 0;
	//init_MUTEX_LOCKED(&mtv818_cb_ptr->isr_sem);
	mtv818_cb_ptr->isr_cnt = 0;	
	init_waitqueue_head(&mtv818_cb_ptr->isr_wq); 	
	
	mtv818_cb_ptr->isr_thread_cb = kthread_run(mtv818_isr_thread, NULL, "mtv_isr_thread");
	if (IS_ERR(mtv818_cb_ptr->isr_thread_cb)) 
	{
		mtv818_cb_ptr->isr_thread_cb = NULL;
		return PTR_ERR(mtv818_cb_ptr->isr_thread_cb);
	}

	return 0;
}

static void mtv818_isr_thread_stop(void)
{
	if(mtv818_cb_ptr->isr_thread_cb == NULL)
		return;

	kthread_stop(mtv818_cb_ptr->isr_thread_cb);
	mtv818_cb_ptr->isr_thread_cb = NULL;	
}
#endif

static int mtv818_power_off(void)
{
	DMBMSG("[mtv818_power_off] START\n");

	if(mtv818_cb_ptr->is_power_on == FALSE)
		return 0;

	RTV_MASTER_CHIP_SEL;
	rtvOEM_PowerOn(0);

#ifdef RTV_DUAL_CHIP_USED
	RTV_SLAVE_CHIP_SEL;
	rtvOEM_PowerOn(0);
#endif

	mtv818_cb_ptr->is_power_on = FALSE;

#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
	free_irq(RAONTV_IRQ_INT, NULL);
	mtv818_reset_tsp();
#endif	

	DMBMSG("[mtv818_power_off] END\n");

	return 0;
}


static int mtv818_power_on(void)
{
	int ret = 0;

	if(mtv818_cb_ptr->is_power_on == TRUE)	
		return 0;

	RTV_MASTER_CHIP_SEL;
	rtvOEM_PowerOn(1);
#ifdef RTV_DUAL_CHIP_USED 
	RTV_SLAVE_CHIP_SEL;
	rtvOEM_PowerOn(1);
#endif
	
	mtv818_cb_ptr->is_power_on = TRUE;
	mtv818_cb_ptr->stop = 0;
	
	switch( mtv818_cb_ptr->tv_mode )
	{
#ifdef RTV_TDMB_ENABLE	
		case DMB_TV_MODE_TDMB :
			ret = rtvTDMB_Initialize(mtv818_cb_ptr->country_band_type);
			DMBMSG("[mtv818_power_on] T-DMB Init done\n" );
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv818_power_on] rtvTDMB_Initialize() failed: %d\n", ret);
				ret = -EFAULT;		
				goto err_return;
			}		
			break;
#endif	

#ifdef RTV_ISDBT_ENABLE
		case DMB_TV_MODE_1SEG :
			ret = rtvISDBT_Initialize(mtv818_cb_ptr->country_band_type, MTV_TS_THRESHOLD_SIZE);
			if(ret  != RTV_SUCCESS)
			{
				DMBERR("[mtv818_power_on] rtvISDBT_Initialize() failed: %d\n", ret);
				ret = -EFAULT;		
				goto err_return;
			}	
			break;
#endif			

#ifdef RTV_FM_ENABLE
		case DMB_TV_MODE_FM :
			ret = rtvFM_Initialize(mtv818_cb_ptr->adc_clk_type, MTV_TS_THRESHOLD_SIZE);
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv818_power_on] rtvFM_Initialize() failed: %d\n", ret);
				ret = -EFAULT;		
				goto err_return;
			}			
			break;
#endif
		default: goto err_return;
	}
	
#if defined(RTV_IF_SPI)
	mtv818_start_read_function(); /* To change stop flag when Power On/OFF. */
#endif	

#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
	mtv818_cb_ptr->isr_cnt = 0;	
	ret = request_irq(RAONTV_IRQ_INT, mtv818_isr, IRQ_TYPE_EDGE_FALLING, RAONTV_DEV_NAME, NULL);	
	if (ret != 0) 
	{
		DMBERR("[mtv818_power_on] Failed to install irq (%d)\n", ret);
		goto err_return;
	}	
#endif	
	
	DMBMSG("[mtv818_power_on] Power on done\n" );

	return 0;

err_return:

	mtv818_power_off();

	return ret;
}



static void mtv818_deinit_device(void)
{
	mtv818_power_off();

#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
//	free_irq(RAONTV_IRQ_INT, NULL);

	mtv818_isr_thread_stop();
#endif	


#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_SPI_SLAVE)	
	mtv818_i2c_exit(); 
  
#elif defined(RTV_IF_SPI)	
	/* Wait for read() compeltion. memory crasy... */
	mtv818_delete_tsp_pool();

	mtv818_spi_exit(); 
#endif	

	if(mtv818_cb_ptr != NULL)
		kfree(mtv818_cb_ptr);


	DMBMSG("[mtv818_deinit_device] END\n");
}


static int mtv818_init_device(void)
{
	int ret = 0;

	mtv818_cb_ptr = kzalloc(sizeof(struct mtv818_cb), GFP_KERNEL);
	if (!mtv818_cb_ptr)
		return -ENOMEM;
	
	mtv818_cb_ptr->is_power_on = 0; // for prove()	
	mtv818_cb_ptr->stop = 0;

	mtv818_configure_gpio();

#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_MPEG2_PARALLEL_TSIF) || defined(RTV_IF_QUALCOMM_TSIF)	
	ret = mtv818_i2c_init();
	if(ret < 0)
	{
		DMBERR("RAONTV I2C driver registe failed\n");
		goto err_free_mem;
	}   
	
#elif defined(RTV_IF_SPI)	
	ret = mtv818_spi_init();
	if(ret < 0)
	{
		DMBERR("RAONTV SPI driver registe failed\n");
		goto err_free_mem;
	}

	/* Init tsp queue.*/
	mtv818_init_tsp_queue();

	ret = mtv818_create_tsp_pool(); 
	if(ret < 0)
	{
		DMBERR("RAONTV SPI TS buffer creation failed\n");
		goto err_free_mem;
	}	
#endif	
	
#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	
	mtv818_cb_ptr->isr_thread_cb = NULL;	
	if ((ret=mtv818_isr_thread_run()) != 0) 
	{		
		DMBERR("[mtv818_power_on] mtv818_isr_thread_run() error\n");
		goto err_free_mem;
	}
#endif	
		
	return 0;
	
err_free_mem:
	kfree(mtv818_cb_ptr);
	mtv818_cb_ptr = NULL;
   
	return ret;	
}


#if defined(RTV_IF_SPI) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE)
static ssize_t tdmb_cif_read(char *buf)
{		
	int ret = -ENODEV;
	UINT i;
	RTV_CIF_DEC_INFO cif_dec_info;
	MTV818_TS_PKT_INFO *tsp = NULL; 
	ssize_t read_len = 0;
	TDMB_CIF_TS_INFO *ret_cif_ts = (TDMB_CIF_TS_INFO *)buf;

#ifndef MTV818_NON_BLOCKING_READ_MODE 	
	/* To release the read() of application as possible as soon, we use a queue count only. */
	ret = wait_event_interruptible(mtv818_cb_ptr->read_wq, 
							  mtv818_tsp_queue.cnt || mtv818_cb_ptr->stop);	
	if(ret < 0)
	{
		mtv818_cb_ptr->stop = 1; // In case, ERESTARTSYS
		DMBERR("[mtv818_read] woken up fail: %d\n", ret);
		goto read_fail;
	}

	if(mtv818_cb_ptr->stop)
	{
		DMBMSG("[mtv818_read] Device stopped. q:%d, stop:%d\n", mtv818_tsp_queue.cnt, mtv818_cb_ptr->stop); 		
		ret = -ENODEV;
		goto read_fail;
	}			
#endif		

	/* Get a new tsp from tsp_queue. */
	tsp = mtv818_get_tsp(); 
	if(tsp == NULL)
	{
		ret = -EAGAIN;
#ifndef MTV818_NON_BLOCKING_READ_MODE 		
		DMBERR("[mtv818_read] Abnormal case\n");
#endif
		goto read_fail;
	}

	/* Set the address of decoded MSC buffer to be received. */
	for(i=0; i<RTV_MAX_NUM_MULTI_SUB_CHANNEL; i++)
	{
		cif_dec_info.msc_buf_ptr[i] = ret_cif_ts->msc_buf[i];
	}	
	rtvCIFDEC_Decode(&cif_dec_info, &tsp->msc_buf[1]/*src MSC*/, tsp->len);					

	mtv818_free_tsp(tsp);  

	/* Copy the decoed inforamtion to user. MSC data was copied in the rtvCIFDEC_Decode(). */
	for(i=0; i<RTV_MAX_NUM_MULTI_SUB_CHANNEL; i++)
	{
		read_len += cif_dec_info.msc_size[i];
			
		ret = copy_to_user(&ret_cif_ts->msc_size[i], &cif_dec_info.msc_size[i], sizeof(unsigned int));
		if (ret < 0) 
		{
			DMBERR("[mtv818_read] copy user fail: %d\n", ret);
			ret = -EFAULT;
			goto read_fail;
		}		

		ret = copy_to_user(&ret_cif_ts->msc_subch_id[i], &cif_dec_info.msc_subch_id[i], sizeof(unsigned int));
		if (ret < 0) 
		{
			DMBERR("[mtv818_read] copy user fail: %d\n", ret);
			ret = -EFAULT;
			goto read_fail;
		}		
	}

	return read_len;

read_fail:	
#ifndef MTV818_NON_BLOCKING_READ_MODE 
	/* Complete the read()*/
	if(mtv818_cb_ptr->stop)
	{		
		complete(&mtv818_cb_ptr->read_exit); // 		
	}
#endif

	return ret;
}
#endif /* #if defined(RTV_IF_SPI) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) */


static ssize_t mtv818_read(struct file *filp, char *buf, size_t count, loff_t *pos)
{    	
	int ret = -ENODEV;
	
#if defined(RTV_IF_SPI)		
	int copy_bytes;
	unsigned int copy_offset;
	MTV818_TS_PKT_INFO *tsp = NULL;	
#ifdef MTV818_NON_BLOCKING_READ_MODE 
	ssize_t read_len = 0;
#else
	ssize_t read_len = count;
#endif

#if defined(RTV_TDMB_ENABLE) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE)
	if(mtv818_cb_ptr->tv_mode == DMB_TV_MODE_TDMB)
	{
		return tdmb_cif_read(buf);
	}
#endif

	if(count == 0)
	{
		DMBERR("[mtv818_read] Invalid length: %d.\n", count);
		return -EAGAIN;
	}

	do
	{
#ifndef MTV818_NON_BLOCKING_READ_MODE 	
		/* To release the read() of application as possible as soon, we use a queue count only. */
		ret = wait_event_interruptible(mtv818_cb_ptr->read_wq, 
							         mtv818_tsp_queue.cnt || mtv818_cb_ptr->stop); 	
		if(ret < 0)
		{
			mtv818_cb_ptr->stop = 1; // In case, ERESTARTSYS
			DMBERR("[mtv818_read] woken up fail: %d\n", ret);
			goto read_fail;
		}

		if(mtv818_cb_ptr->stop)
		{
			DMBMSG("[mtv818_read] Device stopped. q:%d, stop:%d\n", mtv818_tsp_queue.cnt, mtv818_cb_ptr->stop);			
			ret = -ENODEV;
			goto read_fail;
		}			
#endif		

		if(mtv818_cb_ptr->prev_tsp == NULL)
		{
			tsp = mtv818_get_tsp(); /* Get a new tsp from tsp_queue. */
			if(tsp == NULL)
			{
				ret = -EAGAIN;
#ifdef MTV818_NON_BLOCKING_READ_MODE 
				break; // Stop 
#else				
				DMBERR("[mtv818_read] Abnormal case\n");
				goto read_fail;
#endif				
			}
			
			copy_offset = 0;
			mtv818_cb_ptr->prev_tsp = tsp; // Save to use in next time if not finishded.	
			mtv818_cb_ptr->prev_org_tsp_size = tsp->len; 
		}
		else
		{
			tsp = mtv818_cb_ptr->prev_tsp;			
			copy_offset = mtv818_cb_ptr->prev_org_tsp_size - tsp->len;
		}

		copy_bytes = MIN(tsp->len, count);		

		ret = copy_to_user(buf, (&tsp->msc_buf[1] + copy_offset), copy_bytes);
		if (ret < 0) 
		{
			DMBERR("[mtv818_read] copy user fail: %d\n", ret);
			ret = -EFAULT;
			goto read_fail;
		}
		else
		{
			//DMBMSG("[mtv818_read] copy user ret: %d\n", ret);
#ifdef MTV818_NON_BLOCKING_READ_MODE 
			read_len += copy_bytes;
#endif
			buf        += copy_bytes;
			count     -= copy_bytes;
			tsp->len  -= copy_bytes;
			if(tsp->len == 0)
			{
				mtv818_free_tsp(tsp);  

				if(mtv818_cb_ptr->prev_tsp == tsp)
					mtv818_cb_ptr->prev_tsp = NULL; /* All used. */
			}
		}		
	} while(count != 0);

	return  read_len;

read_fail:	
	if(mtv818_cb_ptr->prev_tsp != NULL)
	{
		mtv818_free_tsp(mtv818_cb_ptr->prev_tsp);
		mtv818_cb_ptr->prev_tsp = NULL; 
	}

#ifndef MTV818_NON_BLOCKING_READ_MODE 
	/* Complete the read()*/
	if(mtv818_cb_ptr->stop)
	{		
		complete(&mtv818_cb_ptr->read_exit); //			
	}
#endif

#endif // #if defined(RTV_IF_SPI)		

	return ret;
}



#ifdef RTV_FM_ENABLE
static IOCTL_FM_SCAN_INFO fm_scan_info;
#endif


static int mtv818_ioctl(struct inode *inode, struct file *filp,
				        unsigned int cmd,  unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;	
	RTV_IOCTL_REGISTER_ACCESS ioctl_register_access;	
	RTV_IOCTL_TEST_GPIO_INFO gpio_info;
	U8 reg_page = 0;
#if defined(RTV_TDMB_ENABLE) || defined(RTV_ISDBT_ENABLE)		
	UINT lock_mask;	
#endif	

#ifdef RTV_ISDBT_ENABLE
	UINT isdbt_ch_num;
	RTV_ISDBT_TMCC_INFO isdbt_tmcc_info;
	IOCTL_ISDBT_SIGNAL_INFO isdbt_signal_info;
#endif

#ifdef RTV_TDMB_ENABLE	
	IOCTL_TDMB_SUB_CH_INFO tdmb_sub_ch_info;
	U16 tdmb_sub_ch_id;
	IOCTL_TDMB_SIGNAL_INFO tdmb_signal_info;
   //#ifdef RTV_TDMB_FIC_POLLING_MODE	
	   #if defined(RTV_IF_SPI)		
		U8 fic_buf[384+1];
	   #elif defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
	   	U8 fic_buf[384];
	   #endif
   //#endif
#endif

	U32 ch_freq_khz;

	switch( cmd )
	{
#ifdef RTV_TDMB_ENABLE
		case IOCTL_TDMB_POWER_ON:					
			if (copy_from_user(&mtv818_cb_ptr->country_band_type , argp, sizeof(E_RTV_COUNTRY_BAND_TYPE)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}		
			DMBMSG("[mtv] IOCTL_TDMB_POWER_ON: country_band_type(%d) \n", mtv818_cb_ptr->country_band_type );

			mtv818_cb_ptr->tv_mode = DMB_TV_MODE_TDMB; 					
			ret = mtv818_power_on();
			if(ret != 0)
			{
				DMBMSG("[mtv] IOCTL_TDMB_POWER_ON error: %d\n", ret);
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}		
			break;

		case IOCTL_TDMB_POWER_OFF:
			DMBMSG("[mtv] IOCTL_TDMB_POWER_OFF\n");
			mtv818_power_off();
			break;
	
		case IOCTL_TDMB_SCAN_FREQ:
			if(mtv818_cb_ptr->is_power_on == FALSE)
			{			
				DMBERR("[mtv] Power Down state!Must Power ON\n");
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			if (copy_from_user(&ch_freq_khz, argp, sizeof(U32)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			ret = rtvTDMB_ScanFrequency(ch_freq_khz);
			DMBMSG("[mtv] TDMB SCAN(%d) result: %d\n", ch_freq_khz, ret);
			if(ret != RTV_SUCCESS)
			{
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}

			/* Open FIC. */
			rtvTDMB_OpenFIC();
			break;

		case IOCTL_TDMB_SCAN_STOP:
			rtvTDMB_CloseFIC();
			break;

              case IOCTL_TDMB_READ_FIC:
			if(rtvTDMB_ReadFIC(fic_buf) == 0)
			{
				DMBERR("[mtv] rtvTDMB_ReadFIC() error\n");
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}
  
			//DMBMSG("\fic_buf[0]: 0x%02X, fic_buf[1]: 0x%02X, fic_buf[2]: 0x%02X\n", fic_buf[1], fic_buf[2], fic_buf[3]);

	#if defined(RTV_IF_SPI)		
			if (copy_to_user(argp, &fic_buf[1], 384))
	#else
			if (copy_to_user(argp, fic_buf, 384))
	#endif
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}				
                    break;
                    
		case IOCTL_TDMB_OPEN_SUBCHANNEL: // Start PLAY
			if(mtv818_cb_ptr->is_power_on == FALSE)
			{			
				DMBMSG("[mtv] Power Down state!Must Power ON\n");
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			if (copy_from_user(&tdmb_sub_ch_info, argp, sizeof(IOCTL_TDMB_SUB_CH_INFO)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			DMBMSG("[mtv] IOCTL_TDMB_OPEN_SUBCHANNEL: ch_freq_khz: %d, tdmb_sub_ch_id:%u, service_type: %d\n", tdmb_sub_ch_info.ch_freq_khz, tdmb_sub_ch_info.sub_ch_id, tdmb_sub_ch_info.service_type);

	#if defined(RTV_IF_SPI)
		#if defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE)
			/* If the specified freq is the new freq, we disables the streaming and resets TSP buffer for Multi Sub Channel. */
			if(rtvTDMB_GetPreviousFrequency() != tdmb_sub_ch_info.ch_freq_khz)			
		#endif
			{
				rtvTDMB_DisableStreamOut();
				mtv818_reset_tsp();
			}
	#endif
			ret = rtvTDMB_OpenSubChannel(tdmb_sub_ch_info.ch_freq_khz, tdmb_sub_ch_info.sub_ch_id, tdmb_sub_ch_info.service_type, MTV_TS_THRESHOLD_SIZE);
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv] rtvTDMB_OpenSubChannel() error %d\n", ret);
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			break;		

		case IOCTL_TDMB_CLOSE_SUBCHANNEL: // Stop PLAY
			if (copy_from_user(&tdmb_sub_ch_id, argp, sizeof(UINT)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}
			DMBMSG("[mtv] IOCTL_TDMB_CLOSE_SUBCHANNEL: tdmb_sub_ch_id(%d)\n", tdmb_sub_ch_id);	

			ret = rtvTDMB_CloseSubChannel(tdmb_sub_ch_id);
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv] rtvTDMB_CloseSubChannel() error %d: tdmb_sub_ch_id(%d)\n", ret, tdmb_sub_ch_id);
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			break;		

		case IOCTL_TDMB_GET_LOCK_STATUS:
			lock_mask = rtvTDMB_GetLockStatus();
			DMBMSG("[mtv] IOCTL_TDMB_GET_LOCK_STATUS=%u\n", lock_mask);
			if (copy_to_user(argp,&lock_mask, sizeof(UINT)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			break;

		case IOCTL_TDMB_GET_SIGNAL_INFO:
			tdmb_signal_info.ber = rtvTDMB_GetBER(); 
			tdmb_signal_info.cer = rtvTDMB_GetCER(); 
			tdmb_signal_info.cnr = rtvTDMB_GetCNR(); 
			tdmb_signal_info.per = rtvTDMB_GetPER(); 
			tdmb_signal_info.rssi = rtvTDMB_GetRSSI(); 
			
			if (copy_to_user(argp, &tdmb_signal_info, sizeof(IOCTL_TDMB_SIGNAL_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			break;

		case IOCTL_TEST_TDMB_SET_FREQ:
			if (copy_from_user(&ch_freq_khz, argp, sizeof(U32)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			DMBMSG("[mtv] IOCTL_TEST_TDMB_SET_FREQ: %d\n",  ch_freq_khz);

			ret =rtvTDMB_OpenSubChannel(ch_freq_khz, 0, 0, MTV_TS_THRESHOLD_SIZE); 
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv] rtvTDMB_OpenSubChannel() error %d\n", ret);
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			break;
#endif /* #ifdef RTV_TDMB_ENABLE */			


#ifdef RTV_ISDBT_ENABLE
		case IOCTL_ISDBT_POWER_ON: // with adc clk type
			if (copy_from_user(&mtv818_cb_ptr->country_band_type, argp, sizeof(E_RTV_COUNTRY_BAND_TYPE)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}				

			mtv818_cb_ptr->tv_mode = DMB_TV_MODE_1SEG;
			
			DMBMSG("[mtv] IOCTL_ISDBT_POWER_ON:  country_band_type(%d)\n", mtv818_cb_ptr->country_band_type);
	
			ret = mtv818_power_on();
			if(ret  != 0)
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}	
			break;

		case IOCTL_ISDBT_POWER_OFF:
			DMBMSG("[mtv] IOCTL_ISDBT_POWER_OFF\n");
			mtv818_power_off();
			break;

		case IOCTL_ISDBT_SCAN_FREQ:
			if (copy_from_user(&isdbt_ch_num, argp, sizeof(UINT)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}
			
			ret = rtvISDBT_ScanFrequency(isdbt_ch_num);
			DMBMSG("[mtv] ISDBT SCAN(%d) result: %d\n", isdbt_ch_num, ret);
			if(ret != RTV_SUCCESS)
			{
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			break;
			
		case IOCTL_ISDBT_SET_FREQ:
			if (copy_from_user(&isdbt_ch_num, argp, sizeof(UINT)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}
			
			DMBMSG("[mtv] IOCTL_ISDBT_SET_FREQ: %d\n", isdbt_ch_num);

	#if defined(RTV_IF_SPI)
			/* If the specified freq is the new freq, we disables the streaming and resets TSP buffer. */
			rtvISDBT_DisableStreamOut();
			mtv818_reset_tsp();
	#endif
			ret=rtvISDBT_SetFrequency(isdbt_ch_num);
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv] IOCTL_ISDBT_SET_FREQ error %d\n", ret);
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			break;		

		case IOCTL_ISDBT_GET_LOCK_STATUS:
			lock_mask=rtvISDBT_GetLockStatus();
			if (copy_to_user(argp,&lock_mask, sizeof(UINT)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			break;

		case IOCTL_ISDBT_GET_TMCC:		
			DMBMSG("[mtv] IOCTL_ISDBT_GET_TMCC\n");
			rtvISDBT_GetTMCC(&isdbt_tmcc_info);
			if (copy_to_user(argp,&isdbt_tmcc_info, sizeof(RTV_ISDBT_TMCC_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}		
			break;

		case IOCTL_ISDBT_GET_SIGNAL_INFO:
			//DMBMSG("[mtv] IOCTL_ISDBT_GET_SIGNAL_INFO\n");
			isdbt_signal_info.ber = rtvISDBT_GetBER(); 
			isdbt_signal_info.cnr = rtvISDBT_GetCNR(); 
			isdbt_signal_info.per = rtvISDBT_GetPER(); 
			isdbt_signal_info.rssi = rtvISDBT_GetRSSI(); 
			
			if (copy_to_user(argp, &isdbt_signal_info, sizeof(IOCTL_ISDBT_SIGNAL_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			break;		
#endif /* #ifdef RTV_ISDBT_ENABLE */

#ifdef RTV_FM_ENABLE
		case IOCTL_FM_POWER_ON: // with adc clk type
			if (copy_from_user(&mtv818_cb_ptr->adc_clk_type, argp, sizeof(E_RTV_ADC_CLK_FREQ_TYPE)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			
			mtv818_cb_ptr->tv_mode = DMB_TV_MODE_FM;
			
			DMBMSG("[mtv] IOCTL_FM_POWER_ON: ADC type: %d\n", mtv818_cb_ptr->adc_clk_type); 
			ret = mtv818_power_on();
			if(ret != 0)
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}			
			break;

		case IOCTL_FM_POWER_OFF:
			DMBMSG("[mtv] IOCTL_FM_POWER_OFF\n");
			mtv818_power_off();
			break;

		case IOCTL_FM_SCAN_FREQ:
			if (copy_from_user(&fm_scan_info, argp, sizeof(IOCTL_FM_SCAN_INFO)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			DMBMSG("[mtv] IOCTL_TDMB_SCAN_FREQ: start_freq(%d) ~ end_freq(%d) \n", fm_scan_info.start_freq, fm_scan_info.end_freq);	
			
			ret = rtvFM_ScanFrequency(fm_scan_info.ch_buf, fm_scan_info.num_ch_buf, fm_scan_info.start_freq, fm_scan_info.end_freq);
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv] IOCTL_TDMB_SCAN_FREQ error %d\n", ret);
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			fm_scan_info.num_deteced_ch = ret;

			if (copy_to_user(argp, &fm_scan_info, sizeof(IOCTL_FM_SCAN_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			break;
			
		case IOCTL_FM_SET_FREQ:
			if (copy_from_user(&ch_freq_khz, argp, sizeof(U32)))
			{
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}

			DMBMSG("[mtv] IOCTL_FM_SET_FREQ: %d\n", ch_freq_khz); 		

	#if defined(RTV_IF_SPI)
			/* If the specified freq is the new freq, we disables the streaming and resets TSP buffer. */
			rtvFM_DisableStreamOut();
			mtv818_reset_tsp();
	#endif
	
			ret=rtvFM_SetFrequency(ch_freq_khz);
			if(ret != RTV_SUCCESS)
			{
				DMBERR("[mtv] IOCTL_TDMB_SET_FREQ error %d\n", ret);
				ret = -EFAULT;	
				goto IOCTL_EXIT;
			}
			break;			
#endif /* #ifdef RTV_FM_ENABLE */

		case IOCTL_REGISTER_READ:
			if(mtv818_cb_ptr->is_power_on == FALSE)
			{			
				DMBMSG("[mtv] Power Down state!Must Power ON\n");
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}
			
			if (copy_from_user(&ioctl_register_access, argp, sizeof(RTV_IOCTL_REGISTER_ACCESS)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}		

			//DMBMSG("[mtv] IOCTL_REGISTER_READ: [%d] 0x%02X\n", ioctl_register_access.page, ioctl_register_access.Addr); 	

			switch( mtv818_cb_ptr->tv_mode )
			{
				case DMB_TV_MODE_TDMB:
				case DMB_TV_MODE_FM:
					switch( ioctl_register_access.page )
					{
						case 6: reg_page = 0x06; break; // OFDM
						case 7: reg_page = 0x09; break; // FEC
						case 8: reg_page = 0x0A; break; // FEC
						default: reg_page = mtv_reg_page_addr[ioctl_register_access.page];					
					}
					break;

				case DMB_TV_MODE_1SEG:
					switch( ioctl_register_access.page )
					{
						case 6: reg_page = 0x02; break; // OFDM
						case 7: reg_page = 0x03; break; // FEC
						default: reg_page = mtv_reg_page_addr[ioctl_register_access.page];					
					}
					break;
				default: break;
			}
					
			RTV_REG_MAP_SEL(reg_page); 
			ioctl_register_access.data[0] = RTV_REG_GET(ioctl_register_access.Addr);
		/*{
			unsigned char temp_buf[3];
			RTV_REG_BURST_GET(ioctl_register_access.Addr, temp_buf, 3);
			ioctl_register_access.data[0] = temp_buf[0];

			DMBMSG("temp_buf: 0x%02X, 0x%02X, 0x%02X\n", temp_buf[0], temp_buf[1], temp_buf[2]);
		}*/
			if (copy_to_user(argp, &ioctl_register_access, sizeof(RTV_IOCTL_REGISTER_ACCESS)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}
			break;

		case IOCTL_REGISTER_WRITE:
			if(mtv818_cb_ptr->is_power_on == FALSE)
			{			
				DMBMSG("[mtv] Power Down state!Must Power ON\n");
				ret = -EFAULT;
				goto IOCTL_EXIT;
			}
			
			if (copy_from_user(&ioctl_register_access, argp, sizeof(RTV_IOCTL_REGISTER_ACCESS)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}	

			switch( mtv818_cb_ptr->tv_mode )
			{
				case DMB_TV_MODE_TDMB:
				case DMB_TV_MODE_FM:
					switch( ioctl_register_access.page )
					{
						case 6: reg_page = 0x06; break; // OFDM
						case 7: reg_page = 0x09; break; // FEC
						case 8: reg_page = 0x0A; break; // FEC
						default: reg_page = mtv_reg_page_addr[ioctl_register_access.page];					
					}
					break;

				case DMB_TV_MODE_1SEG:
					switch( ioctl_register_access.page )
					{
						case 6: reg_page = 0x02; break; // OFDM
						case 7: reg_page = 0x03; break; // FEC
						default: reg_page = mtv_reg_page_addr[ioctl_register_access.page];					
					}
					break;
				default: break;
			}
					
			RTV_REG_MAP_SEL(reg_page); 
			RTV_REG_SET(ioctl_register_access.Addr, ioctl_register_access.data[0]);
			break;

		case IOCTL_TEST_GPIO_SET:
			if (copy_from_user(&gpio_info, argp, sizeof(RTV_IOCTL_TEST_GPIO_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}	
			gpio_set_value(gpio_info.pin, gpio_info.value);
			break;

		case IOCTL_TEST_GPIO_GET:
			if (copy_from_user(&gpio_info, argp, sizeof(RTV_IOCTL_TEST_GPIO_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}	
			
			gpio_info.value = gpio_get_value(gpio_info.pin);
			if(copy_to_user(argp, &gpio_info, sizeof(RTV_IOCTL_TEST_GPIO_INFO)))
			{
				ret = -EFAULT;		
				goto IOCTL_EXIT;
			}				
			break;
		
		default:
			DMBERR("[mtv] Invalid ioctl command: %d\n", cmd);
			ret = -ENOTTY;
			break;
	}

IOCTL_EXIT:

	return ret;
}



static int mtv818_open(struct inode *inode, struct file *filp)
{
	DMBMSG("[mtv818_open] called\n");

#if defined(RTV_IF_SPI)
   #ifndef MTV818_NON_BLOCKING_READ_MODE
	if( filp->f_flags & O_NONBLOCK )
	{
		DMBERR("[mtv818_read] Must open with Blocking I/O mode\n");
		return -EAGAIN;
	}
   #endif

	mtv818_start_read_function(); /* Can be used in read() function. */

	/* Init tsp read wait-queue and etc ...*/
	init_waitqueue_head(&mtv818_cb_ptr->read_wq); // Must init before poll().. open().	
	mtv818_tsp_queue.cnt = 0;
#endif

	return 0;
}

static int mtv818_close(struct inode *inode, struct file *filp)
{
//	int ret;

	DMBMSG("\n[mtv818_close] called\n");

	mtv818_power_off();

#if defined(RTV_IF_SPI)	
	mtv818_exit_read_function();
#endif

	DMBMSG("[mtv818_close] END\n");

	return 0;
}


#if defined(RTV_IF_SPI)
// Wakeup to user poll() function
static unsigned int	mtv818_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	/* To release the poll() of application as possible as soon, we use a queue count only. */
	poll_wait(file, &mtv818_cb_ptr->read_wq, wait);

	if(mtv818_cb_ptr->stop)
	{
		mask |= POLLHUP;
	}

	if(mtv818_tsp_queue.cnt)
	{
		mask |= (POLLIN | POLLRDNORM);
	}

	return mask;
}
#endif

static struct file_operations mtv818_fops =
{
    .owner = THIS_MODULE,
    .open = mtv818_open,
    .ioctl = mtv818_ioctl,
    .read = mtv818_read,
#if defined(RTV_IF_SPI)    
    .poll = mtv818_poll,
#endif    
    .release = mtv818_close,
};

static struct miscdevice mtv818_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = RAONTV_DEV_NAME,
    .fops = &mtv818_fops,
};


static const char *build_data = __DATE__;
static const char *build_time = __TIME__;


static int __init mtv818_module_init(void)
{
    	int ret;

	DMBMSG("\t==========================================================\n");
	DMBMSG("\t %s Module Build Date/Time: %s, %s\n", mtv818_misc_device.name, build_data, build_time);
	DMBMSG("\t==========================================================\n\n");
	
       ret = mtv818_init_device();
	if(ret<0)
		return ret;

	/* misc device registration */
	ret = misc_register(&mtv818_misc_device);
	if( ret )
	{
		DMBERR("[mtv818_module_init] misc_register() failed! : %d", ret);
		return ret; 	  	
	}

	return 0;
}


static void __exit mtv818_module_exit(void)
{
	mtv818_deinit_device();
	
	misc_deregister(&mtv818_misc_device);
}

module_init(mtv818_module_init);
module_exit(mtv818_module_exit);
MODULE_DESCRIPTION("MTV818 driver");
MODULE_LICENSE("GPL");

