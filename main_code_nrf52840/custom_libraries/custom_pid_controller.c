
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

static void adcInit(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
    channel_config.reference = NRF_SAADC_REFERENCE_VDD4;
    channel_config.gain = NRF_SAADC_GAIN1_4; 

    err_code = nrf_drv_saadc_init(NULL, saadc_ready_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer, SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}

uint16_t pidGetAdcValue(void)
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

static void pwmInit(void)
{
    // 1-channel PWM, 200Hz, output on DK LED pins:
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, BSP_LED_0);
    pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    // Initialize and enable PWM:
    ret_code_t err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);
}

void pidSetPwmAction(uint8_t value)
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

///////////////////////////////////////////////////////////////////////////
// For PID ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void pidInit(void)
{
    pwmInit();
    adcInit();
}

const static float Ts = 0.1;
const static float alpha = 0.081;
const static float K = 1;
const static float Kp = K*0.00233;
const static float Ki = K*0.02932*Ts/2;
const static float Kd = K*0.000046*2/Ts;

float y_past = 0;
float y_filt_past = 0;
float e_past = 0;
float u_inte_past = 0;
float u_deri_past = 0;

uint8_t pidGetPwmAction(uint16_t adcValue, uint16_t adcReference)
{

    float y = (float) adcValue;
    float y_filt = alpha*y_filt_past + 0.5*(1-alpha)*(y + y_past);

    float y_ref = (float) adcReference;
    float e = y_ref - y_filt;

    float u_prop = Kp*e;
    float u_inte =  u_inte_past + Ki*(e + e_past);
    float u_deri = -u_deri_past + Kd*(y_filt - y_filt_past);

    //NRF_LOG_INFO("e: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(e));
    //NRF_LOG_INFO("u_prop: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(u_prop));
    //NRF_LOG_INFO("u_inte: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(u_inte));
    //NRF_LOG_INFO("u_deri: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(u_deri));
    
    float u = u_prop + u_inte + u_deri;
    uint8_t pwmValue = u;

    y_past = y;
    y_filt_past = y_filt;
    e_past = e;
    u_inte_past = u_inte;
    u_deri_past = u_deri;

    return pwmValue;
}