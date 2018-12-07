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

VS::VS(GPIO *dreq, GPIO *xdcs, GPIO *xcs, GPIO *rst){
//due to limitations in data allocation objects must be passed by reference.
    DREQ = dreq;
    XDCS = xdcs;
    XCS = xcs;
    RST = rst;
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

    FILE *FD = fopen("1:Louis Armstrong - What A Wonderful World (Lyrics).mp3", "r");
    //setVolume(0x50);
    //write_to_sci(sci_vol, 0x0407);

    uint8_t buffer[512];
    uint8_t recv;
    uint8_t *buf;
    uint8_t buf_pos=0;
    int buf_ofs = 0;

    fseek(FD, 0L, SEEK_END); //go to last position
    int file_size = ftell(FD);
    fseek(FD, 0L, SEEK_SET); //go to the front

    printf("The size of the file is %i", file_size);

    if(FD){
        printf("File successfully opened.\n\n");
        while(feof(FD)){//while(buf_ofs < (file_size-1)){ //do 100 read transactions
        fread(buffer, 1, 512, FD); //read 31 elements, the element has a size of 1 bytes, stored into a variable buffer
        buf = buffer;

            while(buf_pos < 512){
                while(!DREQ->read()); //do the next 32 byte transfer only when dreq flag says the queque is availible
                  //if DREQ is high then you may commence the transfer of the bytes

                XDCS->setLow(); //chip select the decoder (active low)
                delay_us(1);
                {
                recv = ssp0_exchange_byte(*buf++);
                }
                XDCS->setHigh();
                delay_us(1);

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
