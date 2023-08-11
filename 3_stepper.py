# accel = 20.0 #steps/sec/sec
# time_passed = 0.000
# steps_done = 0
# cur_speed = 0 #steps/sec
# time_for_next_step = 0.0


# while (steps_done < steps_needed):
#     if (time_passed >= time_for_next_step):
#         self.oneStep(direction, stepstyle)



#
#
#
#

import RPi.GPIO as GPIO
from RpiMotorLib import RpiMotorLib
from time import sleep
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
#GPIO.setup()
GPIO_pins = (26, 20, 21)
direction = 20
step = 21

#motor1 = RpiMotorLib.A4988Nema(direction, step, GPIO_pins, "A4988")
#print("Test A")
#motor1.motor_go(False, "Full", 2000, .005, True, 1)

GPIO.setup(step, GPIO.OUT)

min_period = .01
max_period = .1

# for i in range(2000):
#     GPIO.output(step, 1)
#     sleep(period)
#     GPIO.output(step, 0)
#     sleep(period)
#     print(str(i))


req = 100
pos = 0
period = 0.000

while req > pos:
    print('stepping...')
    # calculate period
    position_diff = abs(pos - req)
    previous_period = period
    #period = max([previous_period + (position_diff * .1), min_period])
    period = min([max([position_diff * position_diff * .00001, min_period]), max_period])
    print('Period: ' + str(period))
    GPIO.output(step, 1)
    sleep(period)
    GPIO.output(step, 0)
    sleep(period)
    pos = pos + 1
    print('Position: ' + str(pos) + "      Request: " + str(req))
