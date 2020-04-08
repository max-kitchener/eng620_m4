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
  {"[1] Left Conveyer\n"},
  {"[2] Right Conveyor\n"},
  {"[3] Both Conveyors\n"},
};

//Used for indicating which menu is active
enum
{
  TOP,
  COUNTERS,
  COUNTERS_CONV,
  RESET,
  RESET_CONV,
  SHUTDOWN,
  DEBUG
} menuLevel;

extern int shutdown;
extern int debug;
extern counter_t counters;

int ui_task(void)
{
  static int ctr;
  static int cnv;

    //state machine dependant on menu level
    switch (menuLevel)
    {
    //Top level of menu, should default to here
    case TOP:

      ui_printf(uiMainMenu, UI_MAIN_ITEMS);

      ui_main(user_input());
      break;

    //Decides which conveyor to print value for
    case DEBUG:
      if (debug == TRUE)
      {
        debug = FALSE;
        printf("\nExiting debug mode\n");
      }
      else
      {
        debug = TRUE;
        printf("\nEntering debug mode\n");
      }
      menuLevel = TOP;
      break;
    
    //User has requested counter values
    case COUNTERS:
      ui_printf(uiCounterMenu, UI_MAIN_ITEMS);
      ctr = user_input();
      menuLevel = COUNTERS_CONV;
      break;

    //Decides which conveyor to print value for
    case COUNTERS_CONV:
      ui_printf(uiConveyorMenu, UI_MAIN_ITEMS);
      cnv = user_input();
      ui_counter(ctr, cnv);
      menuLevel = TOP;
      sleep(3);
      break;

    case RESET:
      ui_printf(uiCounterMenu, UI_MAIN_ITEMS);
      ctr = user_input();
      menuLevel = RESET_CONV;
      break;

    case RESET_CONV:
      ui_printf(uiConveyorMenu, UI_MAIN_ITEMS);
      cnv = user_input();
      ui_reset(ctr, cnv);
      menuLevel = TOP;
      sleep(3);
      break;

    case SHUTDOWN:
      printf("Shutting down\n");
      shutdown = TRUE;
      break;

    default:
      printf("Invalid input\n");
      menuLevel = TOP;
      break;
    }
    if(menuLevel == TOP)
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
}




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
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void ui_main(int menuSelect)
{
  switch (menuSelect)
  {
  case 1:
    menuLevel = DEBUG;
    break;

  case 2:
    menuLevel = COUNTERS;
    break;

  case 3:
    menuLevel = RESET;
    break;

  case 4:
    menuLevel = SHUTDOWN;
    break;

  default:
    printf("Invalid input\n");
    menuLevel = TOP;
    break;
  }
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
    if(cnv && (side+1))
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
      sleep(1);
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
      sleep(1);
    }
  }
}


/**
 * @brief used to read user input from terminal
 *
 * @return int returns int value of user input
 */
int user_input(void)
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
