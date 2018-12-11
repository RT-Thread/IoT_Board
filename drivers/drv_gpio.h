/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-01-05     Bernard      the first version
 */
#ifndef __GPIO_H__
#define __GPIO_H__

#include "board.h"

#if HARDWARE_VERSION == 0x0200U
// EXTERNAL MODULE
// uart
#define PIN_UART2_TX  25        // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX  26        // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc
#define PIN_ADC12_IN9 29        // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2
#define PIN_SPI2_CS   51        // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK  52        // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO 53        // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI 54        // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c
#define PIN_I2C1_SCL  95        // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA  96        // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer
#define PIN_TIM3_CH1  63        // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2  64        // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io
#define PIN_IO_PD12   59        // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13   60        // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14   61        // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15   62        // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8    67        // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3    84        // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0    97        // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1    98        // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR
#define PIN_MOTOR_A   23        // PA0 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B   24        // PA1 :  MOTOR_B      --> MOTOR

// ATK MODULE
#define PIN_GBC_LED   17        // PC2 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY   18        // PC3 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX    33        // PC4 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX    34        // PC5 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY
#define PIN_BEEP      37        // PB2 :  BEEP         --> BEEP
#define PIN_LED_R     38        // PE7 :  LED_R        --> LED
#define PIN_LED_G     39        // PE8 :  LED_G        --> LED
#define PIN_LED_B     40        // PE9 :  LED_B        --> LED
#define PIN_KEY0      55        // PD8 :  KEY0         --> KEY
#define PIN_KEY1      56        // PD9 :  KEY1         --> KEY
#define PIN_KEY2      57        // PD10:  KEY2         --> KEY
#define PIN_WK_UP     58        // PD11:  WK_UP        --> KEY

// INFRARED
#define PIN_EMISSION  35        // PB0 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION 36        // PB1 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR
#define PIN_AP_INT    47        // PB10:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT   48        // PB11:  ICM_INT      --> AXIS SENSOR

// AUDIO
#define PIN_AUDIO_PWR 77        // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS
#define PIN_NRF_IRQ   85        // PD4 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE    86        // PD5 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS    87        // PD6 :  NRF_CS       --> WIRELESS

// spi1 cs
#define PIN_SD_CS     18        // PC13:  SD_CS        --> SD_CARD
// spi3 cs
#define PIN_LCD_CS    88        // PD7 :  LCD_CS       --> LCD

// WiFi IRQ
#define PIN_WIFI_IRQ  81        // PD0 :  WIFI_INT     --> WIFI

#elif HARDWARE_VERSION == 0x0201U

// EXTERNAL MODULE
// uart
#define PIN_UART2_TX  25        // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX  26        // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc
#define PIN_ADC12_IN9 29        // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2
#define PIN_SPI2_CS   51        // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK  52        // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO 53        // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI 54        // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c
#define PIN_I2C1_SCL  95        // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA  96        // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer
#define PIN_TIM3_CH1  63        // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2  64        // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io
#define PIN_IO_PD12   59        // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13   60        // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14   61        // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15   62        // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8    67        // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3    84        // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0    97        // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1    98        // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR
#define PIN_MOTOR_A   24        // PA1 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B   23        // PA0 :  MOTOR_B      --> MOTOR

// ATK MODULE
#define PIN_GBC_LED   97        // PE0 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY   98        // PE1 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX    25        // PA2 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX    26        // PA3 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY
#define PIN_BEEP      35        // PB0 :  BEEP         --> BEEP
#define PIN_LED_R     38        // PE7 :  LED_R        --> LED
#define PIN_LED_B     39        // PE8 :  LED_B        --> LED
#define PIN_LED_G     40        // PE9 :  LED_G        --> LED
#define PIN_KEY0      55        // PD8 :  KEY0         --> KEY
#define PIN_KEY1      56        // PD9 :  KEY1         --> KEY
#define PIN_KEY2      57        // PD10:  KEY2         --> KEY
#define PIN_WK_UP     58        // PD11:  WK_UP        --> KEY

// INFRARED
#define PIN_EMISSION  36        // PB1 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION 37        // PB2 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR
#define PIN_AP_INT     7        // PC13:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT   17        // PC2 :  ICM_INT      --> AXIS SENSOR

// AUDIO
#define PIN_AUDIO_PWR 77        // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS
#define PIN_NRF_IRQ   85        // PD4 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE    86        // PD5 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS    87        // PD6 :  NRF_CS       --> WIRELESS

// spi1 cs
#define PIN_SD_CS     18        // PC3 :  SD_CS        --> SD_CARD
// spi3 cs
#define PIN_LCD_CS    88        // PD7 :  LCD_CS       --> LCD

