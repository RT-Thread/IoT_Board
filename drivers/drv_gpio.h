/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-01-05     Bernard      the first version
 * 2019-06-28     SummerGift   update to the latest version
 */
#ifndef __GPIO_H__
#define __GPIO_H__

#include "board.h"

#define __STM32_PORT(port)  GPIO##port

#define GET_PIN(PORTx,PIN) (rt_base_t)((16 * ( ((rt_base_t)__STM32_PORT(PORTx) - (rt_base_t)GPIOA)/(0x0400UL) )) + PIN)

#define __STM32_PIN(index, gpio, gpio_index)                                \
    {                                                                       \
        index, GPIO##gpio, GPIO_PIN_##gpio_index                            \
    }

#define __STM32_PIN_RESERVE                                                 \
    {                                                                       \
        -1, 0, 0                                                            \
    }

/* STM32 GPIO driver */
struct pin_index
{
    int index;
    GPIO_TypeDef *gpio;
    uint32_t pin;
};

struct pin_irq_map
{
    rt_uint16_t pinbit;
    IRQn_Type irqno;
};

int rt_hw_pin_init(void);


#if HARDWARE_VERSION == 0x0200U
// EXTERNAL MODULE
// uart
#define PIN_UART2_TX    GET_PIN(A, 2)      // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX    GET_PIN(A, 3)      // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc                  
#define PIN_ADC12_IN9   GET_PIN(A, 4)      // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2                 
#define PIN_SPI2_CS     GET_PIN(B, 12)     // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK    GET_PIN(B, 13)     // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO   GET_PIN(B, 14)     // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI   GET_PIN(B, 15)     // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c                  
#define PIN_I2C1_SCL    GET_PIN(B, 8)      // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA    GET_PIN(B, 9)      // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer               
#define PIN_TIM3_CH1    GET_PIN(C, 6)      // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2    GET_PIN(C, 7)      // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io                   
#define PIN_IO_PD12     GET_PIN(D, 12)     // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13     GET_PIN(D, 13)     // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14     GET_PIN(D, 14)     // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15     GET_PIN(D, 15)     // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8      GET_PIN(A, 8)      // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3      GET_PIN(D, 3)      // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0      GET_PIN(E, 0)      // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1      GET_PIN(E, 1)      // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR                  
#define PIN_MOTOR_A     GET_PIN(A, 0)      // PA0 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B     GET_PIN(A, 1)      // PA1 :  MOTOR_B      --> MOTOR

// ATK MODULE           
#define PIN_GBC_LED     GET_PIN(C, 2)      // PC2 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY     GET_PIN(C, 3)      // PC3 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX      GET_PIN(C, 4)      // PC4 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX      GET_PIN(C, 5)      // PC5 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY   
#define PIN_BEEP        GET_PIN(B, 2)      // PB2 :  BEEP         --> BEEP
#define PIN_LED_R       GET_PIN(E, 7)      // PE7 :  LED_R        --> LED
#define PIN_LED_G       GET_PIN(E, 8)      // PE8 :  LED_G        --> LED
#define PIN_LED_B       GET_PIN(E, 9)      // PE9 :  LED_B        --> LED
#define PIN_KEY0        GET_PIN(D, 8)      // PD8 :  KEY0         --> KEY
#define PIN_KEY1        GET_PIN(D, 9)      // PD9 :  KEY1         --> KEY
#define PIN_KEY2        GET_PIN(D, 10)     // PD10:  KEY2         --> KEY
#define PIN_WK_UP       GET_PIN(D, 11)     // PD11:  WK_UP        --> KEY

// INFRARED               
#define PIN_EMISSION    GET_PIN(B, 0)      // PB0 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION   GET_PIN(B, 1)      // PB1 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR               
#define PIN_AP_INT      GET_PIN(B, 10)     // PB10:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT     GET_PIN(B, 11)     // PB11:  ICM_INT      --> AXIS SENSOR

// AUDIO                
#define PIN_AUDIO_PWR   GET_PIN(A, 15)     // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS             
#define PIN_NRF_IRQ     GET_PIN(D, 4)      // PD4 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE      GET_PIN(D, 5)      // PD5 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS      GET_PIN(D, 6)      // PD6 :  NRF_CS       --> WIRELESS

// spi1 cs              
#define PIN_SD_CS       GET_PIN(C, 13)     // PC13:  SD_CS        --> SD_CARD
// spi3 cs              
#define PIN_LCD_CS      GET_PIN(D, 7)      // PD7 :  LCD_CS       --> LCD

