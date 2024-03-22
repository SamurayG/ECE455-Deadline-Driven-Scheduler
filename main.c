
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"
#include <limits.h>
/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"



/*-----------------------------------------------------------*/
#define mainQUEUE_LENGTH 100

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

xQueueHandle xQueue_handle = 0;
xQueueHandle xQueue_TrafficFlow = 0;
xQueueHandle xQueue_Light = 0;
xQueueHandle xQueue_TrafficGeneration = 0;


/*-----------------------------------------------------------*/
enum task_type {PERIODIC,APERIODIC};
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
	uint32_t period;
} dd_task;

struct dd_task_list {
	dd_task task;
	struct dd_task_list *next_task;
};

struct taskMessage {
	char *message;
	TaskHandle_t t_handle;
	enum task_type type;
	uint32_t task_id;
	uint32_t absolute_deadline;
};


//dd_task create_dd_task(struct dd_task_list activeList, TaskHandle_t t_handle, enum task_type type, uint32_t task_id, uint32_t absolute_deadline);
void create_dd_task(TaskHandle_t t_handle, enum task_type type, uint32_t task_id, uint32_t absolute_deadline);
//void delete_dd_task(struct dd_task_list activeList, struct dd_task_list completedList, struct dd_task_list overdueList, uint32_t task_id);
void delete_dd_task();
struct dd_task_list get_active_dd_task_list(void);
struct dd_task_list get_complete_dd_task_list(void);
struct dd_task_list get_overdue_dd_task_list(void);
void addToList(struct dd_task_list list,dd_task task);
void sortByDeadline(struct dd_task_list list);
void ddTaskGenerator(void *pvParameters);
void monitorTask(void);
void dd_scheduler(void *pvParameters);

void edf(struct dd_task_list *activeList);
void createMessage(dd_task task, struct dd_task_list *activeList, struct dd_task_list *periodicList);
void releaseMessage(dd_task newTask, struct dd_task_list *activeList, struct dd_task_list *periodicList);
void deleteMessage(dd_task newTask, struct dd_task_list *activeList, struct dd_task_list *completedList, struct dd_task_list *overdueList);
void vTimerCallback(TimerHandle_t xTimer);
void release_dd_task(uint32_t taskID);
struct dd_task_list getListHead(struct dd_task_list *list);


//Three different lists we need
struct dd_task_list activeList;
struct dd_task_list completedList;
struct dd_task_list overdueList;
struct dd_task_list periodicList;

xQueueHandle xQueueTask = 0;
xQueueHandle xQueueActiveList = 0;
xQueueHandle xQueueCompletedList = 0;
xQueueHandle xQueueOverdueList = 0;
xQueueHandle xQueueMessage = 0;

TaskHandle_t dds_task = NULL;

int main(void)
{


	//create dd_scheduler and the user tasks as tasks here,
	//those tasks call the subtasks templated in this file.

	//struct dd_task_list headTask;

	struct dd_task_list task;
	xQueueMessage = xQueueCreate(100, sizeof(struct taskMessage));

	//xQueueHandle xQueueTask = xQueueCreate(100, sizeof(dd_task));
	xQueueTask = xQueueCreate(100, 100);

	xQueueActiveList = xQueueCreate(100, sizeof(task));
	xQueueCompletedList = xQueueCreate(100, sizeof(task));
	xQueueOverdueList = xQueueCreate(100, sizeof(task));

	//Three total tasks, DD, and 2 user tasks
	xTaskCreate(dd_scheduler, "Deadline Driven Scheduler", configMINIMAL_STACK_SIZE, NULL, 1, &dds_task);
	//User generated tasks or smth


	return 0;
}

void dd_scheduler(void *pvParameters) {


	struct dd_task_list* activeList = malloc(sizeof(struct dd_task_list));
	struct dd_task_list* completedList = malloc(sizeof(struct dd_task_list));
	struct dd_task_list* overdueList = malloc(sizeof(struct dd_task_list));
	struct dd_task_list* periodicList = malloc(sizeof(struct dd_task_list));

	dd_task initialTask;
	initialTask.t_handle = NULL;
	initialTask.type = PERIODIC;
	initialTask.task_id = 0;
	initialTask.absolute_deadline = 0;
	initialTask.completion_time = 0;
	initialTask.release_time = 0;

	//Set initial head node as a NULL task
	activeList->task = initialTask;
	completedList->task = initialTask;
	overdueList->task = initialTask;
	periodicList->task = initialTask;

	struct taskMessage message;

	while(1) {

		if(xQueueReceive(xQueueMessage, &message, 100) == pdPASS) {
			dd_task newTask;
			if(strcmp(message.message, "create") == 0) {
				//dd_task newTask = create_dd_task(NULL, enum task_type APERIODIC, ID, 10);
				//addToList(activeList, newTask);
				//ID++;
				createMessage(newTask, &activeList, &periodicList);
				edf(&activeList);

			} else if(strcmp(message.message, "release")) {

				releaseMessage(newTask, &activeList, &periodicList);
				edf(&activeList);

			} else if(strcmp(message.message, "delete")) {

				deleteMessage(newTask, &activeList, &completedList, &overdueList);
				edf(&activeList);

			} else if(strcmp(message.message, "active")) {
				struct dd_task_list activeTasks = get_active_dd_task_list();

			} else if(strcmp(message.message, "completed")) {
				struct dd_task_list completedTasks = get_complete_dd_task_list();

			} else if(strcmp(message.message, "overdue")) {
				struct dd_task_list overdueTasks = get_overdue_dd_task_list();

			}
		} else {

			vTaskSuspend(NULL);
		}

	}
}


