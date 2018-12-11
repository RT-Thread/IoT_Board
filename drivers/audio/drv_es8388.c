/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-15     ZeroFree     first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <drv_es8388.h>

// #define DBG_ENABLE
#define DBG_LEVEL DBG_INFO
#define DBG_COLOR
#define DBG_SECTION_NAME    "ES8388"
#include <rtdbg.h>

/* Register Definitions */
#define ES8388_CONTROL1             (0x00)      /* 0 */
#define ES8388_CONTROL2             (0x01)      /* 1 */
#define ES8388_CHIPPOWER            (0x02)      /* 2 */
#define ES8388_ADCPOWER             (0x03)      /* 3 */
#define ES8388_DACPOWER             (0x04)      /* 4 */
#define ES8388_CHIPLOPOW1           (0x05)      /* 5 */
#define ES8388_CHIPLOPOW2           (0x06)      /* 6 */
#define ES8388_ANAVOLMANAG          (0x07)      /* 7 */
#define ES8388_MASTERMODE           (0x08)      /* 8 */
#define ES8388_ADCCONTROL1          (0x09)      /* 9 */
#define ES8388_ADCCONTROL2          (0x0a)      /* 10 */
#define ES8388_ADCCONTROL3          (0x0b)      /* 11 */
#define ES8388_ADCCONTROL4          (0x0c)      /* 12 */
#define ES8388_ADCCONTROL5          (0x0d)      /* 13 */
#define ES8388_ADCCONTROL6          (0x0e)      /* 14 */
#define ES8388_ADCCONTROL7          (0x0f)      /* 15 */
#define ES8388_ADCCONTROL8          (0x10)      /* 16 */
#define ES8388_ADCCONTROL9          (0x11)      /* 17 */
#define ES8388_ADCCONTROL10         (0x12)      /* 18 */
#define ES8388_ADCCONTROL11         (0x13)      /* 19 */
#define ES8388_ADCCONTROL12         (0x14)      /* 20 */
#define ES8388_ADCCONTROL13         (0x15)      /* 21 */
#define ES8388_ADCCONTROL14         (0x16)      /* 22 */
#define ES8388_DACCONTROL1          (0x17)      /* 23 */
#define ES8388_DACCONTROL2          (0x18)      /* 24 */
#define ES8388_DACCONTROL3          (0x19)      /* 25 */
#define ES8388_DACCONTROL4          (0x1a)      /* 26 */
#define ES8388_DACCONTROL5          (0x1b)      /* 27 */
#define ES8388_DACCONTROL6          (0x1c)      /* 28 */
#define ES8388_DACCONTROL7          (0x1d)      /* 29 */
#define ES8388_DACCONTROL8          (0x1e)      /* 30 */
#define ES8388_DACCONTROL9          (0x1f)      /* 31 */
#define ES8388_DACCONTROL10         (0x20)      /* 32 */
#define ES8388_DACCONTROL11         (0x21)      /* 33 */
#define ES8388_DACCONTROL12         (0x22)      /* 34 */
#define ES8388_DACCONTROL13         (0x23)      /* 35 */
#define ES8388_DACCONTROL14         (0x24)      /* 36 */
#define ES8388_DACCONTROL15         (0x25)      /* 37 */
#define ES8388_DACCONTROL16         (0x26)      /* 38 */
#define ES8388_DACCONTROL17         (0x27)      /* 39 */
#define ES8388_DACCONTROL18         (0x28)      /* 40 reserved. */
#define ES8388_DACCONTROL19         (0x29)      /* 41 reserved. */
#define ES8388_DACCONTROL20         (0x2a)      /* 42 */
#define ES8388_DACCONTROL21         (0x2b)      /* 43 */
#define ES8388_DACCONTROL22         (0x2c)      /* 44 */
#define ES8388_DACCONTROL23         (0x2d)      /* 45 */
#define ES8388_DACCONTROL24         (0x2e)      /* 46 */
#define ES8388_DACCONTROL25         (0x2f)      /* 47 */
#define ES8388_DACCONTROL26         (0x30)      /* 48 */
#define ES8388_DACCONTROL27         (0x31)      /* 49 */
#define ES8388_DACCONTROL28         (0x32)      /* 50 */
#define ES8388_DACCONTROL29         (0x33)      /* 51 */
#define ES8388_DACCONTROL30         (0x34)      /* 52 */

