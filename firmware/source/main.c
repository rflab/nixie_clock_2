//Copyright (C) 2008 Iwahara Hiroaki All Rights Reserved.

#include <htc.h>
#include <math.h>
//----------------- 
// �R���p�C���I�v�V����
//-----------------

// �R���t�B�O���[�V�������W�X�^���f�o�b�O�p�ɕύX����
//#define USE_DEBUGGER

//---------------
// �R���t�B�O
//---------------
// �R���t�B�O���[�V�������W�X�^
#ifdef USE_DEBUGGER
	__CONFIG(
		CP_OFF
		& CPD_OFF
		& FOSC_INTRC_NOCLKOUT
		& DEBUG_ON // �C���T�[�L�b�g�f�o�b�O�L��
		& BOREN_ON
		& IESO_OFF	// Internal/External Switchover mode
		& MCLRE_ON	// RE3/MCLR pin function is digital input, MCLR internally tied to VDD
		& PWRTE_OFF
		& LVP_OFF
		& FCMEN_ON
		& WDTE_OFF
		& WRT_OFF);
#else
	__CONFIG(
		CP_OFF
		& CPD_OFF
		& FOSC_INTRC_NOCLKOUT
		& DEBUG_OFF	  // �C���T�[�L�b�g�f�o�b�O����
		& BOREN_ON
		& IESO_OFF	 // Internal/External Switchover mode
		& MCLRE_ON // RE3/MCLR pin function is digital input, MCLR internally tied to VDD
		& PWRTE_ON
		& LVP_OFF
		& FCMEN_ON
		& WDTE_OFF
		& WRT_OFF);
#endif

// PIC�̃N���b�N��Hz�Őݒ�(wait�Ƃ��Ŏg����)
#define _XTAL_FREQ 8000000UL 

//---------------
// �萔��defines
//---------------
#define NIL 0

// EEPROM�֘A
#define EEPROM_ADR_CONFIG (0x0)
#define EEPROM_ADR_MAX_BRIGHTNESS (0x1)

#define CONFIG_ALARM_ON (1<<7)
#define CONFIG_AUTO_OFF (1<<6)
#define CONFIG_AUTO_BRIGHTNESS (1<<4)

// I2C�֘A
#define I2C_SLAVE_ADDRESS_RTC 0xa2
#define I2C_READ 1
#define I2C_WRITE 0
// I2C�̃N���b�N����
// �`�����[�g = (Fosc/4)/(�ݒ�l-1)
// ���ݒ�l=((Fosc(8Mhz)/4)/�`�����[�g(100kbps))-1 = (8,000,000/4)/(100,000)-1 = 19
#define I2C_CLOCK_DEVIDE_VAL 19

// AD�֘A
#define MAX_AD_CHANNELS 13

// AD�ϊ�(resolution256) �� LM35(10mV/�x)�� �A���v(7.8�{) �� ���x �� 100�{�l�̕ϊ�
// adval * (5 / 256)�@* 1000 / 10  / 7.8 * 100	= 25.0400641 ���������100�{���ꂽ���x��������
#define ADVAL_2_TMPARATURE 25

// �|�[�g�X�e�[�g
// PORTA 0,1�̓A�i���O���́i���͂ɂ���K�v������j
// PORTB �{�^�����͂��ЂƂ�
// PORTC 3�E4��I2C��SCL/SDA�i���͂ɂ���K�v������j�A0�A1�̓{�^������
// PORTD �͌���
// PORTE MCLR�͓���
// AN 4-2�̓f�W�^��,0�A1�̓A�i���O
// AN 13-8�s���̓f�W�^���Ŏg�p
#define PIO_A 0b00000011
#define PIO_B 0b00100000
#define PIO_C 0b00011011
#define PIO_D 0b00000000
#define PIO_E 0b00001000
#define PIO_ANSEL  0b00000011;
#define PIO_ANSELH 0b00000000;

// �|�[�g���蓖��
#define PIN_LINDECODER_0 RA4
#define PIN_LINDECODER_1 RA5
#define PIN_LINDECODER_2 RA6
#define PIN_LINDECODER_3 RA7
#define PIN_DIGIT_0 RB4
#define PIN_DIGIT_1 RB3
#define PIN_DIGIT_2 RB1
#define PIN_DIGIT_3 RB0
#define PIN_DIGIT_4 RC6
#define PIN_DIGIT_5 RC5
#define PIN_COLON_L RB2
#define PIN_COLON_R RC7
#define PIN_TEMPARATURE_SENSOR RA0
#define PIN_CDS RA1
#define PIN_TRANSISTOR_0 RA2
#define PIN_TRANSISTOR_1 RA3
#define PIN_BUTTON_0 RB5
#define PIN_ICSPDAT RB6
#define PIN_ICSPCLK RB7
#define PIN_BUTTON_1 RC0
#define PIN_BUTTON_2 RC1
#define PIN_SPEAKER RC2
#define PIN_SCL RC3
#define PIN_SDA RC4
#define PIN_NMCLK RE3

//�^�C�}�[0�v���X�P�[��
//	000		  | 1 : 2		| 1 : 1
//	001		  | 1 : 4		| 1 : 2
//	010		  | 1 : 8		| 1 : 4
//	011		  | 1 : 16		| 1 : 8
//	100		  | 1 : 32		| 1 : 16
//	101		  | 1 : 64		| 1 : 32
//	110		  | 1 : 128		| 1 : 64
//	111		  | 1 : 256		| 1 : 128
#define TIMER0_PRESCALER 0b10
#define TMR0_OFFSET 125

//�^�C�}�[1�v���X�P�[��
//	11 = 1:8 Prescale Value
//	10 = 1:4 Prescale Value
//	01 = 1:2 Prescale Value
//	00 = 1:1 Prescale Value
#define TIMER1_PRESCALER (0b01)

//#define TMR1L_1SEC_OFFSET_DEFAULT_L (0x34)
//#define TMR1H_1SEC_OFFSET_DEFAULT_L (0x85)
#define TMR1L_1SEC_OFFSET_DEFAULT_DOWN (0x00)
#define TMR1H_1SEC_OFFSET_DEFAULT_DOWN (0x85)

#define TMR1L_1SEC_OFFSET_DEFAULT_UP (0x00)
#define TMR1H_1SEC_OFFSET_DEFAULT_UP (0x86)

//#define TMR1L_1SEC_OFFSET_DEFAULT (200)
//#define TMR1H_1SEC_OFFSET_DEFAULT (88)
#define TMR1_SOFT_PRESCALER (32)

// �������烉�C���f�R�[�_�p�o�͂֕ϊ�����e�[�u��
const unsigned char NUM_2_LINE[] = 
{
	0,
	1,
	2,
	3,
	4,
	6,
	5,
	7,
	8,
	9,
	10,
};

// �P�x�̉���
#define BRIGHTNESS_HIGHER_MAX (200)
#define BRIGHTNESS_LOWER_THRESHOLD_NIXIE (50)
#define BRIGHTNESS_LOWER_THRESHOLD_NEON  (180)
#define AUTO_OFF_THRESHOLD (30)
#define TRANSITION_RATE (0x3)
#define CLOCK_COLON_BLINK_OFFSET (10)
#define DYNAMIC_OFF_TIMER (0xff - 30)

//---------------
// �}�N��
//---------------
#define STATIC_ASSERT(exp) {char is_size[(exp)?1:0]; (void)is_size;}
#define LO(c) ((c)&0x0f)
#define HI(c) (((c)>>4)&0x0f)

//---------------
// �^��`
//---------------

// bool
typedef enum _e_bool
{
	FALSE,
	TRUE
}e_bool;

// �R�[���o�b�N�^�C�v
typedef e_bool (*t_callback)(unsigned char arg_);

// ��
typedef enum _e_digit
{
	DIGIT_0 = 0,
	DIGIT_1,
	DIGIT_COLON_L,
	DIGIT_2,
	DIGIT_3,
	DIGIT_COLON_R,
	DIGIT_4,
	DIGIT_5,
	DIGIT_OFF
}e_digit;

// �X�e�[�g�}�V���p�X�e�[�g
typedef struct _STATE
{
	t_callback on_button0;
	t_callback on_button1;
	t_callback on_button2;
}STATE;


//---------------
// �O���[�o���ϐ�
//---------------

// �R���t�B�O
volatile static unsigned char g_config_alarm_enable = 1;
volatile static unsigned char g_config_auto_off_enable = 1;
volatile static unsigned char g_config_auto_brightness = 1;
volatile static unsigned char g_config_max_brightness = BRIGHTNESS_HIGHER_MAX;
volatile static unsigned char g_auto_off_count = 0;

// ���݂̃R�[���o�b�N�֐�
//volatile static t_callback g_timer0_callback[3] = {NIL};
volatile static t_callback g_timer1_callback[3] = {NIL};
volatile static t_callback g_mainloop_callback[3] = {NIL};

