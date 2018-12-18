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
//#include "LabSPI.hpp"

#define sci_status 0x01
#define sci_bass 0x02
#define sci_mode 0x00
#define sci_audata 0x05
#define sci_clockf 0x03
#define sci_vol 0x0B

#define write_op_sci 0x02
#define read_op_sci 0x03

typedef enum{
    play = 0,
    pause = 1,
    vol_up = 2,
    vol_down = 3,
    ctrl_off = 4,
    current = 5,
    diff_song = 6
}buttons;

class VS1053{
private:
    LabGPIO_0* DREQ;
    LabGPIO_0* XDCS;
    LabGPIO_0* XCS;
    LabGPIO_0* RST;
    // LabSpi* SPI;

//    volatile bool run_song; //if true continue running song, if false, pause song
    buttons button_play;
    buttons button_vol;
    buttons button_song;
    char *songs[15]; //contains all the songs
    char *song_titles[15]; //contains all the song name including artist (w/o mp3 signature)
    int song_idx; //indexes the song you want - changes when external button is set
    uint16_t volume; //volume that gets set when external button is pressed
    uint8_t vlm;
    int song_pointer;

public:
    VS1053(LabGPIO_0 *dreq, LabGPIO_0 *xdcs, LabGPIO_0 *xcs, LabGPIO_0 *rst);//, LabSpi *spi);
    ~VS1053();
    void vs_init(); /*setup using the SCI register*/
    // void check_dreq(){};
    void send_mp3_data();//FILE *FD);
    void write_to_sci(uint8_t addr, uint16_t data);
    uint16_t read_from_sci(uint8_t addr);
    void setVolume(uint8_t vol); //adjust the volume
    uint8_t ret_vol();
    void volume_up();
    void volume_down();
    void inc_volume();
    void dec_volume();

    void set_button_play(buttons value);
    void set_button_vol(buttons value);
    void set_button_song(buttons value);

    buttons button_play_stat();
    buttons button_vol_stat();
    buttons button_song_stat();

    void songLibrary(); //read all songs from the directory and store their names in the songs array
    char* retSong();
    void getSongs();
    void incSongIdx();
    void decSongIdx();
};


#endif /* VS1053_DRIVER_HPP_ */
