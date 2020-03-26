/* 
 * File:   main.c
 * Author: Mark Kent
 *
 * Created on March 15, 2020, 4:13 PM
 */

typedef unsigned char uint8;
#include <xc.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#define nightmode 1
#define one_wire_zero_macro asm volatile    ("sbi 0x18, 2 \n" "nop \n" "cbi 0x18, 2 \n" "nop \n" "nop \n")
#define one_wire_one_macro asm volatile     ("sbi 0x18, 2 \n" "nop \n" "nop \n" "nop \n" "cbi 0x18, 2 \n")
#if nightmode == 1
    #define LED_LOW_RED()   write_led_rgb(0x03,0x00,0x00)
    #define LED_LOW_GREEN() write_led_rgb(0x00,0x03,0x00)
    #define LED_LOW_BLUE()  write_led_rgb(0x00,0x00,0x03)
#else
    #define LED_LOW_RED()   write_led_rgb(0x0F,0x00,0x00)
    #define LED_LOW_GREEN() write_led_rgb(0x00,0x0F,0x00)
    #define LED_LOW_BLUE()  write_led_rgb(0x00,0x00,0x0F)
#endif
#define NorthOutPin 0
#define NorthInPin  1
#define SouthOutPin 2
#define SouthInPin  3
#define EastOutPin  4
#define EastInPin   5
#define WestOutPin  6
#define WestInPin   7
//Method Declaration
void cofigure_interrupts();
void write_led_byte(uint8);
void write_led_rgb(uint8,uint8,uint8);
void init();
void sleep();

//Global Variables Declaration
uint8 flag_flag = 0x00;
uint8 trigger_flag = 0x00;
uint8 north_data = 0x01;
uint8 south_data = 0x01;
uint8 east_data = 0x01;
uint8 west_data = 0x01;
void write_led_rgb(uint8 red, uint8 green, uint8 blue){
    write_led_byte(green);
    write_led_byte(red);
    write_led_byte(blue);
}
void write_led_byte(uint8 data){
    for(int i = 0; i < 8; i++){
        if(data >= 0b10000000){
            one_wire_one_macro;
        }else{
            one_wire_zero_macro;
        }
        data = data << 1;
    }
}
ISR(PCINT1_vect){
    //Pause Interrupts
    cli();
    
    //Debounce for 35 us
    __builtin_avr_delay_cycles(280);
    
    //Check Flag Button 
    if((PINB & 0b00000001) == 0){
        flag_flag = 0xFF;
    }
    
    //Check Trigger Button
    if((PINB & 0b00000010) == 0){
        trigger_flag = 0xFF;
    }
    sei();
}
ISR(PCINT0_vect){
    cli();
    if((PINA & 0b0000001) == 0){
        
    }
    sei();
}
int main() {
    init();
    __builtin_avr_delay_cycles(800);
    //__builtin_avr_delay_cycles(8000000);
    while(1){
        sleep();
        if(flag_flag == 0xFF){
            flag_flag = 0x00;
            LED_LOW_BLUE();
        }
        if(trigger_flag == 0xFF){
            trigger_flag = 0x00;
            LED_LOW_GREEN();
        }
    }
}

void init(){
    //LED Data Pin      :   OUTPUT LOW
    //Flag Button       :   INPUT_PULLUP
    //Trigger Button    :   INPUT_PULLUP
    DDRB = 0b00000100;
    PORTB = 0b00000011;
    
    //ENABLE PCINT0 and PCINT1 interrupts
    //PCINT1    : PB0 and PB1
    //PCINT0    : PA0 and PA2 and PA5 and PA7
    GIMSK = 0b00110000;
    PCMSK1 = 0b00000011;
    PCMSK0 = 0b10100101;
    
    //Enable SLEEP as POWER Down
    MCUCR = 0b00110000;
    sei();
}

void sleep(){
    if(trigger_flag == 0x00 && flag_flag == 0x00){
        //Condition to prevent sleep if there are things that still need to be done
        return;
    }
    asm volatile("sleep\n");
}
uint8 receive_data_north(){
    uint8 return_data = 0x00;
    asm volatile(
    "ldi %[data], 0xF0 \n"
    : [data] "+r" (return_data)
    : [data_out] "M" (1 << NorthOutPin), [data_in] "M" (1 << NorthInPin) 
    :
    );
    return return_data;
}