// WiFi IRQ             
#define PIN_WIFI_IRQ    GET_PIN(D, 0)      // PD0 :  WIFI_INT     --> WIFI

#elif HARDWARE_VERSION == 0x0201U

// EXTERNAL MODULE
// uart
#define PIN_UART2_TX    GET_PIN(A, 2)      // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX    GET_PIN(A, 3)      // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc                    
#define PIN_ADC12_IN9   GET_PIN(A, 4)      // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2                 
#define PIN_SPI2_CS     GET_PIN(B, 12)     // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK    GET_PIN(B, 13)     // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO   GET_PIN(B, 14)     // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI   GET_PIN(B, 15)     // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c                  
#define PIN_I2C1_SCL    GET_PIN(B, 8)      // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA    GET_PIN(B, 9)      // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer                
#define PIN_TIM3_CH1    GET_PIN(C, 6)      // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2    GET_PIN(C, 7)      // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io                   
#define PIN_IO_PD12     GET_PIN(D, 12)     // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13     GET_PIN(D, 13)     // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14     GET_PIN(D, 14)     // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15     GET_PIN(D, 15)     // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8      GET_PIN(A, 8)      // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3      GET_PIN(D, 3)      // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0      GET_PIN(E, 0)      // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1      GET_PIN(E, 1)      // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR                
#define PIN_MOTOR_A     GET_PIN(A, 1)      // PA1 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B     GET_PIN(A, 0)      // PA0 :  MOTOR_B      --> MOTOR
                       
// ATK MODULE           
#define PIN_GBC_LED     GET_PIN(E, 0)      // PE0 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY     GET_PIN(E, 1)      // PE1 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX      GET_PIN(A, 2)      // PA2 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX      GET_PIN(A, 3)      // PA3 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY   
#define PIN_BEEP        GET_PIN(B, 0)      // PB0 :  BEEP         --> BEEP
#define PIN_LED_R       GET_PIN(E, 7)      // PE7 :  LED_R        --> LED
#define PIN_LED_B       GET_PIN(E, 8)      // PE8 :  LED_B        --> LED
#define PIN_LED_G       GET_PIN(E, 9)      // PE9 :  LED_G        --> LED
#define PIN_KEY0        GET_PIN(D, 8)      // PD8 :  KEY0         --> KEY
#define PIN_KEY1        GET_PIN(D, 9)      // PD9 :  KEY1         --> KEY
#define PIN_KEY2        GET_PIN(D, 10)     // PD10:  KEY2         --> KEY
#define PIN_WK_UP       GET_PIN(D, 11)     // PD11:  WK_UP        --> KEY
                          
// INFRARED               
#define PIN_EMISSION    GET_PIN(B, 1)      // PB1 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION   GET_PIN(B, 2)      // PB2 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR               
#define PIN_AP_INT      GET_PIN(C, 13)     // PC13:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT     GET_PIN(C, 2)      // PC2 :  ICM_INT      --> AXIS SENSOR

// AUDIO 
#define PIN_AUDIO_PWR   GET_PIN(A, 15)     // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS             
#define PIN_NRF_IRQ     GET_PIN(D, 4)      // PD4 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE      GET_PIN(D, 5)      // PD5 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS      GET_PIN(D, 6)      // PD6 :  NRF_CS       --> WIRELESS

// spi1 cs              
#define PIN_SD_CS       GET_PIN(C, 3)      // PC3 :  SD_CS        --> SD_CARD
// spi3 cs              
#define PIN_LCD_CS      GET_PIN(D, 7)      // PD7 :  LCD_CS       --> LCD

// WiFi IRQ             
#define PIN_WIFI_IRQ    GET_PIN(D, 0)      // PD0 :  WIFI_INT     --> WIFI

#elif HARDWARE_VERSION == 0x0202U

