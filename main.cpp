#include "LPC17xx.h"
//#include "FreeRTOS.h"
#include "tasks.hpp"
// #include "examples/examples.hpp"
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
#include "LabGPIO_0.hpp"
#include "LabGPIOInterrupts.hpp"
#include "semphr.h"
#include "spi_sem.h"
#include "io.hpp"
#include "utilities.h"
#include "command_handler.hpp"
#include "handlers.hpp"
#include "event_groups.h"
#include "time.h"
#include <fstream>
#include "LabSPI.hpp"
#include "semphr.h"
#include "ssp0.h"
#include "ssp1.h"
#include "gpio.hpp"
#include "VS1053_driver.hpp"
#include "tasks.hpp"
#include "ff.h"

/* returns minimum of two given numbers */
#define min(a,b) (((a)<(b))?(a):(b))

GPIO _dreq(P0_29);
GPIO _xdcs(P2_7);
GPIO _xcs(P2_6);
GPIO _rst(P0_1);

QueueHandle_t q;
QueueHandle_t q2;
TaskHandle_t xHandle1;
TaskHandle_t xHandle2;

void reader_task(void *pv){
    printf("\n\nEntered reader task.\n\n");

//    F_FILE *fd;
//    fd = f_open(&fd, "1:Watchdog/stuck.txt", FA_OPEN_EXISTING|FA_READ);
//    int file_size = f_size(&fd);

    //FILE *fd = fopen("1:Louis Armstrong - What A Wonderful World (Lyrics).mp3", "r");
    //FILE *fd = fopen("1:Watchdog/stuck.txt", "r");
    //FILE *fd = fopen("1:Fetty Wap - Trap Queen (Official Video) Prod. By Tony Fadd", "r");
    //FILE *fd = fopen("1:The Weeknd - Can't Feel My Face.mp3", "r");
    FILE *fd = fopen("1:Belinda Carlisle - Heaven Is a Place on Earth Lyrics.mp3", "r");

    int dat_ofs = 0;
    uint8_t data[512];

    if(fd){
          printf("File successfully opened.\n\n");
          fseek(fd, 0, SEEK_END); //go to last position
          int file_size = ftell(fd);
          fseek(fd, 0, SEEK_SET); //go to the front

      //while(1){

                while(!feof(fd)){//while(dat_ofs < (file_size-1)){
                    fread(data, 1, 512, fd);
                    //f_read(&fd, 1, 512, &data)
                    xQueueSend(q, &data, 100);

                    //dat_ofs += 512;
                }
                fclose(fd);
        //           }
    }
    else{
        printf("File failed to open\n");
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

        xQueueReceive(q, &data, portMAX_DELAY);
        dat = data;

        dat_pos = 0;
        while(dat_pos < 512){

            while(!_dreq.read());

            _xdcs.setLow(); //chip select the decoder (active low)
            {
                send = *dat++;
                recv = ssp0_exchange_byte(send);
            }
            _xdcs.setHigh();

        dat_pos++;
        }

    }
}

void send_to_mp3(){
    printf("Entered MP3 player function. \n\n");
    //FILE *fd = fopen("1:Louis Armstrong - What A Wonderful World (Lyrics).mp3", "r");
    FILE *fd = fopen("1:Fetty Wap - Trap Queen (Clean).mp3", "r");
    //FILE *fd = fopen("1:The Weeknd - Can't Feel My Face.mp3", "r");
    //FILE *fd = fopen("1:SEA 30 SEC.mp3", "r");
    //FILE *fd = fopen("1:Belinda Carlisle - Heaven Is a Place on Earth Lyrics.mp3", "r");

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

            while(!feof(fd)){//while(!(val < 512)){ //while(1){//while(dat_ofs < (file_size-1)){
            val = fread(data, 1, 512, fd);
//            consume(data, val);

                dat = data;
                //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes
                while(dat_pos < 512){

                    while(!_dreq.read());
                    _xdcs.setLow(); //chip select the decoder (active low)
                    {
                        send = *dat++;
                        recv = ssp0_exchange_byte(send);
                    }
                    _xdcs.setHigh();

                dat_pos++;
                }

                dat_pos = 0;
                //dat_ofs += 512;

//                if(val == 0){
//                    break;
//                }
            }
            fclose(fd);
        }
        else{
            printf("File failed to open\n");
        }

}

//open_task(){
//
//}

void send_to_mp3_2(){
    printf("Entered MP3 player function. \n\n");
    FIL fd;
    FRESULT res;
    char title[100];
    char songName[200];
    //strcpy(title, "1:");
    strcpy(songName, "1:Fetty Wap - Trap Queen (Clean).mp3");
    //strcpy(songName, "1:Belinda Carlisle - Heaven Is a Place on Earth Lyrics.mp3");

    uint8_t data[512];
    uint8_t *dat;
    uint8_t send;
    uint8_t recv;
    unsigned int bytes_read;
    int dat_pos = 0;

    res = f_open(&fd, songName, FA_OPEN_EXISTING | FA_READ);
    printf("The file has been attempted to be opened.");

        if(res == FR_OK){
            printf("File successfully opened.\n\n");

            while(1) { //while(!feof(fd)){//while(1){//while(dat_ofs < (file_size-1)){
            res = f_read(&fd, data, 512, &bytes_read);
            if (res || bytes_read == 0) break;

                dat = data;
                //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes
                while(dat_pos < 512){

                    while(!_dreq.read());
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
            printf("file end\n");
            f_close(&fd);
        }
        else{
            printf("File failed to open\n");
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



int main(void) {

    VS obj(&_dreq, &_xdcs, &_xcs, &_rst);
    obj.init();

    printf("Initialization is Complete.\n");

//    obj.write_to_sci(sci_mode, 0x8C40); //1100 1100 1100 0000
//    obj.write_to_sci(sci_bass, 0x2222); //1100 1100 1100 0000
//    obj.write_to_sci(sci_vol, 0x0707); //1100 1100 1100 0000
//    printf("MODE REGISTER: %x\n", obj.read_from_sci(sci_mode));
//    obj.send_mp3_data();
//    send_to_mp3();

    q = xQueueCreate(1, sizeof(uint8_t)*512); //used for sending the mp3 file data
    q2 = xQueueCreate(1, sizeof(char)*10); //used for sending the title

    const uint32_t STACK_SIZE_WORDS = 1024; //make sure the stack size is big enough to send data through

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    xTaskCreate(reader_task, "t1",  STACK_SIZE_WORDS,(void *) 1, 1, &xHandle1); //sender
    xTaskCreate(player_task, "t2",  STACK_SIZE_WORDS,(void *) 1, 1, &xHandle2); //receiver

    vTaskSuspend(xHandle1);

    scheduler_start();

    vTaskStartScheduler();

    return 0;
}

