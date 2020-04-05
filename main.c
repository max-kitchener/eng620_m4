/*
 * ****************************************************************************
 * File           :       main.c
 * Project        :       Real Time Embedded systems milestone 3
 *
 * Description    :       Simulates the user interface for the conveyor belt
 * ****************************************************************************
 */

// Standard C libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
// Project files
#include "config.h"
#include "cinterface.h"

static int shutdown = FALSE;
static int debug = FALSE;

static int motor;
static int gate;
static int size[2];
static int count[2];


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

// Structure used to hold counter values for both conveyors
struct
{
  int big[2];
  int small[2];
  int collected[2];
} counters;


// User interface functions
void menu_printf(char menuArray[][UI_STRING_LENGTH], int numOptions);
void main_menu(int menuSelect);
void counter_menu(int ctr, int cnv);
void reset_menu(int ctr, int cnv);
int  user_input(void);

// Task function
void conveyor_sim(void);
int size_task(int side);
void count_task(int side);
void gate_task(int side);



void debug_printf(char *dbgMessage, ...);
int random(void);


/**
 * @brief main function for user interface simulation
 *
 */
void main(void)
{
  char string[20] = {0};
  int menuSelect;
  int counters;
  int conveyors;
  time_t t;

  //Initializes random number generator
  //Moved here as kept getting the same number generated
  srand((unsigned) time(&t));


  printf("Conveyor belt UI starting\n");


  //Default to top level menu
  menuLevel = TOP;

  while (shutdown == FALSE)
  {
    //state machine dependant on menu level
    switch (menuLevel)
    {
    //Top level of menu, should default to here
    case TOP:

      conveyor_sim();

      menu_printf(uiMainMenu, UI_MAIN_ITEMS);

      main_menu(user_input());
      break;

    //User has requested counter values
    case COUNTERS:
      menu_printf(uiCounterMenu, UI_MAIN_ITEMS);
      counters = user_input();
      printf("%d\n", counters);
      menuLevel = COUNTERS_CONV;
      break;

    //Decides which conveyor to print value for
    case COUNTERS_CONV:
      menu_printf(uiConveyorMenu, UI_MAIN_ITEMS);
      conveyors = user_input();
      printf("%d\n", conveyors);
      counter_menu(counters, conveyors);
      menuLevel = TOP;
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

    case RESET:
      menu_printf(uiCounterMenu, UI_MAIN_ITEMS);
      counters = user_input();
      menuLevel = RESET_CONV;
      break;

    case RESET_CONV:
      menu_printf(uiConveyorMenu, UI_MAIN_ITEMS);
      conveyors = user_input();
      reset_menu(counters, conveyors);
      menuLevel = TOP;
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
  }
}


/**
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void menu_printf(char menuArray[][UI_STRING_LENGTH], int numOptions)
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
void main_menu(int menuSelect)
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
void counter_menu(int ctr, int cnv)
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
    }
  }
}


/**
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void reset_menu(int ctr, int cnv)
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

/**
 * @brief Same functionality as printf, but will only
 *        print when debugMode is TRUE
 *
 * @param dbgMessage string for printf message
 * @param ...        variable list of arguments fro dbgMessage
 */
void debug_printf(char *dbgMessage, ...)
{
  if (debug == TRUE)
  {
    va_list Args;
    va_start(Args, dbgMessage);

    vprintf(dbgMessage, Args);
  }
}


/**
 * @brief simulates conveyor belt interface, uses random numbers to decide
 *
 */
void conveyor_sim (void)
{
  int size;
  int side = LEFT;
  int numBlocks = 10;
  // Allow up to ten blocks to be placed on conveyor
  for (int i = 0; i < numBlocks; i++)
  {
    size = 0;
    //If odd block is on left conveyor
    side = LEFT;

    //If even block is on RIGHT conveyor
    if(i % 2 == 0)
    {
      side = RIGHT;
    }

    //Detect the size of the block
    size = size_task(side);

    //Process block according to size
    if(size == SIZE_BIG)
    {
      //Wait for block to reach count sensor
      //sleep(COUNT_DELAY);
      //Count collected block
      count_task(side);
    }
    else if( size == SIZE_SMALL)
    {
      //Wait for block to reach gate
      //sleep(GATE_DELAY);
      //Close the gate for a period then open again
      gate_task(side);
    }
    //sleep(2);
  }
}

/**
 * @brief simulates polling side sensors for block detection
 *
 * @param side  - which conveyor to check
 * @return int  - SIZE_NONE, SIZE_SMALL or SIZE_BIG
 */
int size_task(int side)
{
  int sensorVal;
  int returnVal = 0;

  //Read and reset size sensors
  sensorVal = readSizeSensors(side);
  resetSizeSensors(side);

  //Increment counters depending on size of block
  if(sensorVal == SIZE_SMALL)
  {
    counters.small[side]++;
    returnVal = SIZE_SMALL;
  }
  else if(sensorVal == SIZE_BIG)
  {
    counters.big[side]++;
    returnVal = SIZE_BIG;
  }
  return(returnVal);
}

/**
 * @brief Simulates the count sensors for counting collected blocks
 *
 * @param side - which conveyor to check, LEFT or RIGHT
 */
void count_task(int side)
{
  int sensorVal;

  // Read and reset sensors
  sensorVal = readCountSensor(side);
  resetCountSensor(side);

  // Increment collected count when a block is detected
  if(sensorVal == COUNT_BLOCK)
  {
    counters.collected[side]++;
  }
}

/**
 * @brief simulates gate functionality for sorting blocks
 *
 * @param side - Which conveyor to sort, LEFT or RIGHT
 */
void gate_task(int side)
{
  int gateVal;

  // Set gate to close to match side
  if(side == LEFT)
  {
    gateVal = GATE_CLOSED_L;
  }
  else if(side == RIGHT)
  {
    gateVal = GATE_CLOSED_R;
  }

  // close gate(s)
  setGates(gateVal);
  //Wait for block to be pushed off
  sleep(GATE_CLOSE);
  //Open gates
  setGates(GATE_OPEN);
}


/**
 * @brief Generates a random number and returns it.
 *
 * @return int random number from 0-10
 *
 */
int random(void)
{
  // Calculate random number from 0-10
  int input = rand() % 10;
  //debug_printf("%d", input);

  return(input);
}


/* C INTERFACE FUNCTIONS*/

/**
 * @brief simulates reading size sensors by using random numbers to decide which
 *        size of block is detected (if any)
 *
 * @param conveyor - Which conveyor to check, LEFT or RIGHT
 *
 * @return char - SIZE_NONE, SIZE_SMALL, SIZE_BIG
 */
char readSizeSensors(char conveyor)
{
  int randomInt = random();

  // Big block detected
  if ((randomInt <= 8) && (randomInt % 2 == 0))
  {
    debug_printf("Big block detected on %s side\n", sideString[conveyor]);
    size[conveyor] = SIZE_BIG;
  }
  // Small block detected
  else if ((randomInt <= 8) && (randomInt % 2 != 0) )
  {
    debug_printf("Small block detected on %s side\n", sideString[conveyor]);
    size[conveyor] = SIZE_SMALL;
  }

  return (size[conveyor]);
}

/**
 * @brief simulates count sensor function, will always return COUNT_BLOCK
 *
 * @param conveyor - Which conveyor to check
 * @return char - COUNT_BLOCK, COUNT_NONE
 */
char readCountSensor(char conveyor)
{
  int randomInt = random();
  count[conveyor] = COUNT_BLOCK;
  // Left conveyor, big block
  debug_printf("Block collected on %s side\n", sideString[conveyor]);
  return(count[conveyor]);

}

/**
 * @brief
 *
 * @param conveyor
 */
void resetSizeSensors(char conveyor)
{
  size[conveyor] = SIZE_NONE;
}

/**
 * @brief
 *
 * @param conveyor
 */
void resetCountSensor(char conveyor)
{
  count[conveyor] = COUNT_NONE;
}

/**
 * @brief Controls opening and closing of the gates on both sides
 *
 * @param state - GATE_OPEN, GATE_CLOSED_L, GATE_CLOSED_R, GATE_CLOSED_ALL
 */
void setGates(char state)
{
  gate = state;
  debug_printf("Gate state : %s\n", gateString[state]);
}

/**
 * @brief Starts the motors driving the conveyor belts
 *
 */
void startMotor(void)
{
  motor = MOTOR_ON;
}

/**
 * @brief stops the motors driving the conveyor belts
 *
 */
void stopMotor(void)
{
  motor = MOTOR_OFF;
}
