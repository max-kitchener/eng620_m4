/*
 * ****************************************************************************
 * File           : ui.c
 * Project        : Real Time Embedded Systems Coursework
 *
 * Description    : Handles the user interface of the conveyor belt, prints
 *                  menus and read user inputs
 * ****************************************************************************
 * ChangeLog:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../inc/config.h"
#include "../inc/ui.h"




//Menu strings
const char uiMainMenu[UI_MAIN_ITEMS][UI_STRING_LENGTH] = {
  {"\n\nMain menu, choose an option (1-4):\n"},
  {"------------------------------------\n"},
  {"[1] Enter debug mode\n"},
  {"[2] Read counter value\n"},
  {"[3] Reset counter value\n"},
  {"[4] Shutdown\n"}
};

const char uiCounterMenu[UI_COUNTER_ITEMS][UI_STRING_LENGTH] = {
  {"\n\nWhich counter(s)? (1-4):\n"},
  {"------------------------------------\n"},
  {"[1] Small blocks\n"},
  {"[2] Big blocks\n"},
  {"[3] Collected blocks\n"},
  {"[4] All\n"}
};

const char uiConveyorMenu[UI_CONV_ITEMS][UI_STRING_LENGTH] = {
  {"\n\nWhich conveyor(s)? (1-3):\n"},
  {"------------------------------------\n"},
  {"[1] Right Conveyor\n"},
  {"[2] Left Conveyer\n"},
  {"[3] Both Conveyors\n"},
};

extern int shutdown;
extern int debug;
extern counter_t counters;

/**
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void ui_printf(const char menuArray[][UI_STRING_LENGTH], int numOptions)
{
  int menuItem;

  for (menuItem = 0; menuItem < numOptions; menuItem++)
  {
    printf("%s", menuArray[menuItem]);
  }
}


/**
 * @brief Handles user inputs for the main menu and returns the next menu level
 *
 * @param input user value
 * @return menu_t next menu level to display
 */
menu_t ui_main(int input)
{
  menu_t nxtMenu;
  switch (input)
  {
  case 1:
    nxtMenu = DEBUG;
    break;

  case 2:
    nxtMenu = COUNTERS;
    break;

  case 3:
    nxtMenu = RESET;
    break;

  case 4:
    nxtMenu = SHUTDOWN;
    break;

  default:
    printf("Invalid input\n");
    nxtMenu = TOP;
    break;
  }
  return nxtMenu;
}

/**
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void ui_counter(int ctr, int cnv)
{
  int side;
  for ( side = 0; side < 2; side++)
  {
    if(cnv & (side+1))
    {
      printf("%s conveyor:\n", sideString[side]);
      if(ctr == SMALL || ctr == ALL)
      {
        printf("%d small blocks\n", counters.small[side]);
      }
      if(ctr == BIG || ctr == ALL)
      {
        printf("%d big blocks\n", counters.big[side]);
      }
      if(ctr == COLLECTED || ctr == ALL)
      {
        printf("%d blocks collected\n", counters.collected[side]);
      }
      //sleep(1);
    }
  }
}


/**
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void ui_reset(int ctr, int cnv)
{
  int side;
  for ( side = 0; side < 2; side++)
  {
    if(cnv && (side+1))
    {
      printf("%s conveyor:\n", sideString[side]);
      if(ctr == SMALL || ctr == ALL)
      {
        counters.small[side]=0;
        printf("Reset small count\n");
      }
      if(ctr == BIG || ctr == ALL)
      {
        counters.big[side]=0;
        printf("Reset big count\n");
      }
      if(ctr == COLLECTED || ctr == ALL)
      {
        counters.collected[side]=0;
        printf("Reset collected count\n");
      }
      //sleep(1);
    }
  }
}


/**
 * @brief used to read user input from terminal
 *
 * @return int returns int value of user input
 */
int ui_input(void)
{
  int len = 3;
  int val = 0;
  char str[len];

  // Get user input
  fgets(str, len, stdin);
  // Convert to int
  sscanf(str, "%d\n", &val);

  return(val);
}
