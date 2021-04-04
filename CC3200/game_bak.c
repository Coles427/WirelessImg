//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - SPI Demo
// Application Overview - The demo application focuses on showing the required 
//                        initialization sequence to enable the CC3200 SPI 
//                        module in full duplex 4-wire master and slave mode(s).
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup SPI_Demo
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "Adafruit_OLED.h"
#include "Adafruit_GFX.h"
#include "test.h"

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"

// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"
#include "pinmux.h"
#include "adafruit_SSD1351.h"


#define APPLICATION_VERSION     "1.4.0"
#define UART_PRINT              Report
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
#define FFDELAY         120000
#define FREEFALL        0x41
//*****************************************************************************
//
// Application Master/Slave mode selector macro
//
// MASTER_MODE = 1 : Application in master mode
// MASTER_MODE = 0 : Application in slave mode
//
//*****************************************************************************
#define MASTER_MODE      1

#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100

#define MASTER_MSG       "This is CC3200 SPI Master Application\n\r"
#define SLAVE_MSG        "This is CC3200 SPI Slave Application\n\r"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned char g_ucTxBuff[TR_BUFF_SIZE];
static unsigned char g_ucRxBuff[TR_BUFF_SIZE];
static unsigned char ucTxBuffNdx;
static unsigned char ucRxBuffNdx;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//*****************************************************************************
//
//! SPI Slave Interrupt handler
//!
//! This function is invoked when SPI slave has its receive register full or
//! transmit register empty.
//!
//! \return None.
//
//*****************************************************************************
static void SlaveIntHandler()
{
    unsigned long ulRecvData;
    unsigned long ulStatus;

    ulStatus = MAP_SPIIntStatus(GSPI_BASE,true);

    MAP_SPIIntClear(GSPI_BASE,SPI_INT_RX_FULL|SPI_INT_TX_EMPTY);

    if(ulStatus & SPI_INT_TX_EMPTY)
    {
        MAP_SPIDataPut(GSPI_BASE,g_ucTxBuff[ucTxBuffNdx%TR_BUFF_SIZE]);
        ucTxBuffNdx++;
    }

    if(ulStatus & SPI_INT_RX_FULL)
    {
        MAP_SPIDataGetNonBlocking(GSPI_BASE,&ulRecvData);
        g_ucTxBuff[ucRxBuffNdx%TR_BUFF_SIZE] = ulRecvData;
        Report("%c",ulRecvData);
        ucRxBuffNdx++;
    }
}

//*****************************************************************************
//
//! SPI Master mode main loop
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
void MasterMain()
{

    unsigned long ulUserData;
    unsigned long ulDummy;

    //
    // Initialize the message
    //
    memcpy(g_ucTxBuff,MASTER_MSG,sizeof(MASTER_MSG));

    //
    // Set Tx buffer index
    //
    ucTxBuffNdx = 0;
    ucRxBuffNdx = 0;

    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    //
    // Print mode on uart
    //
    Message("Enabled SPI Interface in Master Mode\n\r");

    //
    // User input
    //
    Report("Press any key to transmit data....");

    //
    // Read a character from UART terminal
    //
    ulUserData = MAP_UARTCharGet(UARTA0_BASE);


    //
    // Send the string to slave. Chip Select(CS) needs to be
    // asserted at start of transfer and deasserted at the end.
    //
    MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,50,
            SPI_CS_ENABLE|SPI_CS_DISABLE);

    //
    // Report to the user
    //
    Report("\n\rSend      %s",g_ucTxBuff);
    //Report("Received  %s",g_ucRxBuff);

    //
    // Print a message
    //
    Report("\n\rType here (Press enter to exit) :");

    //
    // Initialize variable
    //
    ulUserData = 0;

    //
    // Enable Chip select
    //
    MAP_SPICSEnable(GSPI_BASE);

    //
    // Loop until user "Enter Key" is
    // pressed
    //
    while(ulUserData != '\r')
    {
        //
        // Read a character from UART terminal
        //
        ulUserData = MAP_UARTCharGet(UARTA0_BASE);

        //
        // Echo it back
        //
        MAP_UARTCharPut(UARTA0_BASE,ulUserData);

        //
        // Push the character over SPI
        //
        MAP_SPIDataPut(GSPI_BASE,ulUserData);
        printf("Dat has been sent to the Slave!\n");

        //
        // Clean up the receive register into a dummy
        // variable
        //

        MAP_SPIDataGet(GSPI_BASE,&ulDummy);

    }

    //
    // Disable chip select
    //
    MAP_SPICSDisable(GSPI_BASE);
}

