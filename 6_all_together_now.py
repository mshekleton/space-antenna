import smbus
import time
from time import sleep
import os
#from RPi import GPIO
import RPi.GPIO as GPIO
from threading import Thread

global cycle
cycle = 0.0

os.system('clear') #clear screen, this is just for the OCD purposes
 
step = 1 #linear steps for increasing/decreasing volume
paused = False #paused state
 
#tell to GPIO library to use logical PIN names/numbers, instead of the physical PIN numbers
GPIO.setmode(GPIO.BCM) 
GPIO.setwarnings(False)

#set up the pins we have been using
clk = 17
dt = 18
#sw = 27
direction = 20
step = 21
 
#set up the GPIO events on those pins
GPIO.setup(clk, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(dt, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(step, GPIO.OUT)
GPIO.setup(direction, GPIO.OUT)
#GPIO.setup(sw, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

min_period = .01
max_period = .1

#get the initial states
counter = 0
clkLastState = GPIO.input(clk)
dtLastState = GPIO.input(dt)
req = 100
pos = 0
period = 0.0003
#eriod = 0.0001

#swLastState = GPIO.input(sw)

# Define some device parameters
I2C_ADDR  = 0x27 # I2C device address
LCD_WIDTH = 16   # Maximum characters per line

# Define some device constants
LCD_CHR = 1 # Mode - Sending data
LCD_CMD = 0 # Mode - Sending command

LCD_LINE_1 = 0x80 # LCD RAM address for the 1st line
LCD_LINE_2 = 0xC0 # LCD RAM address for the 2nd line
LCD_LINE_3 = 0x94 # LCD RAM address for the 3rd line
LCD_LINE_4 = 0xD4 # LCD RAM address for the 4th line

LCD_BACKLIGHT  = 0x08  # On
#LCD_BACKLIGHT = 0x00  # Off

ENABLE = 0b00000100 # Enable bit

# Timing constants
E_PULSE = 0.0005
E_DELAY = 0.0005

#Open I2C interface
#bus = smbus.SMBus(0)  # Rev 1 Pi uses 0
bus = smbus.SMBus(1) # Rev 2 Pi uses 1


# Interrup callback function for encoder clk input
def clkClicked(channel):
        global counter
        global step
 
        clkState = GPIO.input(clk)
        dtState = GPIO.input(dt)
 
        if clkState == 0 and dtState == 1:
                #counter = counter + step
                counter = counter + 1
                #print ("Counter ", counter)
                #lcd_string("AZ REQ: " + str(counter) ,LCD_LINE_2)


# Interrup callback function for encoder dt input
def dtClicked(channel):
        global counter
        global step
 
        clkState = GPIO.input(clk)
        dtState = GPIO.input(dt)
         
        if clkState == 1 and dtState == 0:
                #counter = counter - step
                counter = counter -1
                #print ("Counter ", counter)
                #lcd_string("AZ REQ: " + str(counter) ,LCD_LINE_2)

GPIO.add_event_detect(clk, GPIO.FALLING, callback=clkClicked, bouncetime=1) #, bouncetime=0
GPIO.add_event_detect(dt, GPIO.FALLING, callback=dtClicked, bouncetime=1) #, bouncetime=0

def lcd_init():
  # Initialise display
  lcd_byte(0x33,LCD_CMD) # 110011 Initialise
  lcd_byte(0x32,LCD_CMD) # 110010 Initialise
  lcd_byte(0x06,LCD_CMD) # 000110 Cursor move direction
  lcd_byte(0x0C,LCD_CMD) # 001100 Display On,Cursor Off, Blink Off 
  lcd_byte(0x28,LCD_CMD) # 101000 Data length, number of lines, font size
  lcd_byte(0x01,LCD_CMD) # 000001 Clear display
  time.sleep(E_DELAY)

def lcd_byte(bits, mode):
  # Send byte to data pins
  # bits = the data
  # mode = 1 for data
  #        0 for command

  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT
  bits_low = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT

  # High bits
  bus.write_byte(I2C_ADDR, bits_high)
  lcd_toggle_enable(bits_high)

  # Low bits
  bus.write_byte(I2C_ADDR, bits_low)
  lcd_toggle_enable(bits_low)

def lcd_toggle_enable(bits):
  # Toggle enable
  time.sleep(E_DELAY)
  bus.write_byte(I2C_ADDR, (bits | ENABLE))
  time.sleep(E_PULSE)
  bus.write_byte(I2C_ADDR,(bits & ~ENABLE))
  time.sleep(E_DELAY)

def lcd_string(message,line):
  # Send string to display

  message = message.ljust(LCD_WIDTH," ")

  lcd_byte(line, LCD_CMD)

  for i in range(LCD_WIDTH):
    lcd_byte(ord(message[i]),LCD_CHR)

class Hello5Program:
    def __init__(self):
        self._running = True

    def terminate(self):
        self._running = False
    
    def run(self):
        global cycle
        while self._running:
            time.sleep(1)
            #cycle = cycle + 1.0
            #print("5 second thread cycle+1.0 - ", cycle)
            #lcd_string("AZ REQ: " + str(counter/50) ,LCD_LINE_2)
            #lcd_string("AZ POS: " + str(pos) ,LCD_LINE_1)

def main():
  # Main program block
  global period
  global pos
  global req
  # Initialise display
  lcd_init()
  global FiveSecond

  FiveSecond = Hello5Program()
  FiveSecondThread = Thread(target=FiveSecond.run)11112111111111111
  FiveSecondThread.start()


  while True:
    #time.sleep(.00005) #Why??!?!?! Why can't this be 0?
    #lcd_string("AZ REQ: " + str(counter/50) ,LCD_LINE_2)
    while (counter) > pos:
      #lcd_string("AZ REQ: " + str(counter/50) ,LCD_LINE_2)
      
      sleep_period = period + .05/((counter)-pos)
      #print(str(sleep_period))
      GPIO.output(direction, 1)
      #print('Period: ' + str(period))
      GPIO.output(step, 1)
      sleep(sleep_period)
      GPIO.output(step, 0)
      sleep(sleep_period)
      pos = pos + 1
      #print('Position: ' + str(pos) + "      Request: " + str(counter))
    while (counter) < pos:
      #lcd_string("AZ REQ: " + str(counter/50) ,LCD_LINE_2)

      sleep_period = period + .05/abs((counter)-pos)
      GPIO.output(direction, 0)
      #print('Period: ' + str(period))
      GPIO.output(step, 1)
      sleep(sleep_period) 
      GPIO.output(step, 0)
      sleep(sleep_period)
      pos = pos - 1
      #print('Position: ' + str(pos) + "      Request: " + str(counter))

if __name__ == '__main__':

  try:
    main()
  except KeyboardInterrupt:
    pass
  finally:
    lcd_byte(0x01, LCD_CMD)

FiveSecond.terminate()
GPIO.cleanup()