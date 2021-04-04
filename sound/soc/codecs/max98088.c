/*
 * max98088.c -- MAX98088 ALSA SoC Audio driver
 *
 * Copyright 2010 Maxim Integrated Products
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>

#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include <asm/div64.h>

#include <sound/max98088.h>
#include "max98088.h"

static const u8 max98088_reg[M98088_REG_CNT] = {
       0x00, /* 00 IRQ status */
       0x00, /* 01 MIC status */
       0x00, /* 02 jack status */
       0x00, /* 03 battery voltage */
       0x00, /* 04 */
       0x00, /* 05 */
       0x00, /* 06 */
       0x00, /* 07 */
       0x00, /* 08 */
       0x00, /* 09 */
       0x00, /* 0A */
       0x00, /* 0B */
       0x00, /* 0C */
       0x00, /* 0D */
       0x00, /* 0E */
       0x00, /* 0F interrupt enable */

       0x00, /* 10 master clock */
       0x00, /* 11 DAI1 clock mode */
       0x00, /* 12 DAI1 clock control */
       0x00, /* 13 DAI1 clock control */
       0x00, /* 14 DAI1 format */
       0x00, /* 15 DAI1 clock */
       0x00, /* 16 DAI1 config */
       0x00, /* 17 DAI1 TDM */
       0x00, /* 18 DAI1 filters */
       0x00, /* 19 DAI2 clock mode */
       0x00, /* 1A DAI2 clock control */
       0x00, /* 1B DAI2 clock control */
       0x00, /* 1C DAI2 format */
       0x00, /* 1D DAI2 clock */
       0x00, /* 1E DAI2 config */
       0x00, /* 1F DAI2 TDM */

       0x00, /* 20 DAI2 filters */
       0x00, /* 21 data config */
       0x00, /* 22 DAC mixer */
       0x00, /* 23 left ADC mixer */
       0x00, /* 24 right ADC mixer */
       0x00, /* 25 left HP mixer */
       0x00, /* 26 right HP mixer */
       0x00, /* 27 HP control */
       0x00, /* 28 left REC mixer */
       0x00, /* 29 right REC mixer */
       0x00, /* 2A REC control */
       0x00, /* 2B left SPK mixer */
       0x00, /* 2C right SPK mixer */
       0x00, /* 2D SPK control */
       0x00, /* 2E sidetone */
       0x00, /* 2F DAI1 playback level */

       0x00, /* 30 DAI1 playback level */
       0x00, /* 31 DAI2 playback level */
       0x00, /* 32 DAI2 playbakc level */
       0x00, /* 33 left ADC level */
       0x00, /* 34 right ADC level */
       0x00, /* 35 MIC1 level */
       0x00, /* 36 MIC2 level */
       0x00, /* 37 INA level */
       0x00, /* 38 INB level */
       0x00, /* 39 left HP volume */
       0x00, /* 3A right HP volume */
       0x00, /* 3B left REC volume */
       0x00, /* 3C right REC volume */
       0x00, /* 3D left SPK volume */
       0x00, /* 3E right SPK volume */
       0x00, /* 3F MIC config */

       0x00, /* 40 MIC threshold */
       0x00, /* 41 excursion limiter filter */
       0x00, /* 42 excursion limiter threshold */
       0x00, /* 43 ALC */
       0x00, /* 44 power limiter threshold */
       0x00, /* 45 power limiter config */
       0x00, /* 46 distortion limiter config */
       0x00, /* 47 audio input */
       0x00, /* 48 microphone */
       0x00, /* 49 level control */
       0x00, /* 4A bypass switches */
       0x00, /* 4B jack detect */
       0x00, /* 4C input enable */
       0x00, /* 4D output enable */
       0xF0, /* 4E bias control */
       0x00, /* 4F DAC power */

       0x0F, /* 50 DAC power */
       0x00, /* 51 system */
       0x00, /* 52 DAI1 EQ1 */
       0x00, /* 53 DAI1 EQ1 */
       0x00, /* 54 DAI1 EQ1 */
       0x00, /* 55 DAI1 EQ1 */
       0x00, /* 56 DAI1 EQ1 */
       0x00, /* 57 DAI1 EQ1 */
       0x00, /* 58 DAI1 EQ1 */
       0x00, /* 59 DAI1 EQ1 */
       0x00, /* 5A DAI1 EQ1 */
       0x00, /* 5B DAI1 EQ1 */
       0x00, /* 5C DAI1 EQ2 */
       0x00, /* 5D DAI1 EQ2 */
       0x00, /* 5E DAI1 EQ2 */
       0x00, /* 5F DAI1 EQ2 */

       0x00, /* 60 DAI1 EQ2 */
       0x00, /* 61 DAI1 EQ2 */
       0x00, /* 62 DAI1 EQ2 */
       0x00, /* 63 DAI1 EQ2 */
       0x00, /* 64 DAI1 EQ2 */
       0x00, /* 65 DAI1 EQ2 */
       0x00, /* 66 DAI1 EQ3 */
       0x00, /* 67 DAI1 EQ3 */
       0x00, /* 68 DAI1 EQ3 */
       0x00, /* 69 DAI1 EQ3 */
       0x00, /* 6A DAI1 EQ3 */
       0x00, /* 6B DAI1 EQ3 */
       0x00, /* 6C DAI1 EQ3 */
       0x00, /* 6D DAI1 EQ3 */
       0x00, /* 6E DAI1 EQ3 */
       0x00, /* 6F DAI1 EQ3 */

       0x00, /* 70 DAI1 EQ4 */
       0x00, /* 71 DAI1 EQ4 */
       0x00, /* 72 DAI1 EQ4 */
       0x00, /* 73 DAI1 EQ4 */
       0x00, /* 74 DAI1 EQ4 */
       0x00, /* 75 DAI1 EQ4 */
       0x00, /* 76 DAI1 EQ4 */
       0x00, /* 77 DAI1 EQ4 */
       0x00, /* 78 DAI1 EQ4 */
       0x00, /* 79 DAI1 EQ4 */
       0x00, /* 7A DAI1 EQ5 */
       0x00, /* 7B DAI1 EQ5 */
       0x00, /* 7C DAI1 EQ5 */
       0x00, /* 7D DAI1 EQ5 */
       0x00, /* 7E DAI1 EQ5 */
       0x00, /* 7F DAI1 EQ5 */

       0x00, /* 80 DAI1 EQ5 */
       0x00, /* 81 DAI1 EQ5 */
       0x00, /* 82 DAI1 EQ5 */
       0x00, /* 83 DAI1 EQ5 */
       0x00, /* 84 DAI2 EQ1 */
       0x00, /* 85 DAI2 EQ1 */
       0x00, /* 86 DAI2 EQ1 */
       0x00, /* 87 DAI2 EQ1 */
       0x00, /* 88 DAI2 EQ1 */
       0x00, /* 89 DAI2 EQ1 */
       0x00, /* 8A DAI2 EQ1 */
       0x00, /* 8B DAI2 EQ1 */
       0x00, /* 8C DAI2 EQ1 */
       0x00, /* 8D DAI2 EQ1 */
       0x00, /* 8E DAI2 EQ2 */
       0x00, /* 8F DAI2 EQ2 */

       0x00, /* 90 DAI2 EQ2 */
       0x00, /* 91 DAI2 EQ2 */
       0x00, /* 92 DAI2 EQ2 */
       0x00, /* 93 DAI2 EQ2 */
       0x00, /* 94 DAI2 EQ2 */
       0x00, /* 95 DAI2 EQ2 */
       0x00, /* 96 DAI2 EQ2 */
       0x00, /* 97 DAI2 EQ2 */
       0x00, /* 98 DAI2 EQ3 */
       0x00, /* 99 DAI2 EQ3 */
       0x00, /* 9A DAI2 EQ3 */
       0x00, /* 9B DAI2 EQ3 */
       0x00, /* 9C DAI2 EQ3 */
       0x00, /* 9D DAI2 EQ3 */
       0x00, /* 9E DAI2 EQ3 */
       0x00, /* 9F DAI2 EQ3 */

       0x00, /* A0 DAI2 EQ3 */
       0x00, /* A1 DAI2 EQ3 */
       0x00, /* A2 DAI2 EQ4 */
       0x00, /* A3 DAI2 EQ4 */
       0x00, /* A4 DAI2 EQ4 */
       0x00, /* A5 DAI2 EQ4 */
       0x00, /* A6 DAI2 EQ4 */
       0x00, /* A7 DAI2 EQ4 */
       0x00, /* A8 DAI2 EQ4 */
       0x00, /* A9 DAI2 EQ4 */
       0x00, /* AA DAI2 EQ4 */
       0x00, /* AB DAI2 EQ4 */
       0x00, /* AC DAI2 EQ5 */
       0x00, /* AD DAI2 EQ5 */
       0x00, /* AE DAI2 EQ5 */
       0x00, /* AF DAI2 EQ5 */

       0x00, /* B0 DAI2 EQ5 */
       0x00, /* B1 DAI2 EQ5 */
       0x00, /* B2 DAI2 EQ5 */
       0x00, /* B3 DAI2 EQ5 */
       0x00, /* B4 DAI2 EQ5 */
       0x00, /* B5 DAI2 EQ5 */
       0x00, /* B6 DAI1 biquad */
       0x00, /* B7 DAI1 biquad */
       0x00, /* B8 DAI1 biquad */
       0x00, /* B9 DAI1 biquad */
       0x00, /* BA DAI1 biquad */
       0x00, /* BB DAI1 biquad */
       0x00, /* BC DAI1 biquad */
       0x00, /* BD DAI1 biquad */
       0x00, /* BE DAI1 biquad */
       0x00, /* BF DAI1 biquad */

       0x00, /* C0 DAI2 biquad */
       0x00, /* C1 DAI2 biquad */
       0x00, /* C2 DAI2 biquad */
       0x00, /* C3 DAI2 biquad */
       0x00, /* C4 DAI2 biquad */
       0x00, /* C5 DAI2 biquad */
       0x00, /* C6 DAI2 biquad */
       0x00, /* C7 DAI2 biquad */
       0x00, /* C8 DAI2 biquad */
       0x00, /* C9 DAI2 biquad */
       0x00, /* CA */
       0x00, /* CB */
       0x00, /* CC */
       0x00, /* CD */
       0x00, /* CE */
       0x00, /* CF */

       0x00, /* D0 */
       0x00, /* D1 */
       0x00, /* D2 */
       0x00, /* D3 */
       0x00, /* D4 */
       0x00, /* D5 */
       0x00, /* D6 */
       0x00, /* D7 */
       0x00, /* D8 */
       0x00, /* D9 */
       0x00, /* DA */
       0x70, /* DB */
       0x00, /* DC */
       0x00, /* DD */
       0x00, /* DE */
       0x00, /* DF */

       0x00, /* E0 */
       0x00, /* E1 */
       0x00, /* E2 */
       0x00, /* E3 */
       0x00, /* E4 */
       0x00, /* E5 */
       0x00, /* E6 */
       0x00, /* E7 */
       0x00, /* E8 */
       0x00, /* E9 */
       0x00, /* EA */
       0x00, /* EB */
       0x00, /* EC */
       0x00, /* ED */
       0x00, /* EE */
       0x00, /* EF */

       0x00, /* F0 */
       0x00, /* F1 */
       0x00, /* F2 */
       0x00, /* F3 */
       0x00, /* F4 */
       0x00, /* F5 */
       0x00, /* F6 */
       0x00, /* F7 */
       0x00, /* F8 */
       0x00, /* F9 */
       0x00, /* FA */
       0x00, /* FB */
       0x00, /* FC */
       0x00, /* FD */
       0x00, /* FE */
       0x00, /* FF */
};

