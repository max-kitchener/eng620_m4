/*
 * ****************************************************************************
 * File           : config.h
 * Project        : Real Time Embedded Systems Coursework
 *
 * Description    : Configs used for User Interface menus
 * ****************************************************************************
 * ChangeLog:
 */

#ifndef CONFIG_H
#define CONFIG_H

/*DEFINITIONS*/
/*DELAYS, TIMING ETC */
#define CLOCK_RATE 200
#define TASK_DELAY 4
#define GATE_DELAY 2.5
#define GATE_CLOSE 1.7
#define COUNT_DELAY 4

#define GATE_TIM_NUM 20
#define COUNT_TIM_NUM 20

#ifndef FALSE
  #define FALSE 0
  #define TRUE !FALSE
#endif

//#define RAND_MAX 10

#define LEFT 1
#define RIGHT 0

#define BIG 1
#define SMALL 2
#define COLLECTED 3
#define ALL 4

#define MOTOR_OFF 0
#define MOTOR_ON  1

#define GATE_OPEN         0
#define GATE_CLOSED_L     1
#define GATE_CLOSED_R     2
#define GATE_CLOSED_BOTH  3

#define SIZE_NONE  0
#define SIZE_SMALL 1
#define SIZE_BIG   3

#define COUNT_NONE  0
#define COUNT_BLOCK 1


extern const char sideString[2][6];

typedef struct
{
  int big[2];
  int small[2];
  int collected[2];
}counter_t;

extern counter_t counter;

/* USER INTERFACE */
#define UI_STRING_LENGTH 50
#define UI_MAIN_ITEMS    6
#define UI_COUNTER_ITEMS 6
#define UI_CONV_ITEMS    5


#endif
