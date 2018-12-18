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
#include "uart0_min.h"

#include "LabGPIO_0.hpp"
#include "LabGPIOInterrupts.hpp"
// #include "LabSPI.hpp"
#include "spi_driver.hpp"

/* returns minimum of two given numbers */
#define min(a,b) (((a)<(b))?(a):(b))

int i=0;

LabGPIO_0 _rst(1, 0);
LabGPIO_0 _dreq(29,0);
LabGPIO_0 _xdcs(7, 2);
LabGPIO_0 _xcs(6, 2);
// LabSpi ssp(i);
LabGPIO_0 blue_button(0,0);
LabGPIO_0 vol_up_button(1,2);
LabGPIO_0 vol_down_button(2,2);

VS1053 obj(&_dreq, &_xdcs, &_xcs, &_rst);//, &ssp);

QueueHandle_t q;
QueueHandle_t q2;
TaskHandle_t xHandle1;
TaskHandle_t xHandle2;
TaskHandle_t xHandle3;

SemaphoreHandle_t spi_bus_lock;
BaseType_t *flag;

void reader_task(void *pv){
    while(1){
    printf("\n\nEntered reader task.\n\n");

//    F_FILE *fd;
//    int file_size = f_size(&fd);

    //FILE *fd = fopen("1:songs/Louis Armstrong - What A Wonderful World.mp3", "r");
    //FILE *fd = fopen("1:songs/Fetty Wap - Trap Queen (Clean).mp3", "r");
    //FILE *fd = fopen("1:songs/The Weeknd - Can't Feel My Face.mp3", "r");
    //FILE *fd = fopen("1:songs/Belinda Carlisle - Heaven Is a Place on Earth Lyrics.mp3", "r");
    //FILE *fd = fopen("1:songs/SEA 30 SEC.mp3", "r");
    //FILE *fd = fopen("1:songs/Queen - Bohemian Rhapsody.mp3", "r");
    //FILE *fd = fopen("1:songs/Ariana Grande - Last Christmas.mp3", "r");

    //char tot[128] = {0};
    char dir[10] = "1:songs/";
    //strcat(tot, dir);
    strcat(dir, obj.retSong());

    uart0_puts(dir);

    FILE *fd = fopen(dir, "r");

    int dat_ofs = 0;
    uint8_t data[512];

    if(fd){
          printf("File successfully opened.\n\n");
          fseek(fd, 0, SEEK_END); //go to last position
          int file_size = ftell(fd);
          fseek(fd, 0, SEEK_SET); //go to the front

      //while(1){

                while(!feof(fd)){//while(dat_ofs < (file_size-1)){
                    if(obj.button_vol_stat() == ctrl_off){
                    fread(data, 1, 512, fd);
                    //f_read(&fd, 1, 512, &data)
                    xQueueSend(q, &data, 100);

                    while(obj.button_play_stat() == pause); //don't send any more 512 byte chunks
                    //dat_ofs += 512;
                    }

                    if(obj.button_song_stat() == diff_song){
                        break;
                    }
                }
                fclose(fd);
                obj.set_button_song(current); //reset back to current
        //           }
    }
    else{
        printf("File failed to open\n");
    }
    }
}

void player_task(void *pv){
    printf("Entered player task.\n\n");

    int dat_pos;
    uint8_t recv;
    uint8_t send;
    uint8_t data[512];
    uint8_t *dat;
    char file[32];
    char *fil;
    int name_size;
    int j=0;

    delay_ms(5);

    while(1){
//        if(i<32){
//        xQueueReceive(q2, file++, portMAX_DELAY);
//        i++;
//        }
//        else{

        xQueueReceive(q2, &file, portMAX_DELAY);
        fil = file;
        //xQueueReceive(q2, &file, portMAX_DELAY);
        name_size = strlen(fil);

        printf("File name size: %i \n\n", name_size);

        for(int i=0; i<7; i++){

        //printf("%c", file[i]);
        putchar(*fil++);
        }

        printf("\n");

        break;

    }

    while(1){
//        if(obj.button_vol_stat() == ctrl_off){

        xQueueReceive(q, &data, portMAX_DELAY);
        dat = data;

        dat_pos = 0;
        while(dat_pos < 512){

            while(!_dreq.getLevel());

//            while(!obj.ret_run_song_flag());
            _xdcs.setLow(); //chip select the decoder (active low)
            {
                send = *dat++;
                recv = ssp0_exchange_byte(send);
            }
            _xdcs.setHigh();

        dat_pos++;
//        }
        }
    }
}

void volume_task(void *pv){
    while(1){
        if(xSemaphoreTake(spi_bus_lock, portMAX_DELAY)){
            if(obj.button_vol_stat() == vol_up){
                 printf("Volume is increased. \n");
                 obj.volume_up(); //spi bus will do a SCI transaction to increase volume
            }
            else if(obj.button_vol_stat() == vol_down){
                 printf("Volume is decreased. \n");
                 obj.volume_down(); //spi bus will do a SCI transaction to decrease volume
            }

            obj.set_button_vol(ctrl_off);
            delay_ms(100);
        }
        vTaskDelay(500);
    }
}