// EXTERNAL MODULE
// uart
#define PIN_UART2_TX    GET_PIN(A, 2)      // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX    GET_PIN(A, 3)      // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc                  
#define PIN_ADC12_IN9   GET_PIN(A, 4)      // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2                 
#define PIN_SPI2_CS     GET_PIN(B, 12)     // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK    GET_PIN(B, 13)     // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO   GET_PIN(B, 14)     // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI   GET_PIN(B, 15)     // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c                  
#define PIN_I2C1_SCL    GET_PIN(B, 8)      // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA    GET_PIN(B, 9)      // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer                
#define PIN_TIM3_CH1    GET_PIN(C, 6)      // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2    GET_PIN(C, 7)      // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io                   
#define PIN_IO_PD12     GET_PIN(D, 12)     // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13     GET_PIN(D, 13)     // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14     GET_PIN(D, 14)     // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15     GET_PIN(D, 15)     // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8      GET_PIN(A, 8)      // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3      GET_PIN(D, 3)      // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0      GET_PIN(E, 0)      // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1      GET_PIN(E, 1)      // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR                  
#define PIN_MOTOR_A     GET_PIN(A, 1)      // PA1 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B     GET_PIN(A, 0)      // PA0 :  MOTOR_B      --> MOTOR

// ATK MODULE           
#define PIN_GBC_LED     GET_PIN(E, 0)      // PE0 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY     GET_PIN(E, 1)      // PE1 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX      GET_PIN(A, 2)      // PA2 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX      GET_PIN(A, 3)      // PA3 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY   
#define PIN_BEEP        GET_PIN(B, 2)      // PB2 :  BEEP         --> BEEP
#define PIN_LED_R       GET_PIN(E, 7)      // PE7 :  LED_R        --> LED
#define PIN_LED_G       GET_PIN(E, 8)      // PE8 :  LED_B        --> LED
#define PIN_LED_B       GET_PIN(E, 9)      // PE9 :  LED_G        --> LED
#define PIN_KEY0        GET_PIN(D, 8)      // PD8 :  KEY0         --> KEY
#define PIN_KEY1        GET_PIN(D, 9)      // PD9 :  KEY1         --> KEY
#define PIN_KEY2        GET_PIN(D, 10)     // PD10:  KEY2         --> KEY
#define PIN_WK_UP       GET_PIN(C, 13)     // PC13:  WK_UP        --> KEY

// INFRARED               
#define PIN_EMISSION    GET_PIN(B, 0)      // PB0 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION   GET_PIN(B, 1)      // PB1 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR              
#define PIN_AP_INT      GET_PIN(D, 11)     // PD11:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT     GET_PIN(C, 2)      // PC2 :  ICM_INT      --> AXIS SENSOR

// AUDIO               
#define PIN_AUDIO_PWR   GET_PIN(A, 15)     // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS             
#define PIN_NRF_IRQ     GET_PIN(D, 4)      // PD4 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE      GET_PIN(D, 5)      // PD5 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS      GET_PIN(D, 6)      // PD6 :  NRF_CS       --> WIRELESS

// spi1 cs              
#define PIN_SD_CS       GET_PIN(C, 3)      // PC3 :  SD_CS        --> SD_CARD
// spi3 cs              
#define PIN_LCD_CS      GET_PIN(D, 7)      // PD7 :  LCD_CS       --> LCD

// WiFi IRQ             
#define PIN_WIFI_IRQ    GET_PIN(D, 0)      // PD0 :  WIFI_INT     --> WIFI

#elif HARDWARE_VERSION == 0x0204U

// EXTERNAL MODULE
// uart
#define PIN_UART2_TX    GET_PIN(A, 2)      // PA2 :  UART2_TX     --> EXTERNAL MODULE
#define PIN_UART2_RX    GET_PIN(A, 3)      // PA3 :  UART2_RX     --> EXTERNAL MODULE
// adc                  
#define PIN_ADC12_IN9   GET_PIN(A, 4)      // PA4 :  ADC12_IN9    --> EXTERNAL MODULE
// spi2                 
#define PIN_SPI2_CS     GET_PIN(B, 12)     // PB12:  SPI2_CS      --> EXTERNAL MODULE
#define PIN_SPI2_SCK    GET_PIN(B, 13)     // PB13:  SPI2_SCK     --> EXTERNAL MODULE
#define PIN_SPI2_MISO   GET_PIN(B, 14)     // PB14:  SPI2_MISO    --> EXTERNAL MODULE
#define PIN_SPI2_MOSI   GET_PIN(B, 15)     // PB15:  SPI2_MOSI    --> EXTERNAL MODULE
// i2c                  
#define PIN_I2C1_SCL    GET_PIN(B, 8)      // PB8 :  I2C1_SCL     --> EXTERNAL MODULE
#define PIN_I2C1_SDA    GET_PIN(B, 9)      // PB9 :  I2C1_SDA     --> EXTERNAL MODULE
// timer                
#define PIN_TIM3_CH1    GET_PIN(C, 6)      // PC6 :  TIM3_CH1     --> EXTERNAL MODULE
#define PIN_TIM3_CH2    GET_PIN(C, 7)      // PC7 :  TIM3_CH2     --> EXTERNAL MODULE
// io                   
#define PIN_IO_PD12     GET_PIN(D, 12)     // PD12:  IO_PD12      --> EXTERNAL MODULE
#define PIN_IO_PD13     GET_PIN(D, 13)     // PD13:  IO_PD13      --> EXTERNAL MODULE
#define PIN_IO_PD14     GET_PIN(D, 14)     // PD14:  IO_PD14      --> EXTERNAL MODULE
#define PIN_IO_PD15     GET_PIN(D, 15)     // PD15:  IO_PD15      --> EXTERNAL MODULE
#define PIN_IO_PA8      GET_PIN(A, 8)      // PA8 :  IO_PA8       --> EXTERNAL MODULE
#define PIN_IO_PD3      GET_PIN(D, 3)      // PD3 :  IO_PD3       --> EXTERNAL MODULE
#define PIN_IO_PE0      GET_PIN(E, 0)      // PE0 :  IO_PE0       --> EXTERNAL MODULE
#define PIN_IO_PE1      GET_PIN(E, 1)      // PE1 :  IO_PE1       --> EXTERNAL MODULE

