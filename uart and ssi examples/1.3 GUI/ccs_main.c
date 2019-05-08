#include<stdint.h>
#include <stdbool.h>
#include"inc/hw_memmap.h"
#include"inc/hw_types.h"
#include"driverlib/sysctl.h"
#include"driverlib/gpio.h"
#include"driverlib/uart.h"
#include"driverlib/pin_map.h"
#include"inc/hw_ints.h"
#include"utils/uartstdio.h"
#include"driverlib/interrupt.h"
#include "driverlib/ssi.h"
#include "utils/softssi.h"
#include "driverlib/fpu.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "ex.h"
void initial_console(void);
void UARTIntReceive(void);

#define NUM_UART_DATA  16
#define NUM_SSI_DATA  16

int flag;
uint8_t ui8DataRX[NUM_UART_DATA];
uint32_t pui32DataRx[NUM_SSI_DATA];
int main(void) {
	//flag =0时，还没录入
	initial_console();
	uint32_t index;
	flag=0;
	int i;
	UARTIntRegister(UART0_BASE,UARTIntReceive);
	while(1){
		if(flag==2){
			while(MAP_UARTBusy(UART0_BASE)){}
			for(index = 0 ; index < NUM_UART_DATA ; index++){
				i=1000;
				SSIDataPut(SSI0_BASE, (uint8_t)ui8DataRX[index]);
				SSIDataGet(SSI0_BASE, &pui32DataRx[index]);
				pui32DataRx[index] &= 0x00FF;
				while(i--);
			}
			/*while(SSIBusy(SSI0_BASE)){}
		for(index = 0; index < 16; index++)
		{

		}*/
			while(SSIBusy(SSI0_BASE))
			{
			}
			for(index = 8; index < 16; index++)
			{
				UARTCharPut(UART0_BASE, (uint8_t)pui32DataRx[index]);
			}
			flag=0;
		}
	}
	return 0;
}


void initial_console(void){
	//设置时钟频率为16MHZ，时钟源外部晶振
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	//使能gpio和uart,ssi
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);//使能SSI外设
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	//引脚配置gpio引脚
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);
	GPIOPinConfigure(GPIO_PA4_SSI0RX);// 每个接口设置对应功能
	//gpio 的uart ，ssi功能开启
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0| GPIO_PIN_1);
	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
			GPIO_PIN_2);
	//uart,ssi配置
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 |  UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER,1000000,8);

	SSIEnable(SSI0_BASE);
	//使能处理器中断
	IntMasterEnable();
	//使能中断 UART,ssi连接串口
	IntEnable(INT_UART0);
	// 使能uart中断中的接收和发送中断
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	//Note：使能过程要硬件软件都进行使能并进行联合配置
}


//中断接收函数
void UARTIntReceive(void){
	uint32_t ulStatus;
	//获取中断状态
	ulStatus = UARTIntStatus(UART0_BASE, true);
	uint32_t index;
	while(UARTCharsAvail(UART0_BASE)){
		/*if(flag==1){//flag=1表示止录入了前8位
			for(index = 8 ; index <NUM_UART_DATA  ; index++){
				ui8DataRX[index] = UARTCharGet(UART0_BASE);
			}
			flag=2;
		}*/
		if(flag==0){
			for(index = 0 ; index <NUM_UART_DATA; index++){
				ui8DataRX[index] = UARTCharGet(UART0_BASE);
				if(index==0&&ui8DataRX[index]=='2'){
					ui8DataRX[index]='0';//当首位为0 时标成2，发送SSI前改为0，表示读入
				}
			}
			flag=2;

		}
	}
	//清除中断标志,这步适当提前，因为会要几个时钟周期才能真正清楚。
	UARTIntClear(UART0_BASE, ulStatus);
}
