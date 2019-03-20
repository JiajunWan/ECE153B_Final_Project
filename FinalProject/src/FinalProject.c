#include "board.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

#define GPIO_INTERRUPT_PIN 10              // GPIO pin number mapped to interrupt
#define GPIO_INTERRUPT_PORT GPIOINT_PORT2  // GPIO port number mapped to interrupt
#define GPIO_IRQ_HANDLER GPIO_IRQHandler   // GPIO interrupt IRQ function name
#define GPIO_INTERRUPT_NVIC_NAME GPIO_IRQn // GPIO interrupt NVIC interrupt name

// Timer0 for initial "PRESSALL" display signal and two sides progress bar
#define TIMER0_IRQ_HANDLER TIMER0_IRQHandler   // TIMER0 interrupt IRQ function name
#define TIMER0_INTERRUPT_NVIC_NAME TIMER0_IRQn // TIMER0 interrupt NVIC interrupt name
// Timer1 for LED ClkPin Clock
#define TIMER1_IRQ_HANDLER TIMER1_IRQHandler   // TIMER1 interrupt IRQ function name
#define TIMER1_INTERRUPT_NVIC_NAME TIMER1_IRQn // TIMER1 interrupt NVIC interrupt name
// Timer2 for LED note falling control
#define TIMER2_IRQ_HANDLER TIMER2_IRQHandler   // TIMER2 interrupt IRQ function name
#define TIMER2_INTERRUPT_NVIC_NAME TIMER2_IRQn // TIMER2 interrupt NVIC interrupt name
// Timer3 for Peripheral Buttons
#define TIMER3_IRQ_HANDLER TIMER3_IRQHandler   // TIMER3 interrupt IRQ function name
#define TIMER3_INTERRUPT_NVIC_NAME TIMER3_IRQn // TIMER3 interrupt NVIC interrupt name
#define GPIO_PORT 2
#define DATA_PIN 22
#define CS_PIN 14
#define CLK_PIN 27
#define LEFT_BUTTON_PIN 25
#define MIDDLE_BUTTON_PIN 23
#define RIGHT_BUTTON_PIN 26

// The opcodes for the MAX7221 and MAX7219
uint8_t OP_NOOP = 0;
uint8_t OP_DIGIT0 = 1;
uint8_t OP_DIGIT1 = 2;
uint8_t OP_DIGIT2 = 3;
uint8_t OP_DIGIT3 = 4;
uint8_t OP_DIGIT4 = 5;
uint8_t OP_DIGIT5 = 6;
uint8_t OP_DIGIT6 = 7;
uint8_t OP_DIGIT7 = 8;
uint8_t OP_DECODEMODE = 9;
uint8_t OP_INTENSITY = 10;
uint8_t OP_SCANLIMIT = 11;
uint8_t OP_SHUTDOWN = 12;
uint8_t OP_DISPLAYTEST = 15;
#define LSBFIRST 0
#define MSBFIRST 1
int fallcounter = 0;
int milisecond = 0;
RTC_TIME_T FullTime;
// 32-bit rowcontent for LED display
uint32_t row2content = 0b00000000000000000000000000000000;
uint32_t row1content = 0b00000000000000000000000000000000;
uint32_t row0content = 0b00000000000000000000000000000000;
uint32_t leftcontent = 0b00000000000000000000000000000001;
uint32_t rightcontent = 0b00000000000000000000000000000001;
// Sub-row content for each 8x8 LED display
uint8_t addr3row2content;
uint8_t addr2row2content;
uint8_t addr1row2content;
uint8_t addr0row2content;
uint8_t addr3row1content;
uint8_t addr2row1content;
uint8_t addr1row1content;
uint8_t addr0row1content;
uint8_t addr3row0content;
uint8_t addr2row0content;
uint8_t addr1row0content;
uint8_t addr0row0content;
uint8_t addr3leftcontent;
uint8_t addr2leftcontent;
uint8_t addr1leftcontent;
uint8_t addr0leftcontent;
uint8_t addr3rightcontent;
uint8_t addr2rightcontent;
uint8_t addr1rightcontent;
uint8_t addr0rightcontent;
// Bit-masking
uint32_t addr3and = 0b11111111000000000000000000000000;
uint32_t addr2and = 0b00000000111111110000000000000000;
uint32_t addr1and = 0b00000000000000001111111100000000;
uint32_t addr0and = 0b00000000000000000000000011111111;
// Final score digits
uint8_t Digits[10][8] = {
{
  0b0000000,
  0b0111100,
  0b1100110,
  0b1101110,
  0b1110110,
  0b1100110,
  0b1100110,
  0b0111100
},{
  0b0000000,
  0b0011000,
  0b0011000,
  0b0111000,
  0b0011000,
  0b0011000,
  0b0011000,
  0b1111110
},{
  0b0000000,
  0b0111100,
  0b1100110,
  0b0000110,
  0b0001100,
  0b0110000,
  0b1100000,
  0b1111110
},{
  0b0000000,
  0b0111100,
  0b1100110,
  0b0000110,
  0b0011100,
  0b0000110,
  0b1100110,
  0b0111100
},{
  0b0000000,
  0b0001100,
  0b0011100,
  0b0101100,
  0b1001100,
  0b1111110,
  0b0001100,
  0b0001100
},{
  0b0000000,
  0b1111110,
  0b1100000,
  0b1111100,
  0b0000110,
  0b0000110,
  0b1100110,
  0b0111100
},{
  0b0000000,
  0b0111100,
  0b1100110,
  0b1100000,
  0b1111100,
  0b1100110,
  0b1100110,
  0b0111100
},{
  0b0000000,
  0b1111110,
  0b1100110,
  0b0001100,
  0b0001100,
  0b0011000,
  0b0011000,
  0b0011000
},{
  0b0000000,
  0b0111100,
  0b1100110,
  0b1100110,
  0b0111100,
  0b1100110,
  0b1100110,
  0b0111100
},{
  0b0000000,
  0b0111100,
  0b1100110,
  0b1100110,
  0b0111110,
  0b0000110,
  0b1100110,
  0b0111100
}};

