/*
 * GccApplication1.c
 *
 * Created: 5/2/2020 1:41:52 PM
 * Author : MrTrashCan
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"

typedef struct  
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int milisecond;
	int militarytime;
} datetime;

int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	
char characters[] = "X123A456B789C*0#D";
	
int interval = 250;

int get_key();
int is_pressed(int r, int c);
void increment(datetime *dt);
void display(datetime *dt);
void setDateTime(datetime *dt);
int decrementValue(int current, int limit);
int decrementTime(int current, int limit);

int main(void)
{
    /* Replace with your application code */
	int key;
	datetime dt = {2020,5,2,16,11,55,0,1};
	lcd_init();
    while (1) 
    {
		avr_wait(interval);
		key = get_key();
		if(characters[key] == '*')
		{
			dt.militarytime = 1 - dt.militarytime;
			lcd_clr();
			increment(&dt);
			display(&dt);
			avr_wait(interval);
		}
		if(characters[key] == '#')
		{
			setDateTime(&dt);
			lcd_clr();
		}
		increment(&dt);
		display(&dt);
    }
}

int get_key()
{
	for(int r = 0; r < 4; ++r)
		for(int c = 0; c < 4; ++c)
			if(is_pressed(r,c+4))
				return r*4+c+1;
	return 0;
}

int is_pressed(int r, int c)
{
	DDRC = 0b00000000;
	PORTC = 0b00000000;
	SET_BIT(DDRC, r);
	SET_BIT(PORTC, c);
	avr_wait(1);
	if (!GET_BIT(PINC, c))
		return 1;
	return 0;
}

void increment(datetime *dt)
{
	dt->milisecond += interval;
	if(dt->milisecond >= 1000)
	{
		dt->milisecond = 0;
		dt->second++;
		if(dt->second >= 60)
		{
			dt->second = 0;
			dt->minute++;
			if(dt->minute >= 60)
			{
				dt->minute = 0;
				dt->hour++;
				if(dt->hour >= 24)
				{
					dt->hour = 0;
					dt->day++;
					if(dt->day > days[(dt->month)-1])
					{
						if(dt->month == 2 && dt->day >= 29 && ((dt->year % 4 == 0 && dt->year % 100) || dt->year % 400 == 0))
							return;
						dt->day = 1;
						dt->month++;
						if(dt->month > 12)
						{
							dt->month = 1;
							dt->year++;
						}
					}
				}
			}
		}
	}
}

void display(datetime *dt)
{
	char buf[11] = {0};
	char AM[] = "AM", PM[] = "PM";
	int h = dt->hour;
	sprintf(buf, "%02d/%02d/%04d", dt->month, dt->day, dt->year);
	lcd_pos(0,3);
	lcd_puts(buf);
	lcd_pos(1,4);
	if(dt->militarytime)
		sprintf(buf, "%02d:%02d:%02d", dt->hour, dt->minute, dt->second);
	else
	{
		if(h < 12)
		{
			if(!h)
				h = 12;
			sprintf(buf, "%02d:%02d:%02d%s", h, dt->minute, dt->second, AM);
		}
		else
		{
			if(h != 12)
			{
				h = h % 12;
			}
			sprintf(buf, "%02d:%02d:%02d%s", h, dt->minute, dt->second, PM);
		}
		lcd_pos(1,3);
	}
	lcd_puts(buf);
}

