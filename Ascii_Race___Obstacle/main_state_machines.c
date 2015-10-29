#include <avr/io.h>
#include <avr/interrupt.h>
#include "bit.h"
#include "scheduler.h"
#include "io.c"
#include "timer.h"
#include "usart.h"

enum move_states{init, check, mv_forward, mv_back, mv_up, mv_down, fall_forward}move_state;
unsigned char ind = 1;
unsigned char row = 0;

unsigned char player[] = {14,14,4,31,4,14,10,10};
unsigned char grenade[] = {4,14,27,27,27,27,14,4};

void move_task(){
	unsigned char tmp = (~PINA & 0x1F);
	
	switch(move_state){//transitions
		case init:
			move_state = check;
			break;
		
		case check:
			if(tmp == 0x08){
				LCD_Cursor(row*16 + ind);
				LCD_WriteData(0x20);
				if(ind >= 1 && ind < 16)
					ind++;
				move_state = mv_forward;
			}
			else if(tmp == 0x04){
				LCD_Cursor(row*16 + ind);
				LCD_WriteData(0x20);
				if(ind > 1 && ind <= 16)
					ind--;
				move_state = mv_back;
			}
			else if(tmp == 0x10){
				LCD_Cursor(row*16 + ind);
				LCD_WriteData(0x20);
				if(row == 0) row = 1;
				move_state = mv_down;
			}
			else if(tmp == 0x02){
				LCD_Cursor(row*16 + ind);
				LCD_WriteData(0x20);
				if(row == 1) row = 0;
				move_state = mv_up;
			}
			else{
				move_state = check;
			}
			break;
		
		case mv_back:
			if(tmp == 0x04){
				move_state = fall_forward;
			}
			else{
				move_state = check;
			}
			break;
		
		case mv_down:
			if(tmp == 0x10){
				move_state = fall_forward;
			}
			else{
				move_state = check;
			}
			break;
		
		case mv_up:
			if(tmp == 0x02){
				move_state = fall_forward;
			}
			else{
				move_state = check;
			}
			break;
			
		case mv_forward:
			if(tmp == 0x08){
				move_state = fall_forward;
			}
			else{
				move_state = check;
			}
			break;
		
		case fall_forward:
			if(tmp == 0x00){
				move_state = check;
			}
			else{
				move_state = fall_forward;
			}
			break;
		
		default:
			move_state = init;
			break;
	}
	
	switch(move_state){//actions
		case check:
		case fall_forward:
			break;
		
		case mv_forward:
		case mv_back:
		case mv_down:
		case mv_up:
			LCD_CreateChar(player, 1);
			LCD_Cursor(row*16 + ind);
			LCD_WriteData(0x09);
			break;
			
		default:
			break;
	}
}

enum shoot_states{init_1, wait_A3, shoot} shoot_state;
unsigned char cnt = 0;
void shoot_task(){
	unsigned char tmp = (~PINA & 0x01);
	switch(shoot_state){
		case init_1:
			shoot_state = wait_A3;
			break;
			
		case wait_A3:
			if(tmp == 0x01){
				cnt = ind+1;
				shoot_state = shoot;
			}
			else{
				shoot_state = wait_A3;
			}
			break;
			
		case shoot:
			if(cnt < 16){
				if((cnt-1)!= ind){
					LCD_Cursor(row*16 + cnt-1);
					LCD_WriteData(0x20);
				}
				shoot_state = shoot;
			}
			else shoot_state = wait_A3;
			break;
		
		default:
			shoot_state = init_1;
			break;
	}
	
	switch(shoot_state){
		case wait_A3:
			break;
		
		case shoot:
			if(ind < 16){
				LCD_Cursor(row*16 + cnt);
				if(cnt == 16 || cnt == 15) LCD_WriteData(0x20);
				else LCD_WriteData(0x7E);
				cnt++;
			}			
			break;
		
		default:
			break;
			
	}
}

unsigned char en_pos = 16;
enum enemy_states {init_2, mv_en_1, mv_en_2, rst_pos_1, rst_pos_2} enemy_state;

void enemy_task(){
	switch(enemy_state){
		case init_2:
			enemy_state = mv_en_1;
			break;
		
		case mv_en_1:
			if(en_pos>0 && row==0){
				enemy_state = mv_en_1;
			}
			else if(en_pos == 0){
				enemy_state = rst_pos_1;
			}
			else if(row == 1){
				enemy_state = mv_en_2;
				en_pos = 16;
			}
			break;
		
		case mv_en_2:
			if(en_pos>0 && row==1){
				enemy_state = mv_en_2;
			}
			else if(en_pos == 0){
				enemy_state = rst_pos_2;
			}
			else if(row == 0){
				enemy_state = mv_en_1;
				en_pos = 16;
			}
			break;
		
		case rst_pos_1:
			enemy_state = mv_en_1;
			break;
		
		case rst_pos_2:
			enemy_state = mv_en_2;
			break;
		
		default:
			enemy_state = init_2;
			break;
	}
	
	switch(enemy_state){
		case mv_en_1:
		case  mv_en_2:
			LCD_CreateChar(grenade, 2);
			LCD_Cursor(row*16 + en_pos);
			LCD_WriteData(0x0A);
			en_pos--;
			break;
		
		case rst_pos_1:
		case rst_pos_2:
			en_pos = 16;
			break;
		
		default:
			break;
	}
}

void kill_task(){
	LCD_Cursor(row*16 + cnt + 1);
	LCD_WriteData(0x20);
}

unsigned char score = 7;
void score_task(){
	if(ind == en_pos && score > 0){
		score--;
	}
}

unsigned char start_game = 0;
unsigned char end_game = 0;
unsigned char egm = 1;
const unsigned char* w_msg_1 = "  Race-Obstacle by T.M.S V.0.1";
enum welcome_states{init_5, welcome, idle, wait_egm}wl_state;
	
