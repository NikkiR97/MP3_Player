#include "LPC17xx.h"
#include "stdlib.h"
#include <stdio.h>

typedef enum//helps set variables as predefined constants
{
    /* Fill this out based on the datasheet. */
    //refer to the datasheet table for Control Register
    spi = 0, //Motorola SPI
    ti = 1, //4-wire TI SSI
    nsm = 2 //Microwire Buses
}FM;

typedef enum
{
    Ext = 0, //SSP0 -> configure GPIO pins
    On = 1 //SSP1
}DT;

/**
 * 1) Powers on SPPn peripheral
 * 2) Set peripheral clock
 * 3) Sets pins for specified peripheral to MOSI, MISO, and SCK
 *
 * @param data_size_select transfer size data width; To optimize the code, look for a pattern in the datasheet
 * @param format is the code format for which synchronous serial protocol you want to use.
 * @param divide is the how much to divide the clock for SSP; take care of error cases such as the value of 0, 1, and odd numbers
 *
 * @return true if initialization was successful
 */
static inline bool spi_print(){
  printf("printinnng \n\n");
}

static inline bool spi_init(uint8_t data_size_select, FM format, DT device, uint8_t divide){
printf("Entered spi_init function. \n");

int division_factor;
//use divide directly on CPSR pick the division amount you want

  if(format == spi){

    if((divide != 1 && divide % 2 != 0) || divide <= 0 || divide > 8){

      printf("Invalid Division Input\n\n");
      return false;
    }
    else{
      //use the below code if you would like to divide PCLKSEL1
      if(divide == 8){
          division_factor == 3;
      }
      else if(divide == 4){
          division_factor == 0;
      }
      else if(divide == 2){
          division_factor == 2;
      }

        if(device == Ext){ //enable SSP0

        LPC_SC->PCONP |= (1<<21); //enable SSP0

        LPC_SC->PCLKSEL1 &= ~(0x03<<10); //select PCLK_SSP0, clear clock bits
        LPC_SC->PCLKSEL1 |= (1<<10); //CLK / 1
        LPC_SSP0->CPSR = divide; //SCK Frequency for Continuous Array Read(Low Frequency) is 33Mhx max. here we

        LPC_SSP0->CR0 |=  data_size_select-1; //8-bit operation
        LPC_SSP0->CR1 |=  (1<<1); //set master mode for SJ-board

        }
        else if(device == On){ //enable SSP1
          LPC_SC->PCONP |= (1<<10); //power up SPI functionality

          LPC_SC->PCLKSEL0 &= ~(0x03<<21); //select PCLK_SSP1, clear clock bits
          LPC_SC->PCLKSEL0 |= (1<<21); //CLK / 1
          LPC_SSP1->CPSR = divide; //set to again divide by two

          LPC_SSP1->CR0 |=  data_size_select-1; //perform 8-bit operation
          LPC_SSP1->CR1 |=  (1<<1); //set master mode for SJ-board
        }
        return true;
    }
  }
  else{
    printf("We are not handling other protocols at this time.");
  }

}

/**
 * Transfers a byte via SSP to an external device using the SSP data register.
 * This region must be protected by a mutex static to this class.
 *
 * @return received byte from external device via SSP data register.
 */
static inline uint8_t spi_transfer(uint8_t send){
  if(LPC_SSP0->SR & (1<<1)){ //fifo not full
    LPC_SSP0->DR = send; //send out data on the MISO line
  }
  while(LPC_SSP0->SR & (1 << 4)){ // check the status register and when you see that when ready you can return byte to write
    //waiting
  }
  //checking the busy flag
  //keep looping if the fifo is full, only load data register if the fifo is empty
  return LPC_SSP0->DR; //recieve data from the slave on the MOSI line
}

static inline void set_clock_speed(uint8_t divider){
  LPC_SSP0->CPSR = divider;
}