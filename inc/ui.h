/*
 * ****************************************************************************
 * File           :       ui.h
 * Project        :       Real Time Embedded Systems Coursework
 *
 * Description    :       Header file for ui.c, which handles the user interface
 *                        of the conveyor belt
 * ****************************************************************************
 * ChangeLog:
 */




#ifndef UI_H
#define UI_H



/* USER INTERFACE */
#define UI_STRING_LENGTH 50
#define UI_MAIN_ITEMS    6
#define UI_COUNTER_ITEMS 6
#define UI_CONV_ITEMS    5


#define SMALL 1
#define BIG 2
#define COLLECTED 3
#define ALL 4


//Used for indicating which menu is active
typedef enum
{
  TOP,
  COUNTERS,
  COUNTERS_CONV,
  RESET,
  RESET_CONV,
  SHUTDOWN,
  DEBUG
} menu_t;

extern const char uiMainMenu[UI_MAIN_ITEMS][UI_STRING_LENGTH];
extern const char uiCounterMenu[UI_COUNTER_ITEMS][UI_STRING_LENGTH];
extern const char uiConveyorMenu[UI_CONV_ITEMS][UI_STRING_LENGTH];

// User interface functions
void ui_printf(const char menuArray[][UI_STRING_LENGTH], int numOptions);
menu_t ui_main(int menuSelect);
void ui_counter(int ctr, int cnv);
void ui_reset(int ctr, int cnv);
int  ui_input(void);


#endif
