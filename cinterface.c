/*
 * ****************************************************************************
 * File           :       Stylesheet.c
 * Project        :       QUBE Interface Firmware
 * Organisation   :       SPI Lasers PLC, Southampton, UK
 *
 * Copyright (C) 2019 SPI Lasers PLC
 * All Rights Reserved
 *
 * Description    :
 * ****************************************************************************
 */


/* SECTION Includes ---------------------------------------------------------*/
//Standard C Libraries

//Project Header Files
#include "config.h"
/* !SECTION Includes */


/* SECTION Variable Declarations --------------------------------------------*/

/* SECTION Global Variables -------------------------------------------------*/

/* !SECTION Global Variables */

/* SECTION Local Variables --------------------------------------------------*/
static int motor;
static int gate;
static int size[2];
static int count[2];

char gateString[4][15] = {{"Both open"}, {"Left closed"}, {"Right closed"}, {"Both closed"}};


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


/* !SECTION Local  Variables */
/* !SECTION Variable Declarations */


/* SECTION Functions---------------------------------------------------------*/

/* SECTION Function Declarations --------------------------------------------*/

/* !SECTION Function Declarations */

/* SECTION Global Functions -------------------------------------------------*/

/* !SECTION Global Functions */

/* SECTION Local Functions --------------------------------------------------*/
int random(void);

/**
 * @brief Generates a random number and returns it.
 *
 * @return int random number from 0-10
 *
 */
int random(void)
{
  int input;

  /* set randon number to input */
  input = rand() % 10;
  printf("%d", input);

  return(input);
}




/* Sensor reading functions */

/**
 * @brief
 *
 * @param conveyor
 * @return char
 */
char readSizeSensors(char conveyor)
{
  int randomInt = random();
  int sensorVal = SIZE_NONE;
  // Left conveyor, big block
  if ((randomInt <= 5) && (randomInt % 2 == 0))
  {
    printf("is even and is less than 5\n");
    sensorVal = SIZE_BIG;
  }
  // Left conveyor, small block
  else if ((randomInt <= 5) && (randomInt % 2 != 0) )
  {
    printf("is odd and less than 5\n");
    sensorVal = SIZE_SMALL;
  }
  return sensorVal;
}

/**
 * @brief
 *
 * @param conveyor
 * @return char
 */
char readCountSensor(char conveyor)
{
  int randomInt = random();
  int sensorVal = COUNT_NONE;
  // Left conveyor, big block
  if ((randomInt <= 5) && (randomInt % 2 == 0))
  {
    printf("is even and is less than 5\n");
    sensorVal = COUNT_BLOCK;
  }
  return sensorVal;
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

/* Gate & motor control functions */
/**
 * @brief Set the Gates object
 *
 * @param state
 */
void setGates(char state)
{
  gate = state;
  //debugPrintf("Gate state : %s\n", gateString[state]);
  //delay for x seconds
}

/**
 * @brief
 *
 */
void startMotor(void)
{
  motor = MOTOR_ON;
}

/**
 * @brief
 *
 */
void stopMotor(void)
{
  motor = MOTOR_OFF;
}

/* !SECTION Local Functions */
/* !SECTION Functions */