// Global variables
int perfect = 0;
bool FinalClearDisplay = false;
int good = 0;
int combo = 0;
int left_range[217] = {0};
int right_range[217] = {0};
int current_time = 0;
bool missed = 0;
bool consecutive[3] = {0, 0, 0};
int notefalltime = 960;
int goodthreshold = 300;
int perfectthreshold = 100;
int maxnote = 217;
int notesecond[217] = {0};
int notems[217] = {0};
bool goodcaught[217] = {false};
int currentnote = 0;
int currentcatch = 0;
int nextcatchgoodlowsecond;
int nextcatchgoodlowms;
int nextcatchperfectlowsecond;
int nextcatchperfectlowms;
int nextcatchperfecthighsecond;
int nextcatchperfecthighms;
int nextcatchgoodhighsecond;
int nextcatchgoodhighms;
int next[3] = {0};
int nextcatch[11] = {0};
// Falling notes data in arrays
int noterail[217] = {0, 1, 2, 1, 0, 2, 0, 2, 1, 1, 0, 1, 0, 2, 1, 1, 2, 0, 1, 1, 2, 1, 0, 2, 1, 1, 1, 2, 1, 0, 0, 2, 1, 2, 2, 1, 0, 1, 2, 0, 0, 1, 1, 1, 2, 0, 2, 0, 0, 1, 0, 0, 1, 0, 1, 2, 1, 0, 1, 2, 1, 0, 2, 1, 1, 2, 0, 0, 1, 0, 2, 1, 1, 0, 1, 0, 2, 2, 2, 1, 0, 1, 0, 2, 1, 0, 1, 0, 2, 1, 1, 2, 0, 1, 2, 1, 0, 1, 0, 1, 1, 2, 1, 2, 0, 1, 0, 1, 1, 2, 1, 2, 0, 1, 2, 1, 0, 0, 1, 2, 1, 0, 1, 2, 1, 2, 0, 2, 1, 0, 0, 2, 1, 0, 2, 0, 1, 2, 0, 2, 1, 0, 1, 0, 1, 2, 1, 2, 0, 2, 0, 1, 2, 0, 1, 0, 2, 1, 0, 2, 1, 0, 0, 2, 2, 1, 2, 0, 2, 0, 1, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2, 0, 2, 0, 0, 1, 2, 0, 2, 1, 2, 0, 1, 2, 1, 2, 1, 0, 1, 0, 2, 1, 2, 0, 2, 0, 1, 0, 2, 1, 2, 1, 1, 2, 1, 0, 1};
int catchms[217] = {0, 500, 333, 667, 167, 0, 333, 833, 667, 0, 333, 667, 0, 333, 667, 167, 0, 667, 333, 833, 667, 0, 500, 333, 833, 667, 333, 667, 333, 833, 667, 333, 0, 500, 333, 667, 167, 0, 500, 333, 667, 0, 333, 660, 670, 0, 500, 333, 0, 667, 167, 995, 5, 333, 833, 667, 167, 995, 5, 660, 670, 995, 5, 333, 500, 667, 833, 0, 167, 333, 500, 667, 0, 133, 660, 670, 0, 333, 667, 833, 0, 167, 333, 500, 667, 0, 333, 667, 0, 333, 660, 670, 0, 333, 500, 333, 667, 333, 0, 667, 333, 0, 667, 333, 995, 5, 660, 670, 330, 340, 995, 5, 667, 0, 167, 333, 0, 333, 667, 833, 0, 667, 0, 167, 333, 500, 333, 667, 0, 667, 833, 0, 167, 333, 500, 667, 833, 0, 167, 333, 500, 667, 0, 333, 0, 667, 333, 0, 667, 333, 0, 667, 333, 0, 667, 333, 0, 667, 333, 0, 667, 333, 0, 667, 833, 0, 167, 0, 667, 333, 500, 667, 833, 667, 0, 167, 333, 500, 333, 500, 667, 833, 167, 333, 667, 333, 667, 0, 167, 333, 500, 667, 833, 0, 167, 333, 500, 667, 833, 667, 333, 0, 167, 333, 500, 333, 667, 833, 0, 167, 0, 167, 333, 500, 333, 0, 333};
int catchsecond[217] = {1, 1, 2, 2, 3, 4, 5, 5, 6, 8, 8, 8, 9, 9, 10, 11, 12, 12, 13, 13, 14, 16, 16, 17, 17, 18, 19, 19, 21, 21, 22, 23, 24, 24, 25, 26, 27, 28, 28, 29, 29, 30, 30, 30, 30, 32, 32, 33, 34, 34, 35, 35, 36, 37, 37, 38, 39, 39, 40, 40, 40, 40, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 44, 45, 46, 46, 48, 49, 49, 49, 50, 50, 50, 50, 50, 51, 51, 51, 52, 53, 54, 54, 56, 56, 56, 57, 58, 59, 60, 60, 61, 62, 62, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 67, 67, 67, 68, 69, 69, 69, 70, 70, 72, 72, 72, 72, 73, 73, 74, 74, 74, 75, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 77, 77, 78, 78, 79, 80, 80, 81, 82, 82, 83, 84, 84, 85, 86, 86, 87, 88, 88, 89, 90, 90, 90, 91, 91, 92, 92, 93, 93, 93, 93, 94, 96, 96, 96, 96, 97, 97, 97, 97, 98, 98, 98, 99, 99, 100, 100, 100, 100, 100, 100, 101, 101, 101, 101, 101, 101, 102, 103, 104, 104, 104, 104, 105, 106, 106, 107, 107, 108, 108, 108, 108, 109, 110, 110};