static struct {
       int readable;
       int writable;
       int vol;
} max98088_access[M98088_REG_CNT] = {
       { 0xFF, 0xFF, 1 }, /* 00 IRQ status */
       { 0xFF, 0x00, 1 }, /* 01 MIC status */
       { 0xFF, 0x00, 1 }, /* 02 jack status */
       { 0x1F, 0x1F, 1 }, /* 03 battery voltage */
       { 0xFF, 0xFF, 0 }, /* 04 */
       { 0xFF, 0xFF, 0 }, /* 05 */
       { 0xFF, 0xFF, 0 }, /* 06 */
       { 0xFF, 0xFF, 0 }, /* 07 */
       { 0xFF, 0xFF, 0 }, /* 08 */
       { 0xFF, 0xFF, 0 }, /* 09 */
       { 0xFF, 0xFF, 0 }, /* 0A */
       { 0xFF, 0xFF, 0 }, /* 0B */
       { 0xFF, 0xFF, 0 }, /* 0C */
       { 0xFF, 0xFF, 0 }, /* 0D */
       { 0xFF, 0xFF, 0 }, /* 0E */
       { 0xFF, 0xFF, 0 }, /* 0F interrupt enable */

       { 0xFF, 0xFF, 0 }, /* 10 master clock */
       { 0xFF, 0xFF, 0 }, /* 11 DAI1 clock mode */
       { 0xFF, 0xFF, 0 }, /* 12 DAI1 clock control */
       { 0xFF, 0xFF, 0 }, /* 13 DAI1 clock control */
       { 0xFF, 0xFF, 0 }, /* 14 DAI1 format */
       { 0xFF, 0xFF, 0 }, /* 15 DAI1 clock */
       { 0xFF, 0xFF, 0 }, /* 16 DAI1 config */
       { 0xFF, 0xFF, 0 }, /* 17 DAI1 TDM */
       { 0xFF, 0xFF, 0 }, /* 18 DAI1 filters */
       { 0xFF, 0xFF, 0 }, /* 19 DAI2 clock mode */
       { 0xFF, 0xFF, 0 }, /* 1A DAI2 clock control */
       { 0xFF, 0xFF, 0 }, /* 1B DAI2 clock control */
       { 0xFF, 0xFF, 0 }, /* 1C DAI2 format */
       { 0xFF, 0xFF, 0 }, /* 1D DAI2 clock */
       { 0xFF, 0xFF, 0 }, /* 1E DAI2 config */
       { 0xFF, 0xFF, 0 }, /* 1F DAI2 TDM */

       { 0xFF, 0xFF, 0 }, /* 20 DAI2 filters */
       { 0xFF, 0xFF, 0 }, /* 21 data config */
       { 0xFF, 0xFF, 0 }, /* 22 DAC mixer */
       { 0xFF, 0xFF, 0 }, /* 23 left ADC mixer */
       { 0xFF, 0xFF, 0 }, /* 24 right ADC mixer */
       { 0xFF, 0xFF, 0 }, /* 25 left HP mixer */
       { 0xFF, 0xFF, 0 }, /* 26 right HP mixer */
       { 0xFF, 0xFF, 0 }, /* 27 HP control */
       { 0xFF, 0xFF, 0 }, /* 28 left REC mixer */
       { 0xFF, 0xFF, 0 }, /* 29 right REC mixer */
       { 0xFF, 0xFF, 0 }, /* 2A REC control */
       { 0xFF, 0xFF, 0 }, /* 2B left SPK mixer */
       { 0xFF, 0xFF, 0 }, /* 2C right SPK mixer */
       { 0xFF, 0xFF, 0 }, /* 2D SPK control */
       { 0xFF, 0xFF, 0 }, /* 2E sidetone */
       { 0xFF, 0xFF, 0 }, /* 2F DAI1 playback level */

       { 0xFF, 0xFF, 0 }, /* 30 DAI1 playback level */
       { 0xFF, 0xFF, 0 }, /* 31 DAI2 playback level */
       { 0xFF, 0xFF, 0 }, /* 32 DAI2 playbakc level */
       { 0xFF, 0xFF, 0 }, /* 33 left ADC level */
       { 0xFF, 0xFF, 0 }, /* 34 right ADC level */
       { 0xFF, 0xFF, 0 }, /* 35 MIC1 level */
       { 0xFF, 0xFF, 0 }, /* 36 MIC2 level */
       { 0xFF, 0xFF, 0 }, /* 37 INA level */
       { 0xFF, 0xFF, 0 }, /* 38 INB level */
       { 0xFF, 0xFF, 0 }, /* 39 left HP volume */
       { 0xFF, 0xFF, 0 }, /* 3A right HP volume */
       { 0xFF, 0xFF, 0 }, /* 3B left REC volume */
       { 0xFF, 0xFF, 0 }, /* 3C right REC volume */
       { 0xFF, 0xFF, 0 }, /* 3D left SPK volume */
       { 0xFF, 0xFF, 0 }, /* 3E right SPK volume */
       { 0xFF, 0xFF, 0 }, /* 3F MIC config */

       { 0xFF, 0xFF, 0 }, /* 40 MIC threshold */
       { 0xFF, 0xFF, 0 }, /* 41 excursion limiter filter */
       { 0xFF, 0xFF, 0 }, /* 42 excursion limiter threshold */
       { 0xFF, 0xFF, 0 }, /* 43 ALC */
       { 0xFF, 0xFF, 0 }, /* 44 power limiter threshold */
       { 0xFF, 0xFF, 0 }, /* 45 power limiter config */
       { 0xFF, 0xFF, 0 }, /* 46 distortion limiter config */
       { 0xFF, 0xFF, 0 }, /* 47 audio input */
       { 0xFF, 0xFF, 0 }, /* 48 microphone */
       { 0xFF, 0xFF, 0 }, /* 49 level control */
       { 0xFF, 0xFF, 0 }, /* 4A bypass switches */
       { 0xFF, 0xFF, 0 }, /* 4B jack detect */
       { 0xFF, 0xFF, 0 }, /* 4C input enable */
       { 0xFF, 0xFF, 0 }, /* 4D output enable */
       { 0xFF, 0xFF, 0 }, /* 4E bias control */
       { 0xFF, 0xFF, 0 }, /* 4F DAC power */

       { 0xFF, 0xFF, 0 }, /* 50 DAC power */
       { 0xFF, 0xFF, 0 }, /* 51 system */
       { 0xFF, 0xFF, 0 }, /* 52 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 53 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 54 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 55 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 56 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 57 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 58 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 59 DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 5A DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 5B DAI1 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 5C DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 5D DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 5E DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 5F DAI1 EQ2 */

       { 0xFF, 0xFF, 0 }, /* 60 DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 61 DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 62 DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 63 DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 64 DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 65 DAI1 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 66 DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 67 DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 68 DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 69 DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 6A DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 6B DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 6C DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 6D DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 6E DAI1 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 6F DAI1 EQ3 */

       { 0xFF, 0xFF, 0 }, /* 70 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 71 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 72 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 73 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 74 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 75 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 76 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 77 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 78 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 79 DAI1 EQ4 */
       { 0xFF, 0xFF, 0 }, /* 7A DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 7B DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 7C DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 7D DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 7E DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 7F DAI1 EQ5 */

       { 0xFF, 0xFF, 0 }, /* 80 DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 81 DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 82 DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 83 DAI1 EQ5 */
       { 0xFF, 0xFF, 0 }, /* 84 DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 85 DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 86 DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 87 DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 88 DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 89 DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 8A DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 8B DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 8C DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 8D DAI2 EQ1 */
       { 0xFF, 0xFF, 0 }, /* 8E DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 8F DAI2 EQ2 */

       { 0xFF, 0xFF, 0 }, /* 90 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 91 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 92 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 93 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 94 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 95 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 96 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 97 DAI2 EQ2 */
       { 0xFF, 0xFF, 0 }, /* 98 DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 99 DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 9A DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 9B DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 9C DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 9D DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 9E DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* 9F DAI2 EQ3 */

       { 0xFF, 0xFF, 0 }, /* A0 DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* A1 DAI2 EQ3 */
       { 0xFF, 0xFF, 0 }, /* A2 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A3 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A4 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A5 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A6 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A7 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A8 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* A9 DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* AA DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* AB DAI2 EQ4 */
       { 0xFF, 0xFF, 0 }, /* AC DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* AD DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* AE DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* AF DAI2 EQ5 */

       { 0xFF, 0xFF, 0 }, /* B0 DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* B1 DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* B2 DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* B3 DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* B4 DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* B5 DAI2 EQ5 */
       { 0xFF, 0xFF, 0 }, /* B6 DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* B7 DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* B8 DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* B9 DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* BA DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* BB DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* BC DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* BD DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* BE DAI1 biquad */
       { 0xFF, 0xFF, 0 }, /* BF DAI1 biquad */

       { 0xFF, 0xFF, 0 }, /* C0 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C1 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C2 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C3 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C4 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C5 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C6 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C7 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C8 DAI2 biquad */
       { 0xFF, 0xFF, 0 }, /* C9 DAI2 biquad */
       { 0x00, 0x00, 0 }, /* CA */
       { 0x00, 0x00, 0 }, /* CB */
       { 0x00, 0x00, 0 }, /* CC */
       { 0x00, 0x00, 0 }, /* CD */
       { 0x00, 0x00, 0 }, /* CE */
       { 0x00, 0x00, 0 }, /* CF */

       { 0x00, 0x00, 0 }, /* D0 */
       { 0x00, 0x00, 0 }, /* D1 */
       { 0x00, 0x00, 0 }, /* D2 */
       { 0x00, 0x00, 0 }, /* D3 */
       { 0x00, 0x00, 0 }, /* D4 */
       { 0x00, 0x00, 0 }, /* D5 */
       { 0x00, 0x00, 0 }, /* D6 */
       { 0x00, 0x00, 0 }, /* D7 */
       { 0x00, 0x00, 0 }, /* D8 */
       { 0x00, 0x00, 0 }, /* D9 */
       { 0x00, 0x00, 0 }, /* DA */
       { 0x00, 0x00, 0 }, /* DB */
       { 0x00, 0x00, 0 }, /* DC */
       { 0x00, 0x00, 0 }, /* DD */
       { 0x00, 0x00, 0 }, /* DE */
       { 0x00, 0x00, 0 }, /* DF */

       { 0x00, 0x00, 0 }, /* E0 */
       { 0x00, 0x00, 0 }, /* E1 */
       { 0x00, 0x00, 0 }, /* E2 */
       { 0x00, 0x00, 0 }, /* E3 */
       { 0x00, 0x00, 0 }, /* E4 */
       { 0x00, 0x00, 0 }, /* E5 */
       { 0x00, 0x00, 0 }, /* E6 */
       { 0x00, 0x00, 0 }, /* E7 */
       { 0x00, 0x00, 0 }, /* E8 */
       { 0x00, 0x00, 0 }, /* E9 */
       { 0x00, 0x00, 0 }, /* EA */
       { 0x00, 0x00, 0 }, /* EB */
       { 0x00, 0x00, 0 }, /* EC */
       { 0x00, 0x00, 0 }, /* ED */
       { 0x00, 0x00, 0 }, /* EE */
       { 0x00, 0x00, 0 }, /* EF */

       { 0x00, 0x00, 0 }, /* F0 */
       { 0x00, 0x00, 0 }, /* F1 */
       { 0x00, 0x00, 0 }, /* F2 */
       { 0x00, 0x00, 0 }, /* F3 */
       { 0x00, 0x00, 0 }, /* F4 */
       { 0x00, 0x00, 0 }, /* F5 */
       { 0x00, 0x00, 0 }, /* F6 */
       { 0x00, 0x00, 0 }, /* F7 */
       { 0x00, 0x00, 0 }, /* F8 */
       { 0x00, 0x00, 0 }, /* F9 */
       { 0x00, 0x00, 0 }, /* FA */
       { 0x00, 0x00, 0 }, /* FB */
       { 0x00, 0x00, 0 }, /* FC */
       { 0x00, 0x00, 0 }, /* FD */
       { 0x00, 0x00, 0 }, /* FE */
       { 0xFF, 0x00, 1 }, /* FF */
};

