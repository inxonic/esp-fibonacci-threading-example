# ESP32 Fibonacci numbers multi-threading example

This example code calculates a series of Fibonacci numbers using recursion to see how FreeRTOS makes use of both cores.

In the first pass, all tasks will be bound to the first core. The second pass lets FreeRTOS allocate the tasks dynamically.