typedef uint8_t byte;
bool dual = false;
uint32_t second = 0;
uint8_t spidata[16];
uint8_t status[64];
int state = 0;
int SPI_MOSI = DATA_PIN;
/* The clock is signaled on this pin */
int SPI_CLK = CLK_PIN;
/* This one is driven LOW for chip selection */
int SPI_CS = CS_PIN;
/* The maximum number of devices we use */
int maxDevices = 4;

// Set a row of LEDs in a 8x8 LED matrix with specified address(CS), row number and value in byte(8-bit data)
void setRow(int addr, int row, byte value);

// Set a column of LEDs in a 8x8 LED matrix with specified address(CS), column number and value in byte(8-bit data)
void setColumn(int addr, int col, byte value);

// SPI tranfer data to specified address 8x8 LED with operation code
void spiTransfer(int addr, byte opcode, byte data);

// Clear display for all LEDs
void clearDisplay(int addr);

// RTC used for precise second tracking
void RTC_IRQHandler(void)
{
    second = FullTime.time[RTC_TIMETYPE_MINUTE] * 60 + FullTime.time[RTC_TIMETYPE_SECOND] - 1;
    if (second == -1)
    {
        second = 0;
    }

    if (Chip_RTC_GetIntPending(LPC_RTC, RTC_INT_ALARM))
    { 
		// if alarm is triggered (every second)
        Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM);

		// Reset milisecond and fallcounter used in each second
        milisecond = 0;
        fallcounter = 0;
        if (FullTime.time[RTC_TIMETYPE_SECOND] == 59)
        {
            FullTime.time[RTC_TIMETYPE_SECOND] = 0;
            FullTime.time[RTC_TIMETYPE_MINUTE]++;
        }
        else
        {
            FullTime.time[RTC_TIMETYPE_SECOND]++;
        }
        Chip_RTC_SetFullAlarmTime(LPC_RTC, &FullTime); // increment second to set alarm again
    }
}