//static int max98088_volatile_register(unsigned int reg)
//{
//       return max98088_access[reg].vol;
//}


//------------------------------------------------
// Implementation of I2C functions
//------------------------------------------------
/*
 * read max98088 register cache
 */
static inline unsigned int max98088_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	BUG_ON(reg > (ARRAY_SIZE(max98088_reg)) - 1);
	return cache[reg];
}

inline unsigned int max98088_read(struct snd_soc_codec *codec, unsigned int reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(codec->control_data, (u8)(reg & 0xFF));

	if (ret < 0)
		printk("DEBUG -> %s error!!! [%d]\n",__FUNCTION__,__LINE__);

	return ret;
}

/*
 * write max98088 register cache
 */
static inline void max98088_write_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;

	/* Reset register and reserved registers are uncached */
	if (reg == 0 || reg > ARRAY_SIZE(max98088_reg) - 1)
		return;

	cache[reg] = value;
}

/*
 * write to the max98088 register space
 */
static int max98088_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	max98088_write_reg_cache (codec, reg, value);

	if(i2c_smbus_write_byte_data(codec->control_data, (u8)(reg & 0xFF), (u8)(value & 0xFF))<0) {
		printk("%s error!!! [%d]\n",__FUNCTION__,__LINE__);
		return -EIO;
	}
	
	return 0;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static struct snd_soc_codec *max98088_codec;

