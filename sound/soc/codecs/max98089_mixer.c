/*
 * max98088.c -- MAX98088 ALSA SoC Audio driver
 *
 * Copyright 2010 Maxim Integrated Products
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/map-base.h>
#include <mach/regs-clock.h>

#include <sound/max98088.h>
#include "max98088.h"

//----------------------------------------------------------------------------------------
// audio gain parameter
//----------------------------------------------------------------------------------------
#define   TUNNING_DAI1_VOL    	0x00 // 0dB

#define   TUNNING_SPKMIX_VOL    0x00 // 0dB

#define   TUNNING_SPK_VOL    	0x1B // +6dB
#define   TUNNING_HP_VOL     	0x18 // +3dB

#define 	MIC_PRE_AMP_GAIN	(2<<5) // +20dB
#define 	MIC_PGA_GAIN		(0x0A) // +10dB
#define   	TUNNING_MIC_PGA_GAIN    MIC_PRE_AMP_GAIN

#define 	MIC_ADC_GAIN		(3<<4) // +18dB
#define 	MIC_ADC_VOLUME		(0x03) // 0dB
#define   	TUNNING_ADC_GAIN    MIC_ADC_VOLUME

//----------------------------------------------------------------------------------------
// playback function
//----------------------------------------------------------------------------------------
void max98089_set_playback_speaker(struct snd_soc_codec *codec)
{
	int reg;

	// 0x4D SPK L/R, DAC L/R enable
	reg = snd_soc_read(codec, M98088_REG_4D_PWR_EN_OUT);
	reg &= ~(M98088_HPLEN | M98088_HPREN | M98088_SPLEN | M98088_SPREN | M98088_DALEN | M98088_DAREN |M98088_RECLEN |M98088_RECREN);
	reg |= (M98088_SPLEN | M98088_SPREN | M98088_DALEN | M98088_DAREN);
	snd_soc_write(codec, M98088_REG_4D_PWR_EN_OUT, reg);

	// 0x22 DAI1 TO DAC1 enable
	reg = snd_soc_read(codec, M98088_REG_22_MIX_DAC);
	reg &= ~(M98088_DAI1L_TO_DACL|M98088_DAI2L_TO_DACL|M98088_DAI1R_TO_DACR|M98088_DAI2R_TO_DACR);
	reg |= (M98088_DAI1L_TO_DACL | M98088_DAI1R_TO_DACR);
	snd_soc_write(codec, M98088_REG_22_MIX_DAC, reg);

	// 0x2B DAC1_L TO SPK_MIX_L
	reg = snd_soc_read(codec, M98088_REG_2B_MIX_SPK_LEFT);
	reg |= M98088_DACL_TO_SPK_MIXL;
	snd_soc_write(codec, M98088_REG_2B_MIX_SPK_LEFT, reg);

	// 0x2B DAC1_R TO SPK_MIX_R
	reg = snd_soc_read(codec, M98088_REG_2C_MIX_SPK_RIGHT);
	reg |= M98088_DACR_TO_SPK_MIXR;
	snd_soc_write(codec, M98088_REG_2C_MIX_SPK_RIGHT, reg);

	/* digital domain unmute */
	reg = TUNNING_DAI1_VOL;
	snd_soc_write(codec, M98088_REG_2F_LVL_DAI1_PLAY, reg);

	/* analog domain unmute */
	// 0x2D SPK_MIX_GAIN 
	reg = TUNNING_SPKMIX_VOL;
	snd_soc_write(codec, M98088_REG_2D_MIX_SPK_CNTL, reg);

	// 0x3D,0x3E SPK OUT L/R VOL Unmute, 0dB
	reg = TUNNING_SPK_VOL;
	snd_soc_write(codec, M98088_REG_3D_LVL_SPK_L, reg);
	snd_soc_write(codec, M98088_REG_3E_LVL_SPK_R, reg);

	printk("\t[MAX98089] %s(%d)\n",__FUNCTION__,__LINE__);
	return;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void max98089_set_playback_headset(struct snd_soc_codec *codec)
{
	int reg;

	// 0x4D SPK L/R, DAC L/R enable
	reg = snd_soc_read(codec, M98088_REG_4D_PWR_EN_OUT);
	reg &= ~(M98088_HPLEN | M98088_HPREN | M98088_SPLEN | M98088_SPREN | M98088_DALEN | M98088_DAREN |M98088_RECLEN |M98088_RECREN);
	reg |= (M98088_HPLEN | M98088_HPREN | M98088_DALEN | M98088_DAREN);
	snd_soc_write(codec, M98088_REG_4D_PWR_EN_OUT, reg);

	// 0x22 DAI1 TO DAC1 enable
	reg = snd_soc_read(codec, M98088_REG_22_MIX_DAC);
	reg &= ~(M98088_DAI1L_TO_DACL|M98088_DAI2L_TO_DACL|M98088_DAI1R_TO_DACR|M98088_DAI2R_TO_DACR);
	reg |= (M98088_DAI1L_TO_DACL | M98088_DAI1R_TO_DACR);
	snd_soc_write(codec, M98088_REG_22_MIX_DAC, reg);

	// 0x25 DAC1_L TO HP_MIX_L
	reg = snd_soc_read(codec, M98088_REG_25_MIX_HP_LEFT);
	reg |= M98088_DACL_TO_HP_MIXL;
	snd_soc_write(codec, M98088_REG_25_MIX_HP_LEFT, reg);

	// 0x26 DAC1_R TO HP_MIX_R
	reg = snd_soc_read(codec, M98088_REG_26_MIX_HP_RIGHT);
	reg |= M98088_DACR_TO_HP_MIXR;
	snd_soc_write(codec, M98088_REG_26_MIX_HP_RIGHT, reg);
	
	// HP_MIX TO HP_PGA, 0dB
	snd_soc_write(codec, M98088_REG_27_MIX_HP_CNTL, 0x30);

	/* digital domain unmute */
	reg = TUNNING_DAI1_VOL;
	snd_soc_write(codec, M98088_REG_2F_LVL_DAI1_PLAY, reg);

	/* analog domain unmute */
	// 0x39,0x3A HP OUT L/R VOL Unmute, 0dB
	reg = TUNNING_HP_VOL;
	snd_soc_write(codec, M98088_REG_39_LVL_HP_L, reg);
	snd_soc_write(codec, M98088_REG_3A_LVL_HP_R, reg);

	printk("\t[MAX98089] %s(%d)\n",__FUNCTION__,__LINE__);
	return;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void max98089_set_playback_speaker_headset(struct snd_soc_codec *codec)
{
	int reg;

	// 0x4D SPK L/R, DAC L/R enable
	reg = snd_soc_read(codec, M98088_REG_4D_PWR_EN_OUT);
	reg |= (M98088_HPLEN | M98088_HPREN | M98088_SPLEN | M98088_SPREN | M98088_DALEN | M98088_DAREN);
	snd_soc_write(codec, M98088_REG_4D_PWR_EN_OUT, reg);

	// 0x22 DAI1 TO DAC1 enable
	reg = snd_soc_read(codec, M98088_REG_22_MIX_DAC);
	reg &= ~(M98088_DAI1L_TO_DACL|M98088_DAI2L_TO_DACL|M98088_DAI1R_TO_DACR|M98088_DAI2R_TO_DACR);
	reg |= (M98088_DAI1L_TO_DACL | M98088_DAI1R_TO_DACR);
	snd_soc_write(codec, M98088_REG_22_MIX_DAC, reg);

	snd_soc_write(codec, M98088_REG_4F_DAC_BIAS1, 0x03);

	// 0x2B DAC1_L TO SPK_MIX_L
	reg = M98088_DACL_TO_SPK_MIXL;
	snd_soc_write(codec, M98088_REG_2B_MIX_SPK_LEFT, reg);

	// 0x2B DAC1_R TO SPK_MIX_R
	reg = M98088_DACR_TO_SPK_MIXR;
	snd_soc_write(codec, M98088_REG_2C_MIX_SPK_RIGHT, reg);

	// 0x2B DAC1_L TO SPK_MIX_L
	reg = M98088_DACL_TO_HP_MIXL;
	snd_soc_write(codec, M98088_REG_25_MIX_HP_LEFT, reg);

	// 0x2B DAC1_R TO SPK_MIX_R
	reg = M98088_DACR_TO_HP_MIXR;
	snd_soc_write(codec, M98088_REG_26_MIX_HP_RIGHT, reg);
	
	// HP_MIX TO HP_PGA, 0dB
	snd_soc_write(codec, M98088_REG_27_MIX_HP_CNTL, 0x30);

	/* digital domain unmute */
	reg = TUNNING_DAI1_VOL;
	snd_soc_write(codec, M98088_REG_2F_LVL_DAI1_PLAY, reg);

	/* analog domain unmute */
	// 0x2D SPK_MIX_GAIN 
	reg = TUNNING_SPKMIX_VOL;
	snd_soc_write(codec, M98088_REG_2D_MIX_SPK_CNTL, reg);

	// 0x3D,0x3E SPK OUT L/R VOL Unmute, 0dB
	reg = TUNNING_SPK_VOL;
	snd_soc_write(codec, M98088_REG_3D_LVL_SPK_L, reg);
	snd_soc_write(codec, M98088_REG_3E_LVL_SPK_R, reg);

	// 0x39,0x3A HP OUT L/R VOL Unmute, 0dB
	reg = TUNNING_HP_VOL;
	snd_soc_write(codec, M98088_REG_39_LVL_HP_L, reg);
	snd_soc_write(codec, M98088_REG_3A_LVL_HP_R, reg);

	printk("\t[MAX98089] %s(%d)\n",__FUNCTION__,__LINE__);

	return;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void max98089_disable_playback_path(struct snd_soc_codec *codec, enum playback_path path)
{
	int reg;
	
	reg = snd_soc_read(codec, M98088_REG_4D_PWR_EN_OUT);

	switch(path) {
		case REV : 
		case HP : 
				reg &= ~(M98088_HPLEN | M98088_HPREN);
			break;
		case SPK : 
				reg &= ~(M98088_SPLEN | M98088_SPREN);
			break;
		case SPK_HP :
				reg &= ~(M98088_HPLEN | M98088_HPREN | M98088_SPLEN | M98088_SPREN);
			break;
		case TV_OUT :
				reg &= ~(M98088_HPLEN | M98088_HPREN | M98088_SPLEN | M98088_SPREN);
			break;
		default :
			break;
	}
	snd_soc_write(codec, M98088_REG_4D_PWR_EN_OUT, reg);

	return;
}

//----------------------------------------------------------------------------------------
// recording function
//----------------------------------------------------------------------------------------
void max98089_set_record_main_mic(struct snd_soc_codec *codec)
{
	int reg;

	// 0x4C Mic bias, ADC L enable
	reg = snd_soc_read(codec, M98088_REG_4C_PWR_EN_IN);
	reg &= ~(M98088_ADLEN | M98088_ADREN | M98088_MBEN | M98088_INAEN | M98088_INBEN);
	reg |= (M98088_ADLEN | M98088_ADREN | M98088_MBEN);
	snd_soc_write(codec, M98088_REG_4C_PWR_EN_IN, reg);

	reg = 0x00; // DMIC disable, EXTMIC disable
	snd_soc_write(codec, M98088_REG_48_CFG_MIC, reg);

	reg = 0x00; // MIC2 Bypass disable
	snd_soc_write(codec, M98088_REG_4A_CFG_BYPASS, reg);

	reg = (1<<6); // MIC2 to ADC L/R Mixer
	snd_soc_write(codec, M98088_REG_23_MIX_ADC_LEFT, reg);
	snd_soc_write(codec, M98088_REG_24_MIX_ADC_RIGHT, reg);

	reg = 0x00; // AGC disable
	snd_soc_write(codec, M98088_REG_3F_MICAGC_CFG, reg);
	reg = 0x00; // Noisegate disable
	snd_soc_write(codec, M98088_REG_40_MICAGC_THRESH, reg);

	reg = TUNNING_MIC_PGA_GAIN;
	snd_soc_write(codec, M98088_REG_35_LVL_MIC1, reg);
	snd_soc_write(codec, M98088_REG_36_LVL_MIC2, reg);

	reg = TUNNING_ADC_GAIN;
	snd_soc_write(codec, M98088_REG_33_LVL_ADC_L, reg);
	snd_soc_write(codec, M98088_REG_34_LVL_ADC_R, reg);

	printk("\t[MAX98089] %s(%d)\n",__FUNCTION__,__LINE__);
	return;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void max98089_set_record_headset_mic(struct snd_soc_codec *codec)
{
	printk("\t[MAX98089] %s(%d)\n",__FUNCTION__,__LINE__);
	return;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void max98089_disable_record_path(struct snd_soc_codec *codec, enum record_path rec_path)
{
	int reg;

	printk("\t[MAX98089] %s(%d)- playback path(%d)\n",__FUNCTION__,__LINE__, rec_path);

	switch(rec_path)
	{
		case MAIN:
			break;
		case EAR:
			break;
		default :
			break;
	}

	reg = snd_soc_read(codec, M98088_REG_4C_PWR_EN_IN);
	reg &= ~(M98088_MBEN);
	snd_soc_write(codec, M98088_REG_4C_PWR_EN_IN, reg);

	reg = snd_soc_read(codec, M98088_REG_25_MIX_HP_LEFT);
	reg &= ~(1<<6); //MIC2 TEST
	snd_soc_write(codec, M98088_REG_25_MIX_HP_LEFT, reg);

	reg = snd_soc_read(codec, M98088_REG_26_MIX_HP_RIGHT);
	reg &= ~(1<<6); //MIC2 TEST
	snd_soc_write(codec, M98088_REG_26_MIX_HP_RIGHT, reg);

	return;	
}