#define OUTPUT_MUTED                (0x92)
#define OUTPUT_0DB                  (0x00)

#define ES8388_LADC_VOL             ES8388_ADCCONTROL8
#define ES8388_RADC_VOL             ES8388_ADCCONTROL9
#define ES8388_LDAC_VOL             ES8388_DACCONTROL4
#define ES8388_RDAC_VOL             ES8388_DACCONTROL5
#define ES8388_LOUT1_VOL            ES8388_DACCONTROL24
#define ES8388_ROUT1_VOL            ES8388_DACCONTROL25
#define ES8388_LOUT2_VOL            ES8388_DACCONTROL26
#define ES8388_ROUT2_VOL            ES8388_DACCONTROL27
#define ES8388_ADC_MUTE             ES8388_ADCCONTROL7
#define ES8388_DAC_MUTE             ES8388_DACCONTROL3
#define ES8388_ADCIN                ES8388_ADCPOWER
#define ES8388_IFACE                ES8388_MASTERMODE
#define ES8388_ADC_IFACE            ES8388_ADCCONTROL4
#define ES8388_ADC_SRATE            ES8388_ADCCONTROL5
#define ES8388_DAC_IFACE            ES8388_DACCONTROL1
#define ES8388_DAC_SRATE            ES8388_DACCONTROL2

#define ES8388_CACHEREGNUM          (53)
#define ES8388_SYSCLK               (0)

#define ES8388_PLL1                 (0)
#define ES8388_PLL2                 (1)

/* clock inputs */
#define ES8388_MCLK                 (0)
#define ES8388_PCMCLK               (1)

/* clock divider id's */
#define ES8388_PCMDIV               (0)
#define ES8388_BCLKDIV              (1)
#define ES8388_VXCLKDIV             (2)

/* PCM clock dividers */
#define ES8388_PCM_DIV_1            (0 << 6)
#define ES8388_PCM_DIV_3            (2 << 6)
#define ES8388_PCM_DIV_5_5          (3 << 6)
#define ES8388_PCM_DIV_2            (4 << 6)
#define ES8388_PCM_DIV_4            (5 << 6)
#define ES8388_PCM_DIV_6            (6 << 6)
#define ES8388_PCM_DIV_8            (7 << 6)

/* BCLK clock dividers */
#define ES8388_BCLK_DIV_1           (0 << 7)
#define ES8388_BCLK_DIV_2           (1 << 7)
#define ES8388_BCLK_DIV_4           (2 << 7)
#define ES8388_BCLK_DIV_8           (3 << 7)

/* VXCLK clock dividers */
#define ES8388_VXCLK_DIV_1          (0 << 6)
#define ES8388_VXCLK_DIV_2          (1 << 6)
#define ES8388_VXCLK_DIV_4          (2 << 6)
#define ES8388_VXCLK_DIV_8          (3 << 6)
#define ES8388_VXCLK_DIV_16         (4 << 6)

#define ES8388_DAI_HIFI             (0)
#define ES8388_DAI_VOICE            (1)

#define ES8388_1536FS               (1536)
#define ES8388_1024FS               (1024)
#define ES8388_768FS                (768)
#define ES8388_512FS                (512)
#define ES8388_384FS                (384)
#define ES8388_256FS                (256)
#define ES8388_128FS                (128)

/* REGISTER 0 – CHIP CONTROL 1, DEFAULT 0000 0110 */
#define R00_VMID_DISABLE            (0)
#define R00_VMID_50K                (1)
#define R00_VMID_500K               (2)
#define R00_VMID_5K                 (3)
#define R00_REF_EN                  (1 << 2)
#define R00_REF_DIS                 (0 << 2)

/* REGISTER 1 – CHIP CONTROL 2, DEFAULT 0001 1100 */
#define R01_VREFBUF_ON              (0 << 0)
#define R01_VREFBUF_PDN             (1 << 0)
#define R01_VREF_ON                 (0 << 1)
#define R01_VREF_PDN                (1 << 1)
#define R01_IBIASGEN_ON             (0 << 2)
#define R01_IBIASGEN_PDN            (1 << 2)
#define R01_ANA_ON                  (0 << 3)
#define R01_ANA_PND                 (0 << 3)
#define R01_VREFBUF_LP              (1 << 4)
#define R01_VCMMOD_LP               (1 << 5)

