
#include "custom_pid_controller.h"

#include "bsp.h"
#include "app_pwm.h"
#include "nrf_drv_saadc.h"
#include "custom_log.h"


///////////////////////////////////////////////////////////////////////////
// For ADC ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#define SAMPLES_IN_BUFFER 1

static volatile nrf_saadc_value_t m_buffer[SAMPLES_IN_BUFFER];

static volatile bool isAdcReady;

static void saadc_ready_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code = nrf_drv_saadc_buffer_convert(m_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);
        isAdcReady = true;
    }
}

void adcInit(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);
    channel_config.reference = NRF_SAADC_REFERENCE_VDD4;
    channel_config.gain = NRF_SAADC_GAIN1_4; 

    err_code = nrf_drv_saadc_init(NULL, saadc_ready_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer, SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}

uint16_t adcGetValue(void)
{
    isAdcReady = false;
    nrf_drv_saadc_sample();
    while(!isAdcReady);

    int16_t adcValueRaw = m_buffer[0];
    if (adcValueRaw > 1023) {
        adcValueRaw = 1023;
    }else if (adcValueRaw < 0) {
        adcValueRaw = 0;
    }
    return (uint16_t) adcValueRaw;
}


///////////////////////////////////////////////////////////////////////////
// For PWM ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

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
