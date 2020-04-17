
#include "custom_pid_controller.h"

#include "bsp.h"
#include "app_pwm.h"
#include "custom_log.h"


APP_PWM_INSTANCE(PWM1, 1);                  // Create the instance "PWM1" using TIMER1.

static volatile bool isPwmReady;            // A flag indicating PWM status.

static void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    isPwmReady = true;
}

void pwmInit(void)
{
    // 1-channel PWM, 200Hz, output on DK LED pins:
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, BSP_LED_0);

    // Initialize and enable PWM:
    ret_code_t err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);
}

void pwmSetDutyCycle(uint8_t value)
{ 

    // Try to change to the pwm peripheral once:
    isPwmReady = false;
    ret_code_t err_code = app_pwm_channel_duty_set(&PWM1, 0, value);

    // If it is busy, then wait until it's free and write again:
    if(err_code == NRF_ERROR_BUSY)
    {
        while(!isPwmReady);
        err_code = app_pwm_channel_duty_set(&PWM1, 0, value);
        APP_ERROR_CHECK(err_code);
    }

}