// Timer0 for initial "PRESSALL" display signal and two sides progress bar
void TIMER0_IRQHandler(void)
{
	// Display "PRESSALL"
	if (state == 0)
	{
		setRow(3, 0, 0b00000000);
		setRow(3, 1, 0b11001100);
		setRow(3, 2, 0b10101010);
		setRow(3, 3, 0b10101010);
		setRow(3, 4, 0b11001100);
		setRow(3, 5, 0b10001010);
		setRow(3, 6, 0b10001010);
		setRow(3, 7, 0b00000000);
		setRow(2, 0, 0b00000000);
				setRow(2, 1, 0b11100110);
				setRow(2, 2, 0b10001000);
				setRow(2, 3, 0b11101100);
				setRow(2, 4, 0b10000010);
				setRow(2, 5, 0b10000010);
				setRow(2, 6, 0b11101100);
				setRow(2, 7, 0b00000000);
				setRow(1, 0, 0b00000000);
						setRow(1, 1, 0b01100100);
						setRow(1, 2, 0b10001010);
						setRow(1, 3, 0b11001010);
						setRow(1, 4, 0b00101110);
						setRow(1, 5, 0b00101010);
						setRow(1, 6, 0b11001010);
						setRow(1, 7, 0b00000000);
						setRow(0, 0, 0b00000000);
								setRow(0, 1, 0b10001000);
								setRow(0, 2, 0b10001000);
								setRow(0, 3, 0b10001000);
								setRow(0, 4, 0b10001000);
								setRow(0, 5, 0b10001000);
								setRow(0, 6, 0b11101110);
								setRow(0, 7, 0b00000000);
	}

	// Progress bar
	if (state == 1)
	{
		int i;
		int numcaught = 0;
		leftcontent = 0b00000000000000000000000000000001;
		rightcontent = 0b00000000000000000000000000000001;
		for (i = 0; i < currentcatch; i++)
		{
			if (goodcaught[i] == true)
			{
				numcaught++;
			}
		}

		int leftnumline = currentcatch/6.78;
		int rightnumline = numcaught/6.78;
		i = 0;
		// Bit shifting for progress bar shifting
		for (i = 0; i < leftnumline; i++)
		{
			leftcontent = leftcontent << 1;
			leftcontent = leftcontent + 0b00000000000000000000000000000001;
		}
		rightcontent = leftcontent;

		// Bit masking to generate value for each 8x8 LED
		addr3leftcontent = (leftcontent & addr3and) >> 24;
		addr2leftcontent = (leftcontent & addr2and) >> 16;
		addr1leftcontent = (leftcontent & addr1and) >> 8;
		addr0leftcontent = (leftcontent & addr0and);
		addr3rightcontent = (rightcontent & addr3and) >> 24;
		addr2rightcontent = (rightcontent & addr2and) >> 16;
		addr1rightcontent = (rightcontent & addr1and) >> 8;
		addr0rightcontent = (rightcontent & addr0and);

		// Set rows with coresponding values
		setRow(3, 0, addr3leftcontent);
		setRow(2, 0, addr2leftcontent);
		setRow(1, 0, addr1leftcontent);
		setRow(0, 0, addr0leftcontent);
		setRow(3, 7, addr3rightcontent);
		setRow(2, 7, addr2rightcontent);
		setRow(1, 7, addr1rightcontent);
		setRow(0, 7, addr0rightcontent);
	}

    Chip_TIMER_Disable(LPC_TIMER0);
    Chip_TIMER_Reset(LPC_TIMER0);
    Chip_TIMER_ClearMatch(LPC_TIMER0, 0);
    Chip_TIMER_Enable(LPC_TIMER0);
}

// Timer1 for SPI CLK PIN
void TIMER1_IRQHandler(void)
{
    Chip_TIMER_Disable(LPC_TIMER1);
    Chip_TIMER_Reset(LPC_TIMER1);
    Chip_TIMER_ClearMatch(LPC_TIMER1, 0);
}