// �\���L���̈�
volatile static unsigned char g_disp_flag = 0b11111111;
volatile static unsigned char g_disp[8] = {0, 0, DIGIT_OFF, 0, 0, DIGIT_OFF, 0, 0};
volatile static unsigned char g_disp_prev[8] = {0, 0, DIGIT_OFF, 0, 0, DIGIT_OFF, 0, 0};
volatile static unsigned char g_disp_transition[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
volatile static unsigned char g_disp_capture[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
volatile static unsigned char g_disp_colon[8] = {0, 0, 3, 0, 0, 3, 0, 0};
volatile static unsigned char g_disp_colon_prev[8] = {0, 0, 3, 0, 0, 3, 0, 0};
volatile static unsigned char g_disp_colon_capture[8] = {0, 0, 3, 0, 0, 3, 0, 0};

// ���ԋL���̈�
volatile static unsigned char g_hour	= 0x00;
volatile static unsigned char g_minute	= 0x00;
volatile static unsigned char g_second	= 0x00;
volatile static unsigned char g_year	= 0x00;
volatile static unsigned char g_month	= 0x00;
volatile static unsigned char g_date	= 0x00;
volatile static unsigned char g_week	= 0x00;
volatile static unsigned char g_alarm_hour	  = 0x12;
volatile static unsigned char g_alarm_minute  = 0x00;

// ���x�L���̈�
volatile static unsigned char g_temparature_integer = 0;
volatile static unsigned char g_temparature_fractional = 0;

// �_�Ō��ݒ�
volatile unsigned char g_blink_digits = 0b00000000;

// CdS���邳�L���̈�
volatile unsigned char g_cds_adval = 0;
volatile unsigned char g_cds_100th_place = 0;
volatile unsigned char g_cds_10th_place = 0;
volatile unsigned char g_cds_1th_place = 0;

// TargetBcdUpDown
volatile unsigned char* g_target_bcd_ptr = NIL;
volatile unsigned char g_target_max = 59;
volatile unsigned char g_target_min = 0;

// �{�^��
volatile static unsigned char g_button0_push = 0;
volatile static unsigned char g_button1_push = 0;
volatile static unsigned char g_button2_push = 0;
volatile static unsigned char g_button0_press = 0;
volatile static unsigned char g_button1_press = 0;
volatile static unsigned char g_button2_press = 0;

// �A���[��
volatile static unsigned char g_alarm_trigger = 0;
volatile static unsigned char g_alarm_beep_on = 0;

// �X�e�[�g�}�V��
static STATE* g_state_current;
static STATE g_state_clock			= {NIL, NIL, NIL};
static STATE g_state_alarm			= {NIL, NIL, NIL};
static STATE g_state_temparature	= {NIL, NIL, NIL};
static STATE g_state_config			= {NIL, NIL, NIL};
static STATE g_state_calendar		= {NIL, NIL, NIL};
static STATE g_state_dispoff		= {NIL, NIL, NIL};
static STATE g_state_set_clock		= {NIL, NIL, NIL};
static STATE g_state_set_calendar	= {NIL, NIL, NIL};
static STATE g_state_set_alarm		= {NIL, NIL, NIL};
static STATE g_state_set_config		= {NIL, NIL, NIL};

// �b�莞�v�X�V���x�����A�{����RTC����̃N���b�N�������s���ׂ�
volatile unsigned char timer1h_1sec_offset = TMR1H_1SEC_OFFSET_DEFAULT_DOWN;
volatile unsigned char timer1l_1sec_offset = TMR1L_1SEC_OFFSET_DEFAULT_DOWN;
volatile unsigned char g_next_timer1_ajustment = 0;

// �Ƃ肠�����v���g�^�C�v�錾
e_bool State_ChangeConfig(unsigned char button_no_);
e_bool State_CallbackBlink(unsigned char button_no_);


//---------------
// �ėp�֐�
// ����:wait�v�̊֐��͔�r�Ƃ��𖳎����Ă���̂ł��܂萳�m�ł͂Ȃ�
//---------------
// 100us��wait/
void Wait100us(unsigned char us100_)
{
	unsigned char count = us100_;
	while (count)
	{
		__delay_us(100);
		count--;
	}
}
// 100us��wait
void Wait10us(unsigned char us10_)
{
	unsigned char count = us10_;
	while (count)
	{
		__delay_us(10);
		count--;
	}
}

// 100us��wait
void Wait1us(unsigned char us100_)
{
	unsigned char count = us100_;
	while (count)
	{
		__delay_us(1);
		count--;
	}
}

// 100ms�P�ʂ�wait
void Wait1ms(unsigned char ms_)
{
	unsigned char count = ms_;
	while (count)
	{
		__delay_ms(1);
		count--;
	}
}

// 10ms�P�ʂ�wait
void Wait10ms(unsigned char ms_)
{
	unsigned char count = ms_;
	while (count)
	{
		__delay_ms(10);
		count--;
	}
}

// 100ms�P�ʂ�wait
void Wait100ms(unsigned char ms_)
{
	unsigned char count = ms_;
	while (count)
	{
		__delay_ms(100);
		count--;
	}
}

#if 0
	void Beep(unsigned char loop_)
	{
		unsigned char i;
		for (i=0; i<loop_; ++i)
		{
			PIN_SPEAKER = 1;
			Wait10ms(10);
			PIN_SPEAKER = 0;
			Wait10ms(10);
		}
	}
#else
	#define Beep(loop_) \
	{\
		PIN_SPEAKER = 1;\
		Wait10ms(10);\
		PIN_SPEAKER = 0;\
	}
#endif 

// EEPROM �ǂݍ���
unsigned char Eeprom_Read( unsigned char adr )
{
	EEADR = adr;
	RD = 1;				// Set read bit
	return( EEDATA );		// Return read data
}

// EEPROM��������
void Eeprom_Write( unsigned char adr, unsigned char data )
{
	EEADR = adr;
	EEDATA = data;

	WREN = 1;  			// Set write enable bit
	EECON2 = 0x55;      //���܂��Ȃ�
	EECON2 = 0xaa;      //���܂��Ȃ�

	WR = 1;  				// Set programming bit
	while( EEIF == 0 )		// Wait till write operation complete
	{
		NOP();
	}
	EEIF = 0;				// Clear EEPROM write complete bit
	WREN = 0;  			// Clear write enable bit
}

// I2C�X�^�[�g�R���f�B�V����
void I2c_Start()
{
	unsigned char timeout;
	// �X�^�[�g�R���f�B�V�������s
	SSPIF = 0;
	SEN = 1;
	
	// SEN�����Z�b�g����邩
	// SSPIF���Z�b�g���ꊄ�荞�݂��������邩��҂B
	// //SSPIF�͂ق��ɂ��v�������\��������̂ŁA�����ł�SEN���g�p����
	timeout = 0;
	while (SEN)
	//while (!SSPIF)
	{
		timeout++;
		if (timeout == 0xff)
		{
			Beep(2);
			return;
		}
	}

	// �ꉞ�N���A���Ă����Ȃ��Ɗ��荞�ݗL�������Ɋ��荞�݂ł܂���͂�
	//SSPIF = 0;
}

// I2C���s�[�g�X�^�[�g�R���f�B�V����
void I2c_RepeatStart()
{
	unsigned char timeout;
	// ���s�[�g�X�^�[�g�R���f�B�V�������s
	SSPIF = 0;
	RSEN = 1;
	
	// RSEN�����Z�b�g����邩
	// SSPIF���Z�b�g���ꊄ�荞�݂��������邩��҂B
	// //SSPIF�͂ق��ɂ��v�������\��������̂ŁA�����ł�SEN���g�p����
	timeout = 0;
	while (RSEN)
	// while (!SSPIF)
	{
		timeout++;
		if (timeout == 0xff)
		{		
			Beep(3);
			return;
		}
	}

	// �ꉞ�N���A���Ă����Ȃ��Ɗ��荞�ݗL�������Ɋ��荞�݂ł܂���͂�
	SSPIF = 0;
}

// I2C�X�g�b�v�R���f�B�V����
void I2c_Stop()
{
	unsigned char timeout;
	// �X�g�b�v�R���f�B�V�������s
	SSPIF = 0;
	PEN = 1;

	// RSEN�����Z�b�g����邩
	// SSPIF���Z�b�g���ꊄ�荞�݂��������邩��҂B
	// // SSPIF�͂ق��ɂ��v�������\��������̂ŁA�����ł�PEN���g�p����
	timeout = 0;
	while (PEN)
	//while (!SSPIF)
	{
		timeout++;
		if (timeout == 0xff)
		{
			Beep(4);
			return;
		}
	}
	
	// �ꉞ�N���A���Ă����Ȃ��Ɗ��荞�ݗL�������Ɋ��荞�݂ł܂���͂�
	//SSPIF = 0;
}

// I2C���M
e_bool I2c_Write(unsigned char data_)
{
	unsigned char timeout;
	// ���荞�݃N���A���ăf�[�^���������ނƃf�[�^���M�J�n
	SSPIF = 0;
	SSPBUF = data_;
	
	// ���M�I���҂�
	timeout = 0;
	while (!SSPIF)
	{
		timeout++;
		if (timeout == 0xff)
		{
			Beep(5);
			return FALSE;
		}
	}

	// �ꉞ�N���A���Ă����Ȃ��Ɗ��荞�ݗL�������Ɋ��荞�݂ł܂���͂�
	//SSPIF = 0;

	return TRUE;
}

// I2C��M
unsigned char I2c_Read(unsigned char ack_)
{
	unsigned char timeout;
	
	// I2C��M�J�n
	SSPIF = 0;
	RCEN = 1;

	// ��M�҂�
	timeout = 0;
	while (RCEN)
	//while (!SSPIF)
	{
		timeout++;
		if (timeout == 0xff)
		{
			Beep(6);
			return 0;
		}
	}

	// �A�N�m���b�W�V�[�P���X�J�n
	if (ack_)
	{
		// ack��0
		ACKDT = 0;
	}
	else
	{
		// nack��1
		ACKDT = 1;
	}
	ACKEN = 1;
	SSPIF = 0;
	 
	// �A�N�m���b�W�V�[�P���X�I���҂�
	// ACKEN�N���A��������
	// // SSPIF���Z�b�g���ꊄ�荞�݂��������邩��҂B	
	timeout = 0;
	while (ACKEN)
	//while (!SSPIF)
	{
		timeout++;
		if (timeout == 0xff)
		{
			Beep(7);
			return 0;
		}
	}

	// �擾�����l
	return SSPBUF;
}

// AD�R���o�[�^�̒l���擾����
unsigned char Pic_GetAd(unsigned char ch_)
{
	if (ch_ > MAX_AD_CHANNELS)
	{
		return 0xff;
	}

	// GO
	ADCON0 = (ADCON0 & 0b11000011) | (ch_ << 2);
	ADCON0 |= 0b00000010;

	// AD�ϊ��I���܂ő҂�
	while (ADCON0 & 0b00000010)
	{
		Wait10us(1);
	}
	
	// ���ʂ�ǂݍ���
	return ADRESH;
};

//---------------
// �A�v���P�[�V����
//---------------
/// ���W�X�^�����ݒ�
void InitDevice()
{
	// �����N���b�N�ݒ�
	// bit 7 Unimplemented: Read as �e0�f
	// bit 6-4 IRCF<2:0>: Internal Oscillator Frequency Select bits
	//	111 = 8 MHz
	//	110 = 4 MHz (default)
	//	101 = 2 MHz
	//	100 = 1 MHz
	//	011 = 500 kHz
	//	010 = 250 kHz
	//	001 = 125 kHz
	//	000 = 31 kHz (LFINTOSC)
	// bit 3 OSTS: Oscillator Start-up Time-out Status bit(1)
	//	1 = Device is running from the clock defined by FOSC<2:0> of the CONFIG1 register
	//	0 = Device is running from the internal oscillator (HFINTOSC or LFINTOSC)
	// bit 2 HTS: HFINTOSC Status bit (High Frequency 	 8 MHz to 125 kHz)
	//	1 = HFINTOSC is stable
	//	0 = HFINTOSC is not stable
	// bit 1 LTS: LFINTOSC Stable bit (Low Frequency 	31 kHz)
	//	1 = LFINTOSC is stable
	//	0 = LFINTOSC is not stable
	// bit 0 SCS: System Clock Select bit
	//	1 = Internal oscillator is used for system clock
	//	0 = Clock source defined by FOSC<2:0> of the CONFIG1 register
	OSCCON = 0b01110001; // ����8Mhz

	// AD�R���o�[�^�R���g���[��0
	// bit 7-6 ADCS<1:0>: A/D Conversion Clock Select bits
	//�@ 00 = FOSC/2
	//�@ 01 = FOSC/8
	//�@ 10 = FOSC/32
	// �@11 = FRC (clock derived from a dedicated internal oscillator = 500 kHz max)
	// bit 5-2 CHS<3:0>: Analog Channel Select bits
	// �@0000 = AN0
	//�@�@�@�E�E�E
	// �@1101 = AN13
	// �@1110 = CVREF
	// �@1111 = Fixed Ref (0.6V fixed voltage reference)
	// bit 1 GO/DONE: A/D Conversion Status bit
	// �@1 = A/D conversion cycle in progress. Setting this bit starts an A/D conversion cycle.
	//		This bit is automatically cleared by hardware when the A/D conversion has completed.
	// �@0 = A/D conversion completed/not in progress
	// bit 0 ADON: ADC Enable bit
	// �@1 = ADC is enabled
	// �@0 = ADC is disabled and consumes no operating current
	ADCON0 = 0b10000101; // AD0, FOSC/32, Enable

	// AD�R���o�[�^�R���g���[��1
	// bit 7 ADFM: A/D Conversion Result Format Select bit
	//	1 = Right justified
	//	0 = Left justified
	// bit 6 Unimplemented: Read as �e0�f
	// bit 5 VCFG1: Voltage Reference bit
	//	1 = VREF- pin
	//	0 = VSS
	// bit 4 VCFG0: Voltage Reference bit
	//	1 = VREF+ pin
	//	0 = VDD
	// bit 3-0 Unimplemented: Read as �e0�f
	ADCON1 = 0b00000000; // left justified, Vref=Vdd,Vss
		
	// SSP�R���g���[��0
	// R/W-0 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0
	// WCOL	 SSPOV SSPEN CKP   SSPM3 SSPM2 SSPM1 SSPM0
	// 
	// bit 7 WCOL: Write Collision Detect bit
	//	Master mode:
	//	 1 = A write to the SSPBUF register was attempted while the I2C conditions were not valid for a transmission to be started
	//	 0 = No collision
	//	Slave mode:
	//	 1 = The SSPBUF register is written while it is still transmitting the previous word (must be cleared in software)
	//	 0 = No collision
	// bit 6 SSPOV: Receive Overflow Indicator bit
	//	In SPI mode:
	//	 1 = A new byte is received while the SSPBUF register is still holding the previous data. In case of overflow, the data in SSPSR
	//		 is lost. Overflow can only occur in Slave mode. In Slave mode, the user must read the SSPBUF, even if only transmitting
	//		 data, to avoid setting overflow. In Master mode, the overflow bit is not set since each new reception (and transmission) is
	//		 initiated by writing to the SSPBUF register (must be cleared in software).
	//	 0 = No overflow
	//	In I2 C mode:
	//	 1 = A byte is received while the SSPBUF register is still holding the previous byte. SSPOV is a �gdon�ft care�h in Transmit
	//		 mode (must be cleared in software).
	//	 0 = No overflow
	// bit 5 SSPEN: Synchronous Serial Port Enable bit
	//	In both modes, when enabled, these pins must be properly configured as input or output
	//	In SPI mode:
	//	 1 = Enables serial port and configures SCK, SDO, SDI and SS as the source of the serial port pins
	//	 0 = Disables serial port and configures these pins as I/O port pins
	//	In I2 C mode:
	//	 1 = Enables the serial port and configures the SDA and SCL pins as the source of the serial port pins
	//	 0 = Disables serial port and configures these pins as I/O port pins
	// bit 4 CKP: Clock Polarity Select bit
	//	In SPI mode:
	//	 1 = Idle state for clock is a high level
	//	 0 = Idle state for clock is a low level
	//	In I2 C Slave mode:
	//	 SCK release control
	//	 1 = Enable clock
	//	 0 = Holds clock low (clock stretch). (Used to ensure data setup time.)
	//	In I2 C Master mode:
	//	 Unused in this mode
	// bit 3-0 SSPM<3:0>: Synchronous Serial Port Mode Select bits
	//	0000 = SPI Master mode, clock = FOSC/4
	//	0001 = SPI Master mode, clock = FOSC/16
	//	0010 = SPI Master mode, clock = FOSC/64
	//	0011 = SPI Master mode, clock = TMR2 output/2
	//	0100 = SPI Slave mode, clock = SCK pin, SS pin control enabled
	//	0101 = SPI Slave mode, clock = SCK pin, SS pin control disabled, SS can be used as I/O pin
	//	0110 = I2C Slave mode, 7-bit address
	//	0111 = I2C Slave mode, 10-bit address
	//	1000 = I2C Master mode, clock = FOSC / (4 * (SSPADD+1))
	//	1001 = Load Mask function
	//	1010 = Reserved
	//	1011 = I2C firmware controlled Master mode (Slave idle)
	//	1100 = Reserved
	//	1101 = Reserved
	//	1110 = I2C Slave mode, 7-bit address with Start and Stop bit interrupts enabled
	//	1111 = I2C Slave mode, 10-bit address with Start and Stop bit interrupts enabled
	SSPCON = 0b00101000; // SSP�|�[�g�Ƃ��ė��p�AI2C�}�X�^�[�Ƃ��ė��p

	// SSP�R���g���[��2
	// R/W-0 R-0	 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0
	// GCEN	 ACKSTAT ACKDT ACKEN RCEN  PEN	 RSEN  SEN
	//
	// bit 7 GCEN: General Call Enable bit (in I2C Slave mode only)
	//	1 = Enable interrupt when a general call address (0000h) is received in the SSPSR
	//	0 = General call address disabled
	// bit 6 ACKSTAT: Acknowledge Status bit (in I2C Master mode only)
	//	In Master Transmit mode:
	//	 1 = Acknowledge was not received from slave
	//	 0 = Acknowledge was received from slave
	// bit 5 ACKDT: Acknowledge Data bit (in I2C Master mode only)
	//	In Master Receive mode:
	//	Value transmitted when the user initiates an Acknowledge sequence at the end of a receive
	//	 1 = Not Acknowledge
	//	 0 = Acknowledge
	// bit 4 ACKEN: Acknowledge Sequence Enable bit (in I2C Master mode only)
	//	In Master Receive mode:
	//	 1 = Initiate Acknowledge sequence on SDA and SCL pins, and transmit ACKDT data bit.
	// Automatically cleared by hardware.
	//	0 = Acknowledge sequence idle
	// bit 3 RCEN: Receive Enable bit (in I2C Master mode only)
	//	1 = Enables Receive mode for I2C
	//	0 = Receive idle
	// bit 2 PEN: Stop Condition Enable bit (in I2C Master mode only)
	//	SCK Release Control:
	//	 1 = Initiate Stop condition on SDA and SCL pins. Automatically cleared by hardware.
	//	 0 = Stop condition Idle
	// bit 1 RSEN: Repeated Start Condition Enabled bit (in I2C Master mode only)
	//	1 = Initiate Repeated Start condition on SDA and SCL pins. Automatically cleared by hardware.
	//	0 = Repeated Start condition Idle
	// bit 0 SEN: Start Condition Enabled bit (in I2C Master mode only)
	//	In Master mode:
	//	 1 = Initiate Start condition on SDA and SCL pins. Automatically cleared by hardware.
	//	 0 = Start condition Idle
	//	In Slave mode:
	//	 1 = Clock stretching is enabled forboth slave transmit and slave receive (stretch enabled)
	//	 0 = Clock stretching is disabled
	SSPCON2 = 0b00000000; // �Ƃ肠����Idle��Ԃ�ۂ�

	// SSP�X�e�[�^�X
	// R/W-0 R/W-0 R-0 R-0 R-0 R-0 R-0 R-0
	// SMP	 CKE   D/A P   S   R/W UA  BF
	//
	// bit 7 SMP: Sample bit
	//	SPI Master mode:
	//	 1 = Input data sampled at end of data output time
	//	 0 = Input data sampled at middle of data output time
	//	SPI Slave mode:
	//	 SMP must be cleared when SPI is used in Slave mode
	//	In I2 C Master or Slave mode:
	//	 1 = Slew rate control disabled for standard speed mode (100 kHz and 1 MHz)
	//	 0 = Slew rate control enabled for high speed mode (400 kHz)
	// bit 6 CKE: SPI Clock Edge Select bit
	//	CKP = 0:
	//	 1 = Data transmitted on rising edge of SCK
	//	 0 = Data transmitted on falling edge of SCK
	//	CKP = 1:
	//	 1 = Data transmitted on falling edge of SCK
	//	 0 = Data transmitted on rising edge of SCK
	// bit 5 D/A: Data/Address bit (I2C mode only)
	//	1 = Indicates that the last byte received or transmitted was data
	//	0 = Indicates that the last byte received or transmitted was address
	// bit 4 P: Stop bit
	//	(I2C mode only. This bit is cleared when the MSSP module is disabled, SSPEN is cleared.)
	//	1 = Indicates that a Stop bit has been detected last (this bit is �e0�f on Reset)
	//	0 = Stop bit was not detected last
	// bit 3 S: Start bit
	//	(I2C mode only. This bit is cleared when the MSSP module is disabled, SSPEN is cleared.)
	//	1 = Indicates that a Start bit has been detected last (this bit is �e0�f on Reset)
	//	0 = Start bit was not detected last
	// bit 2 R/W: Read/Write bit information (I2C mode only)
	//	This bit holds the R/W bit information following the last address match. This bit is only valid from the address match to
	//	the next Start bit, Stop bit, or not ACK bit.
	//	In I2 C Slave mode:
	//	 1 = Read
	//	 0 = Write
	//	In I2 C Master mode:
	//	 1 = Transmit is in progress
	//	 0 = Transmit is not in progress
	//	OR-ing this bit with SEN, RSEN, PEN, RCEN, or ACKEN will indicate if the MSSP is in Idle mode.
	// bit 1 UA: Update Address bit (10-bit I2C mode only)
	//	1 = Indicates that the user needs to update the address in the SSPADD register
	//	0 = Address does not need to be updated
	// bit 0 BF: Buffer Full Status bit
	//	Receive (SPI and I2 C modes):
	//	 1 = Receive complete, SSPBUF is full
	//	 0 = Receive not complete, SSPBUF is empty
	//	Transmit (I2 C mode only):
	//	 1 = Data transmit in progress (does not include the ACK and Stop bits), SSPBUF is full
	//	 0 = Data transmit complete (does not include the ACK and Stop bits), SSPBUF is empty
	SSPSTAT = 0b10000000; 

	// SSP�}�X�N
	// bit 7-1 MSK<7:1>: Mask bits
	//	1 = The received address bit n is compared to SSPADD<n> to detect I2C address match
	//	0 = The received address bit n is not used to detect I2C address match
	// bit 0 MSK<0>: Mask bit for I2C Slave mode, 10-bit Address(2)
	//	I2C Slave mode, 10-bit Address (SSPM<3:0> = 0111):
	//	 1 = The received address bit 0 is compared to SSPADD<0> to detect I2C address match
	//	 0 = The received address bit 0 is not used to detect I2C address match
	//
	// Note 1: When SSPCON bits SSPM<3:0> = 1001, any reads or writes to the SSPADD SFR address are accessed
	// through the SSPMSK register.
	// 2: In all other SSP modes, this bit has no effect.
	SSPMSK = 0b00000000;
		
	// ���̑�SSP�֘A
	SSPADD = I2C_CLOCK_DEVIDE_VAL; // I2C�}�X�^�[�ł̓N���b�N�����̒l�ƂȂ�

	SSPBUF = 0b00000000; // �N���A

	// �|�[�g���o�͐ݒ�
	TRISA  = PIO_A;
	TRISB  = PIO_B;
	TRISC  = PIO_C;
	TRISE  = PIO_E;
	PORTA  = 0b00000000; // PORTA �N���A
	PORTB  = 0b00000000; // PORTB �N���A   
	PORTC  = 0b00000000; // PORTC �N���A 
	PORTE  = 0b00001000; // PORTE �N���A 
	ANSEL  = PIO_ANSEL;	 // AN4-2�̓f�W�^��,AN0-1�̓A�i���O
	ANSELH = PIO_ANSELH; // AN13-8�s���̓f�W�^���Ŏg�p
	//ULPWUE = 0; //�������������l�O�̫�H�Ƃ悭�킩��Ȃ�����AD�p�ɐݒ肵�Ȃ��Ƃ��߁H
	//CM1CON0 = 1;

	// �I�v�V�����@�\�ݒ�
	// bit 7 RBPU: PORTB Pull-up Enable bit
	//	1 = PORTB pull-ups are disabled
	//	0 = PORTB pull-ups are enabled by individual PORT latch values
	// bit 6 INTEDG: Interrupt Edge Select bit
	//	1 = Interrupt on rising edge of INT pin
	//	0 = Interrupt on falling edge of INT pin
	// bit 5 T0CS: TMR0 Clock Source Select bit
	//	1 = Transition on T0CKI pin
	//	0 = Internal instruction cycle clock (FOSC/4)
	// bit 4 T0SE: TMR0 Source Edge Select bit
	//	1 = Increment on high-to-low transition on T0CKI pin
	//	0 = Increment on low-to-high transition on T0CKI pin
	// bit 3 PSA: Prescaler Assignment bit
	//	1 = Prescaler is assigned to the WDT
	//	0 = Prescaler is assigned to the Timer0 module
	// bit 2-0 PS<2:0>: Prescaler Rate Select bits
	//	Bit Value | Timer0 Rate | WDT Rate
	//	000		  | 1 : 2		| 1 : 1
	//	001		  | 1 : 4		| 1 : 2
	//	010		  | 1 : 8		| 1 : 4
	//	011		  | 1 : 16		| 1 : 8
	//	100		  | 1 : 32		| 1 : 16
	//	101		  | 1 : 64		| 1 : 32
	//	110		  | 1 : 128		| 1 : 64
	//	111		  | 1 : 256		| 1 : 128
	TMR0 = TMR0_OFFSET; // �Ƃ肠�����^�C�}�[���N���A���Ă���
	OPTION_REG = 0b10000000 | TIMER0_PRESCALER; // �^�C�}�[0��FOSC/4�A�������2�����ݒ�
	
	// �^�C�}�[1�ݒ�
	// bit 7 T1GINV: Timer1 Gate Invert bit(1)
	//	1 = Timer1 gate is active-high (Timer1 counts when gate is high)
	//	0 = Timer1 gate is active-low (Timer1 counts when gate is low)
	// bit 6 TMR1GE: Timer1 Gate Enable bit(2)
	//	If TMR1ON = 0:
	//	 This bit is ignored
	//	If TMR1ON = 1:
	//	 1 = Timer1 counting is controlled by the Timer1 Gate function
	//	 0 = Timer1 is always counting
	// bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
	//	11 = 1:8 Prescale Value
	//	10 = 1:4 Prescale Value
	//	01 = 1:2 Prescale Value
	//	00 = 1:1 Prescale Value
	// bit 3 T1OSCEN: LP Oscillator Enable Control bit
	//	1 = LP oscillator is enabled for Timer1 clock
	//	0 = LP oscillator is off
	// bit 2 T1SYNC: Timer1 External Clock Input Synchronization Control bit
	//	TMR1CS = 1:
	//	�@1 = Do not synchronize external clock input
	//	�@0 = Synchronize external clock input
	// �@TMR1CS = 0:
	// �@�@This bit is ignored. Timer1 uses the internal clock
	// bit 1 TMR1CS: Timer1 Clock Source Select bit
	//	1 = External clock from T1CKI pin (on the rising edge)
	//	0 = Internal clock (FOSC/4)
	// bit 0 TMR1ON: Timer1 On bit
	//	1 = Enables Timer1
	//	0 = Stops Timer1
	T1CON = 0b00000001 | (TIMER1_PRESCALER << 4); // �Q�[�g�͎g�p���Ȃ��A�����N���b�NFOSC/4�A8�����A�N��
	TMR1L = 0;
	TMR1H = 0;

	// �y���t�F�����C���^���v�g�C�l�[�u���ݒ�
	// bit 7 Unimplemented: Read as �e0�f
	// bit 6 ADIE: A/D Converter (ADC) Interrupt Enable bit
	//	1 = Enables the ADC interrupt
	//	0 = Disables the ADC interrupt
	// bit 5 RCIE: EUSART Receive Interrupt Enable bit
	//	1 = Enables the EUSART receive interrupt
	//	0 = Disables the EUSART receive interrupt
	// bit 4 TXIE: EUSART Transmit Interrupt Enable bit
	//	1 = Enables the EUSART transmit interrupt
	//	0 = Disables the EUSART transmit interrupt
	// bit 3 SSPIE: Master Synchronous Serial Port (MSSP) Interrupt Enable bit
	//	1 = Enables the MSSP interrupt
	//	0 = Disables the MSSP interrupt
	// bit 2 CCP1IE: CCP1 Interrupt Enable bit
	//	1 = Enables the CCP1 interrupt
	//	0 = Disables the CCP1 interrupt
	// bit 1 TMR2IE: Timer2 to PR2 Match Interrupt Enable bit
	//	1 = Enables the Timer2 to PR2 match interrupt
	//	0 = Disables the Timer2 to PR2 match interrupt
	// bit 0 TMR1IE: Timer1 Overflow Interrupt Enable bit
	//	1 = Enables the Timer1 overflow interrupt
	//	0 = Disables the Timer1 overflow interrupt
	PIE1 = 0b00000001; // SSP�̓|�[�����O����̂Ŋ��荞�݂͖����A�^�C�}�[�P�L��

	// �y���t�F�����C���^���v�g���N�G�X�g
	// bit 7 Unimplemented: Read as �e0�f
	// bit 6 ADIF: A/D Converter Interrupt Flag bit
	//	1 = A/D conversion complete (must be cleared in software)
	//	0 = A/D conversion has not completed or has not been started
	// bit 5 RCIF: EUSART Receive Interrupt Flag bit
	//	1 = The EUSART receive buffer is full (cleared by reading RCREG)
	//	0 = The EUSART receive buffer is not full
	// bit 4 TXIF: EUSART Transmit Interrupt Flag bit
	//	1 = The EUSART transmit buffer is empty (cleared by writing to TXREG)
	//	0 = The EUSART transmit buffer is full
	// bit 3 SSPIF: Master Synchronous Serial Port (MSSP) Interrupt Flag bit
	//	1 = The MSSP interrupt condition has occurred, and must be cleared in software before returning from the
	//		Interrupt Service Routine. The conditions that will set this bit are:
	//	 SPI
	//	  A transmission/reception has taken place
	//	 I2 C Slave/Master
	//	  A transmission/reception has taken place
	//	 I2 C Master
	//	  The initiated Start condition was completed by the MSSP module
	//	  The initiated Stop condition was completed by the MSSP module
	//	  The initiated restart condition was completed by the MSSP module
	//	  The initiated Acknowledge condition was completed by the MSSP module
	//	  A Start condition occurred while the MSSP module was idle (Multi-master system)
	//	  A Stop condition occurred while the MSSP module was idle (Multi-master system)
	//	0 = No MSSP interrupt condition has occurred
	// bit 2 CCP1IF: CCP1 Interrupt Flag bit
	//	Capture mode:
	//	 1 = A TMR1 register capture occurred (must be cleared in software)
	//	 0 = No TMR1 register capture occurred
	//	Compare mode:
	//	1 = A TMR1 register compare match occurred (must be cleared in software)
	//	 0 = No TMR1 register compare match occurred
	//	PWM mode:
	//	 Unused in this mode
	// bit 1 TMR2IF: Timer2 to PR2 Interrupt Flag bit
	//	1 = A Timer2 to PR2 match occurred (must be cleared in software)
	//	0 = No Timer2 to PR2 match occurred
	// bit 0 TMR1IF: Timer1 Overflow Interrupt Flag bit
	//	1 = The TMR1 register overflowed (must be cleared in software)
	//	0 = The TMR1 register did not overflow
	PIR1 &= 0b11110110; // TMR1���荞�݃N���A,SSPIF�N���A

	// ���荞�ݐݒ�
	// bit 7 GIE: Global Interrupt Enable bit
	//	1 = Enables all unmasked interrupts
	//	0 = Disables all interrupts
	// bit 6 PEIE: Peripheral Interrupt Enable bit
	//	1 = Enables all unmasked peripheral interrupts
	//	0 = Disables all peripheral interrupts
	// bit 5 T0IE: Timer0 Overflow Interrupt Enable bit
	//	1 = Enables the Timer0 interrupt
	//	0 = Disables the Timer0 interrupt
	// bit 4 INTE: INT External Interrupt Enable bit
	//	1 = Enables the INT external interrupt
	//	0 = Disables the INT external interrupt
	// bit 3 RBIE: PORTB Change Interrupt Enable bit(1)
	//	1 = Enables the PORTB change interrupt
	//	0 = Disables the PORTB change interrupt
	// bit 2 T0IF: Timer0 Overflow Interrupt Flag bit(2)
	//	1 = TMR0 register has overflowed (must be cleared in software)
	//	0 = TMR0 register did not overflow
	// bit 1 INTF: INT External Interrupt Flag bit
	//	1 = The INT external interrupt occurred (must be cleared in software)
	//	0 = The INT external interrupt did not occur
	// bit 0 RBIF: PORTB Change Interrupt Flag bit
	//	1 = When at least one of the PORTB general purpose I/O pins changed state (must be cleared in
	//		software)
	//	0 = None of the PORTB general purpose I/O pins have changed state
	INTCON = 0b11100000; // �O���[�o���E�y���t�F�������荞�ݗL�����A�^�C�}�[0�̃I�[�o�[�t���[���荞�݂�L����

}

//--------------
/// �t���[�����[�N
//--------------
/// �R�[���o�b�N�����`�F�C���̎��s
//void DoCallbackChain(t_callback_chain* root_)
//{
//	t_callback_chain* callback = root_;
//	while(callback != NIL)
//	{
//		(*(callback->cur))();
//		callback = callback->next;
//	}
//}
//void DoCallbackChain(volatile t_callback root_[])

#define DoCallbackChain(root_) \
{\
	unsigned char i = 0;\
	while(root_[i] != NIL)\
	{\
		if ((*(root_[i]))(0) == FALSE)\
		{\
			unsigned char j;\
			for (j= i; j<3; ++j)\
			{\
				root_[j] = root_[j+1];\
			}\
		}\
		else\
		{\
			break;\
		}\
	}\
}
	
//--------------
/// �j�L�V�[�ǐ���
/// 
/// Nixie_ChangeDigit�ŕ\����(�\���Ȃ�������)��I��
/// Nixie_ChangeNumber�ŕ\�����鐔����I���i�������\������2,5�̏ꍇ�̓R�����Ȃ̂ŕ\���ɉe���͂��Ȃ��j
/// Nixie_ChangeColon�ŃR����(2,5���̏ꍇ)����сA�h�b�g(2,5���ȊO�̏ꍇ)�𐧌�
//--------------

/// ���݌���ύX����
void Nixie_ChangeDigit(e_digit digit_)
{
	// �\����ON
	switch (digit_)
	{
		case DIGIT_OFF:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_0:
			PIN_DIGIT_0 = 1;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_1:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 1;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_2:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 1;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_3:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 1;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_4:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 1;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_5:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 1;
			PIN_COLON_L = 0;
			PIN_COLON_R = 0;
			break;
		case DIGIT_COLON_L:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 1;
			PIN_COLON_R = 0;
			break;
		case DIGIT_COLON_R:
			PIN_DIGIT_0 = 0;
			PIN_DIGIT_1 = 0;
			PIN_DIGIT_2 = 0;
			PIN_DIGIT_3 = 0;
			PIN_DIGIT_4 = 0;
			PIN_DIGIT_5 = 0;
			PIN_COLON_L = 0;
			PIN_COLON_R = 1;
			break;
		default:
			;
	}
}

/// �����I��p���C���o�͂�ύX
void Nixie_ChangeNumber(unsigned char number_)
{
	if (number_ > sizeof(NUM_2_LINE)/sizeof(NUM_2_LINE[0]))
	{
		return;
	}
	
	{
		unsigned char line_hex;
		line_hex = NUM_2_LINE[number_];
		
		// ������ύX
		PIN_LINDECODER_0 = ((line_hex >> 0) & 1);
		PIN_LINDECODER_1 = ((line_hex >> 1) & 1);
		PIN_LINDECODER_2 = ((line_hex >> 2) & 1);
		PIN_LINDECODER_3 = ((line_hex >> 3) & 1);
	}
}

/// �R�����\���p�̃g�����W�X�^ON/OFF
void Nixie_ChangeColon( unsigned char colon_flag_)
{
	PIN_TRANSISTOR_0 = colon_flag_ & 0x1;
	PIN_TRANSISTOR_1 = (colon_flag_>>1) & 0x1;
}

//--------------
/// RTC����
///
/// ���v���Z�b�g����菇
/// 1) Rtc_UpdatePicTimerByRtc();�@// Rtc��̐��m�Ȏ��Ԃ��擾
/// 2) // �O���[�o���ϐ��ύX
/// 3) Rtc_UpdateRtcByPicTimer(); // Rtc�ɍX�V�������ԏ��𑗐M
//--------------


// �O���[�o���ϐ��ŊǗ����Ă���PIC��̎��ԏ���Rtc�ɔ��f
void Rtc_UpdateRtcByPicTimer()
{
	// stop bit��0
	// RTC��������
	// stop bit��1
	// 
	// ���ׂ��͈͂���Stop�r�b�g��ύX�����L��������邩�킩��Ȃ������̂ŁA
	// RTC�������݂�StopCondition�𑗐M���Ă���ēx�ʐM�����čs�����Ƃɂ����B
 
	// stop
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	I2c_Write(0x00); // config�A�h���X
	I2c_Write(0x20); // STOP=1
	I2c_Write(0x00); // 
	I2c_Stop();
	
	// write
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	I2c_Write(0x02);
	I2c_Write(g_second); // �b�̒l 0-59
	I2c_Write(g_minute); // ���̒l 0-59
	I2c_Write(g_hour);	 // ���̒l 0-23
	//I2c_Write(g_date);	 // ���̒l 1-31
	//I2c_Write(g_week);	 // �j�̒l �����ΐ��؋��y 0123456
	//I2c_Write(0x80|g_month);  // ���̒l (C:MSB)1-12	  C��1�̂Ƃ�21���I
	//I2c_Write(g_year);	 // �N�̒l 00-99
	//I2c_Write(0x80); // AE=1
	//I2c_Write(0x80); // AE=1
	//I2c_Write(0x80); // AE=1
	//I2c_Write(0x80); // AE=1
	I2c_Stop();

	// STOP=0 �Ŏ��v���J�n����
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	I2c_Write(0x00); // config�A�h���X
	I2c_Write(0x00); // STOP=0
	I2c_Stop();
}

// RTC�̃N���b�N�l��ǂݏo��
void Rtc_UpdatePicTimerByRtc()
{
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE);
	I2c_Write(0x02); // �b�̃A�h���X
	I2c_RepeatStart();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_READ);
	g_second = I2c_Read(1); // �b�̒l 0-59
	g_minute = I2c_Read(1); // ���̒l 0-59
	g_hour	 = I2c_Read(1); // ���̒l 0-23
	g_date	 = I2c_Read(1); // ���̒l 1-31
	g_week	 = I2c_Read(1); // �j�̒l �����ΐ��؋��y 0123456
	g_month	 = I2c_Read(1); // ���̒l (C:MSB)1-12	C��1�̂Ƃ�21���I
	g_year	 = I2c_Read(1); // �N�̒l 00-99
	g_alarm_minute = I2c_Read(1); // �A���[��
	g_alarm_hour   = I2c_Read(0); // �A���[��
	I2c_Stop();

	// �L���r�b�g�̎��o��
	g_second  &= 0x7f;
	g_minute  &= 0x7f;
	g_hour	  &= 0x3f;
	g_date	  &= 0x3f;
	g_week	  &= 0x07;
	g_month	  &= 0x1f;
	// g_year	  &= 0xff; �N�͑S�r�b�g�L���Ȃ̂Ń}�X�N���Ȃ�
	// g_century = g_month & 0x80 ? 21 : 20;
	g_alarm_minute &= 0x7f;
	g_alarm_hour &= 0x3f;
}

