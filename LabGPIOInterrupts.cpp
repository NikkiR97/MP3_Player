#include "LPC17xx.h"
#include "LabGPIOInterrupts.hpp"
#include "stdlib.h"
#include "stdint.h"
#include "lpc_isr.h"
#include "LabGPIO_0.hpp"

/**
 * Allocate a lookup table matrix here of function pointers (avoid dynamic allocation)
 * Upon AttachInterruptHandler(), you will store the user's function callback
 * Upon the EINT3 interrupt, you will find out which callback to invoke based on Port/Pin status.
 */

 //All the function pointers associated with the isr's are inputted here
IsrPointer LabGPIOInterrupts::pin_isr_map[2][32] =
{
  {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr} ,

    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr}
};

/**
 * Optional: LabGpioInterrupts could be a singleton class, meaning, only one instance can exist at a time.
 * Look up how to implement this. It is best to not allocate memory in the constructor and leave complex
 * code to the Initialize() that you call in your main()
 */

 LabGPIOInterrupts* LabGPIOInterrupts::instance = 0; //initialize the instance

LabGPIOInterrupts::LabGPIOInterrupts(){
 //leave empty, shouldn't be using at any time
}

LabGPIOInterrupts* LabGPIOInterrupts::CreateOneInstance(){
    if (instance == 0)
    {
        instance = new LabGPIOInterrupts();
    } // newly created instance can be called in main as:  LabGpioInterrupts* s = LabGpioInterrupts::CreateOneInstance();
      //cannot call as LabGpioInterrupts Obj; -> then call: Obj.CreateOneInstance (because the instance would already have been created)

    return instance;
}

/**
 * This should configure NVIC to notice EINT3 IRQs; use NVIC_EnableIRQ()
 */
void LabGPIOInterrupts::Initialize(){
    isr_register(EINT3_IRQn, HandleInterrupt);
    NVIC_EnableIRQ(EINT3_IRQn);
    //LabGPIO_0 var1(0xFF); //sets port 0 pin
    //LabGPIO_0 var2(0xFE); //sets port 2 pin
    //var1.setAsInput();
    //var2.setAsInput();
}

/**
 * This handler should place a function pointer within the lookup table for the HandleInterrupt() to find.
 *
 * @param[in] port         specify the GPIO port, and 1st dimension of the lookup matrix
 * @param[in] pin          specify the GPIO pin to assign an ISR to, and 2nd dimension of the lookup matrix
 * @param[in] pin_isr      function to run when the interrupt event occurs
 * @param[in] condition    condition for the interrupt to occur on. RISING, FALLING or BOTH edges.
 * @return should return true if valid ports, pins, isrs were supplied and pin isr insertion was successful
 */
bool LabGPIOInterrupts::AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition){
//there are only two ports: 0 and 2 -> each of the two columns of the array respectively
//then 32 slots for each pint for each of the ports

if(port == 0){
pin_isr_map[0][pin] = pin_isr; //set up the appropriate pin to the right port
}
else if(port == 2){
  pin_isr_map[1][pin] = pin_isr;
}

//for each if statement set the port flags
      if(port == 0){
         if(condition == kRisingEdge){ //set using |=
            LPC_GPIOINT->IO0IntEnR |= (1<<pin); // GPIO Interrupt Enable for Rising Edge
         }
         else if(condition == kFallingEdge){
            LPC_GPIOINT->IO0IntEnF |= (1<<pin);
         }
         else if(condition == kBothEdges){
            // LPC_GPIOINT->IO0IntEnR |= (1<<pin);
            // LPC_GPIOINT->IO0IntEnF |= (1<<pin);
         }
      }
      else if(port == 2){
        if(condition == kRisingEdge){ //set using |=
           LPC_GPIOINT->IO2IntEnR |= (1<<pin); // GPIO Interrupt Enable for Rising Edge
        }
        else if(condition == kFallingEdge){
           LPC_GPIOINT->IO2IntEnF |= (1<<pin);
        }
        else if(condition == kBothEdges){
           // LPC_GPIOINT->IO2IntEnR |= (1<<pin);
           // LPC_GPIOINT->IO2IntEnF |= (1<<pin);
        }
      }
      return true;
}

/**
 * This function is invoked by the CPU (through Eint3Handler) asynchronously when a Port/Pin
 * interrupt occurs. This function is where you will check the Port status, such as IO0IntStatF,
 * and then invoke the user's registered callback and find the entry in your lookup table.
 *
 * VERY IMPORTANT!
 *  - Be sure to clear the interrupt flag that caused this interrupt, or this function will be called
 *    repetitively and lock your system.
 *  - NOTE that your code needs to be able to handle two GPIO interrupts occurring at the same time.
 */
void LabGPIOInterrupts::HandleInterrupt(){
    //need to execute function associated by the first instance
    //1) check status by & check with pin
    //2) find associated function and execute it
    //3) clear the interrupt flag

    for(int i=0; i<32; i++){
    if( (LPC_GPIOINT->IO0IntStatF & (1<<i)) ||  (LPC_GPIOINT->IO0IntStatR & (1<<i))){ //check status
        pin_isr_map[0][i](); //execute
        LPC_GPIOINT->IO0IntClr = (1<<i); //clr interrupt flag
        break; //exit loop once notice active interrupt
    }

    }

    for(int i=0; i<32; i++){
      if( (LPC_GPIOINT->IO2IntStatF & (1<<i)) || (LPC_GPIOINT->IO2IntStatR & (1<<i))){ //check status
          pin_isr_map[1][i](); //execute
          LPC_GPIOINT->IO2IntClr = (1<<i); //clr interrupt flag
          break; //exit loop once notice active interrupt
      }

    }

}

LabGPIOInterrupts::~LabGPIOInterrupts(){

}