static const char *playback_path[] = {"OFF", "RCV", "SPK", "HP", "SPK_HP", "TV_OUT", };
static const char *record_path[] = {"Main Mic", "Headset Mic", };

static const struct soc_enum path_control_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(playback_path),playback_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(record_path),record_path),
};

extern bool	g_btvout;
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static int max98089_set_playback_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);

	int path_num = ucontrol->value.integer.value[0];

	max98088->cur_path = path_num;
	printk(" %s [%d] param %d\n",__FUNCTION__,__LINE__, path_num);
	
	switch(path_num) {
		case 0 : // headset
		case 1 : // earpiece
			max98089_set_playback_headset(codec);
			break;
		case 2 : // speaker
			max98089_set_playback_speaker(codec);
			break;
		case 3 :
			max98089_set_playback_speaker_headset(codec);
			break;
		case 4 :
			break;
		default :
			break;
	}
	
	if(g_btvout) max98089_disable_playback_path(codec, SPK);

	return 0;
}

static int max98089_get_playback_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
	
	printk(" %s [%d] current playback path %d\n",__FUNCTION__,__LINE__, max98088->cur_path);

	ucontrol->value.integer.value[0] = max98088->cur_path;

	return 0;
}


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static int max98089_set_record_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);

	int path_num = ucontrol->value.integer.value[0];
	
	max98088->rec_path = path_num;
	printk(" %s [%d] param %d\n",__FUNCTION__,__LINE__, path_num);

	switch(path_num) {
		case 0 :
			max98089_set_record_main_mic(codec);
			break;
		case 1 :
			max98089_set_record_main_mic(codec);
			break;
		case 2 :
			max98089_set_record_main_mic(codec);
			break;
		default :
			break;
	}

	return 0;
}

