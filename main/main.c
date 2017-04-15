#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"


#define FIB_LIMIT 40


static const char* TAG = "fib";

QueueHandle_t output_queue;


typedef struct {
    uint32_t start_core;
    uint32_t finish_core;
    int input;
    int output;
} report_t;


esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

int fib(int n)
{
    if ( n < 2 ) return n;
    return fib(n-1) + fib(n-2);
}

void fib_task(void *pvParameter)
{
    int n;
    report_t report;

    n = (int)pvParameter;
    ESP_LOGV(TAG, "Started fib_task for %d", n);
    report.start_core = xPortGetCoreID();
    report.input = n;
    report.output = fib(n);
    report.finish_core = xPortGetCoreID();
    ESP_LOGV(TAG, "Sending report for %d", report.input);
    xQueueSend(output_queue, &report, portMAX_DELAY);
    ESP_LOGV(TAG, "Completed fib_task for %d", n);
    vTaskDelete(NULL);
}

void output_reports(int max)
{
    report_t report;

    for (;;)
    {
        ESP_LOGV(TAG, "Queing for reports");
        xQueueReceive(output_queue, &report, portMAX_DELAY);

        printf("fib(%d) = %d (Started on Core: %u, Finished on core: %u)\n", 
            report.input, report.output, report.start_core, report.finish_core);

        if ( report.input == max ) break;
    }
}

void app_main(void)
{
    int i;
    unsigned int start_time, took_time;

    output_queue = xQueueCreate(20, sizeof(report_t));

    start_time = xTaskGetTickCount();

    for (i=1; i<=FIB_LIMIT; i++)
    {
        ESP_LOGV(TAG, "Starting pinned fib_task for %d", i);
        xTaskCreatePinnedToCore(
            fib_task, "fib_task", 4096, (void*)i, 0, NULL, 0);
    }

    output_reports(FIB_LIMIT);
    took_time = xTaskGetTickCount() - start_time;
    printf("Time passed: %u ticks\n", took_time);

    start_time = xTaskGetTickCount();

    for (i=1; i<=FIB_LIMIT; i++)
    {
        ESP_LOGV(TAG, "Starting dynamic fib_task for %d", i);
        xTaskCreate(fib_task, "fib_task", 4096, (void*)i, 0, NULL);
    }
    output_reports(FIB_LIMIT);
    took_time = xTaskGetTickCount() - start_time;
    printf("Time passed: %u ticks\n", took_time);
}