/// �������A�N�����Ɉ�x�����R�[������Rtc�̃R���t�B�O��ݒ肷��B
/// Rtc�̗����オ���700ms�K�v�Ȃ̂œ�����wait����
void Rtc_Init()
{	  
	unsigned char vl_bit;

	// �����オ���1000ms�҂K�v������
	Wait10ms(100);

	// VL�r�b�g���`�F�b�N
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE);
	I2c_Write(0x02); // �b�̃A�h���X
	I2c_RepeatStart();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_READ);
	vl_bit = (I2c_Read(0) >> 7) & 1; // �o�b�N�A�b�v�ُ팟�oVL
	I2c_Stop();

	// �ُ킪����Ώ���������
	if (vl_bit)
	{
		// �G���[�ʒm�̌�A������
		Beep(1);

		// ������
		I2c_Start();
		I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE);	 // �������݃��[�h
		I2c_Write(0x00); // control0�̃A�h���X
		I2c_Write(0x0); // test=0, STOP=0
		I2c_Write(0x0); // AIE=TIE=0
		I2c_Stop();
		Rtc_UpdateRtcByPicTimer();
	}
}

// �b��
void Rtc_UpdateRtcCalendar()
{
	// stop bit��0
	// RTC��������
	// stop bit��1
	// 
	// ���ׂ��͈͂���Stop�r�b�g��ύX�����L��������邩�킩��Ȃ������̂ŁA
	// RTC�������݂�StopCondition�𑗐M���Ă���ēx�ʐM�����čs�����Ƃɂ����B
 
	// stop
	//I2c_Start();
	//I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	//I2c_Write(0x00); // config�A�h���X
	//I2c_Write(0x20); // STOP=1
	//I2c_Write(0x00);
	//I2c_Stop();
	
	// write
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	I2c_Write(0x05);
	I2c_Write(g_date);	 // ���̒l 1-31
	I2c_Write(g_week);	 // �j�̒l �����ΐ��؋��y 0123456
	I2c_Write(0x80|g_month);  // ���̒l (C:MSB)1-12	  C��1�̂Ƃ�21���I
	I2c_Write(g_year);	 // �N�̒l 00-99
	I2c_Stop();

	// STOP=0 �Ŏ��v���J�n����
	//I2c_Start();
	//I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	//I2c_Write(0x00); // config�A�h���X
	//I2c_Write(0x00); // STOP=0
	//I2c_Stop();

}

