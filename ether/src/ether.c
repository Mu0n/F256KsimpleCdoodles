#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "stdio.h"
#include "..\src\muUtils.h"

#define WIFI_CTRL      0xDD80 // bit0 0=speed 115k 1=912k, bit3 reset FIFOs Tx and Rx
#define WIFI_DATA      0xDD81 //write to send, read to receive
#define WIFI_RX_F      0xDD82 //RX FIFO
#define WIFI_TX_F      0xDD84 //TX FIFO

#define STDPAUSE       5 //frames



void uart_putc(char c) {
    // Wait until TX is ready
    POKE(WIFI_DATA,c);
}


char uart_getc(void) {
    if(PEEKW(WIFI_RX_F)) return PEEK(WIFI_DATA);  // RX ready
	return 0;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

static bool wiz_wait_ok(void) {
    char b0=0, b1=0, b2=0;
/*
    for (;;) {
        char c = uart_getc();
        b0 = b1;
        b1 = b2;
        b2 = c;

        if (b1=='O' && b2=='K')
            return true;

        if (b0=='E' && b1=='R')
            return false;
    }
	*/
	lilpause(30);
	return true;
}

static bool wiz_cmd(const char *cmd) {
    uart_puts(cmd);
    uart_puts("\r\n");
    return wiz_wait_ok();
}

bool wiz_init(void) {
    if (!wiz_cmd("AT")) return false;
	lilpause(STDPAUSE);
    if (!wiz_cmd("AT+RST")) return false;
	lilpause(STDPAUSE);

    // Station mode
    if (!wiz_cmd("AT+CWMODE=1")) return false;
	lilpause(STDPAUSE);

    // Enable DHCP
    if (!wiz_cmd("AT+CWDHCP=1,1")) return false;
	lilpause(STDPAUSE);

    return true;
}

bool wiz_join(const char *ssid, const char *pass) {
    uart_puts("AT+CWJAP=\"");
    uart_puts(ssid);
    uart_puts("\",\"");
    uart_puts(pass);
    uart_puts("\"\r\n");

    return wiz_wait_ok();
}
bool wiz_tcp_open(void) {
    uart_puts("AT+CIPMUX=1\r\n");   // enable multi-connection mode
    wiz_wait_ok();

    uart_puts("AT+CIPSTART=0,\"TCP\",\"192.168.1.177\",80\r\n");
    return wiz_wait_ok();
}
bool wiz_tcp_send(const char *msg) {
    char lenbuf[8];
    int len = strlen(msg);

    uart_puts("AT+CIPSEND=0,");
    sprintf(lenbuf, "%d", len);
    uart_puts(lenbuf);
    uart_puts("\r\n");

    // Wait for '>' prompt
    while (uart_getc() != '>');

    uart_puts(msg);

    return wiz_wait_ok();
}
/* when reading
	3'b000: CPU_D_o = {4'b0000, WIFI_Control_Register[3] , Tx_empty, Rx_empty, WIFI_Control_Register[0]};
	3'b001: CPU_D_o = FIFO_CPU_D_o;
	3'b010: CPU_D_o = WIFI_UART_RxD_FIFO_RD_Count[7:0];
	3'b011: CPU_D_o = {5'b0_0000, WIFI_UART_RxD_FIFO_RD_Count[10:8]};
	3'b100: CPU_D_o = WIFI_UART_TxD_FIFO_WR_Count[7:0];
	3'b101: CPU_D_o = {5'b0_0000, WIFI_UART_TxD_FIFO_WR_Count[10:8]};
*/

uint8_t wifiTest()
{
	
    printf("Starting WizFi360...\r\n");

    if (!wiz_init()) {
        printf("Init failed\r\n");
        return 1;
    }

    if (!wiz_join("BatCave", "leocestleplusbeau")) {
        printf("WiFi join failed\r\n");
        return 1;
    }

    if (!wiz_tcp_open()) {
        printf("TCP open failed\r\n");
        return 1;
    }

    wiz_tcp_send("hello");
	return 0;
}

int main(int argc, char *argv[]) {
	
	POKE(MMU_IO_CTRL, 0x00);
	// XXX GAMMA  SPRITE   TILE  | BITMAP  GRAPH  OVRLY  TEXT
	POKE(VKY_MSTR_CTRL_0, 0b00000111); //sprite,graph,overlay,text
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00010000); //font overlay, double height text, 320x240 at 60 Hz;
POKE(WIFI_CTRL,0x00);
lilpause(10);
	
uint16_t a=1;
uint16_t b=188;
uint16_t c=0.0f;

c = mathUnsignedMultiply(a,b);
uint16_t d = 0;
mathUnsignedDivisionRemainder(c,100,&d);

printf("float c is %d remainder %d",mathUnsignedDivision(c,100),d);
getchar();
while(true)
{
printf("\n-----START TEST\n");
wifiTest();
printf("\n-------END TEST\n");
getchar();
}
return 0;
}
}