#include "LabGpioInterrupts.hpp"

IsrPointer LabGpioInterrupts::pin_isr_map[2][32] = 
{
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}
};

LabGpioInterrupts::LabGpioInterrupts()
{
    printf("Default constructor called!\n"); 
}

void LabGpioInterrupts::Initialize()
{
    // Swap EINT3_IRQn with Eint3Handler
    isr_register(EINT3_IRQn, HandleInterrupt);

    // Enable an External Interrupt
    NVIC_EnableIRQ(EINT3_IRQn);
}

bool LabGpioInterrupts::AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition)
{
    if(port == 0)
    {
        // Store ISR address in IVT
        pin_isr_map[0][pin] = pin_isr;
        //printf("Entered port 0\n");
        if((pin == 0) || (pin == 1) || (pin == 26) || (pin == 29) || (pin == 30))
        {
            //printf("Entered pin 0\n");
            switch (condition)
            {
                case kRisingEdge: 
                    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
                    printf("Rising Edge case enabled for Port 0.\n");
                    break; 
                case kFallingEdge: 
                    LPC_GPIOINT->IO0IntEnF |= (1 << pin);
                    printf("Falling Edge case enabled for Port 0.\n");
                    break;
                case kBothEdges: 
                    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
                    LPC_GPIOINT->IO0IntEnF |= (1 << pin);
                    printf("Falling & Rising Edge cases enabled for Port 0.\n");
                    break; 
                default: 
                    break; 
            }            
        }
        else return false;
    }
    else if (port == 2)
    {
        pin_isr_map[1][pin] = pin_isr;
        if((pin >= 0) && (pin <= 9))
        {
            switch (condition)
            {
                case kRisingEdge: 
                    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
                    printf("Rising Edge case enabled for Port 2.\n");
                    break; 
                case kFallingEdge: 
                    LPC_GPIOINT->IO2IntEnF |= (1 << pin);
                    printf("Falling Edge case enabled for Port 2.\n");
                    break; 
                case kBothEdges: 
                    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
                    LPC_GPIOINT->IO2IntEnF |= (1 << pin);
                    printf("Falling & Rising Edge cases enabled for Port 2.\n");
                    break; 
                default: 
                    break; 
            }  
        }
        else return false; 
    }
    else
    {
        printf("Select Port 0 or 2 only!");
        return false;
    }

    printf("ISR address has been stored into the IVT! \n");
    return true;
}


void LabGpioInterrupts::HandleInterrupt(void)
{
    // Look up IVT and Poll the pins to see where the interrupt is occuring

    for(int pin = 0; pin < 32; pin++)
    {
        // Check Interrupt Status for Rising & Falling Edge 
        if( (LPC_GPIOINT->IO0IntStatR & (1<<pin)) || (LPC_GPIOINT->IO0IntStatF & (1<<pin)) )
        {
            printf("Interrupt on P0.%d\n\n", pin); 
            IsrPointer callback = pin_isr_map[0][pin]; // Find Entry in IVT
            callback(); // Invoke Registered Callback Function 
            LPC_GPIOINT->IO0IntClr=(1<<pin);
            break; 
        }
    }

    // Check Port 2
    for(int pin = 0; pin < 32; pin++)
    {
        // Check Interrupt Status for Rising & Falling Edge 
        if( (LPC_GPIOINT->IO2IntStatR & (1<<pin)) || (LPC_GPIOINT->IO2IntStatF & (1<<pin)) )
        {
            printf("Interrupt on P2.%d\n\n", pin); 
            IsrPointer callback2 = pin_isr_map[1][pin]; 
            callback2(); 
            LPC_GPIOINT->IO2IntClr=(1<<pin);
            break; 
        }
    }
}