// �b��
Rtc_UpdateRtcAlarm()
{
	// stop bit��0
	// RTC��������
	// stop bit��1
	// 
	// ���ׂ��͈͂���Stop�r�b�g��ύX�����L��������邩�킩��Ȃ������̂ŁA
	// RTC�������݂�StopCondition�𑗐M���Ă���ēx�ʐM�����čs�����Ƃɂ����B
 
	// stop
	//I2c_Start();
	//I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	//I2c_Write(0x00); // config�A�h���X
	//I2c_Write(0x20); // STOP=1
	//I2c_Write(0x00);
	//I2c_Stop();
	
	// write
	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	I2c_Write(0x09);
	I2c_Write(0x80 | g_alarm_minute); // �A���[���͋L�������邾���A�ŏ�ʃr�b�g���O�ɂ���΃A���[���M�����o�Ȃ��ݒ肯�ǂǂ����ɂ���g��Ȃ��̂œK��
	I2c_Write(0x80 | g_alarm_hour);   // �A���[���͋L�������邾��
	I2c_Write(0x80); // AE=1
	I2c_Write(0x80); // AE=1
	I2c_Stop();

	// STOP=0 �Ŏ��v���J�n����
	//I2c_Start();
	//I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE); // �������݃��[�h
	//I2c_Write(0x00); // config�A�h���X
	//I2c_Write(0x00); // STOP=0
	//I2c_Stop();
}

