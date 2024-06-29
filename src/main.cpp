#include <Arduino.h>
#include <LiquidCrystal.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "headers/graphic.h"
#include "headers/functions.h"
#include "headers/samples.h"


//=================================================================================
//===================================== TEST ======================================
const uint8_t TEST = 9;

//=================================================================================
//===================================== LCD =======================================
const uint8_t rs = 19;
const uint8_t en = 13;
const uint8_t d4 = 18;
const uint8_t d5 = 17;
const uint8_t d6 = 14; 
const uint8_t d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const uint8_t LCD_HEIGHT = 2;
const uint8_t LCD_WEIGHT = 16;

//=================================================================================
//=================================== ENCODER =====================================
volatile uint8_t Timer_switch = 0;
volatile uint16_t Timer_enc = 0;

typedef struct {
	uint8_t leftPin;
	uint8_t rightPin;
	uint8_t pushPin;

	uint8_t leftPointer;
	uint8_t rightPointer;
	uint8_t pushPointer;
	uint8_t Pointer;
	uint8_t actualPointer;

	uint8_t checkNumberEnc;	
	uint8_t checkNumberBut;
	
	void (*fun_right)(void);
	void (*fun_left)(void);
	void (*fun_push)(void);
	void (*fun_push_after)(void);	
	void set_encoder(uint8_t _leftPin, uint8_t _rightPin, uint8_t _pushpin, uint8_t _numberOfCheck, uint8_t _checkNumberBut){
		Pointer = 0;
		actualPointer = Pointer;

		leftPin = _leftPin;
		rightPin = _rightPin;
		pushPin = _pushpin;
		checkNumberEnc = _numberOfCheck;
		checkNumberBut = _checkNumberBut;
	}
} encoder_t;
encoder_t Encoder_blue;

const uint8_t LEFT = 10;
const uint8_t RIGHT = 11 ;
const uint8_t PUSH = 12;

void check_encoder(encoder_t *enc){
	if(Timer_enc >= enc->checkNumberEnc){
		Timer_enc = 0;

		// encoder sequence left:
		if(!digitalRead(enc->leftPin) && digitalRead(enc->rightPin) && enc->leftPointer == 0 && enc->rightPointer == 0){
			enc->leftPointer = 1;
		}
		if(digitalRead(enc->leftPin) && digitalRead(enc->rightPin) && enc->leftPointer == 1 && enc->rightPointer == 0 ){
			if(enc->fun_left)enc->fun_left();
			enc->leftPointer = 0;
		}

		// encoder sequence right:
		if(!digitalRead(enc->rightPin) && digitalRead(enc->leftPin) && enc->rightPointer == 0 && enc->leftPointer == 0){
			enc->rightPointer = 1;
		}
		if(digitalRead(enc->rightPin) && digitalRead(enc->leftPin) && enc->rightPointer == 1 && enc->leftPointer == 0){
			if(enc->fun_right)enc->fun_right();
			enc->rightPointer = 0;
		}

	}
	
	if(Timer_switch >= enc->checkNumberBut){
		Timer_switch = 0;
		// switch sequence:
		if(!digitalRead(enc->pushPin) && enc->pushPointer == 0){
			if(enc->fun_push) enc->fun_push();
			enc->pushPointer = 1;
		}
		if(digitalRead(enc->pushPin) && enc->pushPointer == 1){
			if(enc->fun_push_after) enc->fun_push_after();
			enc->pushPointer = 0;
		}
	}

}

uint8_t left_sequence[4] = {0, 3, 2, 1};
uint8_t right_sequence[4] = {0, 1, 3, 2};


