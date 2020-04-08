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

// VxWorks Libraries
#include "../VxWorks/vxWorks.h"
#include "../VxWorks/semLib.h"
#include "../VxWorks/taskLib.h"

// Project files
#include "../inc/config.h"
#include "../inc/cinterface.h"
#include "../inc/ui.h"

int shutdown = FALSE;
int debug = FALSE;

const char sideString[2][6] = {{"Right"}, {"Left"}};
const char gateString[4][15] = {{"Both open"}, {"Left closed"}, {"Right closed"}, {"Both closed"}};

// Structure used to hold counter values for both conveyors

SEM_ID test;

counter_t counters;


// Task function
void conveyor_sim(void);
int size_task(int side);
void count_task(int side);
void gate_task(int side);



void debug_printf(char *dbgMessage, ...);



/**
 * @brief main function for user interface simulation
 *
 */
int main(void)
{

  time_t t;

  //Initializes random number generator
  //Moved here as kept getting the same number generated
  srand((unsigned) time(&t));


  printf("Conveyor belt UI starting\n");


  //Default to top level menu
  conveyor_sim();

  while (shutdown == FALSE)
  {

    if(ui_task() == TRUE)
    {
      conveyor_sim();
    }
  }

  if(shutdown == TRUE)
  {
    return EXIT_SUCCESS;
  }
  else
  {
    return EXIT_FAILURE;
  }
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
    debug_printf("Small block detected on %s side\n", sideString[side]);
    counters.small[side]++;
    returnVal = SIZE_SMALL;
  }
  else if(sensorVal == SIZE_BIG)
  {
    debug_printf("Big block detected on %s side\n", sideString[side]);
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
    debug_printf("Block collected on %s side\n", sideString[side]);
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
  debug_printf("Gate state : %s\n", gateString[gateVal]);
  setGates(gateVal);
  //Wait for block to be pushed off
  sleep(GATE_CLOSE);
  //Open gates
  debug_printf("Gate state : %s\n", gateString[gateVal]);
  setGates(GATE_OPEN);
}
