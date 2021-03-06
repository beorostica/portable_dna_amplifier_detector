
#include "custom_timer.h"

#include "app_timer.h"
#include "nrf_drv_clock.h"

/**
 * @brief Create a variables that can hold the timer ID
 */
APP_TIMER_DEF(m_timer_battery_system_id);
APP_TIMER_DEF(m_timer_control_system_save_external_flash_id);
APP_TIMER_DEF(m_timer_control_system_id);
APP_TIMER_DEF(m_timer_detection_system_id);
APP_TIMER_DEF(m_timer_hundred_millis_id);
APP_TIMER_DEF(m_timer_seconds_id);

/**
 * @brief Function starting the internal LFCLK oscillator.
 *
 * @details This is needed by RTC1 which is used by the Application Timer
 *          (When SoftDevice is enabled the LFCLK is always running and this is not needed).
 */
static void lfclk_request(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}


/**
 * @brief Timeout handler for the battery system timer.
 */
static volatile bool isTimerBatterySystemReady;

static void timer_battery_system_handler(void * p_context)
{
    isTimerBatterySystemReady = true;
}

/**
 * @brief Timeout handler for the control system save external flash timer.
 */
static volatile bool isTimerControlSystemSaveExternalFlashReady;

static void timer_control_system_save_external_flash_handler(void * p_context)
{
    isTimerControlSystemSaveExternalFlashReady = true;
}

/**
 * @brief Timeout handler for the control system timer.
 */
static volatile bool isTimerControlSystemReady;

static void timer_control_system_handler(void * p_context)
{
    isTimerControlSystemReady = true;
}

/**
 * @brief Timeout handler for the detection system timer.
 */
static volatile bool isTimerDetectionSystemReady;

static void timer_detection_system_handler(void * p_context)
{
    isTimerDetectionSystemReady = true;
}

/**
 * @brief Timeout handler for the hundred millis timer.
 */
static volatile uint32_t timeHundredMillis;
static volatile bool isTimerHundredMillisReady;

static void timer_hundred_millis_handler(void * p_context)
{
    timeHundredMillis++;
    isTimerHundredMillisReady = true;
}

/**
 * @brief Timeout handler for the seconds timer.
 */
static volatile uint32_t timeSeconds;
static volatile bool isTimerSecondsReady;

static void timer_seconds_handler(void * p_context)
{
    timeSeconds++;
    isTimerSecondsReady = true;
}


/**
 * @brief Create timers.
 */
static void create_timers()
{
    ret_code_t err_code;

    //Create the control system timer:
    err_code = app_timer_create(&m_timer_battery_system_id, APP_TIMER_MODE_REPEATED, timer_battery_system_handler);
    APP_ERROR_CHECK(err_code);

    //Create the control system save external flash timer:
    err_code = app_timer_create(&m_timer_control_system_save_external_flash_id, APP_TIMER_MODE_REPEATED, timer_control_system_save_external_flash_handler);
    APP_ERROR_CHECK(err_code);

    //Create the control system timer:
    err_code = app_timer_create(&m_timer_control_system_id, APP_TIMER_MODE_REPEATED, timer_control_system_handler);
    APP_ERROR_CHECK(err_code);

    //Create the detection system timer:
    err_code = app_timer_create(&m_timer_detection_system_id, APP_TIMER_MODE_REPEATED, timer_detection_system_handler);
    APP_ERROR_CHECK(err_code);

    //Create the hundred millis timer:
    err_code = app_timer_create(&m_timer_hundred_millis_id, APP_TIMER_MODE_REPEATED, timer_hundred_millis_handler);
    APP_ERROR_CHECK(err_code);

    //Create the seconds timer:
    err_code = app_timer_create(&m_timer_seconds_id, APP_TIMER_MODE_REPEATED, timer_seconds_handler);
    APP_ERROR_CHECK(err_code);

}

void timerInit(void)
{
    //Initialize the RTC1:
    lfclk_request();
    app_timer_init();

    //Create Timers:
    create_timers();

}


////////////////////////////////////////////////////////////////////////////////////

void hundredMillisStart(void)
{
    ret_code_t err_code;

    //Start the hundred millis timer:
    timeHundredMillis = 0;
    isTimerHundredMillisReady = true;
    err_code = app_timer_start(m_timer_hundred_millis_id, APP_TIMER_TICKS(100), NULL);
    APP_ERROR_CHECK(err_code);

}

void hundredMillisStop(void)
{
    ret_code_t err_code;

    //Stop the hundred millis timer:
    err_code = app_timer_stop(m_timer_hundred_millis_id);
    APP_ERROR_CHECK(err_code);
    timeHundredMillis = 0;
    isTimerHundredMillisReady = true;

}