CMD_HANDLER_FUNC(playSong)
{
    char name[10]; //standard 32-byte size
    // You can use FreeRTOS API or the wrapper resume() or suspend() methods
    if (cmdParams == "foo.mp3") {
        vTaskResume(xHandle1);
        strcpy(name, cmdParams.c_str());

        printf("The song name is given at the input: %s \n\n", name);
        delay_ms(10);

        xQueueSend(q2, &name, 500);
    }
    else {
        vTaskSuspend(xHandle1);
    }

    return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void send_to_mp3(void *pv){
    delay_ms(500);

    printf("Entered MP3 player function. \n\n");
    //FILE *fd = fopen("1:songs/Ariana Grande - Last Christmas.mp3", "r");
    FILE *fd = fopen("1:songs/Fetty Wap - Trap Queen (Clean).mp3", "r");
    //FILE *fd = fopen("1:songs/The Weeknd - Can't Feel My Face.mp3", "r");
    //FILE *fd = fopen("1:SEA 30 SEC.mp3", "r");
    // FILE *fd = fopen("1:songs/Belinda Carlisle - Heaven Is a Place on Earth Lyrics.mp3", "r");

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

    printf("The file has been attempted to be opened.\n");

        if(fd){
            printf("File successfully opened.\n\n");

            while(1){
            while(!feof(fd) && (obj.button_play_stat() == play)){
            val = fread(data, 1, 512, fd);

                dat = data;
                //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes
                while(dat_pos < 512){

                    while(!_dreq.getLevel());
                    _xdcs.setLow(); //chip select the decoder (active low)
                    {
                        send = *dat++;
                        recv = ssp0_exchange_byte(send);
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

void play_pause_isr(){
    if(obj.button_play_stat() == play){ //if true
        delay_ms(10);
        obj.set_button_play(pause);
//        obj.pauseSong(); //set to false
        printf("Pause the song \n");
    }
    else if(obj.button_play_stat() == pause){//if false
        delay_ms(10);
        obj.set_button_play(play);
//        obj.resumeSong(); //set to true
        printf("Resume the song \n");
    }
}

void volume_up_intr(){ //warning need to relinquish bus control
    printf("Volume up triggered. \n");

    obj.inc_volume();

    long yield = 0;

    xSemaphoreGiveFromISR(spi_bus_lock, &yield);
    portYIELD_FROM_ISR(yield);

}

void volume_down_intr(){ //warning need to relinquish bus control
    printf("Volume down triggered. \n");

    obj.dec_volume();

    long yield = 0;

    xSemaphoreGiveFromISR(spi_bus_lock, &yield);
    portYIELD_FROM_ISR(yield);

}

void select_song_isr(){ //make song change, set bool flag
    obj.set_button_song(diff_song);
}

void up_cursor_isr(){
    obj.incSongIdx();
    delay_ms(100);
}

void down_cursor_isr(){
    obj.decSongIdx();
    delay_ms(100);
}

int main(void) {
   spi_bus_lock = xSemaphoreCreateBinary();
   q = xQueueCreate(1, sizeof(uint8_t)*512); //used for sending the mp3 file data
   q2 = xQueueCreate(1, sizeof(char)*10); //used for sending the title

   const uint32_t STACK_SIZE_WORDS = 1024; //make sure the stack size is big enough to send data through

    int *val;
    bool IntrAttach, IntrAttach2, IntrAttach3;

   obj.vs_init();

   printf("Initialization is Complete.\n");

   obj.songLibrary(); //adding songs to the library

   blue_button.setAsInput();
   vol_up_button.setAsInput();
   vol_down_button.setAsInput();

   LabGPIOInterrupts* gpio_interrupt = LabGPIOInterrupts::CreateOneInstance();
   // Init things once
   gpio_interrupt->Initialize();

//   IntrAttach = gpio_interrupt->AttachInterruptHandler(0, 0, &play_pause_isr, kRisingEdge);
//   IntrAttach2 = gpio_interrupt->AttachInterruptHandler(2, 1, &volume_up_intr, kRisingEdge);
//   IntrAttach3 = gpio_interrupt->AttachInterruptHandler(2, 2, &volume_down_intr, kRisingEdge);
   IntrAttach = gpio_interrupt->AttachInterruptHandler(0, 0, &select_song_isr, kRisingEdge);
   IntrAttach2 = gpio_interrupt->AttachInterruptHandler(2, 1, &up_cursor_isr, kRisingEdge);
   IntrAttach3 = gpio_interrupt->AttachInterruptHandler(2, 2, &down_cursor_isr, kRisingEdge);

   if(IntrAttach && IntrAttach2 && IntrAttach3){
       printf("All interrupts attached to isr's. \n");
   }
   else{
       printf("ISR could not be attached.\n");
   }

   scheduler_add_task(new terminalTask(PRIORITY_HIGH));

   xTaskCreate(reader_task, "t1",  STACK_SIZE_WORDS,(void *) 1, 1, &xHandle1); //sender
   xTaskCreate(player_task, "t2",  STACK_SIZE_WORDS,(void *) 1, 1, &xHandle2); //receiver
   xTaskCreate(volume_task, "t3",  STACK_SIZE_WORDS,(void *) 1, 1, &xHandle3);

   vTaskSuspend(xHandle1);

   scheduler_start();

   vTaskStartScheduler();

//   send_to_mp3(val);

    return 0;
}


/* terminal task can be used to play one song.
 * use other tasks instead to control the tasks.
 */

