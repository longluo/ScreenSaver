/************************************************************************************
** File: - E:\ARM\lm3s8962projects\ScreenSaver\Source\App\Src\main.c
**  
** Copyright (C), Long.Luo, All Rights Reserved!
** 
** Description: 
**      main.c
** 
** Version: 1.0
** Date created: 23:57:25,22/04/2013
** Author: Long.Luo
** 
** --------------------------- Revision History: --------------------------------
** 	<author>	<data>			<desc>
** 
************************************************************************************/

#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_sysctl.h"
#include "hw_types.h"
#include "debug.h"
#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "sysctl.h"
#include "systick.h"
#include "timer.h"
#include "uart.h"
#include "rit128x96x4.h"
#include "globals.h"
//#include "images.h"
#include "random.h"
#include "screen_saver.h"
//#include "sounds.h"


//*****************************************************************************
//
// A set of flags used to track the state of the application.
//
//*****************************************************************************
unsigned long g_ulFlags;

//*****************************************************************************
//
// The speed of the processor clock, which is therefore the speed of the clock
// that is fed to the peripherals.
//
//*****************************************************************************
unsigned long g_ulSystemClock;

//*****************************************************************************
//
// Storage for a local frame buffer.
//
//*****************************************************************************
unsigned char g_pucFrame[6144];

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
// The number of clock ticks that have occurred.  This is used as an entropy
// source for the random number generator; the number of clock ticks at the
// time of a button press or release is an entropic event.
//
//*****************************************************************************
static unsigned long g_ulTickCount = 0;

//*****************************************************************************
//
// The number of clock ticks that have occurred since the last screen update
// was requested.  This is used to divide down the system clock tick to the
// desired screen update rate.
//
//*****************************************************************************
static unsigned char g_ucScreenUpdateCount = 0;

//*****************************************************************************
//
// The number of clock ticks that have occurred since the last application
// update was performed.  This is used to divide down the system clock tick to
// the desired application update rate.
//
//*****************************************************************************
static unsigned char g_ucAppUpdateCount = 0;

//*****************************************************************************
//
// The debounced state of the five push buttons.  The bit positions correspond
// to:
//
//     0 - Up
//     1 - Down
//     2 - Left
//     3 - Right
//     4 - Select
//
//*****************************************************************************
unsigned char g_ucSwitches = 0x1f;

//*****************************************************************************
//
// The vertical counter used to debounce the push buttons.  The bit positions
// are the same as g_ucSwitches.
//
//*****************************************************************************
static unsigned char g_ucSwitchClockA = 0;
static unsigned char g_ucSwitchClockB = 0;

//*****************************************************************************
//
// Handles the SysTick timeout interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    unsigned long ulData, ulDelta;

    //
    // Increment the tick count.
    //
    g_ulTickCount++;

    //
    // Indicate that a timer interrupt has occurred.
    //
    HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK) = 1;

    //
    // Increment the screen update count.
    //
    g_ucScreenUpdateCount++;

    //
    // See if 1/30th of a second has passed since the last screen update.
    //
    if(g_ucScreenUpdateCount == (CLOCK_RATE / 30))
    {
        //
        // Restart the screen update count.
        //
        g_ucScreenUpdateCount = 0;

        //
        // Request a screen update.
        //
        HWREGBITW(&g_ulFlags, FLAG_UPDATE) = 1;
    }

    //
    // Update the music/sound effects.
    //
    //AudioHandler();

    //
    // Increment the application update count.
    //
    g_ucAppUpdateCount++;

    //
    // See if 1/100th of a second has passed since the last application update.
    //
    if(g_ucAppUpdateCount != (CLOCK_RATE / 100))
    {
        //
        // Return without doing any further processing.
        //
        return;
    }

    //
    // Restart the application update count.
    //
    g_ucAppUpdateCount = 0;

    //
    // Run the Ethernet handler.
    //
    //EnetTick(10);

    //
    // Read the state of the push buttons.
    //
    ulData = (GPIOPinRead(GPIO_PORTE_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
                                            GPIO_PIN_2 | GPIO_PIN_3)) |
              (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) << 3));

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ulDelta = ulData ^ g_ucSwitches;

    //
    // Increment the clocks by one.
    //
    g_ucSwitchClockA ^= g_ucSwitchClockB;
    g_ucSwitchClockB = ~g_ucSwitchClockB;

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    g_ucSwitchClockA &= ulDelta;
    g_ucSwitchClockB &= ulDelta;

    //
    // Get the new debounced switch state.
    //
    g_ucSwitches &= g_ucSwitchClockA | g_ucSwitchClockB;
    g_ucSwitches |= (~(g_ucSwitchClockA | g_ucSwitchClockB)) & ulData;

    //
    // Determine the switches that just changed debounced state.
    //
    ulDelta ^= (g_ucSwitchClockA | g_ucSwitchClockB);

    //
    // See if any switches just changed debounced state.
    //
    if(ulDelta)
    {
        //
        // Add the current tick count to the entropy pool.
        //
        RandomAddEntropy(g_ulTickCount);
    }

    //
    // See if the select button was just pressed.
    //
    if ((ulDelta & 0x10) && !(g_ucSwitches & 0x10))
    {
        //
        // Set a flag to indicate that the select button was just pressed.
        //
        HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 1;
    }
}