void setDateTime(datetime *dt)
{
	lcd_clr();
	lcd_pos(0,0);
	char buf[10] = {0};
	int key;
	datetime newdt = {dt->year,dt->month, dt->day, dt->hour, dt->minute, dt->second, 0, dt->militarytime};
	while(1)
	{
		avr_wait(interval);
		increment(dt);
		sprintf(buf, "Year:%04d", newdt.year);
		lcd_puts(buf);
		lcd_pos(0,0);
		key = get_key();
		if(characters[key] == 'A') //Increment
			newdt.year += 1;
		else if(characters[key] == 'B') //Decrement
			newdt.year -= 1;
		else if(characters[key] == 'C') //Continue to month
		{
			lcd_clr();
			while(1)
			{
				avr_wait(interval);
				increment(dt);
				sprintf(buf, "Month:%02d", newdt.month);
				lcd_puts(buf);
				lcd_pos(0,0);
				key = get_key();
				if(characters[key] == 'A') //Increment
					newdt.month = (newdt.month%12)+1;
				else if(characters[key] == 'B') //Decrement
					newdt.month = decrementValue(newdt.month, 12);
				else if(characters[key] == 'C') //Continue to day
				{
					int numOfDay;
					if(newdt.month == 2 && ((newdt.year % 4 == 0 && newdt.year % 100) || newdt.year % 400 == 0))
						numOfDay = 29;
					else
						numOfDay = days[newdt.month-1];
					lcd_clr();
					while(1)
					{
						avr_wait(interval);
						increment(dt);
						sprintf(buf, "Day:%02d", newdt.day);
						lcd_puts(buf);
						lcd_pos(0,0);
						key = get_key();
						if(characters[key] == 'A') //Increment
							newdt.day = (newdt.day%numOfDay)+1;
						else if(characters[key] == 'B') //Decrement
							newdt.day = decrementValue(newdt.day, numOfDay);
						else if(characters[key] == 'C') //Continue to hour
						{
							lcd_clr();
							while(1)
							{
								avr_wait(interval);
								increment(dt);
								sprintf(buf, "Hour:%02d", newdt.hour);
								lcd_puts(buf);
								lcd_pos(0,0);
								key = get_key();
								if(characters[key] == 'A') //Increment
									newdt.hour = (newdt.hour+1)%24;
								else if(characters[key] == 'B') //Decrement
									newdt.hour = decrementTime(newdt.hour, 24);
								else if(characters[key] == 'C') //Continue to minute
								{
									lcd_clr();
									while(1)
									{
										avr_wait(interval);
										increment(dt);
										sprintf(buf, "Minute:%02d", newdt.minute);
										lcd_puts(buf);
										lcd_pos(0,0);
										key = get_key();
										if(characters[key] == 'A') //Increment
											newdt.minute = (newdt.minute+1)%60;
										else if(characters[key] == 'B') //Decrement
											newdt.minute = decrementTime(newdt.minute, 60);
										else if(characters[key] == 'C') //Continue to second
										{
											lcd_clr();
											while(1)
											{
												avr_wait(interval);
												increment(dt);
												sprintf(buf, "Second:%02d", newdt.second);
												lcd_puts(buf);
												lcd_pos(0,0);
												key = get_key();
												if(characters[key] == 'A') //Increment
													newdt.second = (newdt.second+1)%60;
												else if(characters[key] == 'B') //Decrement
													newdt.second = decrementTime(newdt.second, 60);
												else if(characters[key] == 'C') //Confirm
												{
													dt->year = newdt.year;
													dt->month = newdt.month;
													dt->day = newdt.day;
													dt->hour = newdt.hour;
													dt->minute = newdt.minute;
													dt->second = newdt.second;
													dt->milisecond = 0;
													dt->militarytime = newdt.militarytime;
													return;
												}
												else if(characters[key] == 'D') //Cancel second
												{
													newdt.second = 0;
													lcd_clr();
													break;
												}
											}
										}
										else if(characters[key] == 'D') //Cancel minute
										{
											newdt.minute = 0;
											lcd_clr();
											break;
										}
									}
								}
								else if(characters[key] == 'D') //Cancel hour
								{
									newdt.hour = 12;
									lcd_clr();
									break;
								}
							}
						}
						else if(characters[key] == 'D') //Cancel day
						{
							newdt.day = 1;
							lcd_clr();
							break;
						}
					}
				}
				else if(characters[key] == 'D') //Cancel month
				{
					newdt.month = 1;
					lcd_clr();
					break;
				}
			}
		}
		else if(characters[key] == 'D') //Cancel year
			return;
	}
}

int decrementValue(int current, int limit)
{
	if(current - 1 == 0)
		return limit;
	return current - 1;
}

int decrementTime(int current, int limit)
{
	if(current - 1 < 0)
		return limit - 1;
	return current - 1;
}