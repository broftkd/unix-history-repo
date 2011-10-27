/*
 * Copyright (c) 2002-2008 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2008 Atheros Communications, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $FreeBSD$
 */
#ifndef _DEV_ATH_AR5416PHY_H_
#define _DEV_ATH_AR5416PHY_H_

#include "ar5212/ar5212phy.h"

/* For AR_PHY_RADAR0 */
#define	AR_PHY_RADAR_0_FFT_ENA		0x80000000

#define	AR_PHY_RADAR_EXT		0x9940
#define	AR_PHY_RADAR_EXT_ENA		0x00004000

#define	AR_PHY_RADAR_1			0x9958
#define	AR_PHY_RADAR_1_RELPWR_ENA	0x00800000
#define	AR_PHY_RADAR_1_USE_FIR128	0x00400000
#define	AR_PHY_RADAR_1_RELPWR_THRESH	0x003F0000
#define	AR_PHY_RADAR_1_RELPWR_THRESH_S	16
#define	AR_PHY_RADAR_1_BLOCK_CHECK	0x00008000
#define	AR_PHY_RADAR_1_MAX_RRSSI	0x00004000
#define	AR_PHY_RADAR_1_RELSTEP_CHECK	0x00002000
#define	AR_PHY_RADAR_1_RELSTEP_THRESH	0x00001F00
#define	AR_PHY_RADAR_1_RELSTEP_THRESH_S	8
#define	AR_PHY_RADAR_1_MAXLEN		0x000000FF
#define	AR_PHY_RADAR_1_MAXLEN_S		0

#define AR_PHY_CHIP_ID_REV_0    0x80        /* 5416 Rev 0 (owl 1.0) BB */
#define AR_PHY_CHIP_ID_REV_1    0x81        /* 5416 Rev 1 (owl 2.0) BB */

#define RFSILENT_BB             0x00002000      /* shush bb */
#define AR_PHY_RESTART      	0x9970      /* restart */
#define AR_PHY_RESTART_DIV_GC   0x001C0000  /* bb_ant_fast_div_gc_limit */
#define AR_PHY_RESTART_DIV_GC_S 18

/* PLL settling times */
#define RTC_PLL_SETTLE_DELAY		1000    /* 1 ms     */
#define HT40_CHANNEL_CENTER_SHIFT   	10	/* MHz      */

#define AR_PHY_RFBUS_REQ        0x997C
#define AR_PHY_RFBUS_REQ_EN     0x00000001

#define AR_2040_MODE                0x8318
#define AR_2040_JOINED_RX_CLEAR     0x00000001   // use ctl + ext rx_clear for cca

#define AR_PHY_FC_TURBO_SHORT       0x00000002  /* Set short symbols to turbo mode setting */
#define AR_PHY_FC_DYN2040_EN        0x00000004      /* Enable dyn 20/40 mode */
#define AR_PHY_FC_DYN2040_PRI_ONLY  0x00000008      /* dyn 20/40 - primary only */
#define AR_PHY_FC_DYN2040_PRI_CH    0x00000010      /* dyn 20/40 - primary ch offset (0=+10MHz, 1=-10MHz)*/
#define AR_PHY_FC_DYN2040_EXT_CH    0x00000020      /* dyn 20/40 - ext ch spacing (0=20MHz/ 1=25MHz) */
#define AR_PHY_FC_HT_EN             0x00000040      /* ht enable */
#define AR_PHY_FC_SHORT_GI_40       0x00000080      /* allow short GI for HT 40 */
#define AR_PHY_FC_WALSH             0x00000100      /* walsh spatial spreading for 2 chains,2 streams TX */
#define AR_PHY_FC_SINGLE_HT_LTF1    0x00000200      /* single length (4us) 1st HT long training symbol */
#define	AR_PHY_FC_ENABLE_DAC_FIFO   0x00000800

#define AR_PHY_TIMING2      0x9810      /* Timing Control 2 */
#define AR_PHY_TIMING2_USE_FORCE    0x00001000
#define AR_PHY_TIMING2_FORCE_VAL    0x00000fff

#define	AR_PHY_TIMING_CTRL4_CHAIN(_i) \
	(AR_PHY_TIMING_CTRL4 + ((_i) << 12))