void check_absolute_encoder(encoder_t *enc){
	if(Timer_enc >= enc->checkNumberEnc){
		Timer_enc = 0;

		// encoder sequence:
		if(!digitalRead(enc->leftPin) && !digitalRead(enc->rightPin) ){
			enc->actualPointer = 0;
			// Serial.println("actualPointer = 0");
		}
		if(!digitalRead(enc->leftPin) && digitalRead(enc->rightPin)){
			enc->actualPointer = 1;
			// Serial.println("actualPointer = 1");
		}
		if(digitalRead(enc->leftPin) && !digitalRead(enc->rightPin)){
			enc->actualPointer = 2;
			// Serial.println("actualPointer = 2");
		}
		if(digitalRead(enc->leftPin) && digitalRead(enc->rightPin)){
			enc->actualPointer = 3;
			// Serial.println("actualPointer = 3");
		}

		// uint8_t j;
		// for (uint8_t i = 0; i < 4; i++){
		// 	j = i%4;
		// 	if(left_sequence[i] == enc->Pointer && left_sequence[j] == enc->actualPointer) enc->leftPointer =1;
		// 	else enc->leftPointer = 0;
			
		// 	if(right_sequence[i] == enc->Pointer && right_sequence[j] == enc->actualPointer) enc->rightPointer = 1;
		// 	else enc->rightPointer = 0;
		// }
		


		if (!enc->actualPointer && enc->actualPointer != enc->Pointer){
			// Serial.println("pointer != actualPointer");
			// Serial.println(enc->Pointer);
			// Serial.println(enc->actualPointer);
			// if (enc->leftPointer && enc->fun_left) enc->fun_left();
			// if (enc->rightPointer && enc->fun_right) enc->fun_right();			
			if (enc->Pointer == 1 && enc->fun_left) enc->fun_left();
			if (enc->Pointer == 2  && enc->fun_right) enc->fun_right();			
		}



		enc->Pointer = enc->actualPointer;

	}
	
	if(Timer_switch >= enc->checkNumberBut){
		Timer_switch = 0;
		// switch sequence:
		if(!digitalRead(enc->pushPin) && enc->pushPointer == 0){
			if(enc->fun_push) enc->fun_push();
			enc->pushPointer = 1;
		}
		if(digitalRead(enc->pushPin) && enc->pushPointer == 1){
			if(enc->fun_push_after) enc->fun_push_after();
			enc->pushPointer = 0;
		}
	}

}

//=================================================================================
//=================================== PROGRAM =====================================

typedef enum state {INIT, MENU, WAVE} state_t;
state_t state = INIT;
volatile uint16_t Timer_state = 0;
uint8_t flagMenu = 0;

uint8_t refres_pointer = 0;
uint8_t menu_pointer_position = 0;
uint8_t menu_pointer_horizontal = 0;
int8_t menu_index = 0;
element_t * current_element = &elem1;

//==================================================================================
//=================================== GENERATOR ====================================

uint8_t Waveform_idx = SINE;
String Waveform_name[]={"SINE", "TRIANGLE", "SQARE", "SAWTOOTH"};

uint16_t Frequency = 1000;
uint16_t Batt_level = 0;
uint8_t amplitude_show = MAX_AMPLITUDE_SHOw;

volatile uint16_t Max_sample;
volatile uint8_t Amplitude = MAX_AMPLITUDE;
volatile uint16_t Accumulator = 0;
volatile uint16_t Step = LENGTH_SAMPLES * Frequency/SAMPLE_RATE;


uint8_t get_sine(){
	return pgm_read_byte_near(sine_wave + Accumulator);

}
uint8_t get_triangle(){
	uint8_t sample = pgm_read_byte_near(sawtooth_wave + Accumulator);
	if (sample <= 127){
		sample = 255-sample;
	}
	
	return 2*sample;

}
uint8_t get_sawtooth(){
	return pgm_read_byte_near(sawtooth_wave + Accumulator);

}
uint8_t get_square(){
	uint8_t sample = pgm_read_byte_near(sawtooth_wave + Accumulator);
	if (sample<=127){
		return 0;
	}else return 255;

}

void start_generate(){
	noInterrupts();
	enable_timer0(0);	
	enable_timer1(1);
	state = WAVE;
	Encoder_blue.fun_push = NULL;
	Encoder_blue.fun_left = NULL;
	Encoder_blue.fun_right = NULL;
	Encoder_blue.fun_push_after = NULL;

	Accumulator = 0;
	Step = LENGTH_SAMPLES/(SAMPLE_RATE/Frequency);
	Max_sample = LENGTH_SAMPLES;	

	if(Waveform_idx == SINE) {
		get_wave = &get_sine;

	}
	else if(Waveform_idx == TRIANGLE) {
		get_wave = &get_triangle;

	}
	else if (Waveform_idx == SAWTOOTH){
		get_wave = &get_sawtooth;

	}
	else if (Waveform_idx == SQUARE){
		get_wave = &get_square;

	}	


	interrupts();
}

void DAC(uint8_t sample){
	PORTB = (PORTB & 0b11111100) | (sample>>6);
	PORTD = (PORTD & 0b00000011) | (sample<<2);

}

//====================================================================================
//=================================== FOR MENU =======================================
uint8_t upgrade_all = 0;
const uint8_t CURSOR_OFFSET = 7;

