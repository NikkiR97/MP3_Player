#include <stdio.h>
#include "LPC17xx.h"
#include "tasks.hpp"
#include "uart0_min.h"
#include "semphr.h"
#include "lpc_isr.h"
#include "LABGPIO.hpp"
#include "LabGpioInterrupts.hpp"


// Declare instance of semaphore 
SemaphoreHandle_t playSemaphore;
SemaphoreHandle_t pauseSemaphore;
SemaphoreHandle_t reenableSemaphore; 


long yield = 0; 

void port0_isr()
{
    xSemaphoreGiveFromISR(playSemaphore, &yield); 
    portYIELD_FROM_ISR(yield); 

    uart0_puts("playSemaphore given for Port 0 interrupt!\n"); 
}

void port2_isr()
{
    xSemaphoreGiveFromISR(pauseSemaphore, &yield); 
    portYIELD_FROM_ISR(yield);

    uart0_puts("pauseSemaphore given for Port 2 interrupt!\n"); 
}


// vControlLED0
void play(void *pvParameters)
{
    LabGPIO LED0(1, 0); // P1.0
    LED0.setAsOutput(); 
    if(xSemaphoreTake(playSemaphore, portMAX_DELAY))
    {
        printf("xSemaphore taken- LED0 should be blinking!\n");
        for(int i = 0; i < 3; i++)
        {
            LED0.setLow(); vTaskDelay(200); LED0.setHigh(); vTaskDelay(200);
        }
    }
    //xSemaphoreGive(reenableSemaphore);
}

// vControlLED2 
void pause(void *pvParameters)
{
    LabGPIO LED2(1, 4); // P1.4
    LED2.setAsOutput(); 
    if(xSemaphoreTake(pauseSemaphore, portMAX_DELAY))
    {
        printf("pauseSemaphore taken- LED2 should be blinking!\n");
        for(int i = 0; i < 3; i++)
        {
            LED2.setLow(); vTaskDelay(200); LED2.setHigh(); vTaskDelay(200);
        }
    }
    //xSemaphoreGive(reenableSemaphore); 
}

// void reenableInterrupt(void *pvParameters)
// {
//     LabGpioInterrupts *z = new LabGpioInterrupts;
//     bool y = false; 
//     if(xSemaphoreTake(reenableSemaphore, portMAX_DELAY))
//     {
//        y = z->AttachInterruptHandler(2, 0, &port2_isr, kBothEdges);
//        if (y)
//        {
//             uart0_puts("Interrupt has been reenabled!\n");
//        }
        
//     }
// }
 

int main(int argc, char const *argv[])
{
    // ----------- INTERRUPT HANDLING  -----------
    // Declare & Create object to point to LabGpioInterrupts class 
    LabGpioInterrupts *x = new LabGpioInterrupts; 
    bool attach = false;

    // Configure NVIC to notice EINT3 IRQs
    x->Initialize(); 

    // Attach P0.0
    attach = x->AttachInterruptHandler(0, 0, &port0_isr, kRisingEdge);

    // Attach P2.0
    attach = x->AttachInterruptHandler(2, 0, &port2_isr, kBothEdges);

    if (attach)
    {
        printf ("Handler attached successfully!\n\n"); 
    }
    else
    {
        printf("Handler did not attach...\n\n"); 
    } 

    // ----------- LED & SEMAPHORES  -----------

    // Setting stack width
    const uint32_t STACK_SIZE = 512; 

    // Semaphore starts 'empty' when you create it 
    playSemaphore = xSemaphoreCreateBinary();
    pauseSemaphore = xSemaphoreCreateBinary(); 
    reenableSemaphore = xSemaphoreCreateBinary(); 


    xTaskCreate (
        play, 
        "on board LED 0 Enabled",
        STACK_SIZE, 
        NULL, 
        PRIORITY_MEDIUM, 
        NULL 
        ); 

    xTaskCreate (
        pause, 
        "on board LED 2 Enabled",
        STACK_SIZE, 
        NULL, 
        PRIORITY_MEDIUM, 
        NULL 
        );

    // xTaskCreate (
    //     reenableInterrupt, 
    //     "Reenable Interrupts",
    //     STACK_SIZE, 
    //     NULL, 
    //     PRIORITY_LOW, 
    //     NULL 
    //     );

    vTaskStartScheduler();
    while(1);
}
