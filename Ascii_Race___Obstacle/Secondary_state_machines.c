#include <avr/io.h>
#include "usart.h"
#include "io.c"
#include "timer.h"
#include <stdio.h>
#include <string.h>

unsigned char game_time = 0;
unsigned char timer_game[3];
unsigned char timer_game_1[2];
unsigned char score_str[2];

unsigned char hearts[] = {10,21,21,17,17,10,4};

enum heart_states{init_0, st_1, st_2}heart_state;

void heart_task(){
	unsigned char score = (PINB & 0x07);
	switch(heart_state){
		case init_0:
			heart_state = st_1;
			break;
		case st_1:
			if(score >= 3){
				heart_state = st_1;
			}
			else{
				heart_state = st_2;
			}
			break;
		case st_2:
			heart_state = st_1;
			break;
		default:
			heart_state = init_0;
			break;
	}
	
	switch(heart_state){
		case st_1:
			break;
		case st_2:
			switch(score){
				case 1:
					LCD_Cursor(25);
					LCD_WriteData(0x20);
					LCD_Cursor(26);
					LCD_WriteData(0x20);
					LCD_Cursor(30);
					break;
				case 2:
					LCD_Cursor(25);
					LCD_WriteData(0x20);
					LCD_Cursor(30);
					break;
				case 0:
					LCD_Cursor(25);
					LCD_WriteData(0x20);
					LCD_Cursor(26);
					LCD_WriteData(0x20);
					LCD_Cursor(27);
					LCD_WriteData(0x20);
					LCD_Cursor(30);
					break;
				case 3:
					break;
				default:
					break;
			}
			break;
		default:
			break;
			
	}
}

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	DDRA = 0xC0; PORTA = 0x3F;
	DDRB = 0x00; PORTB = 0xFF;
	
	TimerSet(50);
	TimerOn();
	initUSART();
	LCD_init();
	snprintf(timer_game, sizeof(timer_game), "%d", game_time);
	LCD_DisplayString(1, " Time:");
	LCD_DisplayString_new(8, timer_game);
	LCD_DisplayString_new(17, " Lives: ");//position 9
	LCD_CreateChar(hearts, 1);
	LCD_Cursor(25);
	LCD_WriteData(0x09);
	LCD_Cursor(26);
	LCD_WriteData(0x09);
	LCD_Cursor(27);
	LCD_WriteData(0x09);
	LCD_Cursor(30);
	
	unsigned long etime0 = 0;
	unsigned long etime1 = 50;
	
    while(1)
    {							
			if(etime0 >= 1000){		
				if(USART_HasReceived()){
					game_time = USART_Receive();
					USART_Flush();
				}
				if(game_time < 10){
					snprintf(timer_game_1, sizeof(timer_game_1), "%d", game_time);
					LCD_Cursor(8);
					LCD_WriteData(0x20);
					LCD_DisplayString_new(9, timer_game_1);
				}
				else{
					snprintf(timer_game, sizeof(timer_game), "%d", game_time);
					LCD_DisplayString_new(8, timer_game);
				}
				LCD_Cursor(15);
				etime0 = 0;		
			}
			if(etime1 >= 50){				
				heart_task();
				etime1 = 0;
			}	
					
			while(!TimerFlag);
			TimerFlag = 0;
			etime0 += 50;
			etime1 += 50;
    }
}