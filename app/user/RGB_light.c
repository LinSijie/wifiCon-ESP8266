/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: RGB_light.c
 *
 * Description: light demo's function realization
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "RGB_light.h"
#include "user_main.h"

struct light_saved_param light_param;
static bool update_flag = FALSE;
/******************************************************************************
 * FunctionName : RGB_light_get_duty
 * Description  : get duty of each channel
 * Parameters   : uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
RGB_light_get_duty(uint8 channel)
{
    return light_param.pwm_duty[channel];
}

/******************************************************************************
 * FunctionName : RGB_light_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
RGB_light_set_duty(uint32 duty, uint8 channel)
{	
	//os_printf("in RGB_light_set_duty\r\n");
    if (duty != light_param.pwm_duty[channel]) {
    	pwm_set_duty(duty, channel);
		update_flag = TRUE;
        light_param.pwm_duty[channel] = pwm_get_duty(channel);
    }
}

/******************************************************************************
 * FunctionName : RGB_light_get_period
 * Description  : get pwm period
 * Parameters   : NONE
 * Returns      : uint32 : pwm period
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
RGB_light_get_period(void)
{
	//os_printf("in RGB_light_get_duty\r\n");
    return light_param.pwm_period;
}

/******************************************************************************
 * FunctionName : RGB_light_set_period
 * Description  : set pwm period
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
RGB_light_set_period(uint32 period)
{
	//os_printf("in RGB_light_set_period\r\n");
    if (period != light_param.pwm_period) {
        pwm_set_period(period);
		update_flag = TRUE;
        light_param.pwm_period = pwm_get_period();
    }
}

/******************************************************************************
 * FunctionName : RGB_light_start
 * Description  : update pwm params
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
RGB_light_start(void)
{
	//os_printf("in RGB_light_start\r\n");
	if (TRUE == update_flag)
	{
		update_flag = FALSE;
		pwm_start();
	}
}

/******************************************************************************
 * FunctionName : RGB_light_set_color
 * Description  : set pwm period
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
RGB_light_set_color(uint8 r, uint8 g, uint8 b)
{
	//os_printf("in RGB_light_set_color\r\n");
//	uint32 period = RGB_light_get_period();
//	uint32 max_duty = PWM_DUTY_MAX(period);
	uint32 max_duty_r = 16384;
	uint32 max_duty_g = 4096;
	uint32 max_duty_b = 16384;


#ifdef USER_PRINT_DEBUG
	printf("set color: R(%02x) G(%02x) B(%02x)\r\n", r, g, b);
#endif

	// red
	if (0xfd == r)
	{
		r = 0xff;
		g = 0x00;
		b = 0x00;
	}
	else if (0xfc == r)
	{
		r = 0xff;
		g = 0x7f;
		b = 0x00;
	}
	
	// green
	
	// blue
	
#ifdef RGB_COMMON_GND
	RGB_light_set_duty(RGB_TO_DUTY(r, max_duty_r), LIGHT_RED);
	RGB_light_set_duty(RGB_TO_DUTY(g, max_duty_g), LIGHT_GREEN);
	RGB_light_set_duty(RGB_TO_DUTY(b, max_duty_b), LIGHT_BLUE);
#else
	RGB_light_set_duty(RGB_TO_DUTY((uint8)~r, max_duty_r), LIGHT_RED);
	RGB_light_set_duty(RGB_TO_DUTY((uint8)~g, max_duty_g), LIGHT_GREEN);
	RGB_light_set_duty(RGB_TO_DUTY((uint8)~b, max_duty_b), LIGHT_BLUE);
#endif
	RGB_light_start();
}

/******************************************************************************
 * FunctionName : RGB_light_init
 * Description  : light demo init, mainy init pwm
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
RGB_light_init(void)
{
	//os_printf("in RGB_light_init\r\n");
	memset((void*)&light_param, 0, sizeof(light_param));
	if (light_param.pwm_period > PWM_PERIOD_MAX || light_param.pwm_period < PWM_PERIOD_MIN)
	{
		light_param.pwm_period = PWM_PERIOD_MIN;
    }

	uint32 pin_info[PWM_CHANNEL][3] =
								{
									{PWM_1_OUT_IO_MUX,PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM},		
									{PWM_0_OUT_IO_MUX,PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM},	//Green		 
									{PWM_2_OUT_IO_MUX,PWM_2_OUT_IO_FUNC,PWM_2_OUT_IO_NUM}
								};
	
	uint32 pwm_duty_init[PWM_CHANNEL] = {0};
	
    /*PIN FUNCTION INIT FOR PWM OUTPUT*/
    pwm_init(light_param.pwm_period, pwm_duty_init, PWM_CHANNEL, pin_info);

#ifdef USER_PRINT_DEBUG    
    printf("LIGHT PARAM: R: %d \r\n",light_param.pwm_duty[LIGHT_RED]);
    printf("LIGHT PARAM: G: %d \r\n",light_param.pwm_duty[LIGHT_GREEN]);
    printf("LIGHT PARAM: B: %d \r\n",light_param.pwm_duty[LIGHT_BLUE]);

	if (PWM_CHANNEL > LIGHT_WARM_WHITE) 
	{
		printf("LIGHT PARAM: CW: %d \r\n",light_param.pwm_duty[LIGHT_COLD_WHITE]);
		printf("LIGHT PARAM: WW: %d \r\n",light_param.pwm_duty[LIGHT_WARM_WHITE]);
	}
    printf("LIGHT PARAM: P: %d \r\n",light_param.pwm_period);
#endif

}


