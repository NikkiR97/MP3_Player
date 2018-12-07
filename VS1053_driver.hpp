/*
 * VS1053_driver.hpp
 *
 *  Created on: Nov 14, 2018
 *      Author: Nikkitha
 */

#ifndef VS1053_DRIVER_HPP_
#define VS1053_DRIVER_HPP_

#include "LPC17xx.h"
#include "ssp0.h"
#include "ssp1.h"
#include "gpio.hpp"
#include <fstream>

#define sci_status 0x01
#define sci_bass 0x02
#define sci_mode 0x00
#define sci_audata 0x05
#define sci_clockf 0x03
#define sci_vol 0x0B

#define write_op_sci 0x02
#define read_op_sci 0x03

//GPIO DREQ(P0_29);
//GPIO XDCS(P0_0);
//GPIO XCS(P0_30);
//GPIO RST(P0_1);

class VS{
private:
    GPIO* DREQ;
    GPIO* XDCS;
    GPIO* XCS;
    GPIO* RST;
public:
    VS(GPIO *dreq, GPIO *xdcs, GPIO *xcs, GPIO *rst);
    ~VS();
    void init(); /*setup using the SCI register*/
//    void check_dreq(){};
    void send_mp3_data();//FILE *FD);
    void write_to_sci(uint8_t addr, uint16_t data);
    uint16_t read_from_sci(uint8_t addr);
    void setVolume(uint8_t vol);
};


#endif /* VS1053_DRIVER_HPP_ */