// WiFi IRQ
#define PIN_WIFI_IRQ  81        // PD0 :  WIFI_INT     --> WIFI

#elif HARDWARE_VERSION == 0x0202U

// EXTERNAL MODULE
// uart
#define PIN_UART2_TX  25        // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX  26        // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc
#define PIN_ADC12_IN9 29        // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2
#define PIN_SPI2_CS   51        // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK  52        // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO 53        // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI 54        // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c
#define PIN_I2C1_SCL  95        // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA  96        // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer
#define PIN_TIM3_CH1  63        // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2  64        // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io
#define PIN_IO_PD12   59        // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13   60        // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14   61        // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15   62        // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8    67        // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3    84        // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0    97        // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1    98        // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR
#define PIN_MOTOR_A   24        // PA1 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B   23        // PA0 :  MOTOR_B      --> MOTOR

// ATK MODULE
#define PIN_GBC_LED   97        // PE0 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY   98        // PE1 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX    25        // PA2 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX    26        // PA3 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY
#define PIN_BEEP      37        // PB2 :  BEEP         --> BEEP
#define PIN_LED_R     38        // PE7 :  LED_R        --> LED
#define PIN_LED_G     39        // PE8 :  LED_B        --> LED
#define PIN_LED_B     40        // PE9 :  LED_G        --> LED
#define PIN_KEY0      55        // PD8 :  KEY0         --> KEY
#define PIN_KEY1      56        // PD9 :  KEY1         --> KEY
#define PIN_KEY2      57        // PD10:  KEY2         --> KEY
#define PIN_WK_UP      7        // PC13:  WK_UP        --> KEY

// INFRARED
#define PIN_EMISSION  35        // PB0 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION 36        // PB1 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR
#define PIN_AP_INT    58        // PD11:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT   17        // PC2 :  ICM_INT      --> AXIS SENSOR

// AUDIO
#define PIN_AUDIO_PWR 77        // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS
#define PIN_NRF_IRQ   85        // PD4 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE    86        // PD5 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS    87        // PD6 :  NRF_CS       --> WIRELESS

// spi1 cs
#define PIN_SD_CS     18        // PC3 :  SD_CS        --> SD_CARD
// spi3 cs
#define PIN_LCD_CS    88        // PD7 :  LCD_CS       --> LCD

// WiFi IRQ
#define PIN_WIFI_IRQ  81        // PD0 :  WIFI_INT     --> WIFI

#elif HARDWARE_VERSION == 0x0204U

// EXTERNAL MODULE
// uart
#define PIN_UART2_TX  25        // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX  26        // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc
#define PIN_ADC12_IN9 29        // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2
#define PIN_SPI2_CS   51        // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK  52        // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO 53        // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI 54        // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c
#define PIN_I2C1_SCL  95        // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA  96        // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer
#define PIN_TIM3_CH1  63        // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2  64        // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io
#define PIN_IO_PD12   59        // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13   60        // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14   61        // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15   62        // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8    67        // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3    84        // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0    97        // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1    98        // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR
#define PIN_MOTOR_A   24        // PA1 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B   23        // PA0 :  MOTOR_B      --> MOTOR

// ATK MODULE
#define PIN_GBC_LED   97        // PE0 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY   98        // PE1 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX    25        // PA2 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX    26        // PA3 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY
#define PIN_BEEP      37        // PB2 :  BEEP         --> BEEP
#define PIN_LED_R     38        // PE7 :  LED_R        --> LED
#define PIN_LED_G     39        // PE8 :  LED_B        --> LED
#define PIN_LED_B     40        // PE9 :  LED_G        --> LED
#define PIN_KEY2      55        // PD8 :  KEY2         --> KEY
#define PIN_KEY1      56        // PD9 :  KEY1         --> KEY
#define PIN_KEY0      57        // PD10:  KEY0         --> KEY
#define PIN_WK_UP      7        // PC13:  WK_UP        --> KEY

// INFRARED
#define PIN_EMISSION  35        // PB0 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION 36        // PB1 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR
#define PIN_AP_INT    58        // PD11:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT   81        // PC2 :  ICM_INT      --> AXIS SENSOR

// AUDIO
#define PIN_AUDIO_PWR 77        // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS
#define PIN_NRF_IRQ   84        // PD3 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE    85        // PD4 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS    86        // PD5 :  NRF_CS       --> WIRELESS

// spi1 cs
#define PIN_SD_CS     18        // PC3 :  SD_CS        --> SD_CARD
// spi3 cs
#define PIN_LCD_CS    88        // PD7 :  LCD_CS       --> LCD

// WiFi IRQ
#define PIN_WIFI_IRQ  34        // PC5 :  WIFI_INT     --> WIFI

#endif

int rt_hw_pin_init(void);
#endif /* __GPIO_H__ */

