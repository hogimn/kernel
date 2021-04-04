/*
 *  sound/soc/s3c24xx/smdk_wm8580slv.c
 *
 *  Copyright (c) 2009 Samsung Electronics Co. Ltd
 *
 *  This program is free software; you can redistribute  it and/or  modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include <mach/regs-clock.h>
#include <mach/map.h>
#include <mach/regs-audss.h>

#include "../codecs/wm8580.h"
#include "s3c-dma.h"
#include "s3c64xx-i2s.h"
#include "s3c-dma-wrapper.h"


static int set_epll_rate(unsigned long rate)
{
	struct clk *fout_epll;

	fout_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(fout_epll)) {
		printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
		return -ENOENT;
	}

	if (rate == clk_get_rate(fout_epll))
		goto out;

	clk_set_rate(fout_epll, rate);
out:
	clk_put(fout_epll);

	return 0;
}

static int smdk_wm8580_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;

        unsigned int rclk, psr=1;
        int bfs, rfs, ret;
        unsigned long epll_out_rate;
        unsigned int  value = 0;
        unsigned int bit_per_sample, sample_rate;
	unsigned int pll_out;

        switch (params_format(params)) {
        case SNDRV_PCM_FORMAT_U24:
        case SNDRV_PCM_FORMAT_S24:
                bfs = 48;
                bit_per_sample = 24;
                break;
        case SNDRV_PCM_FORMAT_U16_LE:
        case SNDRV_PCM_FORMAT_S16_LE:
                bfs = 32;
                bit_per_sample = 16;
                break;
        default:
                return -EINVAL;
        }

        /* For resample */
        value = params_rate(params);
//      if(value != 44100) {
//              printk("%s: sample rate %d --> 44100\n", __func__, value);
//              value = 44100;
//      }
        sample_rate = value;

        /* The Fvco for WM8580 PLLs must fall within [90,100]MHz.
         * This criterion can't be met if we request PLL output
         * as {8000x256, 64000x256, 11025x256}Hz.
         * As a wayout, we rather change rfs to a minimum value that
         * results in (params_rate(params) * rfs), and itself, acceptable
         * to both - the CODEC and the CPU.
         */
        switch (value) {
        case 16000:
        case 22050:
        case 24000:
        case 32000:
        case 44100:
        case 48000:
        case 88200:
        case 96000:
                if (bfs == 48)
                        rfs = 384;
                else
                        rfs = 256;
                break;
        case 64000:
                rfs = 384;
                break;
        case 8000:
        case 11025:
        case 12000:
                if (bfs == 48)
                        rfs = 768;
                else
                        rfs = 512;
                break;
        default:
                return -EINVAL;
        }

        rclk = value * rfs;

        switch (rclk) {
        case 4096000:
        case 5644800:
        case 6144000:
        case 8467200:
        case 9216000:
                psr = 8;
                break;
        case 8192000:
        case 11289600:
        case 12288000:
        case 16934400:
        case 18432000:
                psr = 4;
                break;
        case 22579200:
        case 24576000:
        case 33868800:
        case 36864000:
                psr = 2;
                break;
        case 67737600:
        case 73728000:
                psr = 1;
                break;
        default:
                printk("Not yet supported!\n");
                return -EINVAL;
        }

        printk("IIS Audio: %uBits Stereo %uHz\n", bit_per_sample, sample_rate);

#ifdef CONFIG_SND_S5P_RP
        /* Find fastest clock for MP3 decoding & post-effect processing.
           Maximux clock is 192MHz for AudioSubsystem.
         */
        for (epll_out_rate = 0; ; psr++) {
                epll_out_rate = rclk * psr;
                if (epll_out_rate > 192000000) {
                        psr--;
                        epll_out_rate = rclk * psr;
                        break;
                }
        }
#else
        epll_out_rate = rclk * psr;
#endif

        /* Set EPLL clock rate */
        ret = set_epll_rate(epll_out_rate);
        if (ret < 0)
                return ret;

        /* Set the Codec DAI configuration */
        ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS);
        if (ret < 0)
                return ret;

        /* Set the AP DAI configuration */
        ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS);
        if (ret < 0)
                return ret;

#if 0 // JNJ WM8580 codec driver does not support set_sysclk()
        ret = snd_soc_dai_set_sysclk(codec_dai, 0, rclk, SND_SOC_CLOCK_IN);
        if (ret < 0)
                return ret;
#endif

        ret = snd_soc_dai_set_sysclk(cpu_dai, S3C64XX_CLKSRC_CDCLK,
                                        0, SND_SOC_CLOCK_OUT);
        if (ret < 0)
                return ret;

        /* We use MUX for basic ops in SoC-Master mode */
        ret = snd_soc_dai_set_sysclk(cpu_dai, S3C64XX_CLKSRC_MUX,
                                        0, SND_SOC_CLOCK_IN);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_PRESCALER, psr-1);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_BCLK, bfs);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C_I2SV2_DIV_RCLK, rfs);
        if (ret < 0)
                return ret;

	return 0;
}

