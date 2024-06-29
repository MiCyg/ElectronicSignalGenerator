
#ifndef functions_h
#define functions_h

#include "samples.h"


// for MENU
typedef struct menu_struct element_t;
struct menu_struct{
	String name;
	String value;
	element_t * prev;
	element_t * next;
	element_t * parent;
	element_t * child; 
	void (*action)(void);

	uint8_t cursor_position;

};
element_t elem1;
element_t elem2;
element_t elem3;
element_t elem4;
element_t elem5;
element_t elem5_b;
element_t elem5_1;
element_t elem5_2;

//pointer to upgrade lcd function:
void (*upgrade_lcd)();

//================ MENU ================
void play_back_after();
void play_back();
void play();

void change_back();
void change_right();
void change_left();
void change_value();

void autor_back();
void autor_action();

void back_action();

void upgrade_menu();
void menu_up();
void menu_down();
void menu_enter();

void set_menu_list();

void startup_dispay();
void autor_display();

void draw_wave(uint8_t);

//=============== HARDWARE =================
void set_timer0();
void enable_timer0(uint8_t);

void set_Timer1();
void enable_timer1(uint8_t);

void set_encoder();
void set_sign();

void set_adc();
void adc_enable(uint8_t);
uint16_t adc_conversion();

void update_batt();

//=================== GENERATOR ===================
const uint32_t SAMPLE_RATE = 80000; 
// const uint16_t MAX_FREQUENCY_IDX = 1000;
// const uint16_t MIN_FREQUENCY_IDX = 1;
const uint16_t MAX_FREQUENCY = 20000;
const uint16_t MIN_FREQUENCY = SAMPLE_RATE/LENGTH_SAMPLES;
const uint16_t MAX_AMPLITUDE = 127;
const uint16_t MIN_AMPLITUDE = 0;
const uint16_t MAX_WAVEFORM_IDX = 3;
const uint16_t MIN_WAVEFORM_IDX = 0;
const uint8_t MAX_AMPLITUDE_SHOw = 100;
const uint8_t MIN_AMPLITUDE_SHOw = 0;

const uint8_t SINE		= 0;
const uint8_t TRIANGLE	= 1;
const uint8_t SQUARE	= 2;
const uint8_t SAWTOOTH 	= 3;



void start_generate();
void DAC(uint8_t);
uint8_t (*get_wave)();


#endif