#define	AR_PHY_TIMING_CTRL4_DO_CAL  0x10000	    /* perform calibration */
#define AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF 0x01F   /* Mask for kcos_theta-1 for q correction */
#define AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF_S   0   /* shift for Q_COFF */
#define AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF 0x7E0   /* Mask for sin_theta for i correction */
#define AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF_S   5   /* Shift for sin_theta for i correction */
#define AR_PHY_TIMING_CTRL4_IQCORR_ENABLE   0x800   /* enable IQ correction */
#define AR_PHY_TIMING_CTRL4_IQCAL_LOG_COUNT_MAX 0xF000  /* Mask for max number of samples (logarithmic) */
#define AR_PHY_TIMING_CTRL4_IQCAL_LOG_COUNT_MAX_S   12  /* Shift for max number of samples */

#define AR_PHY_TIMING_CTRL4_ENABLE_SPUR_RSSI	0x80000000
#define	AR_PHY_TIMING_CTRL4_ENABLE_SPUR_FILTER	0x40000000	/* Enable spur filter */
#define	AR_PHY_TIMING_CTRL4_ENABLE_CHAN_MASK	0x20000000
#define	AR_PHY_TIMING_CTRL4_ENABLE_PILOT_MASK	0x10000000

#define AR_PHY_ADC_SERIAL_CTL       0x9830
#define AR_PHY_SEL_INTERNAL_ADDAC   0x00000000
#define AR_PHY_SEL_EXTERNAL_RADIO   0x00000001

#define AR_PHY_GAIN_2GHZ_BSW_MARGIN	0x00003C00
#define AR_PHY_GAIN_2GHZ_BSW_MARGIN_S	10
#define AR_PHY_GAIN_2GHZ_BSW_ATTEN	0x0000001F
#define AR_PHY_GAIN_2GHZ_BSW_ATTEN_S	0

#define AR_PHY_GAIN_2GHZ_XATTEN2_MARGIN	    0x003E0000
#define AR_PHY_GAIN_2GHZ_XATTEN2_MARGIN_S   17
#define AR_PHY_GAIN_2GHZ_XATTEN1_MARGIN     0x0001F000
#define AR_PHY_GAIN_2GHZ_XATTEN1_MARGIN_S   12
#define AR_PHY_GAIN_2GHZ_XATTEN2_DB         0x00000FC0
#define AR_PHY_GAIN_2GHZ_XATTEN2_DB_S       6
#define AR_PHY_GAIN_2GHZ_XATTEN1_DB         0x0000003F
#define AR_PHY_GAIN_2GHZ_XATTEN1_DB_S       0

#define AR9280_PHY_RXGAIN_TXRX_ATTEN	0x00003F80
#define AR9280_PHY_RXGAIN_TXRX_ATTEN_S	7
#define AR9280_PHY_RXGAIN_TXRX_MARGIN	0x001FC000
#define AR9280_PHY_RXGAIN_TXRX_MARGIN_S	14

#define	AR_PHY_SEARCH_START_DELAY	0x9918		/* search start delay */

#define AR_PHY_EXT_CCA          0x99bc
#define AR_PHY_EXT_CCA_CYCPWR_THR1      0x0000FE00
#define AR_PHY_EXT_CCA_CYCPWR_THR1_S    9
#define AR_PHY_EXT_MINCCA_PWR   0xFF800000
#define AR_PHY_EXT_MINCCA_PWR_S 23
#define AR_PHY_EXT_CCA_THRESH62	0x007F0000
#define AR_PHY_EXT_CCA_THRESH62_S	16
/*
 * This duplicates AR_PHY_EXT_CCA_CYCPWR_THR1; it reads more like
 * an ANI register this way.
 */
#define	AR_PHY_EXT_TIMING5_CYCPWR_THR1		0x0000FE00
#define	AR_PHY_EXT_TIMING5_CYCPWR_THR1_S	9

#define AR9280_PHY_EXT_MINCCA_PWR       0x01FF0000
#define AR9280_PHY_EXT_MINCCA_PWR_S     16

