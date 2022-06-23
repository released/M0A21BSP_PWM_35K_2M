/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "project_config.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;


#define SYS_CLK 								(48000000ul)

#define PWM_ch1_FREQ 						    (31250)	
#define PWM_ch1_no                              (1)
#define PWM_ch1_MASK                            (PWM_CH_1_MASK)

#define PWM_ch1_PSC 					        (5)	
#define PWM_ch1_RESOLUTION                      (100)
#define PWM_ch1_TARGET_DUTY(d)				    ((PWM_ch1_RESOLUTION*(100-d))/100)	//((PWM_RESOLUTION*d)/100)
#define PWM_ch1_DUTY                            (PWM_ch1_TARGET_DUTY(50))		//percent


//#define PWM_ch1_CNR 							((SYS_CLK/PWM_ch1_FREQ)/PWM_PSC - 1)			//Up Counter Type  or Down Counter Type
#define PWM_ch1_CNR 						    (((SYS_CLK/PWM_ch1_FREQ)/PWM_ch1_PSC - 1)>>1 )	//Up-Down Counter Type
#define PWM_ch1_CMR 						    (PWM_ch1_DUTY * (PWM_ch1_CNR + 1)/PWM_ch1_RESOLUTION)

#define CalNewDutyCMR(pwm, u32ChannelNum, u32DutyCycle, u32CycleResolution)	    (u32DutyCycle * (PWM_GET_CNR(pwm, u32ChannelNum) + 1) / u32CycleResolution)
#define CalNewDuty(pwm, u32ChannelNum, u32DutyCycle, u32CycleResolution)		(PWM_SET_CMR(pwm, u32ChannelNum, CalNewDutyCMR(pwm, u32ChannelNum, u32DutyCycle, u32CycleResolution)))


#define PWM_ch2_FREQ                            (2000000ul)
#define PWM_ch2_no                              (4)
#define PWM_ch2_MASK                            (PWM_CH_4_MASK)

uint32_t freq_ch1 = PWM_ch1_FREQ;
uint32_t freq_ch2 = PWM_ch2_FREQ;
uint32_t duty_ch1 = 50;
uint32_t duty_ch2 = 50;

/*_____ M A C R O S ________________________________________________________*/
#define PWM_SET_ALIGNED_TYPE(bpwm, u32ChannelMask, u32AlignedType) ((bpwm)->CTL1 = (u32AlignedType))

/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}


void delay_ms(uint16_t ms)
{
	TIMER_Delay(TIMER0, 1000*ms);
}

void loop(void)
{
    if (is_flag_set(flag_pwm_adjust))
    {
        set_flag(flag_pwm_adjust , DISABLE);
        // PWM_ConfigOutputChannel(PWM0, PWM_ch1_no, freq_ch1 , duty_ch1); 
		CalNewDuty(PWM0, PWM_ch1_no, PWM_ch1_TARGET_DUTY(duty_ch1), PWM_ch1_RESOLUTION);

        PWM_ConfigOutputChannel(PWM0, PWM_ch2_no, freq_ch2 , duty_ch2);

        printf("ch1 freq:%8d,duty:%2d,ch2 freq:%8d,duty:%2d\r\n",freq_ch1,duty_ch1,freq_ch2,duty_ch2);
    }
}

/*
    PA2 , PWM0_CH1 , 
    PC0 , PWM0_CH4 ,         
*/