// Timer2 for LED note falling control
void TIMER2_IRQHandler(void)
{
	if (state == 1){
		// Count milisecond and fall interval
		milisecond += 1;
		fallcounter++;
		if (fallcounter == 30)
		{
			// If the current time is in the time range for next falling note, then display the next falling note
			if ((next[1] == second) && (next[2] <= milisecond))
			{
				if (!dual)
				{
					switch (next[0])
					{
					case 0:
						row0content += 0b10000000000000000000000000000000;
						break;
					case 1:
						row1content += 0b10000000000000000000000000000000;
						break;
					case 2:
						row2content += 0b10000000000000000000000000000000;
						break;
					default:
						break;
					}
				}
				else
				{
					if (noterail[currentnote] == 0)
					{
						row0content += 0b10000000000000000000000000000000;
					}
					if (noterail[currentnote] == 1)
					{
						row1content += 0b10000000000000000000000000000000;
					}
					if (noterail[currentnote] == 2)
					{
						row2content += 0b10000000000000000000000000000000;
					}
				}
				// Detect if all the notes are displayed
				if (currentnote >= (maxnote-2))
				{
					state = 2;
				}
				// Handle next note
				currentnote++;
				next[0] = noterail[currentnote];
				next[1] = notesecond[currentnote];
				next[2] = notems[currentnote];
				// Handling for dual notes
				if ((((notems[currentnote+1] - notems[currentnote]) <= 25) && (notesecond[currentnote+1] == notesecond[currentnote])) || (((notems[currentnote] - notems[currentnote+1]) >= 975) && ((notesecond[currentnote+1] - notesecond[currentnote]) == 1)))
				{
					dual = true;
					currentnote++;
				}
			}
			// Bit masking and display rows
			row2content = row2content >> 1;
			row1content = row1content >> 1;
			row0content = row0content >> 1;
			addr3row2content = (row2content & addr3and) >> 24;
			addr2row2content = (row2content & addr2and) >> 16;
			addr1row2content = (row2content & addr1and) >> 8;
			addr0row2content = (row2content & addr0and);
			addr3row1content = (row1content & addr3and) >> 24;
			addr2row1content = (row1content & addr2and) >> 16;
			addr1row1content = (row1content & addr1and) >> 8;
			addr0row1content = (row1content & addr0and);
			addr3row0content = (row0content & addr3and) >> 24;
			addr2row0content = (row0content & addr2and) >> 16;
			addr1row0content = (row0content & addr1and) >> 8;
			addr0row0content = (row0content & addr0and);
			setRow(3, 5, addr3row2content);
			setRow(2, 5, addr2row2content);
			setRow(1, 5, addr1row2content);
			setRow(0, 5, addr0row2content);
			setRow(3, 6, addr3row2content);
			setRow(2, 6, addr2row2content);
			setRow(1, 6, addr1row2content);
			setRow(0, 6, addr0row2content);
			setRow(3, 3, addr3row1content);
			setRow(2, 3, addr2row1content);
			setRow(1, 3, addr1row1content);
			setRow(0, 3, addr0row1content);
			setRow(3, 4, addr3row1content);
			setRow(2, 4, addr2row1content);
			setRow(1, 4, addr1row1content);
			setRow(0, 4, addr0row1content);
			setRow(3, 1, addr3row0content);
			setRow(2, 1, addr2row0content);
			setRow(1, 1, addr1row0content);
			setRow(0, 1, addr0row0content);
			setRow(3, 2, addr3row0content);
			setRow(2, 2, addr2row0content);
			setRow(1, 2, addr1row0content);
			setRow(0, 2, addr0row0content);
			fallcounter = 0;
		}
	}
	// If game finishes
	if (state == 2) 
	{
		if (!FinalClearDisplay)
		{
			int i;
			for (i = 0; i < maxDevices; i++)
			{
				clearDisplay(i);
			}
			FinalClearDisplay = true;
		}
		else
		{
			// Display the final score
			int totalcaught = 0;
			int n;
			for (n = 0; n < currentcatch; n++)
			{
				if (goodcaught[n] == true)
				{
					totalcaught++;
				}
			}
			int score = totalcaught*100/maxnote;
			if (score >= 100)
			{
				score = 99;
			}
			int tens = score/10;
			int digits = score%10;
			for (n = 0; n < 8; n ++)
			{
				setRow(2, n, Digits[tens][n]);
			}
			for (n = 0; n < 8; n ++)
			{
				setRow(1, n, Digits[digits][n]);
			}
		}
	}
    Chip_TIMER_Disable(LPC_TIMER2);
    Chip_TIMER_Reset(LPC_TIMER2);
    Chip_TIMER_ClearMatch(LPC_TIMER2, 0);
    Chip_TIMER_Enable(LPC_TIMER2);
}