//*****************************************************************************
//
//! SPI Slave mode main loop
//!
//! This function configures SPI modelue as slave and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
void SlaveMain()
{
    //
    // Initialize the message
    //
    memcpy(g_ucTxBuff,SLAVE_MSG,sizeof(SLAVE_MSG));

    //
    // Set Tx buffer index
    //
    ucTxBuffNdx = 0;
    ucRxBuffNdx = 0;

    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_SLAVE,SPI_SUB_MODE_0,
                     (SPI_HW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Register Interrupt Handler
    //
    MAP_SPIIntRegister(GSPI_BASE,SlaveIntHandler);

    //
    // Enable Interrupts
    //
    MAP_SPIIntEnable(GSPI_BASE,SPI_INT_RX_FULL|SPI_INT_TX_EMPTY);

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    //
    // Print mode on uart
    //
    Message("Enabled SPI Interface in Slave Mode\n\rReceived : ");
}

void
DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen)
{
    unsigned char ucBufIndx = 0;
    UART_PRINT("Read contents");
    UART_PRINT("\n\r");
    while(ucBufIndx < ucLen)
    {
        UART_PRINT(" 0x%x, ", pucDataBuf[ucBufIndx]);
        ucBufIndx++;
        if((ucBufIndx % 8) == 0)
        {
            UART_PRINT("\n\r");
        }
    }
    UART_PRINT("\n\r");
}

void draw0(char **decoded){
    int x;
    int dindex = 0;
    int y;


    for(y = 0; y < 32; y++){
        for(x = 0; x <32; x++){
            int k;
            int l;
            for(k = 0; k < 4; k++){
                for(l = 0; l < 4; l++){
                    drawPixel(x*4 + k, y*4 + l, decoded[x][y]);
                }
            }
        }
    }
}

void draw90(char **decoded){
    int x;
    int dindex = 0;
    int y;


    for(y = 0; y < 32; y++){
        for(x = 0; x <32; x++){
            int k;
            int l;
            for(k = 0; k < 4; k++){
                for(l = 0; l < 4; l++){
                    drawPixel(x*4 + k, y*4 + l, decoded[y][x]);
                }
            }
        }
    }
}

void draw180(char **decoded){
    int x;
    int dindex = 0;
    int y;


    for(y = 0; y < 32; y++){
        for(x = 0; x <32; x++){
            int k;
            int l;
            for(k = 0; k < 4; k++){
                for(l = 0; l < 4; l++){
                    drawPixel(x*4 + k, y*4 + l, decoded[31 - x][31 - y]);
                }
            }
        }
    }
}

void draw270(char **decoded){
    int x;
    int dindex = 0;
    int y;


    for(y = 0; y < 32; y++){
        for(x = 0; x <32; x++){
            int k;
            int l;
            for(k = 0; k < 4; k++){
                for(l = 0; l < 4; l++){
                    drawPixel(x*4 + k, y*4 + l, decoded[31 - y][31 - x]);
                }
            }
        }
    }
}



//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************



void start_game(char **decoded, int rotate)
{
    unsigned char ucDevAddr = 0x18;
    unsigned char ucRegOffset = 0x05;
    unsigned char setGRange[256];
    setGRange[0] = ucRegOffset;
    setGRange[1] = 0x08;



    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK); //Enables SPI clock
    MAP_SPIReset(GSPI_BASE);// Resets SPI module
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_8)); //Configuration for SPI module
    MAP_SPIEnable(GSPI_BASE); // Enables SPI module
    UART_PRINT("Enabled I2C to OLED\n\r");
    Adafruit_Init(); // Initilizes OLED
    UART_PRINT("Initilized OLDED\n\r");
    I2C_IF_Write(ucDevAddr,&setGRange[0],2,1);

    switch (rotate){
        case 0:
           draw0(decoded);
        case 1:
           draw90(decoded);
        case 2:
           draw180(decoded);
        case 3:
           draw270(decoded);
    }
}

