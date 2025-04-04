#!/bin/bash
#Gpio read
##P8_31 gpio0[10] input_0
config-pin P8_31 gpio_pd
##P8_32 gpio0[11] input_1
config-pin P8_32 gpio_pd
##P8_33 gpio0[9] input_2
config-pin P8_33 gpio_pd
##P8_35 gpio0[8] input_3
config-pin P8_35 gpio_pd

#Gpio write
##P8_27 gpio2[22] output_0
config-pin P8_27 gpio
##P8_28 gpio2[24] output_1
config-pin P8_28 gpio
##P8_29 gpio2[23] output_2
config-pin P8_29 gpio
##P8_30 gpio2[25] output_3
config-pin P8_30 gpio

#Motor MA
## P8_34 gpio2[17] output_MA_E
config-pin P8_34 gpio
## P8_36 gpio2[16] output_MA_D
config-pin P8_36 gpio
## P8_37 gpio2[14] output_MA_S
config-pin P8_37 gpio
#Motor MB
## P8_38 gpio2[15] output_MB_E
config-pin P8_38 gpio
## P8_39 gpio2[12] output_MB_D
config-pin P8_39 gpio
## P8_40 gpio2[13] output_MB_S
config-pin P8_40 gpio
#Motor MC
## P8_41 gpio2[10] output_MC_E
config-pin P8_41 gpio
## P8_42 gpio2[11] output_MC_D
config-pin P8_42 gpio
## P8_43 gpio2[8] output_MC_S
config-pin P8_43 gpio
#Motor MD
## P8_44 gpio2[9] output_MD_E
config-pin P8_44 gpio
## P8_45 gpio2[6] output_MD_D
config-pin P8_45 gpio
## P8_46 gpio2[7] output_MD_S
config-pin P8_46 gpio
