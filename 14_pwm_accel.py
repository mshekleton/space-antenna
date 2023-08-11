import RPi.GPIO as GPIO
import time
import math

# GPIO pins for motor control
DIR_PIN = 20
STEP_PIN = 21

# Motor parameters
STEPS_PER_REV = 200    # Number of steps per revolution
ACCELERATION = 1000     # Steps/second^2
MAX_SPEED = 1000       # Steps/second
DELAY = 0.001          # Minimum delay between steps in seconds

# Initialize GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(DIR_PIN, GPIO.OUT)
GPIO.setup(STEP_PIN, GPIO.OUT)

# Initialize PWM
pwm = GPIO.PWM(STEP_PIN, 500)  # Frequency: 500 Hz

# Function to move the stepper motor
def move_stepper(direction, num_steps):
    GPIO.output(DIR_PIN, direction)  # Set direction (True for clockwise, False for counterclockwise)
    pwm.start(50)                    # Start PWM with 50% duty cycle

    # Calculate acceleration parameters
    accel_steps = int((MAX_SPEED ** 2) / (2 * ACCELERATION))  # Number of steps to reach maximum speed
    decel_steps = accel_steps  # Number of steps to decelerate to a stop
    total_steps = accel_steps + num_steps + decel_steps

    # Perform steps
    for step in range(total_steps):
        if step < accel_steps:
            # Acceleration phase
            delay = math.sqrt((2 * step) / ACCELERATION)
        elif step < (total_steps - decel_steps):
            # Constant speed phase
            delay = DELAY
        else:
            # Deceleration phase
            decel_step = total_steps - step - 1
            if decel_step > 0:
                delay = math.sqrt((2 * decel_step) / ACCELERATION)
            else:
                delay = 0

        if delay > 0:
            pwm.ChangeFrequency(1 / delay)   # Adjust frequency based on delay
            GPIO.output(STEP_PIN, GPIO.HIGH)
            time.sleep(delay / 2)
            GPIO.output(STEP_PIN, GPIO.LOW)
            time.sleep(delay / 2)

    pwm.stop()                       # Stop PWM
    GPIO.output(STEP_PIN, GPIO.LOW)  # Set STEP pin to low

# Test the stepper motor
try:
    while True:
        steps = int(input("Enter the number of steps (positive for clockwise, negative for counterclockwise): "))
        move_stepper(steps > 0, abs(steps))
except KeyboardInterrupt:
    pass

# Cleanup GPIO
GPIO.cleanup()