/* REGISTER 2 – CHIP POWER MANAGEMENT, DEFAULT 1100 0011 */
#define R02_DACVREF_UP              (0 << 0)
#define R02_DACVREF_DOWN            (1 << 0)
#define R02_ADCVREF_UP              (0 << 1)
#define R02_ADCVREF_DOWN            (1 << 1)
#define R02_DACDLL_UP               (0 << 2)    /* only ES8388 */
#define R02_DACDLL_DOWN             (1 << 2)    /* only ES8388 */
#define R02_ADCDLL_UP               (0 << 3)    /* only ES8388 */
#define R02_ADCDLL_DOWN             (1 << 3)    /* only ES8388 */
#define R02_ADC_STM_RST             (1 << 4)    /* ADC state machine */
#define R02_DAC_STM_RST             (1 << 5)    /* DAC state machine */
#define R02_DAC_DIG_PDN             (1 << 6)    /* resets DAC DSM, DEM, filter and serial data port */
#define R02_ADC_DIG_PDN             (1 << 7)    /* resets ADC DEM, filter and serial data port */

/* REGISTER 3 – ADC POWER MANAGEMENT, DEFAULT 1111 1100 */
#define R03_INT_NORMAL              (0 << 0)
#define R03_INT_LP                  (1 << 0)
#define R03_ADC_NORMAL              (0 << 1)
#define R03_ADC_LP                  (1 << 1)
#define R03_ADC_BIASGEN_NORMAL      (0 << 2)
#define R03_ADC_BIASGEN_PND         (1 << 2)
#define R03_MICB_NORMAL             (0 << 3)
#define R03_MICB_PND                (1 << 3)
#define R03_ADCR_NORMAL             (0 << 4) /* ES8388 only. */
#define R03_ADCR_PND                (1 << 4) /* ES8388 only. */
#define R03_ADCL_NORMAL             (0 << 5)
#define R03_ADCL_PND                (1 << 5)
#define R03_AINR_NORMAL             (0 << 6) /* ES8388 only. */
#define R03_AINR_PND                (1 << 6) /* ES8388 only. */
#define R03_AINL_NORMAL             (0 << 7)
#define R03_AINL_PND                (1 << 7)

/* REGISTER 4 – DAC POWER MANAGEMENT, DEFAULT 1100 0000 */
#define R04_ROUT_DIS                (0 << 2) /* ES8388S only. */
#define R04_ROUT_EN                 (1 << 2) /* ES8388S only. */
#define R04_LOUT_DIS                (0 << 3) /* ES8388S only. */
#define R04_LOUT_EN                 (1 << 3) /* ES8388S only. */
#define R04_ROUT2_DIS               (0 << 2) /* ES8388 only. */
#define R04_ROUT2_EN                (1 << 2) /* ES8388 only. */
#define R04_LOUT2_DIS               (0 << 3) /* ES8388 only. */
#define R04_LOUT2_EN                (1 << 3) /* ES8388 only. */
#define R04_ROUT1_DIS               (0 << 4) /* ES8388 only. */
#define R04_ROUT1_EN                (1 << 4) /* ES8388 only. */
#define R04_LOUT1_DIS               (0 << 5) /* ES8388 only. */
#define R04_LOUT1_EN                (1 << 5) /* ES8388 only. */
#define R04_DACR_UP                 (0 << 6)
#define R04_DACR_PDN                (1 << 6)
#define R04_DACL_UP                 (0 << 7)
#define R04_DACL_PDN                (1 << 7)

/* REGISTER 9 */
#define R09_MICAMPL_DB(db)          ((db/3) << 4)
#define R09_MICAMPR_DB(db)          ((db/3) << 0) /* ES8388 only. */

