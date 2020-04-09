/*
 * ****************************************************************************
 * File           : main.c
 * Project        : Real Time Embedded Systems Coursework
 *
 * Description    : Firmware for controlling the conveyor belt
 * ****************************************************************************
 * ChangeLog:
 */

/* Includes */

/* Standard C libraries */
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"

/* VxWorks Libraries */
#include "vxWorks.h"
#include "sysLib.h"
#include "semLib.h"
#include "taskLib.h"
#include "wdLib.h"
#include "time.h"

/* Local Files */
#include "cinterface.h"
#include "config.h"

/* SEMAPHORES */
/* List of semaphores used */
enum Semaphores
{
  INTERFACE_SEM,
  GATE_SEM,
  R_COUNT_SEM,
  L_COUNT_SEM,
  NUM_SEM /* Used to initialise semaphore array */
};
/* Array to store all semaphore*/
SEM_ID Sem[NUM_SEM];

/* TIMERS */
/* Array of timers for multiple blocks */
/* Half of array is used for each side*/
/* eg. if GATE_TIM_NUM=20 RIGHT uses 0-9 and LEFT 10-19*/
WDOG_ID gateTIM[GATE_TIM_NUM];
WDOG_ID countTIM[COUNT_TIM_NUM];

/* TASKS */
/* List of tasks used for controlling conveyor belt */
enum Tasks
{
  UI_TASK,
  L_SIZE_TASK,
  R_SIZE_TASK,
  L_COUNT_TASK,
  R_COUNT_TASK,
  GATE_TASK,
  NUM_TASKS /* Used to initialise task array*/
};

/* Used for storing task IDs */
int Task[NUM_TASKS];

/* List priorities from high to low */
enum Priority
{
  GATE_PR,
  L_SIZE_PR,
  R_SIZE_PR,
  L_COUNT_PR,
  R_COUNT_PR
};

/* Structure used to hold counter values for both sides of conveyor*/
struct
{
  int big[2];
  int small[2];
  int collected[2];
} counters;

/* Flags */
/* Used  for gate control logic */
int rightGate;
int leftGate;

int shutdownFlg = 0;
int debugMode = 1;

/* REVIEW Rate Monotonic Scheduling */

/* Function prototpyes */
void calibration(void);
void printMenu(char *menuArray, int numOptions);
void shutdown(void);
void debugPrintf(char *dbgMessage, ...);
b
/* Task Functions */
void countTask(int side);
void gateTask(void);
void sizeTask(int side);
void uiTask(void);

/* TODO implement gate control logic*/
/**
 * @brief Callback function for controlling the gates.
 *        Called when a small block is detected by sizeTask
 *
 * @param side - indicates which conveyor
 */
void gateTimerCallback(int side)
{
  if (side == RIGHT)
  {
    rightGate++;
  }
  else if (side == LEFT)
  {
    leftGate++;
  }
  semGive(Sem[GATE_SEM]);
}

/**
 * @brief Callback function for counter sensor at end of conveyor
 *        Called when a big block is detected by sizeTask
 *
 * @param side - Indicates conveyor belt
 */
void countTimerCallback(int side)
{
  /* Not the prettiest logic but seems to work */
  /* LEFT = 1, RIGHT = 0*/
  /* R_COUNT_SEM + RIGHT = R_COUNT_SEM*/
  /* R_COUNT_SEM + LEFT  = L_COUNT_SEM*/
  semGive(Sem[R_COUNT_SEM + side]);
}

/**
 * @brief Main function for coursework, runs calibration and starts tasks and timers for controlling conveyor belt
 *
 */
