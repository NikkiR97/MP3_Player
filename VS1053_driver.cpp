/*
 * VS1053_driver.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: Nikkitha
 */
#include "VS1053_driver.hpp"
#include "ssp0.h"
#include "ssp1.h"
#include "gpio.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "io.hpp"
#include "time.h"
#include "utilities.h"
#include <fstream>
#include "ff.h"
#include "uart0_min.h"
//#include <dirent.h>
#include <sys/types.h>
#include <libgen.h>

VS::VS(GPIO *dreq, GPIO *xdcs, GPIO *xcs, GPIO *rst){
//due to limitations in data allocation objects must be passed by reference.
    DREQ = dreq;
    XDCS = xdcs;
    XCS = xcs;
    RST = rst;
    run_song = false;
    song_idx = 0;
}

VS::~VS(){
/*Do Nothing*/
}

void VS::init(){
    DREQ->setAsInput(); //active high
    XDCS->setAsOutput(); //enabling sci
    XCS->setAsOutput(); //enabling sdi
    RST->setAsOutput();

    XDCS->setHigh(); //active low
    XCS->setHigh(); //active low
    RST->setLow(); //When the XRESET -signal is driven low, VS1053b is reset and all the control registers and
    //internal states are set to the initial values.

    //set clock speed as 24Mhz
    ssp0_init(1);

    RST->setHigh();

//    write_to_sci(sci_mode,(1<<5)|0x0810); //1 0000 & 0000 1000 0001 0000 = 0x0810
    write_to_sci(sci_bass, 0x0000);
    write_to_sci(sci_audata, 0xAC45);
    write_to_sci(sci_clockf, 0x2000); //12mhz

    ssp0_init(12);

    //set base volume as zero
    write_to_sci(sci_vol, 0x6060); //zero volume for now.

    //printf("STATUS REGISTER: %x\n", (read_from_sci(sci_status))>>4 & 0x04);
    printf("STATUS REGISTER: %x\n", read_from_sci(sci_status));
    printf("MODE REGISTER: %x\n", read_from_sci(sci_mode));
    printf("AUDATA REGISTER: %x\n", read_from_sci(sci_audata));
    printf("VOLUME REGISTER: %x\n", read_from_sci(sci_vol));

}

void VS::write_to_sci(uint8_t addr, uint16_t data){
    uint8_t write;
    uint8_t val;
//    uint8_t dat0;
//    uint8_t dat1;
    uint8_t info;
    uint8_t info_;

    uint8_t dat0;
    uint8_t dat1;
    dat0 = data>>8;
    dat1 = data & 0x00ff;

    while(!DREQ->read()); //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes
                 //chip select the decoder (active low)

    printf("The decoder is ready. \n\n");
    delay_ms(100);

    XCS->setLow(); //chip select the decoder (active low)
    {
    //recv = music.transfer(buffer[j]);
    write = ssp0_exchange_byte(write_op_sci);
    printf("recv byte after write op sent: %x \n", write);
    val = ssp0_exchange_byte(addr);
    printf("recv byte after address sent: %x \n", val);
    info = ssp0_exchange_byte(dat0);
    printf("recv byte after info sent: %x \n", info);
    info_ = ssp0_exchange_byte(dat1);
    printf("recv byte after info sent: %x \n", info_);
    }
    XCS->setHigh();
}

uint16_t VS::read_from_sci(uint8_t addr){
    uint8_t read;
    uint8_t val;
//    uint8_t dat0;
//    uint8_t dat1;
    uint8_t data;
    uint16_t data_;

    while(!DREQ->read()); //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes
                 //chip select the decoder (active low)

    printf("The decoder is ready. \n\n");
    delay_ms(100);

    XCS->setLow(); //chip select the decoder (active low)
    {
    //recv = music.transfer(buffer[j]);
    read = ssp0_exchange_byte(read_op_sci);
    printf("recv byte after read op sent: %x \n", read);
    val = ssp0_exchange_byte(addr);
    printf("recv byte after address sent: %x \n", val);
    data = ssp0_exchange_byte(0x00);
    printf("recv byte - data partial - after 0x00 sent: %x \n", data);//((dat0>>4) & 0x04));
    data_ = (data<<8) | ssp0_exchange_byte(0x00);
    printf("recv byte - data - after 0x00 sent: %x \n", data_);
    }
    XCS->setHigh();

    return data_;
}

void VS::setVolume(uint8_t vol){
//    Example: for a volume of -2.0 dB for the left channel and -3.5 dB for the right channel: (2.0/0.5)
//    = 4, 3.5/0.5 = 7 ! SCI_VOL = 0x0407. (pg. 48)
    uint8_t left;
    uint8_t right;
    uint16_t volume;

    left = vol << 1; //multiply by two
    right = vol << 1; //multiply by two
    volume = (left << 8) | right;

    write_to_sci(sci_vol, volume);

}

void VS::send_mp3_data(){//FILE *FD){

    //FILE *FD = fopen("1:Louis Armstrong - What A Wonderful World (Lyrics).mp3", "r");
    //FILE *FD = fopen("1:Fetty Wap - Trap Queen (Clean).mp3", "r");
    //FILE *FD = fopen("1:Ariana Grande - Last Christmas.mp3", "r");
    FILE *FD= fopen("1:The Weeknd - Can't Feel My Face.mp3", "r");
    //setVolume(0x50);
    //write_to_sci(sci_vol, 0x0407);

    uint8_t buffer[512];
    uint8_t recv;
    uint8_t *buf;
    uint8_t buf_pos=0;
    int buf_ofs = 0;
    uint8_t send;

    printf("Playing Ariana Grande - Last Christmas \n\n");

    fseek(FD, 0L, SEEK_END); //go to last position
    int file_size = ftell(FD);
    fseek(FD, 0L, SEEK_SET); //go to the front0_

    printf("The size of the file is %i \n\n", file_size);

    if(FD){
        uart0_puts("File successfully opened.");
        while(!feof(FD)){//while(buf_ofs < (file_size-1)){ //do 100 read transactions
        //uart0_puts("in loop.\n");
        fread(buffer, 1, 512, FD); //read 31 elements, the element has a size of 1 bytes, stored into a variable buffer
        buf = buffer;
        //uart0_puts("in loop_1.\n");
            while(buf_pos < 512){
                //uart0_puts("in loop2.\n");
                while(!DREQ->read()); //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes

                XDCS->setLow(); //chip select the decoder (active low)
                {
                send = buffer[buf_pos];//*buf++;
                    //send = *buf++;
                printf("%d",send);
                recv = ssp0_exchange_byte(send);//buffer[buf_pos]);
                }
                XDCS->setHigh();

                buf_pos++;
            }

            buf_pos = 0; //reset the position indexer
//            buf_ofs += 512;
        }
        fclose(FD);
    }
    else{
        printf("File failed to open\n");
    }
}

FRESULT VS::songLibrary(char* path){
//    DIR *dir;
//    struct dirent *folder;
//    int fd;

//    dir = fdopendir(fd);
//
//    for(int i= 0; i<256; i++){
//        dir->d_name[i];
//    }
//
//    folder = readdir(dir);

    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = songLibrary(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    //printf("long filename: %s\n", *fno.lfname);

    return res;
}

void VS::pauseSong(){ //prevent streaming of bytes ~ will set run_song boolean flag
    run_song = false;
}
void VS::resumeSong(){ //continue streaming of bytes ~ will reset run_song boolean flag
    run_song = true;
}
bool VS::ret_run_song_flag(){
    return run_song;
}
