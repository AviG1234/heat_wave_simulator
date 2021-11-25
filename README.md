# heat_wave_simulator
 # arduino code for system simulating heat wave.
 # temperature measurements were taken from 05/09/2018 to 24/07/2019 at 10 minute intervals.
 # the measurements were taken in the north of Israel in the city of Katzrin, 0.5 cm underneath 
 	bare soil. 
 # temperature for a specific time is the mean of all measurements in that time.
 # controle temperature is the mean in a specific time 
 # weat wave (test) temperature is the mean combine with 2 times the standard definition.
 # the system uses EEPROM memmory to overcome short period power drops. to start the 
 	system at the simulated 09:00:00 time the system must be turned on of and back on in under 
 	30 seconds. at that time the built in LED will tern on (assuming the bord use in of type 
	UNO, MEGA and ZERO, if not change '#define LED_BUILTIN 13' to -> '#define LED_BUILTIN 6').
 
