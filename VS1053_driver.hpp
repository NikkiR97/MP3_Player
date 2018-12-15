/*
 * VS1053_driver.hpp
 *
 *  Created on: Nov 14, 2018
 *      Author: Nikkitha
 */

#ifndef VS1053_HPP_
#define VS1053_HPP_

#include "LPC17xx.h"
#include "ssp0.h"
#include "ssp1.h"
#include "gpio.hpp"
#include <fstream>
#include "ff.h"

#include "LabGPIO_0.hpp"
#include "LabGPIOInterrupts.hpp"
#include "LabSPI.hpp"

#define sci_status 0x01
#define sci_bass 0x02
#define sci_mode 0x00
#define sci_audata 0x05
#define sci_clockf 0x03
#define sci_vol 0x0B

#define write_op_sci 0x02
#define read_op_sci 0x03

class VS1053{
private:
    LabGPIO_0* DREQ;
    LabGPIO_0* XDCS;
    LabGPIO_0* XCS;
    LabGPIO_0* RST;
    // LabSpi* SPI;

    volatile bool run_song; //if true continue running song, if false, pause song
    std::string songs[10]; //contains all the songs
    int song_idx; //indexes the song you want - changes when external button is set
    //uint8_t volume; //volume that gets set when external button is pressed

public:
    VS1053(LabGPIO_0 *dreq, LabGPIO_0 *xdcs, LabGPIO_0 *xcs, LabGPIO_0 *rst);//, LabSpi *spi);
    ~VS1053();
    void vs_init(); /*setup using the SCI register*/
    // void check_dreq(){};
    void send_mp3_data();//FILE *FD);
    void write_to_sci(uint8_t addr, uint16_t data);
    uint16_t read_from_sci(uint8_t addr);
    void setVolume(uint8_t vol); //adjust the volume
    void pauseSong(); //prevent streaming of bytes ~ will set run_song boolean flag
    void resumeSong(); //continue streaming of bytes ~ will reset run_song boolean flag
    bool ret_run_song_flag();
    FRESULT songLibrary(char* path); //read all songs from the directory and store their names in the songs array
};


#endif /* VS1053_DRIVER_HPP_ */