/* REGISTER 10 */
#define R10_CAP_MODE_DIS            (0 << 0) /* ES8388S only. */
#define R10_CAP_MODE_EN             (1 << 0) /* ES8388S only. */
#define R10_DSR_LIN1_RIN1           (0 << 2) /* ES8388 only. */
#define R10_DSR_LIN2_RIN2           (1 << 2) /* ES8388 only. */
#define R10_DSSEL_ONE               (0 << 3) /* ES8388 only. */
#define R10_DSSEL_TWO               (1 << 3) /* ES8388 only. */
#define R10_RINSEL_RIN1             (0 << 4) /* ES8388 only. */
#define R10_RINSEL_RIN2             (1 << 4) /* ES8388 only. */
#define R10_RINSEL_DIFFERENTIAL     (3 << 4) /* ES8388 only. */
#define R10_LINSEL_LIN1             (0 << 6)
#define R10_LINSEL_LIN2             (1 << 6)
#define R10_LINSEL_DIFFERENTIAL     (3 << 6)

/* REGISTER 11 */
#define R11_ASDOUT_EN               (0 << 2)
#define R11_ASDOUT_DIS              (1 << 2)
#define R11_LDCM_DIS                (0 << 6)
#define R11_LDCM_EN                 (1 << 6)
#define R11_DS_LIN1_RIN1            (0 << 7)
#define R11_DS_LIN2_LIN2            (1 << 7)

/* REGISTER 12 */
#define R12_ADC_FMT_I2S             (0 << 0)
#define R12_ADC_FMT_LEFT            (1 << 0)
#define R12_ADC_FMT_PCM             (3 << 0)
#define R12_ADCWL_24BIT             (0 << 2)
#define R12_ADCWL_20BIT             (1 << 2)
#define R12_ADCWL_18BIT             (2 << 2)
#define R12_ADCWL_16BIT             (3 << 2)
#define R12_ADCWL_32BIT             (4 << 2)

/* REGISTER 13 */
#define R13_ADC_FS_256              (0x02 << 0)
#define R13_ADC_FS_1500             (0x1B << 0)
#define R13_ADC_DUAL_MODE           (1 << 5)
#define R13_ADC_RATIO_AUTO          (0 << 6) /* ES8388S only. */
#define R13_ADC_RATIO_MANUAL        (1 << 6) /* ES8388S only. */

/* REGISTER 14 */
#define R14_ADC_RHPF_DIS            (0 << 4) /* ES8388 only. */
#define R14_ADC_RHPF_EN             (1 << 4) /* ES8388 only. */
#define R14_ADC_LHPF_DIS            (0 << 5)
#define R14_ADC_LHPF_EN             (1 << 5)
#define R14_ADC_R_INVERTED          (1 << 6) /* ES8388 only. */
#define R14_ADC_L_INVERTED          (1 << 7)

/* REGISTER 15 */
#define R15_ADC_UNMUTE              (0 << 2)
#define R15_ADC_MUTE                (1 << 2)
#define R15_ADC_SOFT_RAMP_DIS       (0 << 5)
#define R15_ADC_SOFT_RAMP_EN        (1 << 5)
#define R15_ADC_RAMP_4LRCK          (0 << 6)
#define R15_ADC_RAMP_8LRCK          (1 << 6)
#define R15_ADC_RAMP_16LRCK         (2 << 6)
#define R15_ADC_RAMP_32LRCK         (3 << 6)

/* REGISTER 18 */
#define R18_ADC_ALC_OFF             (0 << 6)
#define R18_ADC_ALC_RIGHT           (1 << 6)
#define R18_ADC_ALC_LEFT            (2 << 6)
#define R18_ADC_ALC_STEREO          (3 << 6)
#define R18_ALC_MAXGAIN_N6_5DB      (0 << 3)
#define R18_ALC_MAXGAIN_N0_5DB      (1 << 3)
#define R18_ALC_MAXGAIN_11_5DB      (3 << 3)
#define R18_ALC_MAXGAIN_17_5DB      (4 << 3)
#define R18_ALC_MAXGAIN_23_5DB      (5 << 3)
#define R18_ALC_MAXGAIN_29_5DB      (6 << 3)
#define R18_ALC_MAXGAIN_35_5DB      (7 << 3)
#define R18_ALC_MINGAIN_N12DB       (0 << 0)
#define R18_ALC_MINGAIN_N6DB        (1 << 0)
#define R18_ALC_MINGAIN_0DB         (2 << 0)
#define R18_ALC_MINGAIN_6DB         (3 << 0)
#define R18_ALC_MINGAIN_12DB        (4 << 0)

/* REGISTER 19 */
#define R19_ADC_ALC_LVL_DB(db)  ((int)((db+16.5)/1.5) << 4)
#define R19_ADC_ALC_HLD(hld)    (hld << 0)