static int max98089_get_record_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);

	printk(" %s [%d] current record path %d\n",__FUNCTION__,__LINE__, max98088->rec_path);

	ucontrol->value.integer.value[0] = max98088->rec_path;

	return 0;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static const struct snd_kcontrol_new max98088_snd_controls[] = {

	/* Path Control */
	SOC_ENUM_EXT("Playback Path", path_control_enum[0],
                max98089_get_playback_path, max98089_set_playback_path),

	SOC_ENUM_EXT("MIC Path", path_control_enum[1],
                max98089_get_record_path, max98089_set_record_path),

};


static int max98088_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_add_controls(codec, max98088_snd_controls,
	                    ARRAY_SIZE(max98088_snd_controls));
	snd_soc_dapm_new_widgets(codec);
	return 0;
}

/* codec mclk clock divider coefficients */
static const struct {
       u32 rate;
       u8  sr;
} rate_table[] = {
       {8000,  0x10},
       {11025, 0x20},
       {16000, 0x30},
       {22050, 0x40},
       {24000, 0x50},
       {32000, 0x60},
       {44100, 0x70},
       {48000, 0x80},
       {88200, 0x90},
       {96000, 0xA0},
};

static inline int rate_value(int rate, u8 *value)
{
       int i;

       for (i = 0; i < ARRAY_SIZE(rate_table); i++) {
               if (rate_table[i].rate >= rate) {
                       *value = rate_table[i].sr;
                       return 0;
               }
       }
       *value = rate_table[0].sr;
       return -EINVAL;
}