void createMessage(dd_task task, struct dd_task_list *activeList, struct dd_task_list *periodicList) {
	int releaseTime = xTaskGetTickCount();

	dd_task message =  {
		.t_handle = task.t_handle,
		.type = task.type,
		.task_id = task.task_id,
		.release_time = releaseTime,
		.absolute_deadline = task.absolute_deadline,
		.period = task.period,
		.completion_time = 0

	};

	if(task.type == PERIODIC) {
		message.absolute_deadline = releaseTime + message.period;
		addToList(*periodicList,message);
		TimerHandle_t timer = xTimerCreate("periodicTask", message.period, pdTRUE, (void *) 0,vTimerCallback);
		if(timer == NULL) {
			printf("error\n");
		} else {
			if(xTimerStart(timer,0)) {
				printf("error\n");
			}
		}
	} else {
		addToList(*activeList, message);
	}
}

void releaseMessage(dd_task newTask, struct dd_task_list *activeList, struct dd_task_list *periodicList) {
	int releaseTime = xTaskGetTickCount();
	dd_task task;
	struct dd_task_list *node = &periodicList;
	while(node != NULL) {
		if(node->task.task_id = newTask.task_id) {
			newTask = node->task;
			newTask.release_time = releaseTime;
			newTask.absolute_deadline = releaseTime + newTask.period;
			addToList(*activeList,task);

		}
	}
}

void deleteMessage(dd_task newTask, struct dd_task_list *activeList, struct dd_task_list *completedList, struct dd_task_list *overdueList) {
	int time = xTaskGetTickCount();
	struct dd_task_list head = getListHead(activeList);

	if(head.task.t_handle != NULL) {
		int absoluteDeadline = head.task.absolute_deadline;
		head.task.completion_time = time;
		if(time <= absoluteDeadline) {
			addToList(*completedList,head.task);
		} else {
			addToList(*overdueList, head.task);
		}
	}
}

void edf(struct dd_task_list *activeList) {
	struct dd_task_list *node = activeList;
	if(node != NULL) {
		TaskHandle_t handle = node->task.t_handle;
		uint32_t id = node->task.task_id;
		vTaskPrioritySet(handle, INT_MAX);
		vTaskResume(handle);

		if(node != NULL && node->next_task != NULL) {
			node = node->next_task;
			TaskHandle_t handle = node->task.t_handle;
			if(node->task.task_id != id) {
				vTaskPrioritySet(handle, INT_MIN);
				vTaskSuspend(handle);
			}
		}
	}
}

struct dd_task_list getListHead(struct dd_task_list *list) {

	// Check if the list is empty
	if (list == NULL || list->next_task == NULL) {
		// If the list is empty, return NULL
		struct dd_task_list emptyNode = {0}; // Assuming task values are initialized to 0
		return emptyNode;
	}

	// Store the head node
	struct dd_task_list headNode = *list->next_task;

	// Update the list's head pointer to point to the next node
	*list = *headNode.next_task;

	// Return the removed head node
	return headNode;


}

void vTimerCallback(TimerHandle_t xTimer) {
    // Your callback function code here
	uint32_t task_id = pvTimerGetTimerID(xTimer);
	release_dd_task(task_id);
}



void release_dd_task(uint32_t taskID) {

	struct taskMessage message;
	message.message = "release";
	message.task_id = taskID;


	if(xQueueSend(xQueueMessage, &message, 10) != pdPASS) {
		printf("task %ld failed with message %s\n",  taskID, message.message);

	}

	vTaskResume(dds_task);

}


/**
 * Receives all information necessary to create a new dd_task struct
 * Struct is packaged as a message and sent to a queue for the DDS to receive
 */