/* REGISTER 20 */
#define R20_ADC_ALCDCY_410US        (0 << 4)
#define R20_ADC_ALCDCY_820US        (1 << 4)
#define R20_ADC_ALCATK_104US        (0 << 0)
#define R20_ADC_ALCATK_416US        (2 << 0)

/* REGISTER 21 */
#define R21_ADC_ALCMODE             (0 << 7)
#define R21_ADC_ALCZC_OFF           (0 << 6)
#define R21_ADC_TIME_OUT_OFF        (0 << 5)
#define R21_ADC_ALC_WIN_SIZE_96     (6 << 0)
#define R21_ADC_ALC_WIN_SIZE_102    (7 << 0)

/* REGISTER 22 */
#define R22_ADC_ALC_NGAT_DIS        (0 << 0)
#define R22_ADC_ALC_NGAT_EN         (1 << 0)
#define R22_ADC_ALC_NGG_PGA_HELD    (0 << 1)
#define R22_ADC_ALC_NGG_MUTE_ADC    (1 << 1)
#define R22_ADC_ALC_NGTH_N73_5DB    (2 << 3)
#define R22_ADC_ALC_NGTH_N51DB      (17 << 3)
#define R22_ADC_ALC_NGTH_N40_5DB    (24 << 3)
#define R22_ADC_ALC_NGTH_N39DB      (25 << 3)
#define R22_ADC_ALC_NGTH_N37_5DB    (26 << 3)
#define R22_ADC_ALC_NGTH_N36DB      (27 << 3)
#define R22_ADC_ALC_NGTH_N34_5DB    (28 << 3)
#define R22_ADC_ALC_NGTH_N33DB      (29 << 3)
#define R22_ADC_ALC_NGTH_N31_5DB    (30 << 3)
#define R22_ADC_ALC_NGTH_N30DB      (31 << 3)
#define R22_ADC_ALC_NGTH_DB(db)     ((int)((db + 76.5)/1.5) << 3) /* -30 ~ -76.5, 1.5db/setp */

/* REGISTER 23 */
#define R23_DAC_FMT_I2S         (0 << 0)
#define R23_DAC_FMT_LEFT        (1 << 0)
#define R23_DAC_FMT_PCM         (3 << 0)
#define R23_DAC_DACWL_24BIT     (0 << 3)
#define R23_DAC_DACWL_20BIT     (1 << 3)
#define R23_DAC_DACWL_18BIT     (2 << 3)
#define R23_DAC_DACWL_16BIT     (3 << 3)
#define R23_DAC_DACWL_32BIT     (4 << 3)
#define R23_DAC_LRP_INVERTED    (1 << 6)
#define R23_DAC_DACLRSWAP       (1 << 7)

/* REGISTER 24 */
#define R24_DAC_FS_256          (0x02 << 0)
#define R24_DAC_FS_1500         (0x1B << 0)
#define R24_DAC_RATIO_AUTO      (0 << 6)

/* REGISTER 25 */
#define R25_DAC_UNMUTE          (0 << 2)
#define R25_DAC_MUTE            (1 << 2)
#define R25_DAC_LeR             (1 << 3) /* both channel gain control is set by DAC left gain control register */
#define R25_DAC_SOFT_RAMP_DIS   (0 << 5)
#define R25_DAC_SOFT_RAMP_EN    (1 << 5)
#define R25_DAC_RAMP_4LRCK      (0 << 6)
#define R25_DAC_RAMP_32LRCK     (1 << 6)
#define R25_DAC_RAMP_64LRCK     (2 << 6)
#define R25_DAC_RAMP_128LRCK    (3 << 6)

/* REGISTER 26 - LDAC Digital volume control */
/* REGISTER 27 - RDAC Digital volume control */

/* REGISTER 38 */
#define R38_RMIXSEL_LIN1        (0 << 0)
#define R38_RMIXSEL_LIN2        (1 << 0)
#define R38_RMIXSEL_DF2SE       (2 << 0)
#define R38_RMIXSEL_ADC         (3 << 0)
#define R38_LMIXSEL_LIN1        (0 << 3)
#define R38_LMIXSEL_LIN2        (1 << 3)
#define R38_LMIXSEL_DF2SE       (2 << 3)
#define R38_LMIXSEL_ADC         (3 << 3)

