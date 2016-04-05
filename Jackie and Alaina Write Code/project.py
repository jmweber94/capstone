""" Create a program that takes an input and uses it to turn a stepper motor by an indicated number of steps """

"""#!/usr/bin/python
# Import required libraries
import sys
import time
import RPi.GPIO as GPIO"""
 
"""# Use BCM GPIO references
# instead of physical pin numbers
GPIO.setmode(GPIO.BCM)"""
 
# Define GPIO signals to use
# Physical pins 11,15,16,18
# GPIO17,GPIO22,GPIO23,GPIO24
"""StepPins = [17,22,23,24]"""
 
"""# Set all pins as output
for pin in StepPins:
  print "Setup pins"
  GPIO.setup(pin,GPIO.OUT)
  GPIO.output(pin, False)"""
 
# Define advanced sequence
# as shown in manufacturers datasheet
Seq = [[1,0,0,1],
       [1,0,0,0],
       [1,1,0,0],
       [0,1,0,0],
       [0,1,1,0],
       [0,0,1,0],
       [0,0,1,1],
       [0,0,0,1]]
        
StepCount = len(Seq)
StepDir = 1 # Set to 1 or 2 for clockwise
            # Set to -1 or -2 for anti-clockwise

# printing outputs to verify

print Seq
print StepCount
print StepDir


"""
# Read wait time from command line
if len(sys.argv)>1:
  WaitTime = int(sys.argv[1])/float(1000)
else:
  WaitTime = 10/float(1000)


 """


#Reading arbitrary yes/no signal

response = input("Write a number")

'''
if response>5:
    WaitTime = int(response)/float(1000)
else:
    WaitTime = 10/float(1000)
print WaitTime
'''

# Initialise variables
StepCounter = 0
 
# Start main loop
while True:
    print StepCounter
    print Seq[StepCounter]

    '''
    for pin in range(0,4):
        xpin=StepPins[pin]# Get GPIO
        if Seq[StepCounter][pin]!=0:
          print " Enable GPIO %i" %(xpin)
          GPIO.output(xpin, True)
        else:
          GPIO.output(xpin, False)
    '''

    for x in range (0,4):
        xposition= Seq[1][x] #Define var in terms of position in array
        print "Enable GPIO %i", xposition

        if Seq[StepCounter][x]!=0:
            print "GPIO out", xposition
            #GPIO.output(xpositon,True)
            #pass
        else:
            print "GPIO out %s, False" %(xposition)
            #GPIO.output(xposition, False)
 
    StepCounter += StepDir

# If we reach the end of the sequence
# start again
if (StepCounter>=StepCount):
    StepCounter = 0
if (StepCounter<0):
    StepCounter = StepCount+StepDir

# Wait before moving on
#time.sleep(WaitTime)
quit()

    