bool hundredMillisGetFlag(void)
{
    return isTimerHundredMillisReady;
}

void hundredMillisClearFlag(void)
{
    isTimerHundredMillisReady = false; 
}

uint32_t hundredMillisGetTime(void)
{
    return timeHundredMillis;
}


////////////////////////////////////////////////////////////////////////////////////

void secondsStart(void)
{
    ret_code_t err_code;

    //Start the seconds timer:
    timeSeconds = 0;
    isTimerSecondsReady = true;
    err_code = app_timer_start(m_timer_seconds_id, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);

}

void secondsStop(void)
{
    ret_code_t err_code;

    //Stop the millis timer:
    err_code = app_timer_stop(m_timer_seconds_id);
    APP_ERROR_CHECK(err_code);
    //timeSeconds = 0;
    isTimerSecondsReady = true;
}


bool secondsGetFlag(void)
{
    return isTimerSecondsReady;
}

void secondsClearFlag(void)
{
    isTimerSecondsReady = false; 
}

uint32_t secondsGetTime(void)
{
    return timeSeconds;
}

////////////////////////////////////////////////////////////////////////////////////


void timerDetectionSystem_Start(void)
{
    ret_code_t err_code;

    //Start the detection system timer:
    isTimerDetectionSystemReady = true;
    err_code = app_timer_start(m_timer_detection_system_id, APP_TIMER_TICKS(5000), NULL);
    APP_ERROR_CHECK(err_code);

}

void timerDetectionSystem_Stop(void)
{
    ret_code_t err_code;

    //Stop the detection system timer:
    err_code = app_timer_stop(m_timer_detection_system_id);
    APP_ERROR_CHECK(err_code);
    isTimerDetectionSystemReady = true;
}


bool timerDetectionSystem_GetFlag(void)
{
    return isTimerDetectionSystemReady;
}

void timerDetectionSystem_ClearFlag(void)
{
    isTimerDetectionSystemReady = false;
}


////////////////////////////////////////////////////////////////////////////////////

void timerControlSystem_Start(void)
{
    ret_code_t err_code;

    //Start the temp control system timer:
    isTimerControlSystemReady = true;
    err_code = app_timer_start(m_timer_control_system_id, APP_TIMER_TICKS(100), NULL);
    APP_ERROR_CHECK(err_code);

}

void timerControlSystem_Stop(void)
{
    ret_code_t err_code;

    //Stop the temp control system timer:
    err_code = app_timer_stop(m_timer_control_system_id);
    APP_ERROR_CHECK(err_code);
    isTimerControlSystemReady = true;
}


bool timerControlSystem_GetFlag(void)
{
    return isTimerControlSystemReady;
}

void timerControlSystem_ClearFlag(void)
{
    isTimerControlSystemReady = false;
}


////////////////////////////////////////////////////////////////////////////////////

void timerControlSystem_SaveExternalFlash_Start(void)
{
    ret_code_t err_code;

    //Start the temp control save flash system timer:
    isTimerControlSystemSaveExternalFlashReady = true;
    err_code = app_timer_start(m_timer_control_system_save_external_flash_id, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(err_code);

}

void timerControlSystem_SaveExternalFlash_Stop(void)
{
    ret_code_t err_code;

    //Stop the temp control save flash system timer:
    err_code = app_timer_stop(m_timer_control_system_save_external_flash_id);
    APP_ERROR_CHECK(err_code);
    isTimerControlSystemSaveExternalFlashReady = true;
}


bool timerControlSystem_SaveExternalFlash_GetFlag(void)
{
    return isTimerControlSystemSaveExternalFlashReady;
}

void timerControlSystem_SaveExternalFlash_ClearFlag(void)
{
    isTimerControlSystemSaveExternalFlashReady = false;
}


////////////////////////////////////////////////////////////////////////////////////

void timerBatterySystem_Start(void)
{
    ret_code_t err_code;

    //Start the battery system timer:
    isTimerBatterySystemReady = true;
    err_code = app_timer_start(m_timer_battery_system_id, APP_TIMER_TICKS(5000), NULL);
    APP_ERROR_CHECK(err_code);

}

void timerBatterySystem_Stop(void)
{
    ret_code_t err_code;

    //Stop the battery system timer:
    err_code = app_timer_stop(m_timer_battery_system_id);
    APP_ERROR_CHECK(err_code);
    isTimerBatterySystemReady = true;
}


bool timerBatterySystem_GetFlag(void)
{
    return isTimerBatterySystemReady;
}

void timerBatterySystem_ClearFlag(void)
{
    isTimerBatterySystemReady = false;
}