/* REGISTER 39 */
#define R39_MIXBOTH             (1 << 2) /* ES8388S only. */
#define R39_LI2LOVOL_GAIN_6DB   (0 << 3)
#define R39_LI2LOVOL_GAIN_3DB   (1 << 3)
#define R39_LI2LOVOL_GAIN_0DB   (2 << 3)
#define R39_LI2LOVOL_GAIN_N3DB  (3 << 3)
#define R39_LI2LOVOL_GAIN_N6DB  (4 << 3)
#define R39_LI2LOVOL_GAIN_N9DB  (5 << 3)
#define R39_LI2LOVOL_GAIN_N12DB (6 << 3)
#define R39_LI2LOVOL_GAIN_N15DB (7 << 3) /* default */
#define R39_LI2LO_DIS           (0 << 6) /* LIN to left mixer */
#define R39_LI2LO_EN            (1 << 6)
#define R39_LD2LO_DIS           (0 << 7) /* left DAC to left mixer */
#define R39_LD2LO_EN            (1 << 7)

/* REGISTER 42 */
#define R42_RI2ROVOL_GAIN_6DB   (0 << 3)
#define R42_RI2ROVOL_GAIN_3DB   (1 << 3)
#define R42_RI2ROVOL_GAIN_0DB   (2 << 3)
#define R42_RI2ROVOL_GAIN_N3DB  (3 << 3)
#define R42_RI2ROVOL_GAIN_N6DB  (4 << 3)
#define R42_RI2ROVOL_GAIN_N9DB  (5 << 3)
#define R42_RI2ROVOL_GAIN_N12DB (6 << 3)
#define R42_RI2ROVOL_GAIN_N15DB (7 << 3) /* default */
#define R42_RI2RO_DIS           (0 << 6) /* RIN to right mixer */
#define R42_RI2RO_EN            (1 << 6)
#define R42_RD2RO_DIS           (0 << 7) /* right DAC to right mixer */
#define R42_RD2RO_EN            (1 << 7)

/* REGISTER 43 */
#define R43_DAC_ANACLK_UP       (0 << 2)
#define R43_DAC_ANACLK_PND      (1 << 2)
#define R43_ADC_ANACLK_UP       (0 << 3)
#define R43_ADC_ANACLK_PND      (1 << 3)
#define R43_MCLK_EN             (0 << 4)
#define R43_MCLK_DIS            (1 << 4)
#define R43_OFFSET_EN           (0 << 5)
#define R43_OFFSET_DIS          (1 << 5)
#define R43_LRCK_SEL_DAC        (0 << 6) /* LRCK select if slrck = 1 */
#define R43_LRCK_SEL_ADC        (1 << 6)
#define R43_SLRCK_SAME          (1 << 7)

static uint8_t read_reg(struct rt_i2c_bus_device *dev, uint8_t address)
{
    struct rt_i2c_msg msg[2];
    uint8_t data;

    RT_ASSERT(dev != RT_NULL);

    msg[0].addr = 0x10; /* CE pull-up */
    msg[0].flags = RT_I2C_WR;
    msg[0].len = 1;
    msg[0].buf = &address;

    msg[1].addr = 0x10; /* CE pull-up */
    msg[1].flags = RT_I2C_RD;
    msg[1].len = 1;
    msg[1].buf = &data;

    rt_i2c_transfer(dev, &msg[0], 2);

    return data;
}

static void write_reg_nocheck(struct rt_i2c_bus_device *dev, uint8_t address, uint8_t data)
{
    struct rt_i2c_msg msg;
    rt_uint8_t send_buffer[2];
    int result;

    LOG_D("write 0x%02X ==> %d\n", data, address);

    RT_ASSERT(dev != RT_NULL);

    send_buffer[0] = address;
    send_buffer[1] = data;

    msg.addr = 0x10; /* CE pull-up */
    msg.flags = RT_I2C_WR;
    msg.len = 2;
    msg.buf = send_buffer;

    result = rt_i2c_transfer(dev, &msg, 1);
    if(result != 1)
    {
        LOG_E("i2c error, result %d, reg %X, val %x", result, address, data);
    }

}