void welcome_task(){
	unsigned char tmp = (~PINA & 0x01);
	switch(wl_state){//transitions
		case init_5:
			LCD_DisplayString(1, w_msg_1);
			wl_state = welcome;
			break;
		
		case welcome:
			if(tmp == 0x01){				
				start_game = 1;	
				LCD_ClearScreen();
				wl_state = idle;			
			}
			else{
				wl_state = welcome;
			}			
			break;
		
		case idle:
			if(end_game == 0){
				wl_state = idle;
			}
			else if(end_game == 1){
				wl_state = wait_egm;
			}			
			break;
		
		case wait_egm:
			if(egm == 0){
				wl_state = wait_egm;
			}
			else if(egm == 1){
				wl_state = init_5;
			}
			break;
			
		default:
			wl_state = init_5;
			break;
	}
	
	switch(wl_state){//actions
		case welcome:
			egm = 0;
			end_game = 0;
			break;
					
		case idle:
		case wait_egm:
			break;
		
		default:
			break;
	}
}

unsigned char decision = 0;
unsigned char time = 60;

void decision_task(){
	if(score >= 0 && time >= 0 && ind == 16){
		decision = 1;//win scenario
	}
	else if(score == 0 && time >= 0 && ind < 16){
		decision = 2;//lose scenario
	}
	else if(score >= 0 && time == 0 && ind < 16){
		decision = 3;//time out decision
	}
}

enum finish_states{init_6, wait_decision, write_msg, Idle, reset_game}finish_state;
const unsigned char* lose = "  ** You Lost **";
const unsigned char* win =  "  ** You Won! **";
const unsigned char* time_out =  "  ** Time out! ***";

void finish_task(){
	unsigned char tmp = (~PINA & 0x01);
		switch(finish_state){//transitions
			case init_6:
				finish_state = wait_decision;
				break;
				
			case wait_decision:
				if(decision == 0){
					finish_state = wait_decision;
				}
				else{
					finish_state = write_msg;
				}
				break;
				
			case write_msg:
				finish_state = Idle;
				break;
			
			case Idle:
				if(tmp == 0x01){
					finish_state = reset_game;
				}
				else{
					finish_state = Idle;
				}
				break;
			
			case reset_game:
				finish_state = wait_decision;
				break;
				
			default:
				finish_state = init_6;
				break;
		}
		
		switch(finish_state){//actions
			case wait_decision:
				break;
			
			case write_msg:
				if(decision == 1){
					LCD_ClearScreen();
					LCD_DisplayString(1, win);
					time = 0;
				}
				else if(decision == 2){
					LCD_ClearScreen();
					LCD_DisplayString(1, lose);
					time = 0;
				}
				else if(decision == 3){
					LCD_ClearScreen();
					LCD_DisplayString(1, lose);
					LCD_DisplayString_new(17, time_out);
					time = 0;
				}
				break;
			
			case Idle:
				break;
			
			case reset_game:
				decision = 0;
				start_game = 0;
				end_game = 0;
				egm = 1;
				ind = 1;
				en_pos = 16;
				score = 7;
				time = 60;
				move_state = init;
				wl_state = init_5;
				break;
			
			default:
				break;
		}
}

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;//Data lines
	DDRA = 0xC0; PORTA = 0x3F;//Control Lines .... 0xC0 and 0x3F
	DDRB = 0xFF; PORTB = 0x00;
	
	move_state = init;
	shoot_state = init_1;
	enemy_state = init_2;
	wl_state = init_5;
	finish_state = init_6;
	
	unsigned long etime1 = 100;
	unsigned long etime2 = 50;
	unsigned long etime3 = 500;
	unsigned long etime4 = 10;
	unsigned long etime5 = 50;
	unsigned long etime6 = 50;
	unsigned long etime7 = 150;
	unsigned long etime8 = 1000;
	unsigned long etime9 = 150;
	
	TimerSet(10);
	TimerOn();
	
	initUSART();
	LCD_init();
	LCD_CreateChar(player, 1);
	LCD_Cursor(1);
	LCD_WriteData(0x09);
	
	while(1)
	{	
		if(etime8 >= 1000 && start_game == 1){
			if(time > 0) time--;
			if(USART_IsSendReady())
				USART_Send(time);
			etime8 = 0;
		}
		if(etime6 >= 50){
			welcome_task();
			if(start_game == 0){
					etime1 = 0;
					etime2 = 0;
					etime3 = 0;
					etime4 = 0;
					etime5 = 0;
					etime7 = 0;
					etime6 = 0;
					etime8 = 0;
					etime9 = 0;
			}			
		}
		
		if(etime7 >= 150){
			decision_task();
			etime7 = 0;
		} 	
			
		if(etime1 >= 100 && decision == 0){
			move_task();
			etime1 = 0;
		}
		
		if(etime2 >= 50 && decision == 0){
			shoot_task();
			etime2 = 0;
		}
		
		if(etime3 >= 500 && decision == 0){
			enemy_task();
			etime3 = 0;
		}
		
		if(etime4 >= 10 && decision == 0){
			kill_task();
			etime4 = 0;
		}
		
		if(etime5 >= 50 && decision == 0){
			score_task();
			etime5 = 0;
		}
		
		if(etime9 >= 150){
			finish_task();
			etime9 = 0;
		}
		while(!TimerFlag);
		TimerFlag = 0;
		etime1 += 10;
		etime2 += 10;
		etime3 += 10;
		etime4 += 10;
		etime5 += 10;
		etime6 += 10;
		etime7 += 10;
		etime8 += 10;
		etime9 += 10;
	}
	return 0;
}