// Timer3 for push buttons handling
void TIMER3_IRQHandler(void)
{
	// Initialize RTC
	if (state == 0) {
		// If the three buttons are pressed all together, then the game will start
		if (Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT, LEFT_BUTTON_PIN) && Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT, MIDDLE_BUTTON_PIN) && Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT, RIGHT_BUTTON_PIN)) {
			state = 1;
			// Initialize RTC
			Chip_RTC_Init(LPC_RTC);
    		FullTime.time[RTC_TIMETYPE_SECOND] = 0;
    		FullTime.time[RTC_TIMETYPE_MINUTE] = 0;
    		FullTime.time[RTC_TIMETYPE_HOUR] = 0;
    		FullTime.time[RTC_TIMETYPE_DAYOFMONTH] = 5;
    		FullTime.time[RTC_TIMETYPE_DAYOFWEEK] = 5;
    		FullTime.time[RTC_TIMETYPE_DAYOFYEAR] = 279;
    		FullTime.time[RTC_TIMETYPE_MONTH] = 10;
    		FullTime.time[RTC_TIMETYPE_YEAR] = 2012;
    		Chip_RTC_SetFullTime(LPC_RTC, &FullTime);
    		Chip_RTC_SetFullAlarmTime(LPC_RTC, &FullTime);
    		Chip_RTC_CntIncrIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC, ENABLE);
    		Chip_RTC_AlarmIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC | RTC_AMR_CIIR_IMMIN, ENABLE);
    		Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE | RTC_INT_ALARM);
    		NVIC_EnableIRQ((IRQn_Type)RTC_IRQn);
			Chip_RTC_Enable(LPC_RTC, ENABLE);
		}
	}
	else if (state == 1){
		// Calculate the current time in ms unit
		current_time = second * 1000 + milisecond;
		// If the current time is greater than the right catch range of the last note, then we are out of the range of last note and should catch next note
		if (current_time > right_range[currentcatch])
		{
			currentcatch++;
		}

		// If the current time is in the range of the current note to catch
		if ((current_time > left_range[currentcatch]) && (current_time < right_range[currentcatch]))
		{
			// Catch for left note rail
			if (noterail[currentcatch] == 0)
			{
				// If the button is pressed
				if (Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT, LEFT_BUTTON_PIN) == true)
				{
					// Update the array for whether the specific note is caught
					goodcaught[currentcatch] = true;
				}
			}

			// Catch for middle note rail
			else if (noterail[currentcatch] == 1)
			{
				// If the button is pressed
				if (Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT, MIDDLE_BUTTON_PIN) == true)
				{
					goodcaught[currentcatch] = true;
				}
			}

			// Catch for right note rail
			else if (noterail[currentcatch] == 2)
			{
				if (Chip_GPIO_GetPinState(LPC_GPIO, GPIO_PORT, RIGHT_BUTTON_PIN) == true)
				{
					goodcaught[currentcatch] = true;
				}
			}
		}
	}

    Chip_TIMER_Disable(LPC_TIMER3);
    Chip_TIMER_Reset(LPC_TIMER3);
    Chip_TIMER_ClearMatch(LPC_TIMER3, 0);
    Chip_TIMER_Enable(LPC_TIMER3);
}

// Shift the data to datapin for SPI
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
    uint8_t j;
    for (j = 0; j < 8; j++)
    {
        if (bitOrder == LSBFIRST)
            Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, dataPin,
                                  !!(val & (1 << j)));
        else
            Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, dataPin,
                                  !!(val & (1 << (7 - j))));
        Chip_TIMER_Enable(LPC_TIMER1); // Start TIMER1
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, clockPin, true);
        Chip_TIMER_Enable(LPC_TIMER1); // Start TIMER1
        Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, clockPin, false);
        Chip_TIMER_Enable(LPC_TIMER1); // Start TIMER1
    }
    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, dataPin, true);
}

int getDeviceCount()
{
    return maxDevices;
}

// Shutdown LED matrix
void shutdown(int addr, bool b)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (b)
        spiTransfer(addr, OP_SHUTDOWN, 0);
    else
        spiTransfer(addr, OP_SHUTDOWN, 1);
}

//Set scan limit
void setScanLimit(int addr, int limit)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (limit >= 0 && limit < 8)
        spiTransfer(addr, OP_SCANLIMIT, limit);
}

// Set intensity
void setIntensity(int addr, int intensity)
{
    if (addr < 0 || addr >= maxDevices)
        return;
    if (intensity >= 0 && intensity < 16)
        spiTransfer(addr, OP_INTENSITY, intensity);
}

// Clear display for specific address(CS) 8x8 LED
void clearDisplay(int addr)
{
    int offset;
    if (addr < 0 || addr >= maxDevices)
        return;
    offset = addr * 8;
    int i;
    for (i = 0; i < 8; i++)
    {
        status[offset + i] = 0;
        spiTransfer(addr, i + 1, status[offset + i]);
    }
}

