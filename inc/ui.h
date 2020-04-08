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


int ui_task(void);
// User interface functions
void ui_printf(const char menuArray[][UI_STRING_LENGTH], int numOptions);
void ui_main(int menuSelect);
void ui_counter(int ctr, int cnv);
void ui_reset(int ctr, int cnv);
int  user_input(void);


#endif
