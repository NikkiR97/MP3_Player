/*
 * LabGPIO_O.c
 *
 *  Created on: Aug 30, 2018
 *      Author: Nikkitha
 */

#include "LPC17xx.h" // always set as the first header
#include <LabGPIO_0.hpp>

LabGPIO_0 :: LabGPIO_0(uint8_t pinnum, uint8_t port){
     pin = pinnum;
     if (port == 0) {
       LPC = LPC_GPIO0;
     } else if (port == 2) {
       LPC = LPC_GPIO2;
     }
     else LPC = LPC_GPIO1; //accessing port 1 allocated data
}

void LabGPIO_0 :: setAsInput(){
    LPC->FIODIR &= ~(1 << pin);
}

void LabGPIO_0 :: setAsOutput(){
    LPC->FIODIR |= (1 << pin);
}

void LabGPIO_0 :: setDirection(bool output){
    if(output == true){
        LPC->FIODIR |= (1 << pin);
    }
    else{
        LPC->FIODIR &= ~(1 << pin);
    }
}

void LabGPIO_0 :: setHigh(){
    LPC->FIOSET = (1 << pin);
}

void LabGPIO_0 :: setLow(){
    LPC->FIOCLR = (1 << pin);
}

void LabGPIO_0 :: set(bool High){
    if(High == true){
        setHigh();
    }
    else{
        setLow();
    }
}

bool LabGPIO_0 :: getLevel(){
    return !!(LPC->FIOPIN & (1 << pin));
}

LabGPIO_0::~LabGPIO_0(){
    //no dynamic variables used, did not include anything for destructor
}