// Set led state with address, row and column
void setLed(int addr, int row, int column, bool state)
{
    int offset;
    uint8_t val = 0x00;
    if (addr < 0 || addr >= maxDevices)
        return;
    if (row < 0 || row > 7 || column < 0 || column > 7)
        return;
    offset = addr * 8;
    val = 10000000 >> column;
    if (state)
        status[offset + row] = status[offset + row] | val;
    else
    {
        val = ~val;
        status[offset + row] = status[offset + row] & val;
    }
    spiTransfer(addr, row + 1, status[offset + row]);
}

// Set a row of LEDs in a 8x8 LED matrix with specified address(CS), row number and value in byte(8-bit data)
void setRow(int addr, int row, byte value)
{
    int offset;
    if (addr < 0 || addr >= maxDevices)
        return;
    if (row < 0 || row > 7)
        return;
    offset = addr * 8;
    status[offset + row] = value;
    spiTransfer(addr, row + 1, status[offset + row]);
}

// Set a column of LEDs in a 8x8 LED matrix with specified address(CS), column number and value in byte(8-bit data)
void setColumn(int addr, int col, byte value)
{
    uint8_t val;
    if (addr < 0 || addr >= maxDevices)
        return;
    if (col < 0 || col > 7)
        return;
    int row;
    for (row = 0; row < 8; row++)
    {
        val = value >> (7 - row);
        val = val & 0x01;
        setLed(addr, row, col, val);
    }
}

// SPI tranfer data to specified address 8x8 LED with operation code
void spiTransfer(int addr, byte opcode, byte data)
{
    // Create an array with the data to shift out
    int offset = addr * 2;
    int maxbytes = maxDevices * 2;
    int i;
    for (i = 0; i < maxbytes; i++)
	{
        spidata[i] = (byte)0;
	}
    // Put our device data into the array
    spidata[offset + 1] = opcode;
    spidata[offset] = data;
    // Enable the line
    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, SPI_CS, false);
    // Now shift out the data
    for (i = maxbytes; i > 0; i--)
    {
        shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, spidata[i - 1]);
    }
    // Latch the data onto the display
    Chip_GPIO_SetPinState(LPC_GPIO, GPIO_PORT, SPI_CS, true);
}

