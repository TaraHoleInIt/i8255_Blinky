/**
 * Copyright (c) 2025 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>

#include "ch32fun.h"
#include "ch32v003_GPIO_branchless.h"

#define Config_Pin_RD GPIOv_from_PORT_PIN( GPIO_port_D, 0 )
#define Config_Pin_WR GPIOv_from_PORT_PIN( GPIO_port_D, 2 )

#define Config_Pin_RST GPIOv_from_PORT_PIN( GPIO_port_D, 3 )
#define Config_Pin_CS  GPIOv_from_PORT_PIN( GPIO_port_D, 4 )

#define Config_Pin_A0 GPIOv_from_PORT_PIN( GPIO_port_A, 1 )
#define Config_Pin_A1 GPIOv_from_PORT_PIN( GPIO_port_A, 2 )

#define Config_Delay_i8255_reset_us  50
#define Config_Delay_i8255_wr_us     1
#define Config_Delay_i8255_rd_us     1

enum {
    I8255_Reg_PortA = 0,
    I8255_Reg_PortB = 1,
    I8255_Reg_PortC = 2,
    I8255_Reg_Control = 3
};

void dataBusSetOutput( void );
void dataBusSetInput( void );
uint8_t dataBusRead( void );
void dataBusWrite( uint8_t data );

void i8255Reset( void );
void i8255SetAddress( uint8_t address );
void i8255Select( void );
void i8255Deselect( void );
void i8255BeginRead( void );
void i8255EndRead( void );
void i8255BeginWrite( void );
void i8255EndWrite( void );
void i8255WriteRegister( uint8_t reg, uint8_t data );
uint8_t i8255ReadRegister( uint8_t reg );

void setupGPIO( void );

void dataBusSetOutput( void ) {
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 0 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 1 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 2 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 3 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 4 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 5 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 6 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 7 ), GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
}

void dataBusSetInput( void ) {
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 0 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 1 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 2 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 3 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 4 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 5 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 6 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
    GPIO_pinMode( GPIOv_from_PORT_PIN( GPIO_port_C, 7 ), GPIO_pinMode_I_floating, GPIO_Speed_50MHz );
}

uint8_t dataBusRead( void ) {
    return GPIO_port_digitalRead( GPIO_port_C ) & 0xFF;
}

void dataBusWrite( uint8_t data ) {
    GPIO_port_digitalWrite( GPIO_port_C, data );
}

void setupGPIO( void ) {
    // We use all ports
    GPIO_port_enable( GPIO_port_A );
    GPIO_port_enable( GPIO_port_C );
    GPIO_port_enable( GPIO_port_D );

    // Set data bus to input initially
    dataBusSetInput( );

    // Setup RD and WR pins (active low)
    GPIO_pinMode( Config_Pin_RD, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_digitalWrite_hi( Config_Pin_RD );

    GPIO_pinMode( Config_Pin_WR, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_digitalWrite_hi( Config_Pin_WR );

    // Setup chip select pin (active low)
    GPIO_pinMode( Config_Pin_CS, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_digitalWrite_hi( Config_Pin_CS );

    // Setup reset pin (active high)
    GPIO_pinMode( Config_Pin_RST, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_digitalWrite_lo( Config_Pin_RST );

    // Setup address pins
    GPIO_pinMode( Config_Pin_A0, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_digitalWrite_lo( Config_Pin_A0 );

    GPIO_pinMode( Config_Pin_A1, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz );
    GPIO_digitalWrite_lo( Config_Pin_A1 );
}

void i8255Reset( void ) {
    GPIO_digitalWrite_hi( Config_Pin_RST );
        Delay_Us( Config_Delay_i8255_reset_us );
    GPIO_digitalWrite_lo( Config_Pin_RST );
}

void i8255SetAddress( uint8_t address ) {
    // Only 2 bits for address
    address &= 0x03;

    if ( address & 0x01 ) GPIO_digitalWrite_hi( Config_Pin_A0 );
    else                  GPIO_digitalWrite_lo( Config_Pin_A0 );

    if ( address & 0x02 ) GPIO_digitalWrite_hi( Config_Pin_A1 );
    else                  GPIO_digitalWrite_lo( Config_Pin_A1 );
}

void i8255Select( void ) {
    GPIO_digitalWrite_lo( Config_Pin_CS );
}

void i8255Deselect( void ) {
    GPIO_digitalWrite_hi( Config_Pin_CS );
}

void i8255BeginRead( void ) {
    dataBusSetInput( );

    GPIO_digitalWrite_hi( Config_Pin_WR );
    GPIO_digitalWrite_lo( Config_Pin_RD );
}

void i8255EndRead( void ) {
    GPIO_digitalWrite_hi( Config_Pin_RD );
}

void i8255BeginWrite( void ) {
    GPIO_digitalWrite_hi( Config_Pin_RD );
    GPIO_digitalWrite_lo( Config_Pin_WR );

    dataBusSetOutput( );
}

void i8255EndWrite( void ) {
    GPIO_digitalWrite_hi( Config_Pin_WR );
}

void i8255WriteRegister( uint8_t reg, uint8_t data ) {
    i8255SetAddress( reg );

    i8255Select( );
        dataBusWrite( data );

        i8255BeginWrite( );
            Delay_Us( Config_Delay_i8255_wr_us );
        i8255EndWrite( );
    i8255Deselect( );
}

uint8_t i8255ReadRegister( uint8_t reg ) {
    uint8_t data = 0;

    i8255SetAddress( reg );

    i8255Select( );
        i8255BeginRead( );
            Delay_Us( Config_Delay_i8255_rd_us );
            data = dataBusRead( );
        i8255EndRead( );
    i8255Deselect( );

    return data;
}

int main( void ) {
    SystemInit( );

    while ( ! DebugPrintfBufferFree( ) )
    ;

    setupGPIO( );

    printf( "Resetting i8255... " );
        i8255Reset( );
    printf( "Done.\n" );

    printf( "Configuring i8255... " );
        // Todo: Configure 8255
        i8255WriteRegister( I8255_Reg_Control, 0x80 ); // Mode 0, Ports A,B,C output
        i8255WriteRegister( I8255_Reg_PortA, 0x00 );
    printf( "Done.\n" );

    while ( 1 ) {
        Delay_Ms( 500 );
        i8255WriteRegister( I8255_Reg_PortA, 0x55 );

        printf( "Port A: 0x%02X\n", i8255ReadRegister( I8255_Reg_PortA ) );

        Delay_Ms( 500 );
        i8255WriteRegister( I8255_Reg_PortA, 0xAA );

        printf( "Port A: 0x%02X\n", i8255ReadRegister( I8255_Reg_PortA ) );
    }
}
