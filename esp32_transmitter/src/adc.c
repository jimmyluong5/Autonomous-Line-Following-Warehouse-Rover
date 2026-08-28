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

//to recycle the ADC unit you can do 
//ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle))
//which deinitializes the ADC unit, which frees up its hardware resources and 
//memory when its no longer needed.



//create an ADC unit handle under normal oneshot mode
//global handle to hold the initialized ADC unit 1
adc_oneshot_unit_handle_t adc1_handle;
//this will eventually point to adc1
void init_adc(void) {


    //create the struct for ADC unit 1, you can do the same for ADC2, 
    //just rename .unit_id to ADC_UNIT_2
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id= ADC_UNIT_1, //
        .ulp_mode = ADC_ULP_MODE_DISABLE, //disable the ultra lower power mode,
        //since main CPU is running.
    };
    //call the adc oneshot new unit to build the unit in hardware based on init_config
    //stores the resulting handle into adc1_handle.
    //this will return success or fail or something else.
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    //saves the reference into the address of adc1_handle

    //after the ADC instance is created, set up the adc_oneshot_chan_cfg_t
    //to configure the adc IOs to measure analog signal. 


    //initialize the channel settings
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, //12 BIT RESOLUTION (from 0-4095)
        .atten = ADC_ATTEN_DB_12, //12 db attentuation which measures from 0-3.3V
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOYSTICK_X_CHANNEL, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOYSTICK_Y_CHANNEL, &config));


    //after the above initializations, the ADC is ready measure the analog signals
    //of the ADC pins. 

    //CALL adc_oneshot_read()to get the conversion result of an ADC channel.
    //adc_oneshot_read(adc_oneshot_unit_handle_t handle, adc_channel_t, int* out_raw);

    //test in putty
    ESP_LOGI(TAG, "ADC set on GPIO 4 and 5.");
}   


    
int read_joystick_x(void) {
    int raw_val = 0; //initialize the values to 0

    //then just use the function to read the pins from earlier.
    //use the error to check if its success then read
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOYSTICK_X_CHANNEL, &raw_val));
    return raw_val;
}

int read_joystick_y(void) {
    int raw_val = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, JOYSTICK_Y_CHANNEL, &raw_val));
    return raw_val;
}