void play_back_after(){
	Encoder_blue.fun_push_after = NULL;
	Encoder_blue.fun_push = menu_enter;
	Encoder_blue.fun_left = menu_up;
	Encoder_blue.fun_right = menu_down;


}
void play_back(){
	noInterrupts();
	enable_timer0(1);
	enable_timer1(0);
	state = MENU;
	elem1.name = "Play";

	Encoder_blue.fun_push_after = play_back_after;
	Encoder_blue.fun_push = NULL;
	Encoder_blue.fun_left = NULL;
	Encoder_blue.fun_right = NULL;
	interrupts();

	upgrade_all = 1;
	refres_pointer = 0;
	upgrade_lcd();

}
void play(){
	Encoder_blue.fun_push = NULL;
	Encoder_blue.fun_left = NULL;
	Encoder_blue.fun_right = NULL;
	Encoder_blue.fun_push_after = start_generate;
	// state = WAVE;

	elem1.name = "Stop";
	refres_pointer = 1;
	upgrade_lcd();
	draw_wave(Waveform_idx);
}

// change value:
void change_back(){
	menu_pointer_horizontal = 0;
	Encoder_blue.fun_left = menu_up;
	Encoder_blue.fun_right = menu_down;
	Encoder_blue.fun_push = menu_enter;

	upgrade_lcd();


}
void change_right(){
	if (current_element == &elem2){
		if(Frequency < MAX_FREQUENCY){
			if(Frequency >= 100 && Frequency < 1000) Frequency += 10;
			else if(Frequency >= 1000 && Frequency < 10000) Frequency += 100;
			else if(Frequency >= 10000) Frequency += 1000;
			else Frequency += 5;

		}
		// else Frequency = MAX_FREQUENCY;
		elem2.value = String(Frequency) + "Hz";
		
	}
	else if (current_element == &elem3){
		if(Waveform_idx != MIN_WAVEFORM_IDX) Waveform_idx--;
		elem3.value = String(Waveform_name[Waveform_idx]);

	}
	else if (current_element == &elem4){
		if(amplitude_show < MAX_AMPLITUDE_SHOw) amplitude_show+=5;
		// Amplitude = (amplitude_show/(long)MAX_AMPLITUDE_SHOw)*MAX_AMPLITUDE;
		Amplitude = map(amplitude_show,0,MAX_AMPLITUDE_SHOw,0,MAX_AMPLITUDE);
		elem4.value = String(amplitude_show) + "%";

	}
	upgrade_all = 1;
	upgrade_lcd();
}
void change_left(){
	if (current_element == &elem2){
		if(Frequency > MIN_FREQUENCY){
			if(Frequency > 100 && Frequency <= 1000) Frequency -= 10;
			else if(Frequency > 1000 && Frequency <= 10000) Frequency -= 100;
			else if(Frequency > 10000) Frequency -= 1000;
			else Frequency -= 5;
		}
		
		
		elem2.value = String(Frequency) +"Hz";
	
	}
	else if (current_element == &elem3){
		if(Waveform_idx != MAX_WAVEFORM_IDX)Waveform_idx++;
		elem3.value = String(Waveform_name[Waveform_idx]);

	}
	else if (current_element == &elem4){
		if(amplitude_show > MIN_AMPLITUDE_SHOw) amplitude_show-=5;
		Amplitude = map(amplitude_show,0,MAX_AMPLITUDE_SHOw,0,MAX_AMPLITUDE);
		elem4.value = String(amplitude_show) + "%";
	
	}
	upgrade_all = 1;
	upgrade_lcd();
}
void change_value(){
	menu_pointer_horizontal = CURSOR_OFFSET;
	Encoder_blue.fun_left = change_left;
	Encoder_blue.fun_right = change_right;
	Encoder_blue.fun_push = change_back;

	upgrade_lcd();
}

// show author:
void autor_back(){
	upgrade_lcd = upgrade_menu;
	Encoder_blue.fun_push = menu_enter;
	Encoder_blue.fun_left = menu_up;
	Encoder_blue.fun_right = menu_down;
	upgrade_lcd();
}
void autor_action(){
	upgrade_lcd = &autor_display;
	Encoder_blue.fun_push = autor_back;	
	Encoder_blue.fun_left = NULL;	
	Encoder_blue.fun_right = NULL;	
	upgrade_lcd();
}

// back to parent:
void back_action(){

	current_element = current_element->parent;
	menu_pointer_position = current_element->cursor_position;
	upgrade_lcd();

}