int main(void)
{
    int i = 0;

	// Data initialization for the first sequence of note data
    if (catchms[0] < goodthreshold)
    {
        nextcatchgoodlowsecond = catchsecond[0] - 1;
        nextcatchgoodlowms = 1000 - (goodthreshold - catchms[0]);
    }
    else
    {
        nextcatchgoodlowsecond = catchsecond[0] - 1;
        nextcatchgoodlowms = catchms[0] - goodthreshold;
    }
    if (catchms[0] < perfectthreshold)
    {
        nextcatchperfectlowsecond = catchsecond[0] - 1;
        nextcatchperfectlowms = 1000 - (perfectthreshold - catchms[0]);
    }
    else
    {
        nextcatchperfectlowsecond = catchsecond[0];
        nextcatchperfectlowms = catchms[0] - perfectthreshold;
    }
    if ((1000 - catchms[0]) < goodthreshold)
    {
        nextcatchgoodhighsecond = catchsecond[0] + 1;
        nextcatchgoodhighms = catchms[0] - (1000 - goodthreshold);
    }
    else
    {
        nextcatchgoodhighsecond = catchsecond[0];
        nextcatchgoodhighms = catchms[0] + goodthreshold;
    }
    if ((1000 - catchms[0]) < perfectthreshold)
    {
        nextcatchperfecthighsecond = catchsecond[0] + 1;
        nextcatchperfecthighms = catchms[0] - (1000 - perfectthreshold);
    }
    else
    {
        nextcatchperfecthighsecond = catchsecond[0];
        nextcatchperfecthighms = catchms[0] + perfectthreshold;
    }
    int p;
    for (p = 0; p < (maxnote - 1); p++)
    {
        if (catchms[p] < notefalltime)
        {
            notesecond[p] = catchsecond[p] - 1;
            notems[p] = 1000 - (notefalltime - catchms[p]);
        }
        else
        {
            notesecond[p] = catchsecond[p];
            notems[p] = catchms[p] - notefalltime;
        }
    }
    next[0] = noterail[0];
    next[1] = notesecond[0];
    next[2] = notems[0];
    nextcatch[0] = noterail[0];
    nextcatch[1] = catchsecond[0];
    nextcatch[2] = catchms[0];
    nextcatch[3] = nextcatchgoodlowsecond;
    nextcatch[4] = nextcatchgoodlowms;
    nextcatch[5] = nextcatchperfectlowsecond;
    nextcatch[6] = nextcatchperfectlowms;
    nextcatch[7] = nextcatchperfecthighsecond;
    nextcatch[8] = nextcatchperfecthighms;
    nextcatch[9] = nextcatchgoodhighsecond;
    nextcatch[10] = nextcatchgoodhighms;

	// Initialize the range for note catch
    for (i = 0; i < 217; i++)
    {
        left_range[i] = catchsecond[i] * 1000 + catchms[i] + notefalltime - goodthreshold;
        right_range[i] = catchsecond[i] * 1000 + catchms[i] + notefalltime + goodthreshold;
    }

    // Generic Initialization
    SystemCoreClockUpdate();
    Board_Init();

    int PrescaleValue = 119; // Clock cycle / 1000000 (set to us increments)

    // Initialize TIMER0
    Chip_TIMER_Init(LPC_TIMER0);
    Chip_TIMER_PrescaleSet(LPC_TIMER0, PrescaleValue);
    Chip_TIMER_SetMatch(LPC_TIMER0, 0, 1000000);
    Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);

    // Initialize TIMER1
    Chip_TIMER_Init(LPC_TIMER1);
    Chip_TIMER_PrescaleSet(LPC_TIMER1, PrescaleValue);
    Chip_TIMER_SetMatch(LPC_TIMER1, 0, 1);
    Chip_TIMER_MatchEnableInt(LPC_TIMER1, 0);

    // Initialize TIMER2
    Chip_TIMER_Init(LPC_TIMER2);
    Chip_TIMER_PrescaleSet(LPC_TIMER2, PrescaleValue);
    Chip_TIMER_SetMatch(LPC_TIMER2, 0, 440);
    Chip_TIMER_MatchEnableInt(LPC_TIMER2, 0);

    // Initialize TIMER3
    Chip_TIMER_Init(LPC_TIMER3);
    Chip_TIMER_PrescaleSet(LPC_TIMER3, PrescaleValue);
    Chip_TIMER_SetMatch(LPC_TIMER3, 0, 1000);
    Chip_TIMER_MatchEnableInt(LPC_TIMER3, 0);

    // Configure the GPIO output pins for LED Pins
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT, DATA_PIN);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT, CLK_PIN);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, GPIO_PORT, CS_PIN);

    // Configure the GPIO input pins for buttons
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_PORT, LEFT_BUTTON_PIN);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_PORT, MIDDLE_BUTTON_PIN);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_PORT, RIGHT_BUTTON_PIN);

    NVIC_ClearPendingIRQ(GPIO_INTERRUPT_NVIC_NAME); // Clear pending interrupt flag
    NVIC_EnableIRQ(GPIO_INTERRUPT_NVIC_NAME);
    // Enable interrupt handling in NVIC

	// Timer interrupt initialization
    NVIC_ClearPendingIRQ(TIMER0_INTERRUPT_NVIC_NAME);
    NVIC_EnableIRQ(TIMER0_INTERRUPT_NVIC_NAME);
    NVIC_ClearPendingIRQ(TIMER1_INTERRUPT_NVIC_NAME);
    NVIC_EnableIRQ(TIMER1_INTERRUPT_NVIC_NAME);
    NVIC_ClearPendingIRQ(TIMER2_INTERRUPT_NVIC_NAME);
    NVIC_EnableIRQ(TIMER2_INTERRUPT_NVIC_NAME);
    NVIC_ClearPendingIRQ(TIMER3_INTERRUPT_NVIC_NAME);
    NVIC_EnableIRQ(TIMER3_INTERRUPT_NVIC_NAME);
    Chip_TIMER_Enable(LPC_TIMER0);
    Chip_TIMER_Enable(LPC_TIMER1);
    Chip_TIMER_Enable(LPC_TIMER2);
    Chip_TIMER_Enable(LPC_TIMER3);

	// LED matrix initialization
    for (i = 0; i < 64; i++)
    {
        status[i] = 0x00;
    }
    for (i = 0; i < maxDevices; i++)
    {
        spiTransfer(i, OP_DISPLAYTEST, 0);
        //scanlimit is set to max on startup
        setScanLimit(i, 7);
        //decode is done in source
        spiTransfer(i, OP_DECODEMODE, 0);
        clearDisplay(i);
        // Start the four LED blocks
        shutdown(i, false);
        setIntensity(i, 3);
    }

    while (1)
    {
        __WFI();
    }
    return 0;
}