static int max98088_dai1_hw_params(struct snd_pcm_substream *substream,
                                  struct snd_pcm_hw_params *params,
                                  struct snd_soc_dai *dai)
{
       struct snd_soc_codec *codec = dai->codec;
       struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
       struct max98088_cdata *cdata;
       unsigned long long ni;
       unsigned int rate;
       u8 regval;

       cdata = &max98088->dai[0];

       rate = params_rate(params);

       switch (params_format(params)) {
       case SNDRV_PCM_FORMAT_S16_LE:
               snd_soc_update_bits(codec, M98088_REG_14_DAI1_FORMAT,
                       M98088_DAI_WS, 0);
               break;
       case SNDRV_PCM_FORMAT_S24_LE:
               snd_soc_update_bits(codec, M98088_REG_14_DAI1_FORMAT,
                       M98088_DAI_WS, M98088_DAI_WS);
               break;
       default:
               return -EINVAL;
       }

       snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS, M98088_SHDNRUN, 0);

       if (rate_value(rate, &regval))
               return -EINVAL;

       snd_soc_update_bits(codec, M98088_REG_11_DAI1_CLKMODE,
               M98088_CLKMODE_MASK, regval);
       cdata->rate = rate;

       /* Configure NI when operating as master */
       if (snd_soc_read(codec, M98088_REG_14_DAI1_FORMAT)
               & M98088_DAI_MAS) {
               if (max98088->sysclk == 0) {
                       dev_err(codec->dev, "Invalid system clock frequency\n");
                       return -EINVAL;
               }
               ni = 65536ULL * (rate < 50000 ? 96ULL : 48ULL)
                               * (unsigned long long int)rate;
               do_div(ni, (unsigned long long int)max98088->sysclk);
               snd_soc_write(codec, M98088_REG_12_DAI1_CLKCFG_HI,
                       (ni >> 8) & 0x7F);
               snd_soc_write(codec, M98088_REG_13_DAI1_CLKCFG_LO,
                       ni & 0xFF);
       }

       /* Update sample rate mode */
       if (rate < 50000)
               snd_soc_update_bits(codec, M98088_REG_18_DAI1_FILTERS,
                       M98088_DAI_DHF, 0);
       else
               snd_soc_update_bits(codec, M98088_REG_18_DAI1_FILTERS,
                       M98088_DAI_DHF, M98088_DAI_DHF);

       snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS, M98088_SHDNRUN,
               M98088_SHDNRUN);


    return 0;
}


static int max98088_dai_set_sysclk(struct snd_soc_dai *dai,
                                  int clk_id, unsigned int freq, int dir)
{
       struct snd_soc_codec *codec = dai->codec;
       struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);

       /* Requested clock frequency is already setup */
       if (freq == max98088->sysclk)
               return 0;

       max98088->sysclk = freq; /* remember current sysclk */

       /* Setup clocks for slave mode, and using the PLL
        * PSCLK = 0x01 (when master clk is 10MHz to 20MHz)
        *         0x02 (when master clk is 20MHz to 30MHz)..
        */
       if ((freq >= 10000000) && (freq < 20000000)) {
               snd_soc_write(codec, M98088_REG_10_SYS_CLK, 0x10);
       } else if ((freq >= 20000000) && (freq < 30000000)) {
               snd_soc_write(codec, M98088_REG_10_SYS_CLK, 0x20);
       } else if ((freq >= 30000000) && (freq < 40000000)) {
               snd_soc_write(codec, M98088_REG_10_SYS_CLK, 0x30);
       } else {
               dev_err(codec->dev, "Invalid master clock frequency\n");
               return -EINVAL;
       }

       if (snd_soc_read(codec, M98088_REG_51_PWR_SYS)  & M98088_SHDNRUN) {
               snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS,
                       M98088_SHDNRUN, 0);
               snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS,
                       M98088_SHDNRUN, M98088_SHDNRUN);
       }

       dev_dbg(dai->dev, "Clock source is %d at %uHz\n", clk_id, freq);

       max98088->sysclk = freq;
       return 0;
}

