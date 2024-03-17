/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wwrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
FreeRTOS is a market leading RTOS from Real Time Engineers Ltd. that supports
31 architectures and receives 77500 downloads a year. It is professionally
developed, strictly quality controlled, robust, supported, and free to use in
commercial products without any requirement to expose your proprietary source
code.

This simple FreeRTOS demo does not make use of any IO ports, so will execute on
any Cortex-M3 of Cortex-M4 hardware.  Look for TODO markers in the code for
locations that may require tailoring to, for example, include a manufacturer
specific header file.

This is a starter project, so only a subset of the RTOS features are
demonstrated.  Ample source comments are provided, along with web links to
relevant pages on the http://www.FreeRTOS.org site.

Here is a description of the project's functionality:

The main() Function:
main() creates the tasks and software timers described in this section, before
starting the scheduler.

The Queue Send Task:
The queue send task is implemented by the prvQueueSendTask() function.
The task uses the FreeRTOS vTaskDelayUntil() and xQueueSend() API functions to
periodically send the number 100 on a queue.  The period is set to 200ms.  See
the comments in the function for more details.
http://www.freertos.org/vtaskdelayuntil.html
http://www.freertos.org/a00117.html

The Queue Receive Task:
The queue receive task is implemented by the prvQueueReceiveTask() function.
The task uses the FreeRTOS xQueueReceive() API function to receive values from
a queue.  The values received are those sent by the queue send task.  The queue
receive task increments the ulCountOfItemsReceivedOnQueue variable each time it
receives the value 100.  Therefore, as values are sent to the queue every 200ms,
the value of ulCountOfItemsReceivedOnQueue will increase by 5 every second.
http://www.freertos.org/a00118.html

An example software timer:
A software timer is created with an auto reloading period of 1000ms.  The
timer's callback function increments the ulCountOfTimerCallbackExecutions
variable each time it is called.  Therefore the value of
ulCountOfTimerCallbackExecutions will count seconds.
http://www.freertos.org/RTOS-software-timer.html

The FreeRTOS RTOS tick hook (or callback) function:
The tick hook function executes in the context of the FreeRTOS tick interrupt.
The function 'gives' a semaphore every 500th time it executes.  The semaphore
is used to synchronise with the event semaphore task, which is described next.

The event semaphore task:
The event semaphore task uses the FreeRTOS xSemaphoreTake() API function to
wait for the semaphore that is given by the RTOS tick hook function.  The task
increments the ulCountOfReceivedSemaphores variable each time the semaphore is
received.  As the semaphore is given every 500ms (assuming a tick frequency of
1KHz), the value of ulCountOfReceivedSemaphores will increase by 2 each second.

The idle hook (or callback) function:
The idle hook function queries the amount of free FreeRTOS heap space available.
See vApplicationIdleHook().

The malloc failed and stack overflow hook (or callback) functions:
These two hook functions are provided as examples, but do not contain any
functionality.
*/

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"



/*-----------------------------------------------------------*/
#define mainQUEUE_LENGTH 100
//void create_dd_task( TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline,);
//void delete_dd_task(uint32_t task_id);
//**dd_task_list get_active_dd_task_list(void);
//**dd_task_list get_complete_dd_task_list(void);
//**dd_task_list get_overdue_dd_task_list(void);

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

/*
 * The queue send and receive tasks as described in the comments at the top of
 * this file.
 */
//static void Manager_Task( void *pvParameters );
//static void Blue_LED_Controller_Task( void *pvParameters );
//static void Green_LED_Controller_Task( void *pvParameters );
//static void Red_LED_Controller_Task( void *pvParameters );
//static void Amber_LED_Controller_Task( void *pvParameters );
//static void TrafficFlowAdjustmentTask(void *params);
//static void TrafficCreatorTask(void *params);
//static void TrafficLightTask(void *params);
//static void TrafficDisplayTask(void *params);
//uint16_t getADCValue(void);
//void myADC_Init();
//void myGPIO_Init();

xQueueHandle xQueue_handle = 0;
xQueueHandle xQueue_TrafficFlow = 0;
xQueueHandle xQueue_Light = 0;
xQueueHandle xQueue_TrafficGeneration = 0;


/*-----------------------------------------------------------*/
enum task_type {PERIODIC,APERIODIC};
//#define task_type {PERIODIC, APERIODIC};

struct task_list {
 TaskHandle_t t_handle;
 uint32_t deadline;
 uint32_t task_type;
 uint32_t creation_time;
 struct task_list *next_cell;
 struct task_list *previous_cell;
};

struct overdue_tasks {
TaskHandle_t tid;
uint32_t deadline;
uint32_t task_type;
uint32_t creation_time;
struct overdue_tasks *next_cell; struct
overdue_tasks *previous_cell;
};
//
//struct dd_task {
//	TaskHandle_t t_handle;
//	enum task_type type;
//	uint32_t task_id;
//	uint32_t release_time;
//	uint32_t absolute_deadline;
//	uint32_t completion_time;
//};
//

// task parameter struct
typedef struct{
	uint32_t execution_time;
	uint32_t period;
	uint32_t task_id;
	uint32_t type;
} dd_task_parameters;



typedef struct {
	TaskHandle_t t_handle;
	enum task_type type;
	uint32_t task_id;
	uint32_t release_time;
	uint32_t absolute_deadline;
	uint32_t completion_time;
} dd_task;

