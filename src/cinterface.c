/*
 * ****************************************************************************
 * File           : cinterface.c
 * Project        : Real Time Embedded Systems Coursework
 *
 * Description    :
 * ****************************************************************************
 * ChangeLog:
 */


/* SECTION Includes ---------------------------------------------------------*/
//Standard C Libraries
#include <time.h>
#include <stdlib.h>

//Project Header Files
#include "../inc/config.h"
#include "../inc/cinterface.h"
/* !SECTION Includes */


/* SECTION Variable Declarations --------------------------------------------*/

/* SECTION Global Variables -------------------------------------------------*/

/* !SECTION Global Variables */

/* SECTION Local Variables --------------------------------------------------*/
static int motor;
static int gate;
static int size[2];
static int count[2];



// Function Decleration
int gen_random(void);


// Global functions

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
  // Removes warning messages
  int conv = conveyor;

  int randomInt = gen_random();

  // Big block detected
  if ((randomInt <= 8) && (randomInt % 2 == 0))
  {
    size[conv] = SIZE_BIG;
  }
  // Small block detected
  else if ((randomInt <= 8) && (randomInt % 2 != 0) )
  {
    size[conv] = SIZE_SMALL;
  }

  return (size[conv]);
}

/**
 * @brief simulates count sensor function, will always return COUNT_BLOCK
 *
 * @param conveyor - Which conveyor to check
 * @return char - COUNT_BLOCK, COUNT_NONE
 */
char readCountSensor(char conveyor)
{
  // To remove warnings
  int conv = conveyor;
  //int randomInt = gen_random();
  count[conv] = COUNT_BLOCK;
  // Left conveyor, big block
  return(count[conv]);

}

/**
 * @brief
 *
 * @param conveyor
 */
void resetSizeSensors(char conveyor)
{
  // To remove warnings
  int conv = conveyor;

  size[conv] = SIZE_NONE;
}

/**
 * @brief
 *
 * @param conveyor
 */
void resetCountSensor(char conveyor)
{
  // To remove warnings
  int conv = conveyor;

  count[conv] = COUNT_NONE;
}

/**
 * @brief Controls opening and closing of the gates on both sides
 *
 * @param state - GATE_OPEN, GATE_CLOSED_L, GATE_CLOSED_R, GATE_CLOSED_ALL
 */
void setGates(char state)
{
  // To remove warnings
  int stt = state;

  gate = stt;

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

// Local functions

/**
 * @brief Generates a random number and returns it.
 *
 * @return int random number from 0-10
 *
 */
int gen_random(void)
{
  // Calculate random number from 0-10
  int input = rand() % 10;
  //debug_printf("%d", input);

  return(input);
}
