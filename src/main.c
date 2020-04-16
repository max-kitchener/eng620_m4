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
#include <pthread.h>git
#include <semaphore.h>

// Project files
#include "../inc/config.h"
#include "../inc/cinterface.h"
#include "../inc/ui.h"

//global variables
int calibration = FALSE;
int shutdown = FALSE;
int debug = FALSE;
int gate = GATE_OPEN;

// strings used for debug messages
const char sideString[2][6] = {{"Right"}, {"Left"}};
const char gateString[4][15] = {{"Both open"}, {"Left closed"}, {"Right closed"}, {"Both closed"}};

// List of semaphores used
enum Semaphores
{
  INTERFACE_SEM,
  R_COUNT_SEM,
  L_COUNT_SEM,
  R_GATE_SEM,
  L_GATE_SEM,
  NUM_SEM
};
// array to store semaphore IDs
sem_t sem[NUM_SEM];

// List of tasks
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
// array to store task ids
pthread_t task[NUM_TASKS];


struct
{
  double size[2];
  double count[2];
  double gate[2];
  double ui;
}delay;

// structure used to hold count values for big small and colected blocks
counter_t counters;

// holds the current level of user interface menu
menu_t menuLevel;

// Task function prototypes
void *task_ui(void *arg);
void *task_size(void *arg);
void *task_count(void *args);
void *task_gate(void *arg);

// function prototypes
void debug_printf(char *dbgMessage, ...);
void close_threads(void);
void calibrate(void);
double execution_time(void*(*func)(void *), void  *arg);
/**
 * @brief main function for conveyor belt simulation
 *
 */
int main(void)
{

  //Initializes random number generator used by cinterface.c
  time_t t;
  srand((unsigned)time(&t));

  printf("Conveyor belt UI starting\n");

  //TODO run calibration?

  // initialise semaphores
  sem_init(&sem[INTERFACE_SEM], 0, 1);
  sem_init(&sem[R_COUNT_SEM], 0, 0);
  sem_init(&sem[L_COUNT_SEM], 0, 0);
  sem_init(&sem[R_GATE_SEM], 0, 0);
  sem_init(&sem[L_GATE_SEM], 0, 0);

  calibrate();


  startMotor();


  // start tasks
  pthread_create(&task[L_SIZE_TASK], NULL, &task_size, (void *) LEFT);
  pthread_create(&task[R_SIZE_TASK], NULL, &task_size, (void *) RIGHT);
  pthread_create(&task[L_COUNT_TASK], NULL, &task_count, (void *)LEFT);
  pthread_create(&task[R_COUNT_TASK], NULL, &task_count, (void *)RIGHT);
  pthread_create(&task[L_GATE_TASK], NULL, &task_gate, (void *)LEFT);
  pthread_create(&task[R_GATE_TASK], NULL, &task_gate, (void *)RIGHT);
  sleep(1);
  pthread_create(&task[UI_TASK], NULL, &task_ui, NULL);

  while (shutdown == FALSE)
  {
    //runs in background while
  }

  stopMotor();
  close_threads();

  return EXIT_SUCCESS;
}

/**
 * @brief simulates polling side sensors for block detection
 *
 * @param side  - which conveyor to check
 * @return int  - SIZE_NONE, SIZE_SMALL or SIZE_BIG
 */
void *task_size(void *arg)
{
  int side = (int)arg;
  int sensorVal;

  printf("%s task_size started\n", sideString[side]);

  while (TRUE)
  {
    sem_wait(&sem[INTERFACE_SEM]);
    //Read and reset size sensors
    sensorVal = readSizeSensors(side);
    resetSizeSensors(side);

    //Increment counters depending on size of block
    if (sensorVal == SIZE_SMALL)
    {
      debug_printf("Small block detected on %s side\n", sideString[side]);
      counters.small[side]++;
      //Trigger task_gate function
      sem_post(&sem[R_GATE_SEM + side]);
    }
    else if (sensorVal == SIZE_BIG)
    {
      debug_printf("Big block detected on %s side\n", sideString[side]);
      counters.big[side]++;
      //Trigger task_count function
      sem_post(&sem[R_COUNT_SEM + side]);
    }
    sem_post(&sem[INTERFACE_SEM]);
    if(calibration == TRUE)
    {
      break;
    }
    sleep(1);
  }
  return 0;
}