// menu:
void upgrade_menu(){
	element_t local_element = *current_element;

	if (upgrade_all){
		lcd.clear();
		// draw list
		lcd.setCursor(1, menu_pointer_position);
		lcd.print(local_element.name);
		lcd.setCursor(CURSOR_OFFSET+1, menu_pointer_position);
		
		// if (is_generate) draw_wave(Waveform_idx);
				
		lcd.print(local_element.value);

		if (menu_pointer_position){
			if(local_element.prev){
			lcd.setCursor(1, 0);
			lcd.print(local_element.prev->name);
			lcd.setCursor(CURSOR_OFFSET+1, 0);
			lcd.print(local_element.prev->value);		
			}
		}
		else{
			if(local_element.next){
			lcd.setCursor(1, 1);
			lcd.print(local_element.next->name);
			lcd.setCursor(CURSOR_OFFSET+1, 1);
			lcd.print(local_element.next->value);	
			}

		}
		upgrade_all = 0;
	}

	lcd.setCursor(0,0);
	lcd.print(" ");	
	lcd.setCursor(0,1);
	lcd.print(" ");	
	lcd.setCursor(CURSOR_OFFSET,0);
	lcd.print(" ");	
	lcd.setCursor(CURSOR_OFFSET,1);
	lcd.print(" ");	
	// draw cursor:
	lcd.setCursor(menu_pointer_horizontal, menu_pointer_position);
	lcd.write(0b01111110);
}
void menu_up(){
	if(current_element->next) {
		current_element = current_element->next;
		menu_index++;
		if (menu_pointer_position >= LCD_HEIGHT-1) upgrade_all = 1;
		else menu_pointer_position++;
		
	}

	upgrade_menu();
}
void menu_down(){
	if(current_element->prev) {
		current_element = current_element->prev;
		menu_index--;
		if (menu_pointer_position){
			menu_pointer_position--;
			menu_pointer_horizontal = 0;
		}
		else upgrade_all = 1;
		
	}

	upgrade_menu();
}
void menu_enter(){
	if (current_element->child){	
		current_element->cursor_position = menu_pointer_position;
		current_element = current_element->child;
		menu_index = 0;
		menu_pointer_position = 0;
		menu_pointer_horizontal = 0;
		upgrade_all = 1;
		upgrade_menu();	
	}else if(current_element->action) {
		upgrade_all = 1;
		current_element->action();
	}

}

// tree of menu:
void set_menu_list(){

	//				{*name, 	*value,							*prev, 		*next, 		*parent,	*child, 	(*action)}	
	elem1 = 		{"Play",	"",								NULL, 		&elem2,		NULL,		NULL,		&play };
	elem2 =			{"Freq",	String(Frequency) +"Hz",		&elem1,		&elem3,		NULL,		NULL,		&change_value};
	elem3 =			{"Wave",	Waveform_name[Waveform_idx],	&elem2,		&elem4,		NULL,		NULL,		&change_value};
	elem4 =			{"Amp",		String(amplitude_show) +"%",	&elem3,		&elem5,		NULL,		NULL,		&change_value};
	elem5 =			{"Info",	"->",							&elem4,		NULL,		NULL,		&elem5_1,	NULL};	
		elem5_b =	{"Back",	"<-",							NULL,		&elem5_1,	&elem5,		NULL,		&back_action};
		elem5_1 =	{"Batt",	"",								&elem5_b,	&elem5_2,	&elem5,		NULL,		NULL};
		elem5_2 =	{"Author",	"",								&elem5_1, 	NULL, 		&elem5,		NULL,		&autor_action};

}

// graphic:
void startup_dispay(){
	lcd.clear();	
	lcd.setCursor(3, 0);
	lcd.print("Waveform");
	lcd.setCursor(2, 1);
	lcd.print("generator v1");

}
void autor_display(){
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Designed by");
	lcd.setCursor(0,1);
	lcd.print("Milosz Derzko");

}
void draw_wave(uint8_t index_wave){
	lcd.setCursor(CURSOR_OFFSET + 1, menu_pointer_position);
	lcd.write(byte(index_wave*2));
	lcd.setCursor(CURSOR_OFFSET + 2, menu_pointer_position);
	lcd.write(byte(index_wave*2 + 1));

}


//==================================================================================
//================================== HARDWARE ======================================
void set_timer0(){
	noInterrupts();
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS00) | (1<<CS01);
	OCR0A = 239; // ~1 milisecond
	TIMSK0 = (1<<OCIE0A);
	interrupts();

}
void enable_timer0(uint8_t isEnabled){

	if (isEnabled) TCCR0B |= (1<<CS00) | (1<<CS01);
	else TCCR0B &= ~(1<<CS00) & ~(1<<CS01);

}