static void write_reg(struct rt_i2c_bus_device *dev, uint8_t address, uint8_t data)
{
    uint8_t tmp;
    int retry = 100;

    write_reg_nocheck(dev, address, data);

    while (1)
    {
        tmp = read_reg(dev, address);
        if (tmp == data)
        {
            LOG_D("check pass, %d %02X:%02X\n", address, data, tmp);
            break;
        }

        LOG_D("check %d %02X:%02X\n", address, data, tmp);

        if (!retry--)
        {
            LOG_W("register address %d check timeout!, %02X:%02X\n", address, data, tmp);
            break;
        }

        rt_thread_mdelay(1);
    }
}

void es8388_set_volume(struct rt_i2c_bus_device *dev, uint16_t v) // 0~99
{
    if (v > 100)
        v = 100;

    v = 33 * v / 100; /* 0~33 */

    LOG_I("ES8388S volume set to %d\n", v);

#if 1
    /*
    ES8388  use R46/R47(OUT1VOL) and R48/R49(OUT2VOL),
    ES8388S use R48/R49(OUTVOL).
    */

    write_reg_nocheck(dev, 46, v);
    write_reg_nocheck(dev, 47, v);
#endif

    write_reg(dev, 48, v);
    write_reg(dev, 49, v);
}

void es8388_reset(struct rt_i2c_bus_device *dev)
{
    write_reg_nocheck(dev, ES8388_CONTROL1, 0x80);
    rt_thread_mdelay(10);
    write_reg_nocheck(dev, ES8388_CONTROL1, 0x00);
    rt_thread_mdelay(10);
}

void es8388_suspend(struct rt_i2c_bus_device *dev)
{
    write_reg_nocheck(dev, 0x0F, 0x34);  // Reg 15 0x0F = 0x34 (ADC Mute), ES8388S=0x24
    write_reg_nocheck(dev, 0x19, 0x36);  // Reg 25 0x19 = 0x36 (DAC Mute), ES8388S=0x26
    write_reg(dev, 0x02, 0xF3);  // Reg 02 0x02 = 0xF3 Power down DEM and STM.
    write_reg_nocheck(dev, 0x03, 0xFC);  // Reg 03 0x03 = 0xFC Power Down ADC / Analog Input / Micbias for Record. ES8388S=0xAC
    write_reg(dev, 0x04, 0xC0);  // Reg 04 0x04 = 0xC0 Power down DAC and disable LOUT/ROUIT.

    write_reg(dev, 0x2B, 0x90);
    write_reg_nocheck(dev, 0x05, 0xFF);  // 0xFF, ES8388S=0xE8
    write_reg_nocheck(dev, 0x06, 0xFF);  // 0xFF, ES8388S=0xC3
    write_reg(dev, 0x27, 0x38);
    write_reg(dev, 0x2A, 0x38);
    write_reg(dev, 0x01, 0x7A);  // 3A ==> 7A
}