/**
 * @brief Simulates the count sensors for counting collected blocks
 *
 * @param side - which conveyor to check, LEFT or RIGHT
 */
void *task_count(void *arg)
{
  int side = (int)arg;
  int sensorVal;
  printf("%s task_count started\n", sideString[side]);

  while (TRUE)
  {
    sem_wait(&sem[R_COUNT_SEM + side]);
    //sleep(3);
    // Read and reset sensors
    sensorVal = readCountSensor(side);
    resetCountSensor(side);

    // Increment collected count when a block is detected
    if (sensorVal == COUNT_BLOCK)
    {
      debug_printf("Block collected on %s side\n", sideString[side]);
      counters.collected[side]++;
    }
    if(calibration == TRUE)
    {
      break;
    }
  }
  return NULL;
}

/**
 * @brief simulates gate functionality for sorting blocks
 *
 * @param side - Which conveyor to sort, LEFT or RIGHT
 */
void *task_gate(void *arg)
{
  int side = (int)arg;
  printf("%s task_gate started\n", sideString[side]);

  while (TRUE)
  {
    sem_wait(&sem[R_GATE_SEM + side]);
    // Set gate to close to match side
    if (side == LEFT)
    {
      if (gate == GATE_CLOSED_R)
      {
        gate = GATE_CLOSED_BOTH;
      }
      else
      {
        gate = GATE_CLOSED_L;
      }
    }
    else if (side == RIGHT)
    {
      if (gate == GATE_CLOSED_L)
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
    if (side == LEFT)
    {
      if (gate == GATE_CLOSED_BOTH)
      {
        gate = GATE_CLOSED_R;
      }
      else
      {
        gate = GATE_OPEN;
      }
    }
    else if (side == RIGHT)
    {
      if (gate == GATE_CLOSED_BOTH)
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

    if(calibration == TRUE)
    {
      break;
    }
  }
  return NULL;
}

void *task_ui(void *arg)
{
  static int ctr;
  static int cnv;

  printf("task_ui started\n");

  while (TRUE)
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
  return NULL;
}

/**
 * @brief Same functionality as printf, but will only
 *        print when debugMode is TRUE
 *
 * @param dbgMessage - string for printf message
 * @param ...        - variable list of arguments fro dbgMessage
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

void close_threads(void)
{
  //close all open threads
  for (int tsk = 0; tsk < NUM_TASKS; tsk++)
  {
    pthread_cancel(task[tsk]);
  }

  // close all semaphores
  for (int sm = 0; sm < NUM_SEM; sm++)
  {
    sem_close(&sem[sm]);
  }
}

void calibrate(void)
{
  calibration = TRUE;
  debug = FALSE;

  printf("Execution time\n");

  sem_post(&sem[R_COUNT_SEM]);
  delay.count[0] = execution_time(task_count, 0);
  printf("count task = %f\n\n", delay.count[0]);

  delay.size[0]  = execution_time(task_size, 0);
  printf("size task = %f\n\n", delay.size[0]);

  sem_post(&sem[R_GATE_SEM]);
  delay.gate[0]  = execution_time(task_gate, 0);
  printf("gate task = %f\n\n", delay.gate[0]);

  debug = TRUE;

  printf("Execution time in debug mode\n");
  sem_post(&sem[R_COUNT_SEM]);
  delay.count[1] = execution_time(task_count, 0);
  printf("count task time = %f\n\n", delay.count[1]);

  delay.size[1]  = execution_time(task_size, 0);
  printf("size task time = %f\n\n", delay.size[1]);

  sem_post(&sem[R_GATE_SEM]);
  delay.gate[1]  = execution_time(task_gate, 0);
  printf("gate task time = %f\n\n", delay.gate[1]);

  debug = FALSE;
  calibration = FALSE;
}

double execution_time(void*(*func)(void *), void  *arg)
{
  double exec_tim = 0.0;
  clock_t start = clock();
  clock_t stop;

  func(arg);

  stop = clock();

  exec_tim = (double)(stop-start)/CLOCKS_PER_SEC;

  //printf("execution time = %f\n", exec_tim);

  return exec_tim;
}