void PWM_Init(void)
{
    #if 1
    /* Set PWM0 timer clock prescaler */
    PWM_SET_PRESCALER(PWM0, PWM_ch1_no, PWM_ch1_PSC - 1);
	

    /* Set PWM0 timer period */
    PWM_SET_CNR(PWM0, PWM_ch1_no, PWM_ch1_CNR);
	
    /* Set PWM0 timer duty */
    PWM_SET_CMR(PWM0, PWM_ch1_no, PWM_ch1_CMR);	

    PWM_SET_ALIGNED_TYPE(PWM0, PWM_ch1_MASK, PWM_CENTER_ALIGNED);    
	
    /* Set output level at zero, compare up, period(center) and compare down of specified channel */
//    PWM_SET_OUTPUT_LEVEL(PWM0, PWM_ch1_MASK, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);
    PWM_SET_OUTPUT_LEVEL(PWM0, PWM_ch1_MASK, PWM_OUTPUT_LOW, PWM_OUTPUT_HIGH, PWM_OUTPUT_NOTHING, PWM_OUTPUT_LOW);
	
    #else
    PWM_ConfigOutputChannel(PWM0, PWM_ch1_no, freq_ch1 , 0);
    #endif    
    PWM_EnableOutput(PWM0, PWM_ch1_MASK);
    PWM_Start(PWM0, PWM_ch1_MASK); 


    PWM_ConfigOutputChannel(PWM0, PWM_ch2_no, freq_ch2 , duty_ch2);
    PWM_EnableOutput(PWM0, PWM_ch2_MASK);
    PWM_Start(PWM0, PWM_ch2_MASK);              

}

void GPIO_Init (void)
{
    // SYS->GPA_MFP0 = (SYS->GPA_MFP0 & ~(SYS_GPA_MFP0_PA2MFP_Msk)) | (SYS_GPA_MFP0_PA2MFP_GPIO);
    SYS->GPC_MFP0 = (SYS->GPC_MFP0 & ~(SYS_GPC_MFP0_PC2MFP_Msk)) | (SYS_GPC_MFP0_PC2MFP_GPIO);
		
	// EVM LED_R
    // GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT);

	// EVM button
    GPIO_SetMode(PC, BIT2, GPIO_MODE_OUTPUT);	
}


void TMR1_IRQHandler(void)
{
	// static uint32_t LOG = 0;

	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
        	// printf("%s : %4d\r\n",__FUNCTION__,LOG++);

		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}


void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case 'a':
			case 'A':
                duty_ch1 += 5;
                if (duty_ch1 >= 100)
                {
                    duty_ch1 = 100;
                }
				break;

			case 'd':
			case 'D':    
                duty_ch1 -= 5;
                if (duty_ch1 <= 5)
                {
                    duty_ch1 = 5;
                }                    
				break;    

			case '1':
                duty_ch2 += 5;
                if (duty_ch2 >= 100)
                {
                    duty_ch2 = 100;
                }
				break;                            

			case '2':    
                duty_ch2 -= 5;
                if (duty_ch2 <= 5)
                {
                    duty_ch2 = 5;
                }                    
				break;    

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}

void UART0_IRQHandler(void)
{

    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
            UARTx_Process();
            set_flag(flag_pwm_adjust, ENABLE);            
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	#endif	

}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

//	CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	

//	CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);	

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);

    CLK_EnableModuleClock(PWM0_MODULE);
    SYS_ResetModule(PWM0_RST);

    /* Set PB multi-function pins for UART0 RXD=PB.6 and TXD=PB.4 */
    SYS->GPB_MFP1 = (SYS->GPB_MFP1 & ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk)) |        \
                    (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);

    /*
        PA2 , PWM0_CH1 , 
        PC0 , PWM0_CH4 ,         
    */
    SYS->GPA_MFP0 = (SYS->GPA_MFP0 & ~(SYS_GPA_MFP0_PA2MFP_Msk )) |
                    (SYS_GPA_MFP0_PA2MFP_PWM0_CH1);

    SYS->GPC_MFP0 = (SYS->GPC_MFP0 & ~( SYS_GPC_MFP0_PC0MFP_Msk)) |
                    (SYS_GPC_MFP0_PC0MFP_PWM0_CH4);


   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
    SYS_Init();

	UART0_Init();
	GPIO_Init();
	TIMER1_Init();

    PWM_Init();

    /* Got no where to go, just loop forever */
    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
