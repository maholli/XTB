##############
## Script listens to serial port and writes contents into a file
##############
## requires pySerial to be installed 
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import os, serial
currentDir = os.getcwd()
write_to_file_path = "output.txt"

output_file = open(write_to_file_path, "w+")
ser = serial.Serial('com29', 115200)
collect = True


data = []

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
			# while fileContent:
			# 	try:
			# 		temp = fileContent.strip().split(',')
			# 		print(temp)
			# 		data.append((float(temp[0]),float(temp[1])))
			# 		fileContent = file.readline()
			# 	except ValueError | IndexError:
			# 		break
			# print(data)
