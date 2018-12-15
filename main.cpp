#include "LPC17xx.h"
#include "tasks.hpp"
#include "sys_config.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "printf_lib.h"
#include "tasks.hpp"
#include "uart0_min.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lpc_isr.h"
#include "eint.h"
#include "semphr.h"
#include "spi_sem.h"
#include "io.hpp"
#include "utilities.h"
#include "command_handler.hpp"
#include "handlers.hpp"
#include "event_groups.h"
#include <fstream>
#include "semphr.h"
#include "ssp0.h"
#include "ssp1.h"
#include "gpio.hpp"
#include "VS1053.hpp"
#include "tasks.hpp"
#include "ff.h"
#include "eint.h"

#include "LabGPIO_0.hpp"
#include "LabGPIOInterrupts.hpp"
// #include "LabSPI.hpp"
#include "spi_driver.hpp"

/* returns minimum of two given numbers */
#define min(a,b) (((a)<(b))?(a):(b))    

int i=0;

LabGPIO_0 _dreq(29,0);
LabGPIO_0 _xdcs(7, 2);
LabGPIO_0 _xcs(6, 2);
LabGPIO_0 _rst(1, 0);
// LabSpi ssp(i);     
LabGPIO_0 blue_button(0,0);

VS1053 obj(&_dreq, &_xdcs, &_xcs, &_rst);//, &ssp);

void send_to_mp3(void *pv){
    delay_ms(500);

    printf("Entered MP3 player function. \n\n");
    //FILE *fd = fopen("1:songs/Ariana Grande - Last Christmas.mp3", "r");
    //FILE *fd = fopen("1:Fetty Wap - Trap Queen (Clean).mp3", "r");
    //FILE *fd = fopen("1:songs/The Weeknd - Can't Feel My Face.mp3", "r");
    //FILE *fd = fopen("1:SEA 30 SEC.mp3", "r");
    FILE *fd = fopen("1:songs/Belinda Carlisle - Heaven Is a Place on Earth Lyrics.mp3", "r");

    fseek(fd, 0L, SEEK_END); //go to last position
    int file_size = ftell(fd);
    fseek(fd, 0L, SEEK_SET); //go to the front

    int dat_ofs = 0;
    uint8_t data[512];
    uint8_t *dat;
    uint8_t recv;
    uint8_t send;
    int dat_pos = 0;
    size_t val=513;

    printf("The file has been attempted to be opened.");

        if(fd){
            printf("File successfully opened.\n\n");

            while(1){
            while(!feof(fd) && obj.ret_run_song_flag()){//while(!(val < 512)){ //while(1){//while(dat_ofs < (file_size-1)){
            val = fread(data, 1, 512, fd);
//            consume(data, val);

                dat = data;
                //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes
                while(dat_pos < 512){

                    while(!_dreq.getLevel());
                    _xdcs.setLow(); //chip select the decoder (active low)
                    {
                        send = *dat++;
                        recv = spi_transfer(send);
                    }
                    _xdcs.setHigh();

                dat_pos++;
                }

                dat_pos = 0;
            }
            }

            fclose(fd);
        }
        else{
            printf("File failed to open\n");
        }

}


void red_button_press_isr(){

}

void blue_button_press_isr(){
    if(obj.ret_run_song_flag()){ //if true
        delay_ms(10);
        obj.pauseSong(); //set to false
        printf("Pause the song \n");
    }
    else{//if false
        delay_ms(10);
        obj.resumeSong(); //set to true
        printf("Resume the song \n");
    }
}

void green_button_press_isr(){

}

int main(void) {
    int *val;
    bool IntrAttach;
    // spi_init(8, spi, Ext, 1);

   //  obj.vs_init();
   //  // ssp->init(8, SPI_, External, 1);

    obj.vs_init();

    printf("Initialization is Complete.\n");

   blue_button.setAsInput();

   LabGPIOInterrupts* gpio_interrupt = LabGPIOInterrupts::CreateOneInstance();
   // Init things once
   gpio_interrupt->Initialize();

   IntrAttach = gpio_interrupt->AttachInterruptHandler(0, 0, &blue_button_press_isr, kRisingEdge);

   if(IntrAttach){
       printf("The blue_button_press_isr function is attached. \n");
   }
   else{
       printf("ISR could not be attached.\n");
   }

    send_to_mp3(val);

    return 0;
}


/* terminal task can be used to play one song.
 * use other tasks instead to control the tasks.
 */