//*****************************************************************************
//
// Delay for a multiple of the system tick clock rate.
//
//*****************************************************************************
static void
Delay(unsigned long ulCount)
{
    //
    // Loop while there are more clock ticks to wait for.
    //
    while(ulCount--)
    {
        //
        // Wait until a SysTick interrupt has occurred.
        //
        while(!HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK))
        {
        }

        //
        // Clear the SysTick interrupt flag.
        //
        HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK) = 0;
    }
}




//*****************************************************************************
//
// The main code for the application.  It sets up the peripherals, displays the
// splash screens, and then manages the interaction between the game and the
// screen saver.
//
//*****************************************************************************
int
main(void)
{
    //
    // If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    // a workaround to allow the PLL to operate reliably.
    //
    if(REVISION_IS_A2)
    {
        SysCtlLDOSet(SYSCTL_LDO_2_75V);
    }

    //
    // Set the clocking to run at 50MHz from the PLL.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

    //
    // Get the system clock speed.
    //
    g_ulSystemClock = SysCtlClockGet();

    //
    // Enable the peripherals used by the application.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure the GPIOs used to read the state of the on-board push buttons.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    //
    // Configure the LED, speaker, and UART GPIOs as required.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

    //
    // Initialize the CAN controller.
    //
    //CANConfigure();

    //
    // Intialize the Ethernet Controller and TCP/IP Stack.
    //
    //EnetInit();

    //
    // Configure the first UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    UARTEnable(UART0_BASE);

    //
    // Send a welcome message to the UART.
    //
    UARTCharPut(UART0_BASE, 'W');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'l');
    UARTCharPut(UART0_BASE, 'c');
    UARTCharPut(UART0_BASE, 'o');
    UARTCharPut(UART0_BASE, 'm');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, '\r');
    UARTCharPut(UART0_BASE, '\n');

    //
    // Initialize the OSRAM OLED display.
    //
    RIT128x96x4Init(3500000);

    //
    // Initialize the PWM for generating music and sound effects.
    //
    //AudioOn();

    //
    // Configure SysTick to periodically interrupt.
    //
    SysTickPeriodSet(g_ulSystemClock / CLOCK_RATE);
    SysTickIntEnable();
    SysTickEnable();

    //
    // Delay for a bit to allow the initial display flash to subside.
    //
    Delay(CLOCK_RATE / 4);

    //
    // Play the intro music.
    //
    //AudioPlaySong(g_pusIntro, sizeof(g_pusIntro) / 2);


    //
    // Throw away any button presses that may have occurred while the splash
    // screens were being displayed.
    //
    HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;

    //
    // Loop forever.
    //
    while (1)
    {
        //
        // The button was not pressed during the timeout period, so start
        // the screen saver.
        //
        ScreenSaver();
    }
}