struct dd_task_list {
	dd_task task;
	struct dd_task_list *next_task;
};
//
//typedef struct {
//	dd_task task;
//	struct dd_task_list *next_task;
//}dd_task_list;

void create_dd_task( TaskHandle_t t_handle, enum task_type type, uint32_t task_id, uint32_t absolute_deadline);
void delete_dd_task(uint32_t task_id);
//**dd_task_list get_active_dd_task_list(void);
//**dd_task_list get_complete_dd_task_list(void);
//**dd_task_list get_overdue_dd_task_list(void);
struct dd_task_list get_active_dd_task_list(void);
struct dd_task_list get_complete_dd_task_list(void);
struct dd_task_list get_overdue_dd_task_list(void);
void ddTaskGenerator(void);
void monitorTask(void);


//Three different lists we need
struct dd_task_list activeList;

struct dd_task_list completedList;
struct dd_task_list overdueList;

xQueueHandle xQueueTask = 0;
xQueueHandle xQueueActiveList = 0;
xQueueHandle xQueueCompletedList = 0;
xQueueHandle xQueueOverdueList = 0;
int main(void)
{


	//create dd_scheduler and the user tasks as tasks here,
	//those tasks call the subtasks templated in this file.

	//struct dd_task_list headTask;

	struct dd_task_list task;
	xQueueHandle xQueueMessage = xQueueCreate(100, sizeof(char));

	//xQueueHandle xQueueTask = xQueueCreate(100, sizeof(dd_task));
	xQueueTask = xQueueCreate(100, 100);

	//For get_active_dd_task_list
	xQueueActiveList = xQueueCreate(100, sizeof(task));
	//For get_completed_dd_task_list
	xQueueCompletedList = xQueueCreate(100, sizeof(task));
	//For get_overdue_dd_task_list
	xQueueOverdueList = xQueueCreate(100, sizeof(task));


	//Three total tasks, DD, and 2 user tasks
	xTaskCreate(dd_scheduler, "Deadline Driven Scheduler", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	//User generated tasks or smth


	return 0;
}

void dd_scheduler(void) {
	
	//Actual dd scheduler, run the tasks in there

}

/**
 * Referenced as release_dd_task in lab manual
 * Receives all information necessary to create a new dd_task struct
 * Struct is packaged as a message and sent to a queue for the DDS to receive
 */
void create_dd_task(TaskHandle_t t_handle, enum task_type type, uint32_t task_id, uint32_t absolute_deadline) {
	//Assign release time to new task

//	struct dd_task newTask(t_handle, type, task_id); /*(uint32_t)0, absolute_deadline, (uint32_t)0); */

//	struct dd_task {
//		TaskHandle_t t_handle;
//		enum task_type type;
//		uint32_t task_id;
//		uint32_t release_time;
//		uint32_t absolute_deadline;
//		uint32_t completion_time;
//	};

	//TODO: use system clock for release time
	dd_task newTask;
	newTask.t_handle = t_handle;
	newTask.type = type;
	newTask.task_id = task_id;
	newTask.release_time = 0;
	newTask.absolute_deadline = absolute_deadline;
	newTask.completion_time = 0;

	//Add DD-Task to Active Task List
	addToActiveList(newTask);

	//Sort list by deadline
	sortByDeadline();

	//Set priorities of User-Defined Tasks accordingly
}


void delete_dd_task(uint32_t task_id) {
	//Assign completion time to newly-completed DD-Task


	//Remove DD-Task from Active Task List and add to Completed Task List


	//Sort Active Task List by deadline
	sortByDeadline();

	//Set priorities of the User-Defined Tasks accordingly

}


/**
 * Sends a message to a queue requesting the Active Task List from DDS
 * Once a response is received from the DDS, the function returns the list
 */
struct dd_task_list get_active_dd_task_list(void) {



}

/**
 * Sends a message to a queue requesting the Completed Task List from the DDS
 * One a response is received from the DDS, the function returns the list
 */
struct dd_task_list get_complete_dd_task_list(void) {

}

/**
 * Sends a message to a queue requesting the Overdue Task List from the DDS
 * Once a response is received from the DDS, the function returns the list
 */
struct dd_task_list get_overdue_dd_task_list(void) {

}

void sortListByDeadline() {

}

void addToActiveList(struct dd_task_list head, dd_task newTask) {
	struct dd_task_list node = head;
	while(node.task != NULL) {
		node = *node.next_task;
	}
	node.task = newTask;
}


void ddTaskGenerator(void) {
	dd_task_parameters *dd_task_parameters = (dd_task_parameters*) pvParameters;

	for(;;){
		// call create_dd_task and pass task parameters

	}
	//delay the task generator task for the period of task?
}


/**
 *
 * Responsible for reporting the following system information
 * 		- Number of active DD-Tasks
 * 		- Number of completed DD-Tasks
 * 		- Number of overdue DD-Tasks
 *
 * Collects information from DDS using get_active_dd_task_list,get_complete_dd_task_list, and get_overdue_dd_task_list
 */
void monitorTask(void) {

}

void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
	FreeRTOSConfig.h.

	This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	/* TODO: Setup the clocks, etc. here, if they were not configured before
	main() was called. */
}