static int max98088_dai1_set_fmt(struct snd_soc_dai *codec_dai,
                                unsigned int fmt)
{
       struct snd_soc_codec *codec = codec_dai->codec;
       struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
       struct max98088_cdata *cdata;
       u8 reg15val;
       u8 reg14val = 0;

       cdata = &max98088->dai[0];

       if (fmt != cdata->fmt) {
               cdata->fmt = fmt;

               switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
               case SND_SOC_DAIFMT_CBS_CFS:
                       /* Slave mode PLL */
                       snd_soc_write(codec, M98088_REG_12_DAI1_CLKCFG_HI,
                               0x80);
                       snd_soc_write(codec, M98088_REG_13_DAI1_CLKCFG_LO,
                               0x00);
                       break;
               case SND_SOC_DAIFMT_CBM_CFM:
                       /* Set to master mode */
                       reg14val |= M98088_DAI_MAS;
                       break;
               case SND_SOC_DAIFMT_CBS_CFM:
               case SND_SOC_DAIFMT_CBM_CFS:
               default:
                       dev_err(codec->dev, "Clock mode unsupported");
                       return -EINVAL;
               }

               switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
               case SND_SOC_DAIFMT_I2S:
                       reg14val |= M98088_DAI_DLY;
                       break;
               case SND_SOC_DAIFMT_LEFT_J:
                       break;
               default:
                       return -EINVAL;
               }

               switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
               case SND_SOC_DAIFMT_NB_NF:
                       break;
               case SND_SOC_DAIFMT_NB_IF:
                       reg14val |= M98088_DAI_WCI;
                       break;
               case SND_SOC_DAIFMT_IB_NF:
                       reg14val |= M98088_DAI_BCI;
                       break;
               case SND_SOC_DAIFMT_IB_IF:
                       reg14val |= M98088_DAI_BCI|M98088_DAI_WCI;
                       break;
               default:
                       return -EINVAL;
               }

               snd_soc_update_bits(codec, M98088_REG_14_DAI1_FORMAT,
                       M98088_DAI_MAS | M98088_DAI_DLY | M98088_DAI_BCI |
                       M98088_DAI_WCI, reg14val);

               reg15val = M98088_DAI_BSEL64;
               if (max98088->digmic)
                       reg15val |= M98088_DAI_OSR64;
               snd_soc_write(codec, M98088_REG_15_DAI1_CLOCK, reg15val);
       }

       return 0;
}

static int max98088_trigger(struct snd_pcm_substream *substream, int cmd, struct snd_soc_dai *codec_dai)
{
	int ret = 0;
	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			break;

		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
			break;

		default:
			break;
	}
	return ret;
}

static void max98088_sync_cache(struct snd_soc_codec *codec)
{
       u16 *reg_cache = codec->reg_cache;
       int i;

       if (!codec->cache_sync)
               return;

       codec->cache_only = 0;

       /* write back cached values if they're writeable and
        * different from the hardware default.
        */
       for (i = 1; i < codec->reg_cache_size; i++) {
               if (!max98088_access[i].writable)
                       continue;

               if (reg_cache[i] == max98088_reg[i])
                       continue;

               snd_soc_write(codec, i, reg_cache[i]);
       }

       codec->cache_sync = 0;
}

static int max98088_set_bias_level(struct snd_soc_codec *codec,
                                  enum snd_soc_bias_level level)
{
	switch (level) {
	case SND_SOC_BIAS_ON:
	       break;
	
	case SND_SOC_BIAS_PREPARE:
	       break;
	
	case SND_SOC_BIAS_STANDBY:
	       if (codec->bias_level == SND_SOC_BIAS_OFF)
	               max98088_sync_cache(codec);
	       break;
	
	case SND_SOC_BIAS_OFF:
	       codec->cache_sync = 1;
	       break;
	}
	codec->bias_level = level;
	return 0;
}

#define MAX98088_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
	SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000)
#define MAX98088_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_dai_ops max98088_dai1_ops = {
	.set_sysclk = max98088_dai_set_sysclk,
	.set_fmt = max98088_dai1_set_fmt,
	.hw_params = max98088_dai1_hw_params,
	.trigger = max98088_trigger,
};

struct snd_soc_dai max98088_dai = {
	.name = "max98089",
	.playback = {
	       .stream_name = "HiFi Playback",
	       .channels_min = 1,
	       .channels_max = 2,
	       .rates = MAX98088_RATES,
	       .formats = MAX98088_FORMATS,
	},
	.capture = {
	       .stream_name = "HiFi Capture",
	       .channels_min = 1,
	       .channels_max = 2,
	       .rates = MAX98088_RATES,
	       .formats = MAX98088_FORMATS,
	},
	.ops = &max98088_dai1_ops,
};
EXPORT_SYMBOL_GPL(max98088_dai);

#ifdef CONFIG_PM
static int max98088_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	max98088_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int max98088_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	max98088_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	return 0;
}
#else
#define max98088_suspend NULL
#define max98088_resume NULL
#endif