void progStart(void)
{
  int tim; /*Used for timer initialisation for loop*/
  char rxChar;

  /* Mutually exclusive to prevent reentrance in the interface library */
  Sem[INTERFACE_SEM] = semMCreate(SEM_Q_PRIORITY);

  Sem[GATE_SEM]    = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
  Sem[R_COUNT_SEM] = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
  Sem[L_COUNT_SEM] = semBCreate(SEM_Q_FIFO, SEM_EMPTY);


  /* Initialise watchdog timer arrays */
  for (tim = 0; tim < GATE_TIM_NUM; tim++)
  {
    gateTIM[tim] = wdCreate();
  }
  for (tim = 0; tim < COUNT_TIM_NUM; tim++)
  {
    countTIM[tim] = wdCreate();
  }

  /* user prompt for running calibration routine */
  printf("Run calibration(y/n)?\n");
  rxChar = getchar();

  /* ASCII hex codes for y, Y and 1*/
  if (rxChar == 0x59 || rxChar == 0x79 || rxChar == 0x31)
  {
    /*FIXME when not called the default system clock rate is used*/
    /* does this affect the performance? */
    calibration();
  }
  else
  {
    printf("Skipped calibration\n");
    startMotor();
  }

  /* Start tasks */
  /*                                     Task Name,   Priority, Options, Stack,   Function Pointer, Arguments*/
  Task[L_SIZE_TASK]  = taskSpawn( "CW_l_size_task",  L_SIZE_PR,       0, 20000,  (FUNCPTR)sizeTask, LEFT, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  Task[R_SIZE_TASK]  = taskSpawn( "CW_r_size_task",  R_SIZE_PR,       0, 20000,  (FUNCPTR)sizeTask, RIGHT, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  Task[L_COUNT_TASK] = taskSpawn("CW_l_count_task", L_COUNT_PR,       0, 20000, (FUNCPTR)countTask, LEFT, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  Task[R_COUNT_TASK] = taskSpawn("CW_r_count_task", R_COUNT_PR,       0, 20000, (FUNCPTR)countTask, RIGHT, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  Task[GATE_TASK]    = taskSpawn(   "CW_gate_task",    GATE_PR,       0, 20000,  (FUNCPTR)gateTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  /*
  Task[UI_TASK] =       taskSpawn(     "CW_ui_task",        UI_PR,       0,     20000,    (FUNCPTR)uiTask,      0,0,0,0,0,0,0,0,0,0);
  */

  /* Give interface semaphore to start tasks */
  semGive(Sem[INTERFACE_SEM]);

  /* Run until user requests shutdown */
  while (shutdownFlg == FALSE)
  {
    /* Wait for 5 minutes */
    taskDelay(250 * sysClkRateGet());
    /* Restart motors as it stops after certain period */
    semTake(Sem[INTERFACE_SEM], WAIT_FOREVER);
    startMotor();
    semGive(Sem[INTERFACE_SEM]);
  }
}

/**
 * @brief Task for handling size detection
 *
 * @param side - Indicates which conveyor belt to monitor
 */
void sizeTask(int side)
{
  int sensorVal;
  int gateTimCnt  = 0;
  int countTimCnt = 0;

  /* States for size detection FSM */
  typedef enum Size_State
  {
    WAITING,
    DETECTED,
    BIG,
    SMALL
  } Size_State;

  Size_State state = WAITING;
  printf("%s size task started\n", sideString[side]);

  while (1)
  {
    semTake(Sem[INTERFACE_SEM], WAIT_FOREVER);
    sensorVal = readSizeSensors(side);
    resetSizeSensors(side);
    /* Block detection FSM */
    switch (state)
    {
    /* No blocks detected yet*/
    case WAITING:
      /* Block is in front of first sensor */
      if (sensorVal == 1)
      {
        /* Change state */
        state = DETECTED;
        debugPrintf("%s DETECTED\n", sideString[side]);
      }
      break;

    /* Block has been detected but not identified */
    case DETECTED:
      /* Block in front of both sensors */
      if (sensorVal == 3)
      {
        /* Change state*/
        state = BIG;

        counters.big[side]++;

        /* Start watchdog timer for triggering count sensor task */
        wdStart(countTIM[countTimCnt + (side * (COUNT_TIM_NUM / 2))], COUNT_DELAY * sysClkRateGet(), (FUNCPTR)countTimerCallback, side);

        debugPrintf("%s %i Big blocks detected\n", sideString[side], counters.big[side]);

        /* increment watchdog timer index and boundary check*/
        countTimCnt++;
        if (countTimCnt == (COUNT_TIM_NUM / 2))
        {
          countTimCnt = 0;/*Reset*/
        }
      }
      /* Nothing in front of sensors */
      else if (sensorVal == 0)
      {
        state = SMALL;
        counters.small[side]++;

        wdStart(gateTIM[gateTimCnt + (side * (GATE_TIM_NUM / 2))], GATE_DELAY * sysClkRateGet(), (FUNCPTR)gateTimerCallback, side);

        debugPrintf("%s %i Small blocks detected\n", sideString[side], counters.small[side]);

        gateTimCnt++;
        if (gateTimCnt == (GATE_TIM_NUM / 2))
        {
          gateTimCnt = 0;
        }
      }
      break;
    /*REVIEW functionality for SMALL and BIG states?*/
    default:
      /* Reset state to WAITING*/
      state = WAITING;
      break;
    }
    /* Give semaphore back and delay to allow other tasks to function */
    semGive(Sem[INTERFACE_SEM]);
    taskDelay(TASK_DELAY);
  }
}

/**
 * @brief Task for monitoring count sensors at the end of the
 *
 * @param side - RIGHT or LEFT to indicate conveyor belt
 */
void countTask(int side)
{
  int sensorVal = 0;
  int detected = 0;
  int Count = 0;

  printf("%s side count sensor task started\n", sideString[side]);

  while (1)
  {
    /* Janky logic, described in countTimerCallback */
    semTake(Sem[R_COUNT_SEM + side], WAIT_FOREVER);
    semTake(Sem[INTERFACE_SEM], WAIT_FOREVER);

    /* Read sensor value and reset to keep interface happy*/
    sensorVal = readCountSensor(side);
    resetCountSensor(side);

    /* FIXME Is detected redundant now that count watchdog is being used?*/
    /* Only increment count if the sensor hasn't already detected the block */
    if ((sensorVal == 1) && (detected == 0))
    {
      detected = 1;
      counters.collected[side]++;
      debugPrintf("%s %i blocks counted\n", sideString[side], counters.collected[side]);
    }
    else
    {
      detected = 0;
    }
    semGive(Sem[INTERFACE_SEM]);
  }
}


/* REVIEW seperate tasks for each gate? */
/**
 * @brief Task for controlling the gates, triggered by timer interrupt when gateSEM is given
 *
 */
void gateTask(void)
{
  int GateVal;

  printf("Gate task started\n");

  while (1)
  {
    /* Semaphore given by gateTimerCallback*/
    semTake(Sem[GATE_SEM], WAIT_FOREVER);
    GateVal = 0;

    /* check counters and set gateVal accordingly*/
    if (leftGate >= 1)
    {
      GateVal += 2;
    }
    if (rightGate >= 1)
    {
      GateVal += 1;
    }

    /* Close gates and wait for GATE_CLOSE seconds till opening*/
    setGates(GateVal);
    taskDelay(GATE_CLOSE * sysClkRateGet());

    /* count down side counters*/
    leftGate--;
    rightGate--;
    GateVal = 0;

    if (leftGate > 1)
    {
      GateVal += 2;
    }
    else if (leftGate <= 0)
    {
      leftGate = 0;
    }

    if (rightGate > 1)
    {
      GateVal += 1;
    }
    else if (rightGate <= 0)
    {
      rightGate = 0;
    }
    setGates(GateVal);
  }
}

/* REVIEW this function can be simplifiied by creating functions for some of the repetetive code*/
/**
 * @brief Called once at startup before other tasks have been started
 *
 */
void calibration(void)
{
  int countSensor = 0;
  int sizeSensor = 0;
  int distance = 0;
  /* Used to count time delay of tasks */
  struct timespec start;
  struct timespec stop;
  struct timespec res;

  printf("Starting calibration\n");

  startMotor();

  /* Set number of clock ticks per seconds */
  sysClkRateSet(CLOCK_RATE);
  printf("Ticks per second = %i\n", sysClkRateGet());

  /* Get the Real-Time clock resolution */
  clock_getres(CLOCK_REALTIME, &res);
  printf("Tick resolution: %d ms\n", (res.tv_nsec / 1000000));

  /*
  Keeps taking time values until start and stop have the same tv_sec value.
  This is to simplify delay calculations as only tv_nsec will be needed
  */
  while (1)
  {
    /* Calculate time taken to read and reset size sensors*/
    clock_gettime(CLOCK_REALTIME, &start);
    readSizeSensors(RIGHT);
    resetSizeSensors(RIGHT);
    clock_gettime(CLOCK_REALTIME, &stop);

    /* Only break when second values are the same for easier calculations*/
    if (start.tv_sec == stop.tv_sec)
    {
      break;
    }
  }

  printf("Read size sensor delay %d ms\n", (stop.tv_nsec - start.tv_nsec) / 1000000);

  /*
  Keeps taking time values until start and stop have the same tv_sec value.
  This is to simplify delay calculations as only tv_nsec will be needed
  */
  while (1)
  {
    /* Calculate time taken to read and reset size sensors*/
    clock_gettime(CLOCK_REALTIME, &start);
    readCountSensor(RIGHT);
    resetCountSensor(RIGHT);
    clock_gettime(CLOCK_REALTIME, &stop);

    if (start.tv_sec == stop.tv_sec)
    {
      break;
    }
  }

  printf("Read count sensor delay %d ms\n", (stop.tv_nsec - start.tv_nsec) / 1000000);

  /* Calculate time between size and count sensors */
  printf("Put a large block on the right belt\n");

  /* Wait for block to be in front of both sensors*/
  while (sizeSensor != 3)
  {
    sizeSensor = readSizeSensors(RIGHT);
    resetSizeSensors(RIGHT);
  }
  clock_gettime(CLOCK_REALTIME, &start);
  printf("Block detected\n");

  /* Wait for block to be in front of count sensor */
  while (countSensor != 1)
  {
    distance++;

    countSensor = readCountSensor(RIGHT);
    resetCountSensor(RIGHT);
  }
  clock_gettime(CLOCK_REALTIME, &stop);
  printf("Time between sensors %d.%d\n", (stop.tv_sec - start.tv_sec), (stop.tv_nsec - start.tv_nsec));
  printf("%d ticks between sensors\n", distance);
}

/**
 * @brief Same functionality as printf, but will only
 *        print when debugMode is TRUE
 *
 * @param dbgMessage string for printf message
 * @param ...        variable list of arguments fro dbgMessage
 */
void debugPrintf(char *dbgMessage, ...)
{
  if (debugMode == TRUE)
  {
    va_list Args;
    va_start(Args, dbgMessage);

    vprintf(dbgMessage, Args);
  }
}

/* FIXME test this actually works*/
/**
 * @brief reuseable function for calculating the execution time of a function.
 *        Takes a variable number of arguments for function so different functions
 *        can be called
 *
 * @param start - timestamp for start time
 * @param stop  - timestamp for stop time
 * @param func  - pointer to the function to test
 * @param ...   - optional list of arguments for function
 */
void calExecutionTime(timespec start, timespec stop,  int func, ...)
{
  /* Initialise and fetch list of optional arguments */
  va_list Args;
  va_start(Args, func);

  /*
  Keeps taking time values until start and stop have the same tv_sec value.
  This is to simplify delay calculations as only tv_nsec will be needed
  */
  while (1)
  {
    clock_gettime(CLOCK_REALTIME, &start);
    func(arg);
    clock_gettime(CLOCK_REALTIME, &stop);

    /* Break out of loop when tv_sec values are the same*/
    if (start.tv_sec == stop.tv_sec) break;
  }
}


/* TODO implement and test these functions for displaying the UI*/
/* REVIEW should all these ui functions have there own c file?*/
/**
 * @brief Task for handling user interface, reads messages from the serial interface
 *
 */
void uiTask(void)
{
  int counterSel;
  int conveyorSel;
  char menuSelect;
  printf("UI task started\n");

  while (1)
  {
    printMenu(uiMainMenu, UI_MAIN_ITEMS);

    menuSelect = getchar();

    switch (menuSelect)
    {
    /* ASCII 1 character*/
    case 0x31:
      printf("Entering debug mode, press n to cancel\n");
      debugMode = 1;
      break;
    /* ASCII 2 character*/
    case 0x32:
      printMenu(uiCounterMenu, UI_COUNTER_ITEMS);
      counterSel = getchar();
      printMenu(uiConveyorMenu, UI_CONV_ITEMS);
      conveyorSel = getchar();

      break;
    /* ASCII 3 character*/
    case 0x33:
      /* code */
      break;
    /* ASCII 4 character*/
    case 0x34:
      /* code */
      break;
    /* ASCII 5 character*/
    case 0x35:
      /* code */
      break;
    /* ASCII 6 character*/
    case 0x36:
      printf("Are you sure?(y/n)\n");

      /* ASCII y character*/
      if (getchar() == 0x79)
      {
        shutdown();
      }
      else
      {
        printf("Shutdown cancelled\n");
      }

      break;

    default:
      break;
    }
  }
}

/**
 * @brief closes down all active tasks, semaphores and timers created by progStart
 *
 */
void shutdown(void)
{
  int task;
  int semaphore;
  int timer;

  printf("Shutting down\n");

  /*Delete all active tasks */
  for (task = 0; task < NUM_TASKS; task++)
  {
    taskDelete(Task[task]);
  }

  for (semaphore = 0; semaphore < NUM_SEM; semaphore++)
  {
    semDelete(Sem[semaphore]);
  }

  for (timer = 0; timer < GATE_TIM_NUM; timer++)
  {
    wdCancel(gateTIM[timer]);
    wdDelete(gateTIM[timer]);
  }
}


/**
 * @brief prints all options for the ui menu menuArray
 *
 * @param menuArray  pointer to array of strings for menu options
 * @param numOptions  number of options in menuArray
 */
void printMenu(char *menuArray, int numOptions)
{
  int menuItem;

  for (menuItem = 0; menuItem < numOptions; menuItem++)
  {
    printf("%s", menuArray[menuItem]);
  }
}

/**
 * @brief Prints menu for displaying different counter values
 *
 */
void counterMenu(void)
{
  char menuSelect;

  printMenu(uiCounterMenu, UI_COUNTER_ITEMS);
  menuSelect = getchar();

  if (menuSelect == 0x33)
  {
    printf("%d small blocks counted on %s conveyor\n", counters.small[LEFT], sideString[LEFT]);
    printf("%d small blocks counted on %s conveyor\n", counters.small[RIGHT], sideString[RIGHT]);
  }
  else if (menuSelect == 0x32)
  {
    printf("%d small blocks counted on %s conveyor\n", counters.small[RIGHT], sideString[RIGHT]);
  }
  else if (menuSelect == 0x33)
  {
    printf("%d small blocks counted on %s conveyor\n", counters.small[LEFT], sideString[LEFT]);
  }
  else
  {
    printf("Invalid input\n");
  }
}

int conveyorMenu(void)
{
}

int resetMenu(void)
{
