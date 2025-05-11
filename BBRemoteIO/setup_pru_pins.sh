#!/bin/bash

#sudo chmod +x setup_pru_pins.sh
#sudo ./setup_pru_pins.sh

# gpio input
config-pin P8_27 gpio_pd # input 0
config-pin P8_29 gpio_pd # input 1
config-pin P8_28 gpio_pd # input 2
config-pin P8_30 gpio_pd # input 3

# analog trigger
config-pin P9_14 gpio_pd
#MA input trigger
config-pin P8_42 gpio_pd
#MB input trigger
config-pin P8_43 gpio_pd
# gpio input trigger
config-pin P8_45 gpio_pd
#motor trigger
config-pin P8_46 gpio_pd

# Salidas step time
config-pin P8_41 pruout #M1
config-pin P8_42 pruout #M2
config-pin P8_39 pruout #M2
config-pin P8_40 pruout #M3
