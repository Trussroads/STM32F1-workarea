#!/bin/bash

# Use this script to program your STM32VL Discovery board like so
# ./program.sh GPIO-example/main.elf

openocd -f board/stm32vldiscovery.cfg -c init -c targets -c "halt" -c "flash write_image erase $1" -c "verify_image $1" -c "reset run" -c shutdown