#define AR_PHY_HALFGI           0x99D0      /* Timing control 3 */
#define AR_PHY_HALFGI_DSC_MAN   0x0007FFF0
#define AR_PHY_HALFGI_DSC_MAN_S 4
#define AR_PHY_HALFGI_DSC_EXP   0x0000000F
#define AR_PHY_HALFGI_DSC_EXP_S 0

#define AR_PHY_HEAVY_CLIP_ENABLE    0x99E0

#define AR_PHY_HEAVY_CLIP_FACTOR_RIFS	0x99ec
#define AR_PHY_RIFS_INIT_DELAY		0x03ff0000

#define AR_PHY_M_SLEEP      0x99f0      /* sleep control registers */
#define AR_PHY_REFCLKDLY    0x99f4
#define AR_PHY_REFCLKPD     0x99f8

#define	AR_PHY_CALMODE		0x99f0
/* Calibration Types */
#define	AR_PHY_CALMODE_IQ		0x00000000
#define	AR_PHY_CALMODE_ADC_GAIN		0x00000001
#define	AR_PHY_CALMODE_ADC_DC_PER	0x00000002
#define	AR_PHY_CALMODE_ADC_DC_INIT	0x00000003
/* Calibration results */
#define	AR_PHY_CAL_MEAS_0(_i)	(0x9c10 + ((_i) << 12))
#define	AR_PHY_CAL_MEAS_1(_i)	(0x9c14 + ((_i) << 12))
#define	AR_PHY_CAL_MEAS_2(_i)	(0x9c18 + ((_i) << 12))
/* This is AR9130 and later */
#define	AR_PHY_CAL_MEAS_3(_i)	(0x9c1c + ((_i) << 12))

/*
 * AR5416 still uses AR_PHY(263) for current RSSI;
 * AR9130 and later uses AR_PHY(271).
 */
#define	AR9130_PHY_CURRENT_RSSI	0x9c3c		/* rssi of current frame rx'd */

#define AR_PHY_CCA          0x9864
#define AR_PHY_MINCCA_PWR   0x0FF80000
#define AR_PHY_MINCCA_PWR_S 19
#define AR9280_PHY_MINCCA_PWR       0x1FF00000
#define AR9280_PHY_MINCCA_PWR_S     20
#define AR9280_PHY_CCA_THRESH62     0x000FF000
#define AR9280_PHY_CCA_THRESH62_S   12

#define AR_PHY_CH1_CCA          0xa864
#define AR_PHY_CH1_MINCCA_PWR   0x0FF80000
#define AR_PHY_CH1_MINCCA_PWR_S 19
#define AR_PHY_CCA_THRESH62     0x0007F000
#define AR_PHY_CCA_THRESH62_S   12
#define AR9280_PHY_CH1_MINCCA_PWR   0x1FF00000
#define AR9280_PHY_CH1_MINCCA_PWR_S 20

#define AR_PHY_CH2_CCA          0xb864
#define AR_PHY_CH2_MINCCA_PWR   0x0FF80000
#define AR_PHY_CH2_MINCCA_PWR_S 19

#define AR_PHY_CH1_EXT_CCA          0xa9bc
#define AR_PHY_CH1_EXT_MINCCA_PWR   0xFF800000
#define AR_PHY_CH1_EXT_MINCCA_PWR_S 23
#define AR9280_PHY_CH1_EXT_MINCCA_PWR   0x01FF0000
#define AR9280_PHY_CH1_EXT_MINCCA_PWR_S 16

#define AR_PHY_CH2_EXT_CCA          0xb9bc
#define AR_PHY_CH2_EXT_MINCCA_PWR   0xFF800000
#define AR_PHY_CH2_EXT_MINCCA_PWR_S 23

#define AR_PHY_RX_CHAINMASK     0x99a4

#define	AR_PHY_NEW_ADC_DC_GAIN_CORR(_i)	(0x99b4 + ((_i) << 12))
#define	AR_PHY_NEW_ADC_GAIN_CORR_ENABLE	0x40000000
#define	AR_PHY_NEW_ADC_DC_OFFSET_CORR_ENABLE	0x80000000
#define	AR_PHY_MULTICHAIN_GAIN_CTL	0x99ac