int es8388_init(struct rt_i2c_bus_device *dev)
{
    // set chip to slave mode
    write_reg(dev, ES8388_MASTERMODE, 0x00);       // slave mode.

    // power down DEM and STM of ADC and DAC
    write_reg(dev, 2, R02_DACVREF_DOWN | R02_ADCVREF_DOWN | R02_ADC_STM_RST | R02_DAC_STM_RST | R02_DAC_DIG_PDN | R02_ADC_DIG_PDN);

    // set DACLRC and ADCLRC same
    write_reg(dev, 43, R43_SLRCK_SAME | R43_DAC_ANACLK_UP | R43_ADC_ANACLK_UP | R43_MCLK_EN | R43_OFFSET_EN);
    // set chip to play&record mode
    //write_reg(dev, 0, R00_VMID_500K | R00_REF_EN);
    write_reg(dev, 0, 0x36);
    // power up analog and lbias
    write_reg_nocheck(dev, 1, R01_VREFBUF_ON | R01_VREF_ON | R01_IBIASGEN_ON | R01_ANA_ON);

    // Power up DAC and analog Output. R04_ROUT1_EN | R04_LOUT1_EN |
    //write_reg(dev, 4, R04_ROUT_EN | R04_LOUT_EN | R04_DACR_UP | R04_DACL_UP);
    write_reg_nocheck(dev, 4, R04_ROUT_EN | R04_LOUT_EN | R04_ROUT1_EN | R04_LOUT1_EN | R04_DACR_UP | R04_DACL_UP);

    write_reg(dev, 23, R23_DAC_FMT_I2S | R23_DAC_DACWL_16BIT);
    write_reg(dev, 24, R24_DAC_RATIO_AUTO); /*  */

    // Set DAC Digital Volume
    write_reg(dev, 26, 0x00); /* left  DAC digital volume control, 0DB. */
    write_reg(dev, 27, 0x00); /* right DAC digital volume control, 0DB. */

    // UnMute DAC
    write_reg(dev, 25, R25_DAC_UNMUTE | R25_DAC_SOFT_RAMP_DIS | R25_DAC_RAMP_4LRCK);

    /******************* ADC config begin *******************/
    // Power up ADC / Analog Input / Micbias for Record
    write_reg(dev, 3, R03_INT_NORMAL | R03_ADC_NORMAL | R03_ADC_BIASGEN_NORMAL | R03_MICB_NORMAL | R03_ADCR_NORMAL | R03_ADCL_NORMAL | R03_AINR_NORMAL | R03_AINL_NORMAL);
    // Select Analog input channel for ADC,  R10_RINSEL_DIFFERENTIAL | R10_DSSEL_ONE
    write_reg(dev, 10, R10_CAP_MODE_DIS | R10_LINSEL_DIFFERENTIAL);
    write_reg(dev, 11, R11_ASDOUT_EN | R11_DS_LIN1_RIN1 | 0x02); // default 110 => 010
    // Select  PGA  Gain  for  ADC analog input
    write_reg(dev, 9, R09_MICAMPL_DB(24));  /* 0~24db. ==> max33 */
    // digital interface.
    write_reg(dev, 12, R12_ADC_FMT_I2S | R12_ADCWL_16BIT);
    write_reg(dev, 13, R13_ADC_RATIO_AUTO); /* ADC ratio selection for slave mode. */
    // Set ADC Digital Volume
    write_reg(dev, 16, 0); /* left  ADC Digital Volume */
    write_reg(dev, 17, 0); /* right ADC Digital volume, ES8388 only. */
    // UnMute ADC
    write_reg(dev, 15, R15_ADC_UNMUTE | R15_ADC_SOFT_RAMP_DIS | R15_ADC_RAMP_4LRCK);

    // Set ALC mode if necessary
    write_reg(dev, 14, R14_ADC_LHPF_EN);  /* R14_ADC_RHPF_EN */
    write_reg(dev, 19, R19_ADC_ALC_LVL_DB(-1.5) | R19_ADC_ALC_HLD(0));
    write_reg(dev, 20, R20_ADC_ALCDCY_410US | R20_ADC_ALCATK_104US); // 48=> 8, 1:6
    write_reg(dev, 21, R21_ADC_ALCMODE | R21_ADC_ALCZC_OFF | R21_ADC_TIME_OUT_OFF | R21_ADC_ALC_WIN_SIZE_96);
    write_reg(dev, 22, R22_ADC_ALC_NGAT_EN | R22_ADC_ALC_NGTH_DB(-33) | R22_ADC_ALC_NGG_PGA_HELD); /* -30 ~ -76.5, 1.5db/setp */
    write_reg(dev, 18, R18_ADC_ALC_LEFT | R18_ALC_MAXGAIN_23_5DB | R18_ALC_MINGAIN_0DB);
    /******************* ADC config end *******************/

    // Set Mixer for PA Output
    write_reg(dev, 38, R38_RMIXSEL_ADC | R38_LMIXSEL_ADC);
    write_reg(dev, 39, R39_LI2LO_DIS | R39_LI2LOVOL_GAIN_0DB | R39_LD2LO_EN);
    //write_reg(dev, 40, 0x28); //@TODO: no description, write 38 but return 28
    //write_reg(dev, 41, 0x28); //@TODO: no description, write 38 but return 28
    write_reg(dev, 42, R42_RI2RO_DIS | R42_RI2ROVOL_GAIN_0DB | R42_RD2RO_EN);

    // Set output volume.
    es8388_set_volume(dev, 0);

    // Power up DEM and STM
    write_reg(dev, 2, R02_DACVREF_UP | R02_ADCVREF_UP | R02_DACDLL_UP | R02_ADCDLL_UP);

    // Set output volume.
    es8388_set_volume(dev, 65);

    return RT_EOK;
}