// �b��
// RTC�̃N���b�N�l��ǂݏo��
unsigned char Rtc_GetSecond()
{
	unsigned char second;

	I2c_Start();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_WRITE);
	I2c_Write(0x02); // �b�̃A�h���X
	I2c_RepeatStart();
	I2c_Write(I2C_SLAVE_ADDRESS_RTC|I2C_READ);
	second = I2c_Read(0); // �b�̒l 0-59
	I2c_Stop();

	// �L���r�b�g�̎��o��
	return second & 0x7f;
}

//--------------
// �֗��֐�
//--------------

//
void ClearColon()
{
	g_disp_colon[0] = 0;
	g_disp_colon[1] = 0;
	//g_disp_colon[2] = 0;
	g_disp_colon[3] = 0;
	g_disp_colon[4] = 0;
	//g_disp_colon[5] = 0;
	g_disp_colon[6] = 0;
	g_disp_colon[7] = 0;
}


//--------------
// �\������
//--------------
e_bool Disp_CallbackOff()
{
	Nixie_ChangeDigit(DIGIT_OFF);
	return FALSE;
}

e_bool Disp_CallbackNormal()
{
	static unsigned char digit= 0;
	static unsigned char state = 0;
	unsigned char brightness_threshold;
	unsigned char brightness_max;

	// �_������
	switch (state)
	{
		case 0:
			// �������
			Nixie_ChangeDigit(DIGIT_OFF);
		
			// ���̕\����
			++digit;
			if (digit >= DIGIT_OFF)
				digit = DIGIT_0;
	
			// �g�����W�V�����̒���
			if (0xff-TRANSITION_RATE > g_disp_transition[digit])
				g_disp_transition[digit] += TRANSITION_RATE;
			else
				g_disp_transition[digit] = 0xff;

			// ����R�[���o�b�N�܂ł̎��Ԃ𒲐�
			TMR0 = g_cds_adval > 32 ? g_cds_adval - 32 : g_cds_adval;
			// TMR0 = DYNAMIC_OFF_TIMER;

			// ���X�e�[�g
			state = 1;

			break;
		case 1:
			// �h���h�̐�����_��
			Nixie_ChangeNumber(g_disp[digit]);
			Nixie_ChangeColon(g_disp_colon[digit]);

			if (g_disp_flag & 1<<digit)
				Nixie_ChangeDigit(digit);
			else
				Nixie_ChangeDigit(DIGIT_OFF);
				

			// ���邳�̉��������肷��
			if ((digit == 2)
			||  (digit == 5))
			{
				brightness_threshold = BRIGHTNESS_LOWER_THRESHOLD_NEON;

				// todo �R�����̏ꍇ�͂Ƃ肠�����_�ł��邾���B
				// TMR0 = 0x0;
				state = 0;
				break;
			}
			else
			{
				brightness_threshold = BRIGHTNESS_LOWER_THRESHOLD_NIXIE;
			}
			if (brightness_threshold > g_cds_adval)
			{
				brightness_max = brightness_threshold;
			}
			else
			{
				brightness_max = g_cds_adval;
			}


			// ���邳�����肷��
			// ����̏ꍇ�̓g�����W�V�����I�����Ă���
			if (brightness_max > g_disp_transition[digit])
			{
				TMR0 = 0xff - g_disp_transition[digit];
				state = 2;
			}
			else
			{
				TMR0 = 0xff - brightness_max;
				g_disp_transition[digit] == 0xff;
				state = 0;
			}

			break;
		case 2:
			// �h�ȑO�h�̐�����_��
			Nixie_ChangeNumber(g_disp_prev[digit]);
			Nixie_ChangeColon(g_disp_colon_prev[digit]);

			// ���邳�̉��������肷��
			if ((digit == 2)
			||  (digit == 5))
			{
				brightness_threshold = BRIGHTNESS_LOWER_THRESHOLD_NEON;
			}
			else
			{
				brightness_threshold = BRIGHTNESS_LOWER_THRESHOLD_NIXIE;
			}

			// ���邳�̏�������肷��
			if (brightness_threshold > g_cds_adval)
			{
				brightness_max = brightness_threshold;
			}
			else
			{
				brightness_max = g_cds_adval;
			}

			// ���邳�����肷��
			// ����̏ꍇ�̓g�����W�V�����I�����Ă���
			if (brightness_max > g_disp_transition[digit])
			{
				TMR0 = 0xff - (brightness_max - g_disp_transition[digit]);
			}

			state = 0;

			break;
		default:
			;
	}

	return TRUE;
}

// g_disp_flag �����킹�ē_�ł���
e_bool State_CallbackBlink(unsigned char button_no_)
{
	static unsigned char devide_counter = 0;

	++devide_counter;
	if (devide_counter > 8)
	{
		devide_counter = 0;
	}

	if (devide_counter < 4)
	{
		g_disp_flag |= g_blink_digits;
	}
	else
	{
		g_disp_flag = g_disp_flag & (~g_blink_digits);
	}

	return TRUE;
}

// ��b�ԃ����_���\������FALSE��Ԃ�
e_bool State_CallbackRandomDisp(unsigned char button_no_)
{
	static unsigned char counter = 0;
	
	// ������ۂ�
	unsigned char val = (unsigned char)rand() % 10;

	g_disp_flag = 0b11111111;
	g_disp[0] = val + 7 > 9 ? val + 7 - 10 : val + 7;
	g_disp[1] = val + 3 > 9 ? val + 3 - 10 : val + 3;
	g_disp[3] = val + 1 > 9 ? val + 1 - 10 : val + 1;
	g_disp[4] = val + 6 > 9 ? val + 6 - 10 : val + 6;
	g_disp[6] = val + 2 > 9 ? val + 2 - 10 : val + 2;
	g_disp[7] = val + 8 > 9 ? val + 8 - 10 : val + 8;

	g_disp_colon[2] = 0x3;
	g_disp_colon[5] = 0x3;

	++counter;
	if (counter == 30)
	{
		counter = 0;
		return FALSE;
	}

	//TMR1L = TMR1L_1SEC_OFFSET_DEFAULT;
	TMR1H = 0xa0;
	
	return TRUE;
}

// ��\�����[�h
e_bool State_CallbackDispoff(unsigned char button_no_)
{
	unsigned char i;

	g_disp_flag = 0b00000000;

	for (i=0; i<8;++i)
	{
		g_disp_colon_prev[i] = 0;
		g_disp_colon[i] = 0;
		g_disp_prev[i] = 0;
		g_disp[i] = 0;
		g_disp_transition[i] = 0x0;
	}
	return TRUE;
}