#define	AR_PHY_EXT_CCA0			0x99b8
#define	AR_PHY_EXT_CCA0_THRESH62	0x000000FF
#define	AR_PHY_EXT_CCA0_THRESH62_S	0

#define AR_PHY_CH1_EXT_CCA          0xa9bc
#define AR_PHY_CH1_EXT_MINCCA_PWR   0xFF800000
#define AR_PHY_CH1_EXT_MINCCA_PWR_S 23

#define AR_PHY_CH2_EXT_CCA          0xb9bc
#define AR_PHY_CH2_EXT_MINCCA_PWR   0xFF800000
#define AR_PHY_CH2_EXT_MINCCA_PWR_S 23
#define AR_PHY_ANALOG_SWAP      0xa268
#define AR_PHY_SWAP_ALT_CHAIN   0x00000040
#define AR_PHY_CAL_CHAINMASK	0xa39c

#define AR_PHY_SWITCH_CHAIN_0     0x9960
#define AR_PHY_SWITCH_COM         0x9964

#define AR_PHY_RF_CTL2                  0x9824
#define AR_PHY_TX_FRAME_TO_DATA_START	0x000000FF
#define AR_PHY_TX_FRAME_TO_DATA_START_S	0
#define AR_PHY_TX_FRAME_TO_PA_ON	0x0000FF00
#define AR_PHY_TX_FRAME_TO_PA_ON_S	8

#define AR_PHY_RF_CTL3                  0x9828
#define AR_PHY_TX_END_TO_A2_RX_ON       0x00FF0000
#define AR_PHY_TX_END_TO_A2_RX_ON_S     16

#define AR_PHY_RF_CTL4                    0x9834
#define AR_PHY_RF_CTL4_TX_END_XPAB_OFF    0xFF000000
#define AR_PHY_RF_CTL4_TX_END_XPAB_OFF_S  24
#define AR_PHY_RF_CTL4_TX_END_XPAA_OFF    0x00FF0000
#define AR_PHY_RF_CTL4_TX_END_XPAA_OFF_S  16
#define AR_PHY_RF_CTL4_FRAME_XPAB_ON      0x0000FF00
#define AR_PHY_RF_CTL4_FRAME_XPAB_ON_S    8
#define AR_PHY_RF_CTL4_FRAME_XPAA_ON      0x000000FF
#define AR_PHY_RF_CTL4_FRAME_XPAA_ON_S    0

#define	AR_PHY_SYNTH_CONTROL	0x9874

#define	AR_PHY_FORCE_CLKEN_CCK	0xA22C
#define	AR_PHY_FORCE_CLKEN_CCK_MRC_MUX	0x00000040

#define AR_PHY_POWER_TX_SUB     0xA3C8
#define AR_PHY_POWER_TX_RATE5   0xA38C
#define AR_PHY_POWER_TX_RATE6   0xA390
#define AR_PHY_POWER_TX_RATE7   0xA3CC
#define AR_PHY_POWER_TX_RATE8   0xA3D0
#define AR_PHY_POWER_TX_RATE9   0xA3D4

#define	AR_PHY_TPCRG1_PD_GAIN_1 	0x00030000
#define	AR_PHY_TPCRG1_PD_GAIN_1_S	16
#define	AR_PHY_TPCRG1_PD_GAIN_2		0x000C0000
#define	AR_PHY_TPCRG1_PD_GAIN_2_S	18
#define	AR_PHY_TPCRG1_PD_GAIN_3		0x00300000
#define	AR_PHY_TPCRG1_PD_GAIN_3_S	20

#define	AR_PHY_TPCRG1_PD_CAL_ENABLE	0x00400000
#define	AR_PHY_TPCRG1_PD_CAL_ENABLE_S	22

#define AR_PHY_VIT_MASK2_M_46_61 0xa3a0
#define AR_PHY_MASK2_M_31_45     0xa3a4
#define AR_PHY_MASK2_M_16_30     0xa3a8
#define AR_PHY_MASK2_M_00_15     0xa3ac
#define AR_PHY_MASK2_P_15_01     0xa3b8
#define AR_PHY_MASK2_P_30_16     0xa3bc
#define AR_PHY_MASK2_P_45_31     0xa3c0
#define AR_PHY_MASK2_P_61_45     0xa3c4

