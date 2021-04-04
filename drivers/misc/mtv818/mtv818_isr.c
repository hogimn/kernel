#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>


#include "./src/raontv.h"
#include "./src/raontv_internal.h"

#include "mtv818.h"


#if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE))	

#if defined(RTV_TDMB_ENABLE)	
  #ifndef RTV_TDMB_FIC_POLLING_MODE
	U8 fic_buf[384+1];
  #endif
#endif


int isr_cnt = 0;
int isr_handler =0;


void mtv818_isr_handler(void)
{
	U8 int_type_val1;
#if defined(RTV_IF_SPI)
   #if defined(RTV_TDMB_ENABLE) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /* CIF mode */
	UINT msc_total;    
	U8 msc_tsize[2+1];	
   #endif

	UINT nExistMscMask = 0;
	MTV818_TS_PKT_INFO *tsp = NULL;
#endif

	RTV_GUARD_LOCK;

	RTV_MASTER_CHIP_SEL;

reProc:

//DMBMSG("[ISR Handler %d]\n", isr_handler++);
	RTV_REG_MAP_SEL(DD_PAGE);
	int_type_val1 = RTV_REG_GET(INT_E_STATL);

//if((isr_cnt++ %1000) == 0)	
	//DMBMSG("[mtv isr] 0x%02X\n", int_type_val1);
	
#if defined(RTV_IF_SPI) 
	/*==============================================================================
	 * Processing MSC1 interrupt.
	 * T-DMB: 1 VIDEO data or 1 DATA/AUDIO data
	 * 1 seg: 1 VIDEO data
	 * FM: PCM data
	 *============================================================================*/  
	if( int_type_val1 & (MSC1_E_INT|MSC1_E_OVER_FLOW|MSC1_E_UNDER_FLOW) )
	{
		if( int_type_val1 & (MSC1_E_OVER_FLOW|MSC1_E_UNDER_FLOW) )   // MSC1 memory overflow or under run
		{
			DMBMSG("[mtv isr] OF/UF: 0x%02X\n", int_type_val1);
			
			RTV_REG_SET(MSC1_E_CON, 0x00);  // MSC1 memory control register clear.
			//RTV_REG_SET(MSC1_E_CON, 0x0D);  // MSC1 memory enable.
			RTV_REG_SET(MSC1_E_CON, 0x05);  // MSC1 memory enable.
			RTV_REG_SET(INT_E_UCLRL, 0x04); // MSC1 Interrupt status clear.
		}
		else
		{	
			nExistMscMask |= 0x01;

			/* Allocate a TS packet from TSP pool. */
			tsp = mtv818_alloc_tsp();
			if(tsp != NULL)
			{
		#ifndef RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE
				RTV_REG_MAP_SEL(MSC1_PAGE);
				RTV_REG_BURST_GET(0x10, tsp->msc_buf, MTV_TS_THRESHOLD_SIZE+1); 
				tsp->len = MTV_TS_THRESHOLD_SIZE;

			//	printk("[0x%02X], [0x%02X], [0x%02X], [0x%02X]\n", tsp->msc_buf[1], tsp->msc_buf[2], tsp->msc_buf[3],tsp->msc_buf[4]);			

		#else
				if(mtv818_cb_ptr->tv_mode != DMB_TV_MODE_TDMB)
				{
					RTV_REG_MAP_SEL(MSC1_PAGE);
					RTV_REG_BURST_GET(0x10, tsp->msc_buf, MTV_TS_THRESHOLD_SIZE+1); 
					tsp->len = MTV_TS_THRESHOLD_SIZE;
				}
				else
				{	/* CIF Mode */
					RTV_REG_MAP_SEL(DD_PAGE);
					RTV_REG_BURST_GET(MSC1_E_TSIZE_H, msc_tsize, 2+1);
					msc_total = (msc_tsize[1] << 8) | msc_tsize[2]; 		
					 
					RTV_REG_MAP_SEL(MSC1_PAGE);
					RTV_REG_BURST_GET(0x10, tsp->msc_buf, msc_total+1); 
					tsp->len = msc_total;										
				}
		#endif
				RTV_REG_MAP_SEL(DD_PAGE);
				RTV_REG_SET(INT_E_UCLRL, 0x04); // MSC1 Interrupt status clear.

				/* Enqueue a ts packet into ts data queue. */
				mtv818_put_tsp(tsp);	
			}
			else
			{
				RTV_REG_SET(MSC1_E_CON, 0x00);	// MSC1 memory control register clear.
				//RTV_REG_SET(MSC1_E_CON, 0x0D);	// MSC1 memory enable.
				RTV_REG_SET(MSC1_E_CON, 0x05);	// MSC1 memory enable.
				RTV_REG_SET(INT_E_UCLRL, 0x04); // MSC1 Interrupt status clear.
		
				DMBERR("[mtv818_isr_handler] No more TSP buffer\n");
			}
		}
	}
#endif /* #if defined(RTV_IF_SPI) */

	/*==============================================================================
	 * Processing FIC and FEC RE_CONFIG interrupts.
	 * T-DMB: 1 FIC data
	 *============================================================================*/  
#if defined(RTV_TDMB_ENABLE)	&& !defined(RTV_TDMB_FIC_POLLING_MODE) 
	if(int_type_val1 & FIC_E_INT) // FIC interrupt
	{
	    // DMBMSG("FIC_E_INT occured!\n");
		RTV_REG_MAP_SEL(FIC_PAGE);

	#if defined(RTV_IF_MPEG2_SERIAL_TSIF) || defined(RTV_IF_SPI_SLAVE) || defined(RTV_IF_QUALCOMM_TSIF) || defined(RTV_IF_MPEG2_PARALLEL_TSIF)
		RTV_REG_BURST_GET(0x10, fic_buf, 192);			
		RTV_REG_BURST_GET(0x10, fic_buf+192, 192);	

	#elif defined(RTV_IF_SPI) 
		RTV_REG_BURST_GET(0x10, fic_buf, 384+1);
	#endif	

		RTV_REG_MAP_SEL(DD_PAGE);
		RTV_REG_SET(INT_E_UCLRL, 0x01); // FIC interrupt status clear
	}   
#endif	

	/*==============================================================================
	 * Processing MSC0 interrupt for multi sub channel mode (CIF Mode).
	 * T-DMB: Max 4 AUDIO/DATA data
	 *============================================================================*/  
#if defined(RTV_IF_SPI) 
   #if defined(RTV_TDMB_ENABLE) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) /* CIF Mode */	 
	if( int_type_val1 & (MSC0_E_INT|MSC0_E_OVER_FLOW|MSC0_E_UNDER_FLOW) )
	{
		if( int_type_val1 & (MSC0_E_OVER_FLOW|MSC0_E_UNDER_FLOW) ) // MSC0 memory overflow or under run
		{
			RTV_REG_MAP_SEL(DD_PAGE);
			RTV_REG_SET(MSC0_E_CON, 0x00);	// MSC0 memory control register clear.
			RTV_REG_SET(MSC0_E_CON, 0x0C);	// MSC0 memory enable.
			RTV_REG_SET(INT_E_UCLRL, 0x02); // MSC0 Interrupt clear.
		}
		else
		{		
			nExistMscMask |= 0x02;

			if(tsp == NULL)
			{
				/* Allocate a TS packet from TSP pool. */
				tsp = mtv818_alloc_tsp();
				if(tsp == NULL) /* */
				{
					RTV_REG_MAP_SEL(DD_PAGE);
					RTV_REG_SET(MSC0_E_CON, 0x00);	// MSC0 memory control register clear.
					RTV_REG_SET(MSC0_E_CON, 0x0C);	// MSC0 memory enable.
					RTV_REG_SET(INT_E_UCLRL, 0x02); // MSC0 Interrupt clear.
				
					DMBERR("[mtv818_isr_handler] No more TSP buffer for MSC 0\n");
				}					
			}
			else
			{
				RTV_REG_MAP_SEL(DD_PAGE);
				RTV_REG_BURST_GET(MSC0_E_TSIZE_H, msc_tsize, 2+1);
				msc_total = (msc_tsize[1] << 8) | msc_tsize[2]; 		

				RTV_REG_MAP_SEL(MSC0_PAGE);
				RTV_REG_BURST_GET(0x10, (tsp->msc_buf+tsp->len), msc_total+1); 
				tsp->len += msc_total;	// Atfer MSC1 if exist.									
			
				RTV_REG_MAP_SEL(DD_PAGE);
				RTV_REG_SET(INT_E_UCLRL,0x02); // MSC0 Interrupt clear. 		
			}
		}
	}
   #endif /* #if defined(RTV_TDMB_ENABLE) && defined(RTV_TDMB_MULTI_SUB_CHANNEL_ENABLE) */

	if(tsp != NULL)
	{
    #ifndef MTV818_NON_BLOCKING_READ_MODE
		wake_up(&mtv818_cb_ptr->read_wq);			
    #endif
	}

	if(nExistMscMask != 0)
	{
		nExistMscMask = 0;
		goto reProc;
	}

#endif /* #if defined(RTV_IF_SPI)  */

	RTV_GUARD_FREE;
}


int mtv818_isr_thread(void *data)
{
	DMBMSG("[mtv818_isr_thread] Start...\n");

	while(!kthread_should_stop()) 
	{
		wait_event_interruptible(mtv818_cb_ptr->isr_wq,
				  			     kthread_should_stop() || mtv818_cb_ptr->isr_cnt);

		if(kthread_should_stop())
			break;

		mtv818_isr_handler();

		mtv818_cb_ptr->isr_cnt--;
	}

	DMBMSG("[mtv818_isr_thread] Exit.\n");

	return 0;
}


irqreturn_t mtv818_isr(int irq, void *param)
{
	//DMBMSG("[ISR %d]\n", ++isr_cnt);

	mtv818_cb_ptr->isr_cnt++;
	wake_up(&mtv818_cb_ptr->isr_wq);

	return IRQ_HANDLED;
}

#endif /* #if defined(RTV_IF_SPI) || (defined(RTV_TDMB_ENABLE) && !defined(RTV_TDMB_FIC_POLLING_MODE)) */

