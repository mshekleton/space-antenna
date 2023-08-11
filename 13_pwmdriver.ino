import RPi.GPIO as GPIO
import time

# GPIO pins for motor control
DIR_PIN = 20
STEP_PIN = 21

# Motor parameters
STEPS_PER_REV = 200  # Number of steps per revolution
DELAY = 0.001        # Delay between steps in seconds

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

    # Perform steps
    for _ in range(num_steps):
        pwm.ChangeFrequency(500)     # Adjust frequency (if needed)
        pwm.ChangeDutyCycle(50)      # Adjust duty cycle (if needed)
        GPIO.output(STEP_PIN, GPIO.HIGH)
        time.sleep(DELAY)
        GPIO.output(STEP_PIN, GPIO.LOW)
        time.sleep(DELAY)

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