/* SMDK64xx Playback widgets */
static const struct snd_soc_dapm_widget wm8580_dapm_widgets_pbk[] = {
	SND_SOC_DAPM_HP("Front-L/R", NULL),
	SND_SOC_DAPM_HP("Center/Sub", NULL),
	SND_SOC_DAPM_HP("Rear-L/R", NULL),
};

/* SMDK64xx Capture widgets */
static const struct snd_soc_dapm_widget wm8580_dapm_widgets_cpt[] = {
	SND_SOC_DAPM_MIC("MicIn", NULL),
	SND_SOC_DAPM_LINE("LineIn", NULL),
};

/* SMDK-PAIFTX connections */
static const struct snd_soc_dapm_route audio_map_tx[] = {
	/* MicIn feeds AINL */
	{"AINL", NULL, "MicIn"},

	/* LineIn feeds AINL/R */
	{"AINL", NULL, "LineIn"},
	{"AINR", NULL, "LineIn"},
};

/* SMDK-PAIFRX connections */
static const struct snd_soc_dapm_route audio_map_rx[] = {
	/* Front Left/Right are fed VOUT1L/R */
	{"Front-L/R", NULL, "VOUT1L"},
	{"Front-L/R", NULL, "VOUT1R"},

	/* Center/Sub are fed VOUT2L/R */
	{"Center/Sub", NULL, "VOUT2L"},
	{"Center/Sub", NULL, "VOUT2R"},

	/* Rear Left/Right are fed VOUT3L/R */
	{"Rear-L/R", NULL, "VOUT3L"},
	{"Rear-L/R", NULL, "VOUT3R"},
};