static int max98088_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;

       struct max98088_priv *max98088;
       struct max98088_cdata *cdata;
       int ret = 0;


	socdev->card->codec = max98088_codec;
	codec = max98088_codec;
	max98088 = snd_soc_codec_get_drvdata(codec);

       codec->cache_sync = 1;

	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		dev_err(codec->dev, "failed to create pcms\n");
		return ret;
	}

	/* initialize private data */
	max98088->sysclk = (unsigned)-1;
	max98088->eq_textcnt = 0;
	
	cdata = &max98088->dai[0];
	cdata->rate = (unsigned)-1;
	cdata->fmt  = (unsigned)-1;
	cdata->eq_sel = 0;
	
	max98088->ina_state = 0;
	max98088->inb_state = 0;
	max98088->ex_mode = 0;
	max98088->digmic = 0;
	max98088->mic1pre = 0;
	max98088->mic2pre = 0;
	
	ret = snd_soc_read(codec, M98088_REG_FF_REV_ID);
	if (ret < 0) {
	       dev_err(codec->dev, "Failed to read device revision: %d\n",
	               ret);
	       goto err_access;
	}
	dev_info(codec->dev, "revision %c\n", ret + 'A');
	
    snd_soc_write(codec, M98088_REG_51_PWR_SYS, 0x00);
	
	/* initialize registers cache to hardware default */
	max98088_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	
	snd_soc_write(codec, M98088_REG_0F_IRQ_ENABLE, 0x00);
	
	snd_soc_write(codec, M98088_REG_4E_BIAS_CNTL, 0xF0); // 
	snd_soc_write(codec, M98088_REG_50_DAC_BIAS2, 0x05); // DAI1 enable, DAI2 disable
	
//	snd_soc_write(codec, M98088_REG_16_DAI1_IOCFG, M98088_S1NORMAL|M98088_SDATA);
	snd_soc_write(codec, M98088_REG_16_DAI1_IOCFG, 0x43);
//	snd_soc_write(codec, M98088_REG_1E_DAI2_IOCFG,
//	       M98088_S2NORMAL|M98088_SDATA);
//	max98089_disable_record_path(codec, MAIN);
	max98089_set_record_main_mic(codec);

    snd_soc_write(codec, M98088_REG_51_PWR_SYS, 0x80);
	
	max98088_add_widgets(codec);

err_access:
       return ret;
}

static int max98088_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);

	max98088_set_bias_level(codec, SND_SOC_BIAS_OFF);
	kfree(max98088->eq_texts);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_max98088 = {
       .probe   = max98088_probe,
       .remove  = max98088_remove,
       .suspend = max98088_suspend,
       .resume  = max98088_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_max98088);

static int max98088_register(struct max98088_priv *max98088,
			   enum snd_soc_control_type control)
{
	int ret;
	struct snd_soc_codec *codec = &max98088->codec;

	if (max98088_codec) {
		dev_err(codec->dev, "Another MAX98088 is registered\n");
		ret = -EINVAL;
		goto err;
	}

	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	snd_soc_codec_set_drvdata(codec, max98088);
	codec->name = "MAX98089";
	codec->owner = THIS_MODULE;
	codec->bias_level = SND_SOC_BIAS_OFF;
//	codec->set_bias_level = max98088_set_bias_level;
	codec->dai = &max98088_dai;
	codec->num_dai = 1;
	codec->reg_cache_size = ARRAY_SIZE(max98088->reg_cache);
	codec->reg_cache = &max98088->reg_cache;
	codec->write = max98088_write;
	codec->read = max98088_read;

	memcpy(codec->reg_cache, max98088_reg, sizeof(max98088_reg));

	max98088_dai.dev = codec->dev;
	max98088_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	max98088_codec = codec;

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register codec: %d\n", ret);
		goto err;
	}

	ret = snd_soc_register_dai(&max98088_dai);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register DAI: %d\n", ret);
		goto err_codec;
	}

	return 0;

err_codec:
	snd_soc_unregister_codec(codec);
err:
	kfree(max98088);
	return ret;
}

static int max98088_i2c_probe(struct i2c_client *i2c,
                            const struct i2c_device_id *id)
{
	struct max98088_priv *max98088;
	struct snd_soc_codec *codec;
	
	max98088 = kzalloc(sizeof(struct max98088_priv), GFP_KERNEL);
	if (max98088 == NULL)
		return -ENOMEM;

	codec = &max98088->codec;
	
	i2c_set_clientdata(i2c, max98088);
	codec->control_data = i2c;
	codec->dev = &i2c->dev;

	return max98088_register(max98088, SND_SOC_I2C);
}

static int __devexit max98088_i2c_remove(struct i2c_client *client)
{
	struct max98088_priv *max98088 = i2c_get_clientdata(client);
	struct snd_soc_codec *codec = &max98088->codec;

	snd_soc_unregister_codec(codec);
	snd_soc_unregister_dai(&max98088_dai);
	kfree(max98088);
	return 0;
}

static const struct i2c_device_id max98088_i2c_id[] = {
       { "max98089", 0 },
       { }
};
MODULE_DEVICE_TABLE(i2c, max98088_i2c_id);

static struct i2c_driver max98088_i2c_driver = {
       .driver = {
               .name = "max98089",
               .owner = THIS_MODULE,
       },
       .probe  = max98088_i2c_probe,
       .remove = max98088_i2c_remove,
       .id_table = max98088_i2c_id,
};

static int __init max98088_init(void)
{
       int ret;

       ret = i2c_add_driver(&max98088_i2c_driver);
       if (ret)
               pr_err("Failed to register max98088 I2C driver: %d\n", ret);

       return ret;
}
module_init(max98088_init);

static void __exit max98088_exit(void)
{
       i2c_del_driver(&max98088_i2c_driver);
}
module_exit(max98088_exit);

MODULE_DESCRIPTION("ALSA SoC MAX98088 driver");
MODULE_AUTHOR("Peter Hsiang, Jesse Marroquin");
MODULE_LICENSE("GPL");