// ���̕\���ɉE�ړ�����
e_bool State_CallbackRightChange(unsigned char button_no_)
{
	const unsigned char NUM_COLON = 6;
	static unsigned char g_time = 0;
	unsigned char i;

	++g_time;
	if ((g_time == 2)
	||  (g_time == 5)
	||  (g_time == 2 + NUM_COLON)
	||  (g_time == 5 + NUM_COLON))
	{
		++g_time;
	}

	if (g_time >= NUM_COLON+8)
	{
		// done
		g_time = 0;
		for (i=0; i<8;++i)
		{
			g_disp_colon_prev[i] = 0;
			g_disp_colon[i] = 0;
			g_disp_prev[i] = 0;
		}
		return FALSE;
	}

	if (g_time < NUM_COLON)
	{
		g_disp[0] = 10;
		g_disp_colon[0] = 3;
	}
	else
	{
		g_disp[0] = g_disp_capture[7 - (g_time - NUM_COLON)];
		g_disp_colon[0] = g_disp_colon_capture[7 - (g_time - NUM_COLON)];
	}
	
	g_disp[1] = g_disp_prev[0];
	g_disp[3] = g_disp_prev[1];
	g_disp[4] = g_disp_prev[3];
	g_disp[6] = g_disp_prev[4];
	g_disp[7] = g_disp_prev[6];
	g_disp_colon[1] = g_disp_colon_prev[0];
	g_disp_colon[3] = g_disp_colon_prev[1];
	g_disp_colon[4] = g_disp_colon_prev[3];
	g_disp_colon[6] = g_disp_colon_prev[4];
	g_disp_colon[7] = g_disp_colon_prev[6];
		
	for (i = 0; i<8; ++i)
	{
		g_disp_prev[i] = g_disp[i];
		g_disp_colon_prev[i] = g_disp_colon[i];
	}
	
	return TRUE;
}

// ���̕\���ɍ��ړ�����
e_bool State_CallbackLeftChange(unsigned char button_no_)
{
	const unsigned char NUM_COLON = 6;
	static unsigned char g_time = 0;
	unsigned char i;

	++g_time;
	if ((g_time == 2)
	||  (g_time == 5)
	||  (g_time == 2 + NUM_COLON)
	||  (g_time == 5 + NUM_COLON))
	{
		++g_time;
	}

	if (g_time >= NUM_COLON+8)
	{
		// done
		g_time = 0;
		for (i=0; i<8;++i)
		{
			g_disp_colon_prev[i] = 0;
			g_disp_colon[i] = 0;
			g_disp_prev[i] = 0;
		}
		return FALSE;
	}

	if (g_time < NUM_COLON)
	{
		g_disp[7] = 10;
		g_disp_colon[7] = 3;
	}
	else
	{
		g_disp[7] = g_disp_capture[g_time - NUM_COLON];
		g_disp_colon[7] = g_disp_colon_capture[g_time - NUM_COLON];
	}
	
	g_disp[6] = g_disp_prev[7];
	g_disp[4] = g_disp_prev[6];
	g_disp[3] = g_disp_prev[4];
	g_disp[1] = g_disp_prev[3];
	g_disp[0] = g_disp_prev[1];
	g_disp_colon[6] = g_disp_colon_prev[7];
	g_disp_colon[4] = g_disp_colon_prev[6];
	g_disp_colon[3] = g_disp_colon_prev[4];
	g_disp_colon[1] = g_disp_colon_prev[3];
	g_disp_colon[0] = g_disp_colon_prev[1];
		
	for (i = 0; i<8; ++i)
	{
		g_disp_prev[i] = g_disp[i];
		g_disp_colon_prev[i] = g_disp_colon[i];
	}
	
	return TRUE;
}


e_bool State_CallbackClock(unsigned char button_no_)
{
	static unsigned char colon_on = 0;
	static unsigned char devide_counter = 0;

	g_disp_flag = 0b11111111;

	// 
	TMR1L = timer1l_1sec_offset;
	TMR1H = timer1h_1sec_offset;

	//
	++devide_counter;
	if (devide_counter >= TMR1_SOFT_PRESCALER)
	{
		devide_counter = 0;
		
		// BCD�`���Ŏ��ԍX�V
		g_disp_prev[0] = LO(g_second);
		g_disp_transition[0] = 0;
		g_second += 0x01;
		if (LO(g_second) >= 10)
		{
			g_disp_prev[1] = HI(g_second);
			g_disp_transition[1] = 0;
			g_second &= 0xf0;
			g_second += 0x10;
			if (HI(g_second) >= 6)
			{
				g_disp_prev[3] = LO(g_minute);
				g_disp_transition[3] = 0;
				g_second = 0;
				g_minute += 0x01;
				if (LO(g_minute) >= 10)
				{				
					g_disp_prev[4] = HI(g_minute);
					g_disp_transition[4] = 0;
					g_minute &= 0xf0;
					g_minute += 0x10;
					if (HI(g_minute)  >= 6)
					{
						g_disp_prev[6] = LO(g_hour);
						g_disp_transition[6] = 0;
						g_minute = 0;
						g_hour += 0x01;
						if(HI(g_hour) < 2 && LO(g_hour) >= 10)
						{
							g_disp_prev[0] = HI(g_hour);
							g_disp_transition[7] = 0;
							g_hour &= 0xf0;
							g_hour += 0x10;
						}
						else if (HI(g_hour) >= 2 && LO(g_hour) >= 4)
						{
							g_disp_prev[0] = HI(g_hour);
							g_disp_transition[7] = 0;
							g_hour = 0;
						}
					}
				}
				// 1���Ɉ�x�^�C�}�[���x�������s��
				g_next_timer1_ajustment = 1;
			}
		}
		
		// ���v�����̃f�o�b�O
		//g_disp[4] = timer1l_1sec_offset % 10;
		//g_disp[6] = (timer1l_1sec_offset / 10) % 10;
		//g_disp[7] = (timer1l_1sec_offset / 100) % 10;
		//g_disp[0] = timer1l_1sec_offset % 10;
		//g_disp[1] = (timer1l_1sec_offset / 10) % 10;
		//g_disp[3] = (timer1l_1sec_offset / 100) % 10;
		//g_disp[0] = g_auto_off_count % 10;
		//g_disp[1] =  (g_auto_off_count/ 10) % 10;
		//g_disp[3] =(g_auto_off_count / 100) % 10;
		
		// �\���f�[�^�Z�b�g
		g_disp[4] = (g_minute>>4) & 0xf;
		g_disp[6] = g_hour & 0xf;
		g_disp[7] = (g_hour>>4) & 0xf;
		g_disp[0] = g_second & 0xf ;
		g_disp[1] = (g_second>>4) & 0xf;
		g_disp[3] = g_minute & 0xf;
	}
	else if (devide_counter == CLOCK_COLON_BLINK_OFFSET)
	{
		// �R�����̓_��
		g_disp_colon_prev[2] = colon_on;
		g_disp_colon_prev[5] = colon_on;
		colon_on = colon_on == 0 ? 0b11 : 0;
		g_disp_colon[2] = colon_on;
		g_disp_colon[5] = colon_on;
		g_disp_transition[2] = 0;
		g_disp_transition[5] = 0;
	}

	return TRUE;
}

void SetConfigDigits(unsigned char blink_, unsigned char max_, unsigned char min_, volatile unsigned char* target_)
{
	g_blink_digits = blink_;
	g_target_max = max_;
	g_target_min = min_;
	g_target_bcd_ptr = target_;
}

e_bool State_CallbackSetClock(unsigned char button_no_)
{
	static unsigned char state = 0;
	static unsigned char prev_press = 1;
	unsigned char push = 0;
	g_disp_flag = 0b11111111;

	if (PIN_BUTTON_2 == 0)
	{
		if (prev_press == 0)
		{
			push = 1;
		}
		else
		{
			push = 0;
		}
		prev_press = 1;
	}
	else
	{
		push = 0;
		prev_press = 0;
	}

	
	switch(state)
	{
		case 0:
			SetConfigDigits(0b11000000, 23, 0, &g_hour);
			state = 1;
			break;			
		case 1:
			if (push != 0)
			{
				SetConfigDigits(0b00011000, 59, 0, &g_minute);
				state = 2;
			}
			break;
		case 2:
			if (push != 0)
			{
				SetConfigDigits(0b00000011, 59, 0, &g_second);
				state = 3;
			}
			break;
		case 3:
			if (push != 0)
			{
				state = 0;
				Rtc_UpdateRtcByPicTimer();
				g_timer1_callback[0] = State_CallbackRandomDisp;
				g_timer1_callback[1] = State_CallbackClock;
				g_timer1_callback[2] = NIL;
				g_state_current = &g_state_clock;
			}
			break;
		default :
			return FALSE;
	}

	g_disp[0] = LO(g_second);
	g_disp[1] = HI(g_second);
	g_disp[3] = LO(g_minute);
	g_disp[4] = HI(g_minute);
	g_disp[6] = LO(g_hour);
	g_disp[7] = HI(g_hour);
	g_disp_colon[2] = 0b11;
	g_disp_colon[5] = 0b11;
	ClearColon();

	State_CallbackBlink(button_no_);
	return TRUE;
}

// �J�����_�[���[�h
e_bool State_CallbackCalendar(unsigned char button_no_)
{
	g_disp_flag = 0b11111111;
	g_disp[0] = LO(g_date);
	g_disp[1] = HI(g_date);
	g_disp[3] = LO(g_month);
	g_disp[4] = HI(g_month);
	g_disp[6] = LO(g_year);
	g_disp[7] = HI(g_year);
	ClearColon();

	g_disp_colon[2] = 0b11;
	g_disp_colon[5] = 0b11;

	return TRUE;
}


e_bool State_CallbackSetCalendar(unsigned char button_no_)
{
	static unsigned char state = 0;
	static unsigned char prev_press = 1;
	unsigned char push = 0;
	g_disp_flag = 0b11111111;

	if (PIN_BUTTON_2 == 0)
	{
		if (prev_press == 0)
		{
			push = 1;
		}
		else
		{
			push = 0;
		}
		prev_press = 1;
	}
	else
	{
		push = 0;
		prev_press = 0;
	}

	
	switch(state)
	{
		case 0:
			SetConfigDigits(0b11000000, 99, 0, &g_year);
			state = 1;
			break;
		case 1:
			if (push != 0)
			{
				SetConfigDigits(0b00011000, 12, 1, &g_month);
				state = 2;
			}
			break;
		case 2:
			if (push != 0)
			{
				SetConfigDigits(0b00000011, 31, 1, &g_date);
				state = 3;
			}
			break;
		case 3:
			if (push != 0)
			{
				state = 0;
				Rtc_UpdateRtcCalendar();
				g_timer1_callback[0] = State_CallbackRandomDisp;
				g_timer1_callback[1] = State_CallbackCalendar;
				g_timer1_callback[2] = NIL;
				g_state_current = &g_state_calendar;
			}
			break;
		default :
			return FALSE;
	}

	g_disp[0] = LO(g_date);
	g_disp[1] = HI(g_date);
	g_disp[3] = LO(g_month);
	g_disp[4] = HI(g_month);
	g_disp[6] = LO(g_year);
	g_disp[7] = HI(g_year);
	g_disp_colon[2] = 0b11;
	g_disp_colon[5] = 0b11;
	ClearColon();

	State_CallbackBlink(button_no_);

	return TRUE;
}

e_bool State_CallbackAlarm(unsigned char button_no_)
{
	static unsigned char state = 0;
	g_disp_flag = 0b11111111;

	g_disp[0] = 0;
	g_disp[1] = 0;
	g_disp[3] = LO(g_alarm_minute);
	g_disp[4] = HI(g_alarm_minute);
	g_disp[6] = LO(g_alarm_hour);
	g_disp[7] = HI(g_alarm_hour);
	g_disp_colon[2] = 0b11;
	g_disp_colon[5] = 0b11;
	ClearColon();
	
	switch (state)
	{
	case 0:
		g_disp_colon[2] = 1;
		g_disp_colon[5] = 0;
		state = 1;
		break;
	case 1:
		g_disp_colon[2] = 2;
		g_disp_colon[5] = 0;
		state = 2;
		break;
	case 2:
		g_disp_colon[2] = 0;
		g_disp_colon[5] = 2;
		state = 3;
		break;
	case 3:
		g_disp_colon[2] = 0;
		g_disp_colon[5] = 1;
		state = 0;
		break;
	}

	return TRUE;
}



