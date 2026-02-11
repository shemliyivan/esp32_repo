#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t task1Handler = NULL;
TaskHandle_t task2Handler = NULL;
TaskHandle_t taskWatcherHandler = NULL;

void Task1(void* arg){
    int counter = 0;
    for(int i = 0; i < 10; ++i){
        printf("Task 1 is runnning, counter = %d\n", counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

void Task2(void* arg){
    int counter = 0;
    for(int i = 0; i < 10; ++i){
        printf("Task 2 is runnning, counter = %d\n", counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

void TaskWatcher(void* arg){
    for(int i = 0; i < 10; ++i){
        char buffer[1024];
        vTaskList(buffer);
        printf("Task Name\tStatus\tPrio\tStack\tTask#\n%s\n", buffer);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    xTaskCreate(Task1, "Task 1", 2048, NULL, 1, &task1Handler);
    xTaskCreate(Task2, "Task 2", 2048, NULL, 1, &task2Handler);
    xTaskCreate(TaskWatcher, "Task Watcher", 8192, NULL, 1, &taskWatcherHandler);
}