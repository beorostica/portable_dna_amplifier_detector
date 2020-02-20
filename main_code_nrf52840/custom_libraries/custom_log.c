
#include "custom_log.h"

#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

void logInit(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}