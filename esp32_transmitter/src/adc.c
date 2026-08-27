#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "adc.h"

//esp-idf gives 
// adc_oneshot_new_unit()
// adc_oneshot_config_channel()
// adc_oneshot_read()

//i write the joystick part.
//text string used to print messages to serial
static const char *TAG = "ADC_DRIVER"; 

//create an ADC unit handle under normal oneshot mode


//global handle to hold the initialized ADC unit 1
adc_oneshot_unit_handle_t adc1_handle;
//this will eventually point to adc1
void init_adc(void) {

    //create the struct for ADC unit 1, you can do the same for ADC2, 
    //just rename .unit_id to ADC_UNIT_2
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id= ADC_UNIT_1, //
        .ulp_mode = ADC_ULP_MODE_DISABLE, //disable the ultra lower power mode.
    };
}
    