e_bool State_CallbackSetAlarm(unsigned char button_no_)
{
	static unsigned char state = 0;
	static unsigned char prev_press = 1;
	unsigned char push = 1;
	g_disp_flag = 0b11111100;

	if (PIN_BUTTON_2 == 0)
	{
		if (prev_press == 0)
		{
			push = 1;
		}
		else
		{
			push = 0;
		}
		prev_press = 1;
	}
	else
	{
		push = 0;
		prev_press = 0;
	}

	
	switch(state)
	{
		case 0:
			SetConfigDigits(0b11000000, 23, 0, &g_alarm_hour);
			state = 1;
			break;
		case 1:
			if (push != 0)
			{
				SetConfigDigits(0b00011000, 59, 0, &g_alarm_minute);
				state = 2;
			}
			break;
		case 2:
			if (push != 0)
			{
				state = 0;
				Rtc_UpdateRtcAlarm();
				g_timer1_callback[0] = State_CallbackRandomDisp;
				g_timer1_callback[1] = State_CallbackAlarm;
				g_timer1_callback[2] = NIL;
				g_state_current = &g_state_alarm;
			}
			break;
		default :
			return FALSE;
	}

	g_disp[0] = 0;
	g_disp[1] = 0;
	g_disp[3] = LO(g_alarm_minute);
	g_disp[4] = HI(g_alarm_minute);
	g_disp[6] = LO(g_alarm_hour);
	g_disp[7] = HI(g_alarm_hour);
	g_disp_colon[2] = 0b11;
	g_disp_colon[5] = 0b11;
	ClearColon();

	State_CallbackBlink(button_no_);

	return TRUE;
}


// ���x�v���[�h
e_bool State_CallbackTemparature(unsigned char button_no_)
{
	g_disp_flag = 0b11111111;

	g_disp[0] = 10;
	g_disp[1] = (g_temparature_fractional >> 4) & 0xf;
	g_disp[3] = g_temparature_integer & 0xf;
	g_disp[4] = (g_temparature_integer >> 4) & 0xf;;
	g_disp[6] = 10;
	g_disp[7] = 10;
	g_disp_colon[0] = 0b11;
	g_disp_colon[1] = 0;
	g_disp_colon[2] = 0b01;
	g_disp_colon[3] = 0;
	g_disp_colon[4] = 0;
	g_disp_colon[5] = 0;
	g_disp_colon[6] = 0b11;
	g_disp_colon[7] = 0b11;

	// ���̃R�[���o�b�N�܂ł̎��Ԃ𒲐�
	//TMR1L = TMR1L_1SEC_OFFSET;
	//TMR1H = TMR1H_1SEC_OFFSET;

	return TRUE;
}

e_bool State_CallbackConfig(unsigned char button_no_)
{
	static unsigned char state = 0;
	static unsigned char count = 0;

	g_disp_flag = 0b11111111;

	ClearColon();
	g_disp_colon[0] = 0b11;
	g_disp_colon[1] = 0b11;
	g_disp_colon[2] = 0b0;
	g_disp_colon[3] = 0b11;
	g_disp_colon[4] = 0b11;
	g_disp_colon[5] = 0b0;
	g_disp_colon[6] = 0b11;
	g_disp_colon[7] = 0b11;

	g_disp[0] = g_cds_1th_place;
	g_disp[1] = g_cds_10th_place;
	//g_disp[2] = 0;
	g_disp[3] = g_cds_100th_place;
	g_disp[4] = g_config_auto_brightness ? 1 : 0;
	//g_disp[5] = 0;
	g_disp[6] = g_config_auto_off_enable ? 1 : 0;
	g_disp[7] = g_config_alarm_enable ? 1 : 0;

	count++;
	if (count >= 3)
	{
		count = 0;
		switch (state)
		{
		case 0:
			g_disp_colon[0] = 0;
			g_disp_colon[1] = 0b11;
			state = 1;
			break;
		case 1:
			g_disp_colon[1] = 0;
			g_disp_colon[3] = 0b11;
			state = 2;
			break;
		case 2:
			g_disp_colon[3] = 0;
			g_disp_colon[4] = 0b11;
			state = 3;
			break;
		case 3:
			g_disp_colon[4] = 0;
			g_disp_colon[6] = 0b11;
			state = 4;
			break;
		case 4:
			g_disp_colon[6] = 0;
			g_disp_colon[7] = 0b11;
			state = 5;
			break;
		case 5:
			g_disp_colon[7] = 0;
			g_disp_colon[0] = 0b11;
			state = 0;
			break;
		}
	}


	// ���̃R�[���o�b�N�܂ł̎��Ԃ𒲐�
	//TMR1L = TMR1L_1SEC_OFFSET;
	//TMR1H = TMR1H_1SEC_OFFSET;

	return TRUE;
}

e_bool State_CallbackSetConfig(unsigned char button_no_)
{
	static unsigned char state = 0;
	static unsigned char prev_press = 1;
	unsigned char push = 0;
	g_disp_flag = 0b11111111;

	if (PIN_BUTTON_2 == 0)
	{
		if (prev_press == 0)
		{
			push = 1;
		}
		else
		{
			push = 0;
		}
		prev_press = 1;
	}
	else
	{
		push = 0;
		prev_press = 0;
	}
	
	switch(state)
	{
		case 0:
			SetConfigDigits(0b10000000, 1, 0, &g_config_alarm_enable);
			state = 1;
			break;
		case 1:
			if (push != 0)
			{
				SetConfigDigits(0b01000000, 1, 0, &g_config_auto_off_enable);
				state = 2;
			}	
			break;
		case 2:
			if (push != 0)
			{
				SetConfigDigits(0b00010000, 1, 0, &g_config_auto_brightness);
				state = 3;
			}
			break;
		case 3:
			if (push != 0)
			{
				SetConfigDigits(
					0b00001011,
					BRIGHTNESS_HIGHER_MAX,
					BRIGHTNESS_LOWER_THRESHOLD_NIXIE,
					&g_config_max_brightness);
				state = 4;
			}
			break;
		case 4:
			if (push != 0)
			{
				unsigned char config;
				config = g_config_alarm_enable <<7
					| g_config_auto_off_enable <<6 
					| g_config_auto_brightness << 4;

				Eeprom_Write(EEPROM_ADR_CONFIG, config);
				Eeprom_Write(EEPROM_ADR_MAX_BRIGHTNESS, g_config_max_brightness);
				g_timer1_callback[0] = State_CallbackRandomDisp;
				g_timer1_callback[1] = State_CallbackConfig;
				g_timer1_callback[2] = NIL;
				g_state_current = &g_state_config;
				state = 0;
			}
			break;
		default :
			return FALSE;
	}

	g_disp[0] = g_cds_1th_place;
	g_disp[1] = g_cds_10th_place;
	//g_disp[2] = 0;
	g_disp[3] = g_cds_100th_place;
	g_disp[4] = g_config_auto_brightness ? 1 : 0;
	//g_disp[5] = 0;
	g_disp[6] = g_config_auto_off_enable ? 1 : 0;
	g_disp[7] = g_config_alarm_enable ? 1 : 0;
	ClearColon();

	State_CallbackBlink(button_no_);
	return TRUE;
}

e_bool Mainloop_CallbackNormal(unsigned char button_no_)
{
	static unsigned char devide_count = 0;
	static unsigned char auto_off_devide_count = 0;
	static unsigned char alarm_loop_limit = 0;

	// ����
	++devide_count;
	if (devide_count < 10)
	{
		return TRUE;
	}
	devide_count = 0;

	// ���x
	{
		unsigned char adval = Pic_GetAd(0);
		unsigned char adval_i = adval/(100/ADVAL_2_TMPARATURE);
		unsigned char adval_f = (adval%4)*25;
		g_temparature_integer	 = (((adval_i / 10) % 10) << 4) | (adval_i % 10);
		g_temparature_fractional = (((adval_f / 10) % 10) << 4);// | (adval_f % 10);
	}
	
	// AD�R���o�[�^���d�҂����Ă݂�
	Wait10ms(1);

	// �b��@���v�̒���
	// �����̃^�C�}�[�Ői�߂����Ԃ�������΃^�C�}�[���荞�݂̊Ԋu��L�΂�
	// �X�ɂ���Ƃ��͂قڕK��g_second==0
	if (g_next_timer1_ajustment != 0)
	{
		g_next_timer1_ajustment = 0;

		char second = Rtc_GetSecond();
		
		if (second > 48) // 0x30
		{
			second -= 96; //0x60
		}
		
		if (second <= (char)g_second)
		{
			timer1h_1sec_offset = TMR1H_1SEC_OFFSET_DEFAULT_DOWN;
			timer1l_1sec_offset = TMR1L_1SEC_OFFSET_DEFAULT_DOWN;
		}
		else
		{
			timer1h_1sec_offset = TMR1H_1SEC_OFFSET_DEFAULT_UP;
			timer1l_1sec_offset = TMR1L_1SEC_OFFSET_DEFAULT_UP;
		}
	}

 
	// �A���[��
	// ���̕ӂ��K��
	if ((g_hour == g_alarm_hour)
	&&  (g_minute == g_alarm_minute)
	&&  (g_second == 0))
	{
		g_alarm_trigger = 1;
		alarm_loop_limit = 0;
	}
	if (g_alarm_trigger != 0)
	{
		++alarm_loop_limit;
		if ((alarm_loop_limit == 0xff)
		||  (g_button0_press | g_button1_press | g_button2_press))
		{
			g_alarm_trigger = 0;
			g_alarm_beep_on = 0;
		}
	}

	// ���邳
	{
		if (g_config_auto_brightness)
		{
			g_cds_adval = Pic_GetAd(1);
			if (g_cds_adval > g_config_max_brightness)
				g_cds_adval = g_config_max_brightness;
		}
		else
		{
			g_cds_adval = g_config_max_brightness;
		}

		// �\���p�̒l�����C�����[�`����Ōv�Z
		// Config�p��max�l��\�����郂�[�h���p��
		if (g_state_current == &g_state_set_config)
		{
			g_cds_adval = g_config_max_brightness;
		}
		g_cds_100th_place = (g_cds_adval / 100) % 10;
		g_cds_10th_place  = (g_cds_adval / 10) % 10;
		g_cds_1th_place   = g_cds_adval % 10;
				
		// �Â����Ԃ���������\���������p
		if (g_config_auto_off_enable)
		{
			if (g_cds_adval < AUTO_OFF_THRESHOLD)
			{
				++auto_off_devide_count;
				if (auto_off_devide_count == 0x14)
				{
					auto_off_devide_count = 0;
					if (g_auto_off_count != 0xff)
					{
						++g_auto_off_count;
					}
				}

				if (g_button0_press | g_button1_press | g_button2_press)
				{
					g_auto_off_count = 0;
				}
			}
			else
			{
				g_auto_off_count = 0;
			}
		}
	}

	return TRUE;
}

//---------------
// �X�e�[�g�n���h��
//---------------
void SelectTimer1Callback(unsigned char button_no_)
{
	unsigned int i;

	if (button_no_ == 0)
		g_timer1_callback[0] = State_CallbackRightChange;
	else if (button_no_ == 1)
		g_timer1_callback[0] = State_CallbackLeftChange;
	else
		g_timer1_callback[0] = State_CallbackRandomDisp;

	for (i = 0; i<8; ++i)
	{
		g_disp_prev[i] = g_disp[i];
		g_disp_colon_prev[i] = g_disp_colon[i];
	}
}

e_bool State_ChangeClock(unsigned char button_no_)
{
	unsigned char i;
	Rtc_UpdatePicTimerByRtc();
	g_disp_capture[0] = LO(g_second);
	g_disp_capture[1] = HI(g_second);
	//g_disp_capture[2] = 0;
	g_disp_capture[3] = LO(g_minute);
	g_disp_capture[4] = HI(g_minute);
	//g_disp_capture[5] = 0;
	g_disp_capture[6] = LO(g_hour);
	g_disp_capture[7] = HI(g_hour);

	SelectTimer1Callback(button_no_);
	g_timer1_callback[1] = State_CallbackClock;
	g_timer1_callback[2] = NIL;
	g_state_current = &g_state_clock;

	return TRUE;
}

