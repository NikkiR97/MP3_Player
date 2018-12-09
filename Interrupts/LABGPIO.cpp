#include "LPC17xx.h"
#include "LabGPIO.hpp"
#include <stdio.h>

LabGPIO::LabGPIO(uint8_t port_num, uint8_t pin_num) 
{ 
	pin = pin_num; 
	switch (port_num)
	{
		// gpio will point to port's starting memory location
		case 0: 
			gpio = LPC_GPIO0;
			break;  

		case 1: 
			gpio = LPC_GPIO1;
			break; 

		case 2: 
			gpio = LPC_GPIO2;
			break;

		default: 
			break; 
	}
} 

void LabGPIO::setAsInput() 
{
	// Sets pin as input | Input = 0 
	gpio->FIODIR &= ~(1 << pin);
}

void LabGPIO::setAsOutput() 
{
	// Sets pin as output
	gpio->FIODIR |= (1 << pin);
}

void LabGPIO::setDirection(bool output)
{
	// Input determines whether direction is an output/input
	(output) ? setAsOutput() : setAsInput(); 
}

void LabGPIO::setHigh() 
{
	// Set pin as high 
	gpio->FIOPIN |= (1 << pin);
}

void LabGPIO::setLow()
{
	// Set pin as low
	gpio->FIOPIN &= ~(1 << pin);
}

void LabGPIO::set(bool high)
{
	// Set pin as high/low
	(high) ? setHigh() : setLow(); 
}

bool LabGPIO::getLevel() 
{
	// Checks if pin is high/low
	if(gpio->FIOPIN & (1 << pin)) 
	{
    	return true; // high
	}
	else 
	{
    	return false; // low
	}
}

LabGPIO::~LabGPIO()
{
	printf("LabGPIO destructor called!\n");
}