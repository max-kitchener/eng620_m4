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
#include <pthread.h>
#include <semaphore.h>

// Project files
#include "../inc/config.h"
#include "../inc/cinterface.h"
#include "../inc/ui.h"

int shutdown = FALSE;
int debug = FALSE;
int gate = GATE_OPEN;

const char sideString[2][6] = {{"Right"}, {"Left"}};
const char gateString[4][15] = {{"Both open"}, {"Left closed"}, {"Right closed"}, {"Both closed"}};

// Structure used to hold counter values for both conveyors

//SEM_ID test;
enum Semaphores
{
  INTERFACE_SEM,
  R_COUNT_SEM,
  L_COUNT_SEM,
  R_GATE_SEM,
  L_GATE_SEM,
  NUM_SEM
};

sem_t sem[NUM_SEM];

enum Tasks
{
  UI_TASK,
  L_SIZE_TASK,
  R_SIZE_TASK,
  L_COUNT_TASK,
  R_COUNT_TASK,
  L_GATE_TASK,
  R_GATE_TASK,
  NUM_TASKS
};

pthread_t task[NUM_TASKS];



counter_t counters;

menu_t menuLevel;


void *task_ui(void);
void *task_size(void *arg);
void *task_count(void *args);
void *task_gate(void *arg);



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

  sem_init(&sem[INTERFACE_SEM], 0, 1);
  sem_init(&sem[R_COUNT_SEM], 0, 0);
  sem_init(&sem[L_COUNT_SEM], 0, 0);
  sem_init(&sem[R_GATE_SEM], 0, 0);
  sem_init(&sem[L_GATE_SEM], 0, 0);

  pthread_create(&task[L_SIZE_TASK], NULL, task_size, LEFT);
  pthread_create(&task[R_SIZE_TASK], NULL, task_size, RIGHT);
  pthread_create(&task[L_COUNT_TASK], NULL, task_count, LEFT);
  pthread_create(&task[R_COUNT_TASK], NULL, task_count, RIGHT);
  pthread_create(&task[L_GATE_TASK], NULL, task_gate, LEFT);
  pthread_create(&task[R_GATE_TASK], NULL, task_gate, RIGHT);
  pthread_create(&task[UI_TASK], NULL, task_ui, NULL);

  while (shutdown == FALSE)
  {

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
 * @brief simulates polling side sensors for block detection
 *
 * @param side  - which conveyor to check
 * @return int  - SIZE_NONE, SIZE_SMALL or SIZE_BIG
 */
void *task_size(void *arg)
{
  int side = arg;
  int sensorVal;
  int returnVal = 0;

  printf("%s task_size started\n", sideString[side]);

  while(shutdown == FALSE)
  {
    sem_wait(&sem[INTERFACE_SEM]);
    //Read and reset size sensors
    sensorVal = readSizeSensors(side);
    resetSizeSensors(side);

    //Increment counters depending on size of block
    if(sensorVal == SIZE_SMALL)
    {
      debug_printf("Small block detected on %s side\n", sideString[side]);
      counters.small[side]++;
      returnVal = SIZE_SMALL;
      sem_post(&sem[R_GATE_SEM+side]);
    }
    else if(sensorVal == SIZE_BIG)
    {
      debug_printf("Big block detected on %s side\n", sideString[side]);
      counters.big[side]++;
      returnVal = SIZE_BIG;
      sem_post(&sem[R_COUNT_SEM+side]);
    }
    sem_post(&sem[INTERFACE_SEM]);
    sleep(1);
  }
}

/**
 * @brief Simulates the count sensors for counting collected blocks
 *
 * @param side - which conveyor to check, LEFT or RIGHT
 */
void *task_count(void *arg)
{
  int side = arg;
  int sensorVal;
  printf("%s count task started\n", sideString[side]);

  while(shutdown == FALSE)
  {
    sem_wait(&sem[R_COUNT_SEM + side]);
    //sleep(3);
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
}

/**
 * @brief simulates gate functionality for sorting blocks
 *
 * @param side - Which conveyor to sort, LEFT or RIGHT
 */
void *task_gate(void *arg)
{
  int side = arg;

  while( shutdown == FALSE)
  {
    sem_wait(&sem[R_GATE_SEM + side]);
    // Set gate to close to match side
    if(side == LEFT)
    {
      if(gate == GATE_CLOSED_R)
      {
        gate = GATE_CLOSED_BOTH;
      }
      else
      {
        gate = GATE_CLOSED_L;
      }
    }
    else if(side == RIGHT)
    {
      if(gate == GATE_CLOSED_L)
      {
        gate = GATE_CLOSED_BOTH;
      }
      else
      {
        gate = GATE_CLOSED_R;
      }
    }
    // close gate(s)
    debug_printf("Gate state : %s\n", gateString[gate]);
    setGates(GATE_CLOSED_BOTH);
    //Wait for block to be pushed off
    sleep(2);
    // Set gate to close to match side
    if(side == LEFT)
    {
      if(gate == GATE_CLOSED_BOTH)
      {
        gate = GATE_CLOSED_R;
      }
      else
      {
        gate = GATE_OPEN;
      }
    }
    else if(side == RIGHT)
    {
      if(gate == GATE_CLOSED_BOTH)
      {
        gate = GATE_CLOSED_L;
      }
      else
      {
        gate = GATE_OPEN;
      }
    }
    //Open gates
    debug_printf("Gate state : %s\n", gateString[gate]);
    setGates(GATE_OPEN);
  }
}

void *task_ui(void)
{
  static int ctr;
  static int cnv;
    while(shutdown == FALSE)
    {
      //state machine dependant on menu level
      switch (menuLevel)
      {
      //Top level of menu, should default to here
      case TOP:

        ui_printf(uiMainMenu, UI_MAIN_ITEMS);

        menuLevel = ui_main(ui_input());
        break;

      // Enters and exits debug mode
      // TODO stop running ui when in debug?
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
        ctr = ui_input();
        menuLevel = COUNTERS_CONV;
        break;

      //Decides which conveyor to print value for
      case COUNTERS_CONV:
        ui_printf(uiConveyorMenu, UI_MAIN_ITEMS);
        cnv = ui_input();
        ui_counter(ctr, cnv);
        menuLevel = TOP;
        //sleep(3);
        break;

      case RESET:
        ui_printf(uiCounterMenu, UI_MAIN_ITEMS);
        ctr = ui_input();
        menuLevel = RESET_CONV;
        break;

      case RESET_CONV:
        ui_printf(uiConveyorMenu, UI_MAIN_ITEMS);
        cnv = ui_input();
        ui_reset(ctr, cnv);
        menuLevel = TOP;
        //sleep(3);
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
      sleep(2);
  }
}