e_bool State_ChangeSetClock(unsigned char button_no_)
{
	g_timer1_callback[0] = State_CallbackSetClock;
	g_timer1_callback[1] = NIL;
	g_state_current = &g_state_set_clock;
	return TRUE;
}

e_bool State_ChangeTemparature(unsigned char button_no_)
{
	g_disp_capture[0] = 10;
	g_disp_capture[1] = (g_temparature_fractional >> 4) & 0xf;
	g_disp_capture[3] = g_temparature_integer & 0xf;
	g_disp_capture[4] = (g_temparature_integer >> 4) & 0xf;;
	g_disp_capture[6] = 10;
	g_disp_capture[7] = 10;
	g_disp_colon_capture[0] = 0b11;
	g_disp_colon_capture[1] = 0;
	g_disp_colon_capture[2] = 0b01;
	g_disp_colon_capture[3] = 0;
	g_disp_colon_capture[4] = 0;
	g_disp_colon_capture[5] = 0;
	g_disp_colon_capture[6] = 0b11;
	g_disp_colon_capture[7] = 0b11;

	SelectTimer1Callback(button_no_);
	g_timer1_callback[1] = State_CallbackTemparature;
	g_timer1_callback[2] = NIL;
	g_state_current = &g_state_temparature;

	return TRUE;
}

e_bool State_ChangeCalendar(unsigned char button_no_)
{
	Rtc_UpdatePicTimerByRtc();
	g_disp_capture[0] = LO(g_date);
	g_disp_capture[1] = HI(g_date);
	//g_disp_capture[2] = 0;
	g_disp_capture[3] = LO(g_month);
	g_disp_capture[4] = HI(g_month);
	//g_disp_capture[5] = 0;
	g_disp_capture[6] = LO(g_year);
	g_disp_capture[7] = HI(g_year);

	SelectTimer1Callback(button_no_);
	g_timer1_callback[1] = State_CallbackCalendar;
	g_timer1_callback[2] = NIL;
	g_state_current = &g_state_calendar;

	return TRUE;
}

e_bool State_ChangeSetCalendar(unsigned char button_no_)
{
	g_timer1_callback[0] = State_CallbackSetCalendar;
	g_timer1_callback[1] = NIL;
	g_state_current = &g_state_set_alarm;
	return TRUE;
}

e_bool State_ChangeAlarm(unsigned char button_no_)
{
	g_disp_capture[0] = 0;
	g_disp_capture[1] = 0;
	//g_disp_capture[2] = 0;
	g_disp_capture[3] = LO(g_alarm_minute);
	g_disp_capture[4] = HI(g_alarm_minute);
	//g_disp_capture[5] = 0;
	g_disp_capture[6] = HI(g_alarm_hour);
	g_disp_capture[7] = HI(g_alarm_hour);


	SelectTimer1Callback(button_no_);
	g_timer1_callback[1] = State_CallbackAlarm;
	g_timer1_callback[2] = NIL;
	g_state_current = &g_state_alarm;

	return TRUE;
}

e_bool State_ChangeSetAlarm(unsigned char button_no_)
{
	g_timer1_callback[0] = State_CallbackSetAlarm;
	g_timer1_callback[1] = NIL;
	g_state_current = &g_state_set_alarm;
	return TRUE;
}


e_bool State_ChangeConfig(unsigned char button_no_)
{
	g_disp_capture[0] = g_cds_1th_place;
	g_disp_capture[1] = g_cds_10th_place;
	//g_disp_capture[2] = 0;
	g_disp_capture[3] = g_cds_100th_place;
	g_disp_capture[4] = g_config_auto_brightness ? 1 : 0;
	//g_disp_capture[5] = 0;
	g_disp_capture[6] = g_config_auto_off_enable ? 1 : 0;
	g_disp_capture[7] = g_config_alarm_enable ? 1 : 0;

	SelectTimer1Callback(button_no_);
	g_timer1_callback[1] = State_CallbackConfig;
	g_timer1_callback[2] = NIL;
	g_state_current = &g_state_config;

	return TRUE;
}

e_bool State_ChangeDispOff(unsigned char button_no_)
{
	unsigned char i;
	SelectTimer1Callback(button_no_);

	for (i=0; i<8; ++i)
	{
		g_disp_capture[i] = 10;
		g_disp_colon_capture[i] = 0b11;
	}
	
	g_timer1_callback[1] = State_CallbackDispoff;
	g_timer1_callback[2] = NIL;
	g_state_current = &g_state_dispoff;

	return TRUE;
}

e_bool State_ChangeSetConfig(unsigned char button_no_)
{
	g_timer1_callback[0] = State_CallbackSetConfig;
	g_timer1_callback[1] = NIL;
	g_state_current = &g_state_set_config;
	return TRUE;
}


e_bool TargetUp(unsigned char arg_)
{
	if (*g_target_bcd_ptr >= g_target_max)
		*g_target_bcd_ptr = g_target_min;
	else
		++(*g_target_bcd_ptr);

	return TRUE;
}

e_bool TargetDown(unsigned char arg_)
{
	if (*g_target_bcd_ptr <= g_target_min)
		*g_target_bcd_ptr = g_target_max;
	else
		--(*g_target_bcd_ptr);

	return TRUE;
}

e_bool TargetBcdUp(unsigned char arg_)
{
	unsigned char dec = 10*HI(*g_target_bcd_ptr) + LO(*g_target_bcd_ptr);
	dec = dec >= g_target_max ? g_target_min : dec + 1;
	*g_target_bcd_ptr = (dec/10<<4) + dec%10;
	return TRUE;
}

e_bool TargetBcdDown(unsigned char arg_)
{
	unsigned char dec = 10*HI(*g_target_bcd_ptr) + LO(*g_target_bcd_ptr);
	dec = dec <= g_target_min ? g_target_max : dec - 1;
	*g_target_bcd_ptr = (dec/10<<4) + dec%10;
	return TRUE;
}

// ������
void InitApplication()
{
	unsigned char config;
	
	// Rtc����������сA���Ԏ擾
	Rtc_Init();
	Rtc_UpdatePicTimerByRtc();

	// config
	config = Eeprom_Read(EEPROM_ADR_CONFIG);
	g_config_alarm_enable = config & CONFIG_AUTO_OFF ? 1:0;
	g_config_auto_off_enable = config & CONFIG_ALARM_ON ? 1:0;
	g_config_auto_brightness = config & CONFIG_AUTO_BRIGHTNESS ? 1:0;
	g_config_max_brightness = Eeprom_Read(EEPROM_ADR_MAX_BRIGHTNESS);

	// �X�e�[�g�}�V����������
	//�ŏ��͎��v�X�e�[�g
	g_state_clock.on_button0			= State_ChangeCalendar;
	g_state_clock.on_button1			= State_ChangeDispOff;
	g_state_clock.on_button2			= State_ChangeSetClock;
	g_state_calendar.on_button0			= State_ChangeTemparature;
	g_state_calendar.on_button1			= State_ChangeClock;
	g_state_calendar.on_button2			= State_ChangeSetCalendar;
	g_state_temparature.on_button0		= State_ChangeAlarm;
	g_state_temparature.on_button1		= State_ChangeCalendar;
	g_state_temparature.on_button2		= NIL;
	g_state_alarm.on_button0			= State_ChangeConfig;
	g_state_alarm.on_button1			= State_ChangeTemparature;
	g_state_alarm.on_button2			= State_ChangeSetAlarm;
	g_state_config.on_button0			= State_ChangeDispOff;
	g_state_config.on_button1			= State_ChangeAlarm;
	g_state_config.on_button2			= State_ChangeSetConfig;
	g_state_dispoff.on_button0			= State_ChangeClock;
	g_state_dispoff.on_button1			= State_ChangeConfig;
	g_state_dispoff.on_button2			= NIL;
	g_state_set_clock.on_button0		= TargetBcdUp;
	g_state_set_clock.on_button1		= TargetBcdDown;
	g_state_set_clock.on_button2		= NIL;
	g_state_set_calendar.on_button0		= TargetBcdUp;
	g_state_set_calendar.on_button1		= TargetBcdDown;
	g_state_set_calendar.on_button2		= NIL;
	g_state_set_alarm.on_button0		= TargetBcdUp;
	g_state_set_alarm.on_button1		= TargetBcdDown;
	g_state_set_alarm.on_button2		= NIL;
	g_state_set_config.on_button0 	    = TargetUp;
	g_state_set_config.on_button1 	    = TargetDown;
	g_state_set_config.on_button2 	    = NIL;

	// �R�[���o�b�N��o�^
	g_timer1_callback[0] = State_CallbackClock;
	g_timer1_callback[1] = NIL;
	g_mainloop_callback[0]	 = Mainloop_CallbackNormal;
	g_state_current = &g_state_clock;
}

// ���荞��
void interrupt intr(void)
{
	if (T0IF)
	{
		// �v���N���A
		T0IF = 0;

		// �R�[���o�b�N
		//DoCallbackChain(g_timer0_callback);
		if (g_auto_off_count == 0xff)
			Disp_CallbackOff();
		else
			Disp_CallbackNormal();
	}
	else if (TMR1IF)
	{
		// ��U�^�C�}�[���~
		//T1CONbits.TMR1ON = 0;
		
		// �v���N���A
		TMR1IF = 0;

		// �R�[���o�b�N
		DoCallbackChain(g_timer1_callback);
		
		// ��U�^�C�}�[���ĊJ
		//T1CONbits.TMR1ON = 1;
	}
	else if (SSPIF)
	{
		// ���荞�݂ŗv���N���A�͂��Ȃ�
		// SSPIF = 0; 
	}
}


// ���C��
void main(void)
{
	static unsigned char alarm_count = 0;

	// ������
	InitDevice();
	InitApplication();

	while (1)
	{
		// �{�^���擾
		if (!PIN_BUTTON_0)
		{
			if (g_button0_press == 0)
			{
				g_button0_push = 1;
			}
			else
			{
				g_button0_push = 0;
			}

			g_button0_press = 1;
		}
		else
		{
			g_button0_push = 0;
			g_button0_press = 0;
		}

		if (!PIN_BUTTON_1)
		{	
			if (g_button1_press == 0)
			{
				g_button1_push = 1;
			}
			else
			{
				g_button1_push = 0;
			}

			g_button1_press = 1;
		}
		else
		{
			g_button1_press = 0;
			g_button1_push = 0;
		}
					
		if (!PIN_BUTTON_2)
		{	
			if (g_button2_press == 0)
			{
				g_button2_push = 1;
			}
			else
			{
				g_button2_push = 0;
			}

			g_button2_press = 1;
		}
		else
		{
			g_button2_press = 0;
			g_button2_push = 0;
		}

		// �{�^���v�b�V���R�[���o�b�N
		if ((g_state_current->on_button0 != NIL)
		&&  (g_button0_push))
		{
			(*(g_state_current->on_button0))(0);		
		}

		if ((g_state_current->on_button1 != NIL)
		&&  (g_button1_push))
		{
			(*(g_state_current->on_button1))(1);		
		}

		if ((g_state_current->on_button2 != NIL)
		&&  (g_button2_push))
		{
			(*(g_state_current->on_button2))(2);		
		}

		// �K���X�s�[�J�[�A���[���D��
		if (g_alarm_trigger != 0)
		{
			++alarm_count;
			if (alarm_count > 10)
			{
				alarm_count = 0;
				g_alarm_beep_on = g_alarm_beep_on == 1 ? 0 : 1; 
			}
		}
		if (g_button0_press | g_button1_press | g_button2_press | g_alarm_beep_on)
		{
			PIN_SPEAKER = 1;
		}
		else
		{
			PIN_SPEAKER = 0;
		}

		// ���C���R�[���o�b�N
		DoCallbackChain(g_mainloop_callback);

		Wait10ms(1);
	}
}