void set_Timer1(){
	noInterrupts();
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) | (1<<CS10);
	OCR1A = 200; // for 80kHz
	TIMSK1 = (1<<OCIE1A);

	interrupts();
}
void enable_timer1(uint8_t isEnabled){
	if (isEnabled) TCCR1B |= (1<<CS10);
	else TCCR1B &= ~(1<<CS10);	
}

void set_encoder(){

	pinMode(LEFT, INPUT_PULLUP);
	pinMode(RIGHT, INPUT_PULLUP);
	pinMode(PUSH, INPUT_PULLUP);
	Encoder_blue.set_encoder(LEFT, RIGHT, PUSH, 2 , 13);
	Encoder_blue.fun_push_after = NULL;
	Encoder_blue.fun_right = menu_down;
	Encoder_blue.fun_left = menu_up;
	Encoder_blue.fun_push = menu_enter;


}

void set_sign(){

	lcd.begin(16, 2);
	
	lcd.createChar(0, sine1);
	lcd.createChar(1, sine2);
	lcd.createChar(2, tri1);
	lcd.createChar(3, tri2);
	lcd.createChar(4, sqr1);
	lcd.createChar(5, sqr2);
	lcd.createChar(6, saw1);
	lcd.createChar(7, saw2);

	// lcd.createChar(8, pointer);

}

void set_adc(){
	DDRC |= (1<<PC6);
	ADMUX = (1<<REFS0) | (1<<REFS1) | 6;
	ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2) | (1<<ADEN);
}
void adc_enable(uint8_t is_enabled){

	if (is_enabled) ADCSRA |= (1<<ADEN);
	else ADCSRA &= ~(1<<ADEN);

}
uint16_t adc_conversion(){
	uint16_t adc;
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	adc = ADC;
	return adc;
}

void update_batt() {
	const uint8_t NUMBER_REPEAT = 8;
	Batt_level = 0;
	adc_enable(1);	
	for (uint8_t i = 0; i < NUMBER_REPEAT; i++)
	{
		Batt_level += adc_conversion();
	}
	adc_enable(0);		
	Batt_level /= NUMBER_REPEAT; 

							//magic number of DAC:
	elem5_1.value = String(Batt_level*(1.1/1024.0)*(111.0/12.1)) + "V";
}

//===================================================================================
//=================================== SETUP =========================================
void setup() {
	// Serial.begin(9600);
	// Serial.println("Start");

	// noInterrupts();
	upgrade_lcd = &startup_dispay;
	get_wave = &get_sine;



	pinMode(TEST,OUTPUT);
	digitalWrite(TEST,HIGH);

	// set:
	set_sign();
	set_encoder();
	set_timer0();
	set_adc();

	// set generator:
	set_Timer1();
	enable_timer1(0);


	set_menu_list();

	// DAC:
	DDRD = 0b11111100;
	DDRB = 0b00000011;


	//battery indicator:
	update_batt();
	// interrupts();
}

//===================================================================================
//=================================== MAIN LOOP =====================================
void loop() {

	if(state == INIT){
		
		//first Step:
		if(flagMenu == 0){
			upgrade_lcd();
			flagMenu = 1;
		}

		if(Timer_state >= 1000){
			upgrade_lcd = &upgrade_menu;
			flagMenu = 0;
			state = MENU;
			Timer_state= 0;
		}


	}
	else if (state == MENU)
	{
		if(flagMenu == 0){
			upgrade_all = 1;
			upgrade_lcd();
			flagMenu = 1;
		}		

		if(Timer_state >= 3000){
			if(current_element->parent == &elem5){
				update_batt();
			}
			Timer_state= 0;
			upgrade_all = 1;

			if (!refres_pointer) upgrade_lcd();
			
		}

		check_encoder(&Encoder_blue);
	}
	else if(state == WAVE){
		// delay(1000);
		if(!digitalRead(Encoder_blue.pushPin)){
			play_back();
		}

		
	}

}

//============ INTERRUPTS ============
ISR(TIMER0_COMPA_vect){
	Timer_enc++;
	Timer_switch++;
	Timer_state++;
}


ISR(TIMER1_COMPA_vect,){
	// uint16_t a = Accumulator;
	Accumulator += Step;
	if (Accumulator < Max_sample);
	else Accumulator = (0);

	// a += Step;
	// if (a < Max_sample);
	// else a = (Max_sample-a);	

	// uint8_t sample = ;
	// Accumulator ^= 1;
	DAC((get_wave()*Amplitude)>>7);
	// digitalWrite(TEST,digitalRead(TEST)^1);
	// DAC(Accumulator*255);
}



