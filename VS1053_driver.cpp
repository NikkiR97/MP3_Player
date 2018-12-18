/*
 * VS1053.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: Nikkitha
 */
#include "VS1053.hpp"
// #include "ssp0.h"
// #include "ssp1.h"
// #include "gpio.hpp"

#include "LabGPIO_0.hpp"
// #include "LabSPI.hpp"
#include "LabGPIOInterrupts.hpp"
#include "spi_driver.hpp"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "io.hpp"
#include "time.h"
#include "utilities.h"
#include <fstream>
#include "ff.h"
#include "uart0_min.h"
#include "printf_lib.h"
#include "utilities.h"
//#include <dirent.h>
#include <sys/types.h>
#include <libgen.h>
#include <errno.h>


 VS1053::VS1053(LabGPIO_0 *dreq, LabGPIO_0 *xdcs, LabGPIO_0 *xcs, LabGPIO_0 *rst){//, LabSpi *spi){
//due to limitations in data allocation objects must be passed by reference.
    DREQ = dreq;
    XDCS = xdcs;
    XCS = xcs;
    RST = rst;
//    run_song = true;
    song_idx = 0;

    button_play = play; //initialize to pause state
    button_vol = ctrl_off;
    volume = 28;
    vlm = 0x20;
    song_pointer = 0; //always will start off by playing the song first on the list
    button_song = current;
}

VS1053::~VS1053(){
/*Do Nothing*/
}

void VS1053::vs_init(){

    DREQ->setAsInput(); //active high
    XDCS->setAsOutput(); //enabling sci
    XCS->setAsOutput(); //enabling sdi
    RST->setAsOutput();

    XDCS->setHigh(); //active low
    XCS->setHigh(); //active low
    RST->setLow(); //When the XRESET -signal is driven low, VS10531053b is reset and all the control registers and
    //internal states are set to the initial values.

    //set clock speed as 24Mhz
    ssp0_init(1);
//    spi_init(8, spi, Ext, 16);

    RST->setHigh();

//    write_to_sci(sci_mode,(1<<5)|0x0810); //1 0000 & 0000 1000 0001 0000 = 0x0810
    write_to_sci(sci_bass, 0x0000);
    write_to_sci(sci_audata, 0xAC45);
    write_to_sci(sci_clockf, 0x2000); //12mhz
    write_to_sci(sci_vol, 0x4040); //zero volume for now.

    // set_clock_speed(8); //12 Mhz
    ssp0_init(12);

    //set base volume as zero
//    spi_init(8, spi, Ext, 8);

    //printf("STATUS REGISTER: %x\n", (read_from_sci(sci_status))>>4 & 0x04);
    printf("STATUS REGISTER: %x\n", read_from_sci(sci_status));
    printf("MODE REGISTER: %x\n", read_from_sci(sci_mode));
    printf("AUDATA REGISTER: %x\n", read_from_sci(sci_audata));
    printf("VOLUME REGISTER: %x\n", read_from_sci(sci_vol));

}

void VS1053::write_to_sci(uint8_t addr, uint16_t data){
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

    while(!DREQ->getLevel()); //do the next 32 byte transfer only when dreq flag says the queque is availible
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

uint16_t VS1053::read_from_sci(uint8_t addr){
    uint8_t read;
    uint8_t val;
//    uint8_t dat0;
//    uint8_t dat1;
    uint8_t data;
    uint16_t data_;

    while(!DREQ->getLevel()); //do the next 32 byte transfer only when dreq flag says the queque is availible
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

void VS1053::setVolume(uint8_t vol){
    /*WARNING: Need to relinquish bus to allow for setVolume to execute its SCI operation!! */

//    Example: for a volume of -2.0 dB for the left channel and -3.5 dB for the right channel: (2.0/0.5)
//    = 4, 3.5/0.5 = 7 ! SCI_VOL = 0x0407. (pg. 48)
    vlm = vol;

    uint8_t left;
    uint8_t right;
    uint16_t LEFT = 0x0000;
    uint16_t RIGHT = 0x0000;

    left = vol << 1; // multiply by 2
    right = vol << 1; //multiply by 2
    LEFT = left;
    RIGHT = right;

    volume = (LEFT << 8) | RIGHT;

    printf("The volume is being set as %X \n", volume);
    write_to_sci(sci_vol, volume);
}

uint8_t VS1053::ret_vol(){
    return vlm;
}

void VS1053::volume_up(){
    if(ret_vol() > 0x0A){
    printf("The volume is being increased from %x\n", ret_vol());
    setVolume(ret_vol() - 4);
    }
}

void VS1053::volume_down(){
    if(ret_vol() < 0xFA){
    printf("The volume is being decreased from %x\n", ret_vol());
    setVolume(ret_vol() + 4);
    }
}

void VS1053::inc_volume(){
     button_vol = vol_up;
}

void VS1053::dec_volume(){
     button_vol = vol_down;
}

void VS1053::send_mp3_data(){//FILE *FD){

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
                while(!DREQ->getLevel()); //do the next 32 byte transfer only when dreq flag says the queque is availible
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

void VS1053::songLibrary(){
    FRESULT res;
    DIR dir;

    static FILINFO fno;
    int i=0;
    //max byte length of lfname is 128:
    char lfname[128];

    res = f_opendir(&dir, "1:/songs");
    if (res == FR_OK) {
        for (;;) {
         fno.lfname = lfname; //type cast TCHAR* to char*
         fno.lfsize = _MAX_LFN;

         res = f_readdir(&dir, &fno);
         if (res != FR_OK || fno.fname[0] == 0) break; //break on error or end of directory

         if(fno.fattrib & AM_DIR){ //verified directory

         }
         else{ //verified file
             printf("short name: %s/%s ------", "songs", fno.fname);
             if(i<SONG_COUNT+1){
             songs[i] = new char[128+1];
             strcpy(songs[i],fno.lfname);
             printf("long name: %s/%s\n", "songs", songs[i]);

//             char * pch= strstr(songs[i],".mp3");
//             char *first = songs[i];
//             strncpy(song_titles[i], songs[i], pch - first);
//
//             printf("The song titles are: %s", song_titles[i]);
             i++;
             }
         }

        }
    }
}

buttons VS1053::button_play_stat(){
    return button_play;
}

buttons VS1053::button_vol_stat(){
    return button_vol;
}

buttons VS1053::button_song_stat(){
    return button_song;
}

void VS1053::set_button_play(buttons value){
    button_play = value;
}

void VS1053::set_button_vol(buttons value){
    button_vol = value;
}

void VS1053::set_button_song(buttons value){
    button_song = value;
}

char* VS1053::retSong(){
    return songs[song_pointer];
}

void VS1053::incSongIdx(){ //toggle through lcd display
    if(song_pointer < SONG_COUNT){
        song_pointer++;
        printf("Song %i: %s\n", song_pointer, retSong());
    }
}

void VS1053::decSongIdx(){ //toggle through lcd display
    if(song_pointer > 0){
        song_pointer--;
        printf("Song %i: %s\n", song_pointer, retSong());
    }
}

int VS1053::retSongIdx(){
    return song_pointer;
}

void VS1053::setSongIdx(int idx){
    song_pointer = idx;
}

