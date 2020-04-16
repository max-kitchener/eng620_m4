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
#include <stdbool.h>

// Project files
#include "../inc/config.h"
#include "../inc/cinterface.h"
#include "../inc/ui.h"

//global variables
bool calibration = FALSE;
bool shutdown = FALSE;
bool debug = FALSE;
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

// structure for storing the execution time of
struct
{
  double size[2];
  double count[2];
  double gate[2];
  double ui;
  double count_sensor;
  double gate_shut;
}delay;

// structure used to hold count values for big small and collected blocks
counter_t counters;

// holds the current level of user interface menu
menu_t menuLevel;

// Task function prototypes
void *task_count(void *args);
void *task_gate(void *arg);
void *task_size(void *arg);
void *task_ui(void *arg);

// function prototypes
void calculate_delay(void);
void calibrate(void);
void conveyor_shutdown(void);
void debug_printf(char *dbgMessage, ...);
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
  conveyor_shutdown();

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
    sleep(delay.count_sensor);
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
    sleep(delay.gate_shut);
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

/**
 * @brief task for running the user interface for the conveyor belt
 *
 */
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

/**
 * @brief
 *
 */
void conveyor_shutdown(void)
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

/**
 * @brief calibration routine, should run on startup. Calculates execution
 *        time of tasks when debug mode is off and on
 *
 */
void calibrate(void)
{
  calibration = TRUE;
  debug = FALSE;


  char dbgString[][4] = {"off", "on"};

  // calculate execution time of tasks
  for (int i = 0; i < 2; i++)
  {

    sem_post(&sem[R_COUNT_SEM]);
    delay.count[debug] = execution_time(task_count, 0);

    delay.size[debug]  = execution_time(task_size, 0);

    sem_post(&sem[R_GATE_SEM]);
    delay.gate[debug]  = execution_time(task_gate, 0);

    printf("\n\nExecution time, debug %s\n", dbgString[debug]);
    printf("count task = %f\n", delay.count[debug]);
    printf("size task = %f\n", delay.size[debug]);
    printf("gate task = %f\n\n", delay.gate[debug]);
    debug = !debug;
  }


  calculate_delay();

  printf("\nCalibration complete!\n\n\n");

  calibration = FALSE;
}


/**
 * @brief calculate the execution time for a function.
 *        the calculated value is based on the number
 *        of clock cycles and ignores sleep() delays.
 *
 * @param func pointer to function to run
 * @param arg argument(if any) for func
 *
 * @return calculated execution time
 */
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

/**
 * @brief This function calculates the time taken for a
 *        big block to go from the size sensor to the
 *        count sensor
 */
void calculate_delay()
{
  clock_t start;
  clock_t stop;

  int sensorval = SIZE_NONE;
  int rnd = 0;

  printf("Place large block on conveyor\n");

  sleep(1);

  // wait for large block to be detected
  while(sensorval != SIZE_BIG)
  {
    sensorval = readSizeSensors(RIGHT);
    resetSizeSensors(RIGHT);
  }

  // take start time
  start = time(NULL);
  printf("Block detected\n");


  // delay for randomly generated number of seconds
  rnd = rand() % 10;
  printf("rand = %i\n", rnd);
  sleep(rnd);

  // wait for a block to be detected by count sensor
  sensorval = COUNT_NONE;
  while(sensorval != COUNT_BLOCK)
  {
    sensorval = readCountSensor(RIGHT);
    resetCountSensor(RIGHT);
  }

  // take stop time
  stop = time(NULL);
  printf("Block collected\n");

  //calculate time between size sensor, and the count sensor and gate
  delay.count_sensor = (double)(stop-start);
  delay.gate_shut = delay.count_sensor * 0.8;
  printf("count delay : %f\n gate delay : %f\n",delay.count_sensor,delay.gate_shut);
}
