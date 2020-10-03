
#include "custom_pid_controller.h"

#include "bsp.h"
#include "app_pwm.h"
#include "nrf_drv_saadc.h"
#include "custom_log.h"


///////////////////////////////////////////////////////////////////////////
// For ADC ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


#define SAMPLES_IN_BUFFER 3

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

    nrf_saadc_channel_config_t channel_0_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
    channel_0_config.gain = NRF_SAADC_GAIN1_4;
    channel_0_config.reference = NRF_SAADC_REFERENCE_VDD4;
    nrf_saadc_channel_config_t channel_1_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN3);
    channel_1_config.gain = NRF_SAADC_GAIN1_4;
    channel_1_config.reference = NRF_SAADC_REFERENCE_VDD4;
    nrf_saadc_channel_config_t channel_2_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN6);
    channel_2_config.gain = NRF_SAADC_GAIN1_4;
    channel_2_config.reference = NRF_SAADC_REFERENCE_VDD4;

    err_code = nrf_drv_saadc_init(NULL, saadc_ready_callback);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_channel_init(0, &channel_0_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_channel_init(1, &channel_1_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_channel_init(2, &channel_2_config);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_saadc_buffer_convert(m_buffer, SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}

adc_data_t pidGetAdcValues(void)
{
    isAdcReady = false;
    nrf_drv_saadc_sample();
    while(!isAdcReady);

    adc_data_t adcData;

    int16_t adc0ValueRaw = m_buffer[0];
    if (adc0ValueRaw > 1023) {    adc0ValueRaw = 1023;}
    else if (adc0ValueRaw < 0) {  adc0ValueRaw = 0;}
    adcData.adcHeater = adc0ValueRaw;

    int16_t adc1ValueRaw = m_buffer[1];
    if (adc1ValueRaw > 1023) {    adc1ValueRaw = 1023;}
    else if (adc1ValueRaw < 0) {  adc1ValueRaw = 0;}
    adcData.adcHotlid = adc1ValueRaw;

    int16_t adc2ValueRaw = m_buffer[2];
    if (adc2ValueRaw > 1023) {    adc2ValueRaw = 1023;}
    else if (adc2ValueRaw < 0) {  adc2ValueRaw = 0;}
    adcData.adcTamb = adc2ValueRaw;

    return adcData;
}


///////////////////////////////////////////////////////////////////////////
// For PWM ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#define PIN_HEATER  NRF_GPIO_PIN_MAP(0,15)
#define PIN_PELTIER NRF_GPIO_PIN_MAP(0,14)
#define PIN_HOTLID  NRF_GPIO_PIN_MAP(0,13)

APP_PWM_INSTANCE(PWM1, 1);                  // Create the instance "PWM1" using TIMER1.

static volatile bool isPwmReady;            // A flag indicating PWM status.

static void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    isPwmReady = true;
}

static void pwmInit(void)
{
    // 1-channel PWM, 200Hz, output on DK LED pins:
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, PIN_HEATER);
    //app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, PIN_HEATER, PIN_PELTIER);
    pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
    //pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;

    // Initialize and enable PWM:
    ret_code_t err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);
}

void pid1SetPwmAction(uint8_t value)
{ 
    uint8_t channel = 0;

    // Try to change to the pwm peripheral once:
    isPwmReady = false;
    ret_code_t err_code = app_pwm_channel_duty_set(&PWM1, channel, value);

    // If it is busy, then wait until it's free and write again:
    if(err_code == NRF_ERROR_BUSY)
    {
        while(!isPwmReady);
        err_code = app_pwm_channel_duty_set(&PWM1, channel, value);
        APP_ERROR_CHECK(err_code);
    }

}

////////////////////////////////////////////////////////////////////////////////
APP_PWM_INSTANCE(PWM2, 2);                  // Create the instance "PWM2" using TIMER2.

static volatile bool isPwm2Ready;            // A flag indicating PWM status.

static void pwm2_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    isPwm2Ready = true;
}

void pwm2Init(void)
{
    // 1-channel PWM, 200Hz, output on DK LED pins:
    app_pwm_config_t pwm2_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, PIN_HOTLID);
    pwm2_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    // Initialize and enable PWM:
    ret_code_t err_code = app_pwm_init(&PWM2, &pwm2_cfg, pwm2_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM2);
}

void pid2SetPwmAction(uint8_t value)
{ 
    // Try to change to the pwm peripheral once:
    isPwm2Ready = false;
    ret_code_t err_code = app_pwm_channel_duty_set(&PWM2, 0, value);

    // If it is busy, then wait until it's free and write again:
    if(err_code == NRF_ERROR_BUSY)
    {
        while(!isPwm2Ready);
        err_code = app_pwm_channel_duty_set(&PWM2, 0, value);
        APP_ERROR_CHECK(err_code);
    }
}

///////////////////////////////////////////////////////////////////////////
// For PID ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void pidInit(void)
{
    pwmInit();
    pwm2Init();
    adcInit();
}

//const static float Ts = 0.1;
//const static float alpha = 0.081;
//const static float K = 1;
//const static float Kp = K*0.00233;
//const static float Ki = K*0.02932*Ts/2;
//const static float Kd = K*0.000046*2/Ts;

//float y_past = 0;
//float y_filt_past = 0;
//float e_past = 0;
//float u_inte_past = 0;
//float u_deri_past = 0;

//uint8_t pid1GetPwmAction(uint16_t adcValue, uint16_t adcReference)
//{
//
//    float y = (float) adcValue;
//    float y_filt = alpha*y_filt_past + 0.5*(1-alpha)*(y + y_past);
//
//    float y_ref = (float) adcReference;
//    float e = y_ref - y_filt;
//
//    float u_prop = Kp*e;
//    float u_inte =  u_inte_past + Ki*(e + e_past);
//    float u_deri = -u_deri_past + Kd*(y_filt - y_filt_past);
//
//    //NRF_LOG_INFO("e: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(e));
//    //NRF_LOG_INFO("u_prop: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(u_prop));
//    //NRF_LOG_INFO("u_inte: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(u_inte));
//    //NRF_LOG_INFO("u_deri: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(u_deri));
//    
//    float u = u_prop + u_inte + u_deri;
//    uint8_t pwmValue = u;
//
//    y_past = y;
//    y_filt_past = y_filt;
//    e_past = e;
//    u_inte_past = u_inte;
//    u_deri_past = u_deri;
//
//    return pwmValue;
//}

const float Kp1 = 100;
uint8_t pid1GetPwmAction(uint16_t adcValue, uint16_t adcReference)
{
    float y = (float) adcValue;
    float y_ref = (float) adcReference;
    float e = y_ref - y;

    float u_prop = Kp1*e;
    float u = u_prop;

    if(u < 0){   u = 0;}
    if(u > 255){ u = 255;}

    return (uint8_t)u;
}

const float Kp2 = 10;
uint8_t pid2GetPwmAction(uint16_t adcValue, uint16_t adcReference)
{
    float y = (float) adcValue;
    float y_ref = (float) adcReference;
    float e = y_ref - y;

    float u_prop = Kp2*e;
    float u = u_prop;

    if(u < 0){   u = 0;}
    if(u > 255){ u = 255;}

    return (uint8_t)u;
}