// MOTOR                  
#define PIN_MOTOR_A     GET_PIN(A, 1)      // PA1 :  MOTOR_A      --> MOTOR
#define PIN_MOTOR_B     GET_PIN(A, 0)      // PA0 :  MOTOR_B      --> MOTOR

// ATK MODULE
#define PIN_GBC_LED     GET_PIN(E, 0)      // PE0 :  GBC_LED      --> ATK MODULE
#define PIN_GBC_KEY     GET_PIN(E, 1)      // PE1 :  GBC_KEY      --> ATK MODULE
#define PIN_GBC_RX      GET_PIN(A, 2)      // PA2 :  GBC_RX       --> ATK MODULE
#define PIN_GBC_TX      GET_PIN(A, 3)      // PA3 :  GBC_TX       --> ATK MODULE

// BEEP && LED && KEY
#define PIN_BEEP        GET_PIN(B, 2)      // PB2 :  BEEP         --> BEEP
#define PIN_LED_R       GET_PIN(E, 7)      // PE7 :  LED_R        --> LED
#define PIN_LED_G       GET_PIN(E, 8)      // PE8 :  LED_B        --> LED
#define PIN_LED_B       GET_PIN(E, 9)      // PE9 :  LED_G        --> LED
#define PIN_KEY2        GET_PIN(D, 8)      // PD8 :  KEY2         --> KEY
#define PIN_KEY1        GET_PIN(D, 9)      // PD9 :  KEY1         --> KEY
#define PIN_KEY0        GET_PIN(D, 10)     // PD10:  KEY0         --> KEY
#define PIN_WK_UP       GET_PIN(C, 13)     // PC13:  WK_UP        --> KEY

// INFRARED
#define PIN_EMISSION    GET_PIN(B, 0)      // PB0 :  EMISSION     --> INFRARED EMISSION
#define PIN_RECEPTION   GET_PIN(B, 1)      // PB1 :  RECEPTION    --> INFRARED RECEPTION

// SENSOR
#define PIN_AP_INT      GET_PIN(D, 11)     // PD11:  AP_INT       --> ALS&PS SENSOR
#define PIN_ICM_INT     GET_PIN(C, 2)      // PC2 :  ICM_INT      --> AXIS SENSOR

// AUDIO                
#define PIN_AUDIO_PWR   GET_PIN(A, 15)     // PA15:  AUDIO_PWR    --> AUDIO && POWER

// WIRELESS             
#define PIN_NRF_IRQ     GET_PIN(D, 3)      // PD3 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE      GET_PIN(D, 4)      // PD4 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS      GET_PIN(D, 5)      // PD5 :  NRF_CS       --> WIRELESS

// spi1 cs              
#define PIN_SD_CS       GET_PIN(C, 3)      // PC3 :  SD_CS        --> SD_CARD
// spi3 cs              
#define PIN_LCD_CS      GET_PIN(D, 7)      // PD7 :  LCD_CS       --> LCD

// WiFi IRQ
#define PIN_WIFI_IRQ    GET_PIN(C, 5)      // PC5 :  WIFI_INT     --> WIFI

#endif

int rt_hw_pin_init(void);
#endif /* __GPIO_H__ */

