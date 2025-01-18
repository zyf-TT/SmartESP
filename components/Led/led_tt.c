#include "led_tt.h"
#include "uart_tt.h"

void led_main_task(void *arg)
{
 
        vTaskDelay( 500 );
        
}

void led_task(void *arg)
{
    char led1_state = 0;
    led_init();
    
    while(1)
    {
        gpio_set_level(18, led1_state);
        led1_state = !led1_state;
        vTaskDelay( 500 );
        
    }
}

void led_init(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;//禁止中断
    io_conf.mode = GPIO_MODE_OUTPUT;      //设置为输出模式
    io_conf.pin_bit_mask = (1ULL<<18) ;//要设置的引脚的位掩码，例如gpio18 /19
    io_conf.pull_up_en = 0;//关闭上拉模式
    io_conf.pull_down_en = 0;//关闭下拉模式
    gpio_config(&io_conf);//配置GPIO
}