#define	AR_PHY_SPUR_REG         0x994c
#define	AR_PHY_SFCORR_EXT	0x99c0
#define	AR_PHY_SFCORR_EXT_M1_THRESH	0x0000007F
#define	AR_PHY_SFCORR_EXT_M1_THRESH_S	0
#define	AR_PHY_SFCORR_EXT_M2_THRESH	0x00003F80
#define	AR_PHY_SFCORR_EXT_M2_THRESH_S	7
#define	AR_PHY_SFCORR_EXT_M1_THRESH_LOW	0x001FC000
#define	AR_PHY_SFCORR_EXT_M1_THRESH_LOW_S	14
#define	AR_PHY_SFCORR_EXT_M2_THRESH_LOW	0x0FE00000
#define	AR_PHY_SFCORR_EXT_M2_THRESH_LOW_S	21
#define	AR_PHY_SFCORR_SPUR_SUBCHNL_SD_S	28

/* enable vit puncture per rate, 8 bits, lsb is low rate */
#define AR_PHY_SPUR_REG_MASK_RATE_CNTL       (0xFF << 18)
#define AR_PHY_SPUR_REG_MASK_RATE_CNTL_S     18

#define AR_PHY_SPUR_REG_ENABLE_MASK_PPM      0x20000     /* bins move with freq offset */
#define AR_PHY_SPUR_REG_MASK_RATE_SELECT     (0xFF << 9) /* use mask1 or mask2, one per rate */
#define AR_PHY_SPUR_REG_MASK_RATE_SELECT_S   9
#define AR_PHY_SPUR_REG_ENABLE_VIT_SPUR_RSSI 0x100
#define AR_PHY_SPUR_REG_SPUR_RSSI_THRESH     0x7F
#define AR_PHY_SPUR_REG_SPUR_RSSI_THRESH_S   0

#define AR_PHY_PILOT_MASK_01_30   0xa3b0
#define AR_PHY_PILOT_MASK_31_60   0xa3b4

#define AR_PHY_CHANNEL_MASK_01_30 0x99d4
#define AR_PHY_CHANNEL_MASK_31_60 0x99d8

#define	AR_PHY_CL_CAL_CTL	0xA358		/* carrier leak cal control */
#define	AR_PHY_CL_CAL_ENABLE	0x00000002
#define	AR_PHY_PARALLEL_CAL_ENABLE	0x00000001

/* empirically determined "good" CCA value ranges from atheros */
#define	AR_PHY_CCA_NOM_VAL_5416_2GHZ		-90
#define	AR_PHY_CCA_NOM_VAL_5416_5GHZ		-100
#define	AR_PHY_CCA_MIN_GOOD_VAL_5416_2GHZ	-100
#define	AR_PHY_CCA_MIN_GOOD_VAL_5416_5GHZ	-110
#define	AR_PHY_CCA_MAX_GOOD_VAL_5416_2GHZ	-80
#define	AR_PHY_CCA_MAX_GOOD_VAL_5416_5GHZ	-90

/* ar9280 specific? */
#define	AR_PHY_XPA_CFG		0xA3D8
#define	AR_PHY_FORCE_XPA_CFG	0x000000001
#define	AR_PHY_FORCE_XPA_CFG_S	0

#define	AR_PHY_CCK_TX_CTRL_TX_DAC_SCALE_CCK	0x0000000C
#define	AR_PHY_CCK_TX_CTRL_TX_DAC_SCALE_CCK_S	2

#define	AR_PHY_TX_PWRCTRL9			0xa27C
#define	AR_PHY_TX_DESIRED_SCALE_CCK		0x00007C00
#define	AR_PHY_TX_DESIRED_SCALE_CCK_S		10
#define	AR_PHY_TX_PWRCTRL9_RES_DC_REMOVAL	0x80000000
#define	AR_PHY_TX_PWRCTRL9_RES_DC_REMOVAL_S	31

#define	AR_PHY_MODE_ASYNCFIFO			0x80	/* Enable async fifo */

#endif /* _DEV_ATH_AR5416PHY_H_ */