static int smdk_wm8580_init_paiftx(struct snd_soc_codec *codec)
{
	int ret;

	/* Add smdk64xx specific Capture widgets */
	snd_soc_dapm_new_controls(codec, wm8580_dapm_widgets_cpt,
				  ARRAY_SIZE(wm8580_dapm_widgets_cpt));

	/* Set up PAIFTX audio path */
	snd_soc_dapm_add_routes(codec, audio_map_tx, ARRAY_SIZE(audio_map_tx));

	/* Enabling the microphone requires the fitting of a 0R
	 * resistor to connect the line from the microphone jack.
	 */
	snd_soc_dapm_disable_pin(codec, "MicIn");

	/* signal a DAPM event */
	snd_soc_dapm_sync(codec);

	/* Set the Codec DAI configuration */
	ret = snd_soc_dai_set_fmt(&wm8580_dai[WM8580_DAI_PAIFTX],
					SND_SOC_DAIFMT_I2S
					 | SND_SOC_DAIFMT_NB_NF
					 | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* Set the AP DAI configuration */
	ret = snd_soc_dai_set_fmt(&s3c64xx_i2s_v4_dai,
					SND_SOC_DAIFMT_I2S
					 | SND_SOC_DAIFMT_NB_NF
					 | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* Set WM8580 to drive MCLK from its MCLK-pin */
	ret = snd_soc_dai_set_clkdiv(&wm8580_dai[WM8580_DAI_PAIFTX],
					WM8580_MCLK, WM8580_CLKSRC_MCLK);
	if (ret < 0)
		return ret;

	/* Explicitly set WM8580-ADC to source from MCLK */
	ret = snd_soc_dai_set_clkdiv(&wm8580_dai[WM8580_DAI_PAIFTX],
				WM8580_ADC_CLKSEL, WM8580_CLKSRC_ADCMCLK);
	if (ret < 0)
		return ret;

	return 0;
}

static int smdk_wm8580_init_paifrx(struct snd_soc_codec *codec)
{
	int ret;

	/* Add smdk64xx specific Playback widgets */
	snd_soc_dapm_new_controls(codec, wm8580_dapm_widgets_pbk,
				  ARRAY_SIZE(wm8580_dapm_widgets_pbk));

	/* Set up PAIFRX audio path */
	snd_soc_dapm_add_routes(codec, audio_map_rx, ARRAY_SIZE(audio_map_rx));

	/* signal a DAPM event */
	snd_soc_dapm_sync(codec);

	/* Set the Codec DAI configuration */
	ret = snd_soc_dai_set_fmt(&wm8580_dai[WM8580_DAI_PAIFRX],
					SND_SOC_DAIFMT_I2S
					 | SND_SOC_DAIFMT_NB_NF
					 | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* Set the AP DAI configuration */
	ret = snd_soc_dai_set_fmt(&s3c64xx_i2s_v4_dai,
					SND_SOC_DAIFMT_I2S
					 | SND_SOC_DAIFMT_NB_NF
					 | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* Set WM8580 to drive MCLK from its MCLK-pin */
	ret = snd_soc_dai_set_clkdiv(&wm8580_dai[WM8580_DAI_PAIFRX],
					WM8580_MCLK, WM8580_CLKSRC_MCLK);
	if (ret < 0)
		return ret;

	/* Explicitly set WM8580-DAC to source from MCLK */
	ret = snd_soc_dai_set_clkdiv(&wm8580_dai[WM8580_DAI_PAIFRX],
				WM8580_DAC_CLKSEL, WM8580_CLKSRC_MCLK);
	if (ret < 0)
		return ret;

	return 0;
}
static struct snd_soc_ops smdk_wm8580_ops = {
	.hw_params = smdk_wm8580_hw_params,
};

static struct snd_soc_dai_link smdk_dai[] = {
{
	.name = "WM8580 PAIF RX ",
	.stream_name = "Playback",
	.cpu_dai = &s3c64xx_i2s_v4_dai,
	.codec_dai = &wm8580_dai[WM8580_DAI_PAIFRX],
	.init = smdk_wm8580_init_paifrx,
	.ops = &smdk_wm8580_ops,
},
{
	.name = "WM8580 PAIF TX ",
	.stream_name = "Capture",
	.cpu_dai = &s3c64xx_i2s_v4_dai,
	.codec_dai = &wm8580_dai[WM8580_DAI_PAIFTX],
	.init = smdk_wm8580_init_paiftx,
	.ops = &smdk_wm8580_ops,
},
};

static struct snd_soc_card smdk = {
	.name = "smdk",
	.platform = &s3c_dma_wrapper,
	.dai_link = smdk_dai,
	.num_links = ARRAY_SIZE(smdk_dai),
};

static struct snd_soc_device smdk_snd_devdata = {
	.card = &smdk,
	.codec_dev = &soc_codec_dev_wm8580,
};

static struct platform_device *smdk_snd_device;

static int __init smdk_audio_init(void)
{
	int ret;

//#ifdef CONFIG_BOARD_SM5S4210
#if 0
        u32 val;

        /* We use I2SCLK for rate generation, so set EPLLout as
         * the parent of I2SCLK.
         */
        val = readl(S5P_CLKSRC_AUDSS);
        val &= ~(0x3<<2);
        val |= (1<<0);
        writel(val, S5P_CLKSRC_AUDSS);

        val = readl(S5P_CLKGATE_AUDSS);
        val |= (0x7f<<0);
        writel(val, S5P_CLKGATE_AUDSS);
#endif

	smdk_snd_device = platform_device_alloc("soc-audio", -1);
	if (!smdk_snd_device)
		return -ENOMEM;

	platform_set_drvdata(smdk_snd_device, &smdk_snd_devdata);
	smdk_snd_devdata.dev = &smdk_snd_device->dev;

	ret = platform_device_add(smdk_snd_device);
	if (ret)
		platform_device_put(smdk_snd_device);

//#ifdef CONFIG_BOARD_SM5S4210
#if 0
        set_epll_rate(11289600*4);

        /* Set the Codec DAI configuration */
        ret = snd_soc_dai_set_fmt(&wm8580_dai, SND_SOC_DAIFMT_I2S
                                         | SND_SOC_DAIFMT_NB_NF
                                         | SND_SOC_DAIFMT_CBS_CFS);
        if (ret < 0)
                return ret;
        /* Set the AP DAI configuration */
        ret = snd_soc_dai_set_fmt(&s3c64xx_i2s_v4_dai, SND_SOC_DAIFMT_I2S
                                         | SND_SOC_DAIFMT_NB_NF
                                         | SND_SOC_DAIFMT_CBS_CFS);


        ret = snd_soc_dai_set_sysclk(&wm8580_dai, 0, 11289600, SND_SOC_CLOCK_IN);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_sysclk(&s3c64xx_i2s_v4_dai, S3C64XX_CLKSRC_CDCLK,
                                        0, SND_SOC_CLOCK_OUT);
        if (ret < 0)
                return ret;

        /* We use MUX for basic ops in SoC-Master mode */
        ret = snd_soc_dai_set_sysclk(&s3c64xx_i2s_v4_dai, S3C64XX_CLKSRC_MUX,
                                        0, SND_SOC_CLOCK_IN);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_clkdiv(&s3c64xx_i2s_v4_dai, S3C_I2SV2_DIV_PRESCALER, 4-1);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_clkdiv(&s3c64xx_i2s_v4_dai, S3C_I2SV2_DIV_BCLK, 32);
        if (ret < 0)
                return ret;

        ret = snd_soc_dai_set_clkdiv(&s3c64xx_i2s_v4_dai, S3C_I2SV2_DIV_RCLK, 256);
        if (ret < 0)
                return ret;
#endif

	return ret;
}

static void __exit smdk_audio_exit(void)
{
	platform_device_unregister(smdk_snd_device);
}

module_init(smdk_audio_init);

MODULE_AUTHOR("Seungwhan Youn, <sw.youn@samsung.com>");
MODULE_DESCRIPTION("ALSA SoC SMDK WM8994(Codec Slave) Audio Driver");
MODULE_LICENSE("GPL");