//dd_task create_dd_task(struct dd_task_list activeList, TaskHandle_t t_handle, enum task_type type, uint32_t task_id, uint32_t absolute_deadline) {
void create_dd_task(TaskHandle_t t_handle, enum task_type type, uint32_t task_id, uint32_t absolute_deadline) {

	struct taskMessage msg;
	msg.message = "create";
	msg.t_handle = t_handle;
	msg.type = type;
	msg.task_id = task_id;
	msg.absolute_deadline = absolute_deadline;

	if(xQueueSend(xQueueMessage, &msg, 10) != pdPASS) {
		printf("task %ld failed with message %s\n",  msg.task_id, msg.message);
	}

	vTaskResume(dds_task);


}


void delete_dd_task()  {

	struct taskMessage msg;
	msg.message = "delete";

	if(xQueueSend(xQueueMessage, &msg, 10) != pdPASS)  {
		printf("task %s failed\n", msg.message);
	}

	vTaskResume(dds_task);
	vTaskSuspend(NULL);


}


/**
 * Sends a message to a queue requesting the Active Task List from DDS
 * Once a response is received from the DDS, the function returns the list
 */
struct dd_task_list get_active_dd_task_list(void) {

	char message[] = "active";
	struct dd_task_list taskList;

	//Send message requesting active list
	xQueueSend(xQueueMessage, &message, 100);

	//Receive Active Task List
	xQueueReceive(xQueueActiveList, &taskList, 100);

	return taskList;

}

/**
 * Sends a message to a queue requesting the Completed Task List from the DDS
 * One a response is received from the DDS, the function returns the list
 */
struct dd_task_list get_complete_dd_task_list(void) {

	char message[] = "completed";
	struct dd_task_list taskList;

	//Send message requesting active list
	xQueueSend(xQueueMessage, &message, 100);

	//Receive Completed Task List
	xQueueReceive(xQueueCompletedList, &taskList, 100);

	return taskList;
}

/**
 * Sends a message to a queue requesting the Overdue Task List from the DDS
 * Once a response is received from the DDS, the function returns the list
 */
struct dd_task_list get_overdue_dd_task_list(void) {

	char message[] = "overdue";
	struct dd_task_list taskList;

	//Send message requesting active list
	xQueueSend(xQueueMessage, &message, 100);

	//Receive Overdue Task List
	xQueueReceive(xQueueOverdueList, &taskList, 100);

	return taskList;
}

void sortListByDeadline(struct dd_task_list **head) {

	struct dd_task_list *sorted = NULL;
	struct dd_task_list *current = *head;

	while (current != NULL) {
		struct dd_task_list *next = current->next_task;

		// Find the position to insert the current task in the sorted list
		if (sorted == NULL || current->task.absolute_deadline < sorted->task.absolute_deadline) {
			// Insert at the beginning of the sorted list
			current->next_task = sorted;
			sorted = current;
		} else {
			// Traverse the sorted list to find the correct position
			struct dd_task_list *temp = sorted;
			while (temp->next_task != NULL && temp->next_task->task.absolute_deadline <= current->task.absolute_deadline) {
				temp = temp->next_task;
			}
			// Insert current task after temp
			current->next_task = temp->next_task;
			temp->next_task = current;
		}

		current = next;
	}
	// Update head to point to the sorted list
	*head = sorted;

}


//Adds to list based on deadline, no need to sort
void addToList(struct dd_task_list list, dd_task task) {
//	struct dd_task_list node = list;
//	struct dd_task_list newNode;
//	newNode.task = task;
//	newNode.next_task = NULL;
//
//	while(node.next_task != NULL) {
//		node = *node.next_task;
//	}
//	node.next_task = &newNode;
	struct dd_task_list *node = &list;
	struct dd_task_list *prev = NULL;
	struct dd_task_list *newNode = (struct dd_task_list*)malloc(sizeof(struct dd_task_list));
	if (newNode == NULL) {
		// Handle memory allocation failure
		return;
	}
	newNode->task = task;
	newNode->next_task = NULL;

	while (node != NULL && node->task.absolute_deadline < task.absolute_deadline) {
		prev = node;
		node = node->next_task;
	}

	if (prev == NULL) {
		newNode->next_task = &list;
		list = *newNode;
	} else {
		newNode->next_task = prev->next_task;
		prev->next_task = newNode;
	}
}


void ddTaskGenerator(void *pvParameters) {
	dd_task_parameters *dd_task_parameters = (dd_task_parameters*) pvParameters;

	for(;;){
		// call create_dd_task and pass task parameters
		create_dd_task(task_parameters[0].type, task_parameters[0].task_id, task_parameters[0].period, task_parameters[0].execution_time);
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
