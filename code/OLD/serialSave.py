##############
## Script listens to serial port and writes contents into a file
##############
## requires pySerial to be installed 
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import os, serial, time
currentDir = os.getcwd()
timestr = time.strftime("%Y%m%d-%H%M%S")

write_to_file_path = "output_"+timestr+".txt"

output_file = open(write_to_file_path, "w+")
ser = serial.Serial('com9', 115200)
collect = True

while collect == True:
	try:
		line = ser.readline()
		line = line.decode("utf-8") #ser.readline returns a binary, convert to string
		print(line.strip())
		output_file.write(line.strip()+'\n')
	except:
		output_file.close()
		collect = False
		with open(currentDir+'\\'+write_to_file_path) as file:
			fileContent = file.readline()

