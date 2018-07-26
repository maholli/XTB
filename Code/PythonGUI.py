import serial
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation
from matplotlib import style
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter import *

import time

import sys
import glob


ser = serial.Serial()
ser.baudrate = 115200
frame_active = False

def get_serial_ports():
	if sys.platform.startswith('win'):
		ports = ['COM%s' % (i + 1) for i in range(256)]
	elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
		# this excludes your current terminal "/dev/tty"
		ports = glob.glob('/dev/tty[A-Za-z]*')
	elif sys.platform.startswith('darwin'):
		ports = glob.glob('/dev/tty.*')
	else:
		raise EnvironmentError('Unsupported platform')

	result = []
	for port in ports:
		try:
			s = serial.Serial(port)
			s.close()
			result.append(port)
		except (OSError, serial.SerialException):
			pass
	
	return result

def refresh_serial_ports():
	port_dropdown["menu"].delete(0, "end")
	for c in get_serial_ports():
		port_dropdown["menu"].add_command(label=c, command=tk._setit(dVar_port, c))

def set_serial_port():
	if (ser.isOpen()):
		ser.close()
		disable_regs()
		start_button.config(state=DISABLED)
		port_button.config(text="Connect")
		port_dropdown.config(state=NORMAL)
		refresh_button.config(state=NORMAL)
	else:
		ser.port = dVar_port.get()
		ser.open()
		ADC_stop()
		enable_regs()
		start_button.config(state=NORMAL)
		port_button.config(text="Disconnect")
		port_dropdown.config(state=DISABLED)
		refresh_button.config(state=DISABLED)

def clear_data():
	global yList, num_samples, dataFile
	yList = []
	num_samples = 0
	dataFile.close()
	dataFile = open("temp_data.txt", "w")
	data_slider.config(from_=0, to=0)


dataFile = open("temp_data.txt", "w")

def save_data():
	path = filedialog.asksaveasfilename(filetypes=(("text files", "*.txt"),("all files","*.*")))
	if (path != ""):
		if (path[-4:] != ".txt"):
			path += ".txt"
		global dataFile
		dataFile.close()
		data_out = open(path, "w")
		lines = [line.rstrip("\n") for line in open("temp_data.txt")]
		for line in lines:
			if line != "":
				data_out.write(line + "\n")

		dataFile = open("temp_data.txt", "a")

def open_data():
	path = filedialog.askopenfilename(filetypes=(("text files", "*.txt"),("all files","*.*")))
	if (path != ""):
		global saved_yList
		saved_yList = []
		#if (ser.isOpen()):
			#ser.close()
		lines = [line.rstrip('\n') for line in open(path)]
		for line in lines:
			if line != "":
				saved_yList.append(float(line[(line.find(",")+1):]))
		data_slider.config(from_=min(len(saved_yList), MAX_LIST_SIZE), to=len(saved_yList))
		data_slider.set(min(len(saved_yList), MAX_LIST_SIZE))

#========================== ADC FUNCTIONS ==========================#
def ADC_readout():
	ser.write(b"readout__")

def ADC_test():
	ser.write(b"test__")
reg_objects = []


def enable_regs():
	for i in range(0, len(reg_objects)):
		reg_objects[i].config(state=NORMAL)


def disable_regs():
	for i in range(0, len(reg_objects)):
		reg_objects[i].config(state=DISABLED)

def ADC_stop():
	dataFile.flush()
	global frame_active
	frame_active = False

	global saved_yList
	saved_yList = []
	lines = [line.rstrip('\n') for line in open("temp_data.txt")]
	for line in lines:
		if line != "":
			saved_yList.append(float(line[(line.find(",")+1):]))

	start_button.config(state=NORMAL)
	stop_button.config(state=DISABLED)
	save_button.config(state=NORMAL)
	open_button.config(state=NORMAL)
	clear_button.config(state=NORMAL)
	port_button.config(state=NORMAL)
	#port_dropdown.config(state=NORMAL)

	enable_regs()
	if (ser.isOpen()):
		ser.write(b"stop__")
		#ser.close()
		refresh_serial_ports()

def ADC_start():
	#ser.port = dVar_port.get()
	#ser.open()
	global frame_active
	frame_active = True

	start_button.config(state=DISABLED)
	stop_button.config(state=NORMAL)
	save_button.config(state=DISABLED)
	open_button.config(state=DISABLED)
	clear_button.config(state=DISABLED)
	port_button.config(state=DISABLED)
	#port_dropdown.config(state=DISABLED)

	disable_regs()
	ser.write(b"start__")

def ADC_setpins(a, b, c):
	message = "setinputpins "+ dVar_pos.get() +" "+ dVar_neg.get() +"__"
	ser.write(message.encode())

def ADC_setidac(a, b, c):
	message = "setidacpins "+ dVar_idac1.get() +" "+ dVar_idac2.get() + " " + dVar_idac.get() + "__"
	ser.write(message.encode())

def ADC_setgain(a, b, c):
	message = "setgain "+ dVar_gain.get() +"__"
	ser.write(message.encode())

def ADC_setdatarate(a, b, c):
	message = "setdatarate "+ dVar_data.get() +"__"
	ser.write(message.encode())

def ADC_setpga(a, b, c):
	message = "setpga "+ dVar_pga.get() +"__"
	ser.write(message.encode())

def ADC_setfilter(a, b, c):
	message = "setfilter "+ dVar_filter.get() +"__"
	ser.write(message.encode())

def ADC_setchop(a, b, c):
	message = "setchop "+ dVar_chop.get() +"__"
	ser.write(message.encode())

def ADC_setvbias(a, b, c):
	message = "setvbias "+dVar_vbias.get()+"__"
	ser.write(message.encode())
	time.sleep(.1)
	message = "setvbiaspins "+str(c0.get())+" "+str(c1.get())+" "+str(c2.get())+" "+str(c3.get())+" "+str(c4.get())+" "+str(c5.get())+"__"
	ser.write(message.encode())

def ADC_pin_vars(a, b, c):
	message = "setreadmode "
	if pinmode.get() == 1:
		csmode.set(value=0)
		message += "pin__"
	else:
		pinmode.set(value=0)
		csmode.set(value=1)
		message += "currentspin__"
	ser.write(message.encode())

def ADC_cs_vars(a, b, c):
	message = "setreadmode "
	if csmode.get() == 1:
		pinmode.set(value=0)
		message += "currentspin__"
	else:
		pinmode.set(value=1)
		csmode.set(value=0)
		message += "pin__"
	ser.write(message.encode())

def ADC_set_cs_pins(a, b, c):
	message = "setcspins "+dVar_csN.get()+" "+dVar_csE.get()+" "+dVar_csW.get()+" "+dVar_csS.get()+"__"
	ser.write(message.encode())

#========================== UI ELEMENTS ==========================#
root = tk.Tk()
tk.Tk.wm_title(root, "Sensor Analysis")
#label = tk.Label(root, text="Sensor Analysis")
#label.grid(row=0, column=4)

LARGE_FONT= ("Verdana", 12)

quit_button = ttk.Button(root, text="Quit",command=quit)
quit_button.grid(row=0, column=5)

#pause_button = ttk.Button(root, text="Pause (3s)", command=ADC_test)
#pause_button.grid(row=3, column=5)

start_button = ttk.Button(root, text="Start", command=ADC_start)
start_button.grid(row=1, column=5)

stop_button = ttk.Button(root, text="Stop", command=ADC_stop, state=DISABLED)
stop_button.grid(row=2, column=5, sticky=N)

clear_button = ttk.Button(root, text="Clear Data", command=clear_data)
clear_button.grid(row=3, column=5)

open_button = ttk.Button(root, text="View Saved Data", command=open_data)
open_button.grid(row=46, column=7)

save_button = ttk.Button(root, text="Save All Data to File", command=save_data)
save_button.grid(row=46, column=5, columnspan=2)

root.grid_columnconfigure(6, minsize=60)
root.grid_columnconfigure(10, minsize=40)


pinmode = IntVar(value=1);
pin_mode_button = Checkbutton(root, text="Input Pins", font=LARGE_FONT, variable=pinmode)
pin_mode_button.grid(row=0, column=12, columnspan=2, sticky=S);
pinmode.trace("w", ADC_pin_vars)
reg_objects.append(pin_mode_button)

dVar_pos = StringVar(root)
dVar_pos.set("COM")
pos_dropdown = OptionMenu(root, dVar_pos, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "COM")
pos_dropdown.grid(row=1, column=12)
poslabel = tk.Label(root, text="Input\nPositive", font=LARGE_FONT)
poslabel.grid(row=2, column=12, sticky=N)
dVar_pos.trace("w", ADC_setpins)
reg_objects.append(pos_dropdown)

dVar_neg = StringVar(root)
dVar_neg.set("COM")
neg_dropdown = OptionMenu(root, dVar_neg, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "COM")
neg_dropdown.grid(row=1, column=13)
neglabel = tk.Label(root, text="Input\nNegative", font=LARGE_FONT)
neglabel.grid(row=2, column=13, sticky=N)
dVar_neg.trace("w", ADC_setpins)
reg_objects.append(neg_dropdown)

idaclabeltop = tk.Label(root, text="IDAC Pins", font=LARGE_FONT)
idaclabeltop.grid(row=0, column=6, columnspan=2, sticky=S)

dVar_idac1 = StringVar(root)
dVar_idac1.set("Off")
idac1_dropdown = OptionMenu(root, dVar_idac1, "Off", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11")
idac1_dropdown.grid(row=1, column=6)
idac1label = tk.Label(root, text="Input\nPositive", font=LARGE_FONT)
idac1label.grid(row=2, column=6, sticky=N)
dVar_idac1.trace("w", ADC_setidac)
reg_objects.append(idac1_dropdown)

dVar_idac2 = StringVar(root)
dVar_idac2.set("Off")
idac2_dropdown = OptionMenu(root, dVar_idac2, "Off", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11")
idac2_dropdown.grid(row=1, column=7)
idac2label = tk.Label(root, text="Input\nNegative", font=LARGE_FONT)
idac2label.grid(row=2, column=7, sticky=N)
dVar_idac2.trace("w", ADC_setidac)
reg_objects.append(idac2_dropdown)

dVar_idac = StringVar(root)
dVar_idac.set("Off")
idac_dropdown = OptionMenu(root, dVar_idac, "Off", "10", "50", "100", "250", "500", "750", "1000", "1500", "2000")
idac_dropdown.grid(row=1, column=9)
idaclabel = tk.Label(root, text="IDAC Current (uA)", font=LARGE_FONT)
idaclabel.grid(row=0, column=9, sticky=S)
dVar_idac.trace("w", ADC_setidac)
reg_objects.append(idac_dropdown)

dVar_gain = StringVar(root)
dVar_gain.set("1")
gain_dropdown = OptionMenu(root, dVar_gain, "1", "2", "4", "8", "16", "32", "64", "128")
gain_dropdown.grid(row=5, column=7)
gainlabel = tk.Label(root, text="Gain", font=LARGE_FONT)
gainlabel.grid(row=4, column=7, sticky=S)
dVar_gain.trace("w", ADC_setgain)
reg_objects.append(gain_dropdown)

dVar_data = StringVar(root)
dVar_data.set("100")
data_dropdown = OptionMenu(root, dVar_data, "60", "100", "200", "400", "800", "1000", "2000", "4000")
data_dropdown.grid(row=8, column=7, sticky=E)
datalabel = tk.Label(root, text="Datarate\n(SPS)", font=LARGE_FONT)
datalabel.grid(row=7, column=7, sticky=S)
dVar_data.trace("w", ADC_setdatarate)
reg_objects.append(data_dropdown)

port_options = get_serial_ports()

dVar_port = StringVar(root)
dVar_port.set(port_options[0])
port_dropdown = OptionMenu(root, dVar_port, *port_options)
port_dropdown.grid(row=33, column=5, sticky=E)
portlabel = tk.Label(root, text="Port", font=LARGE_FONT)
portlabel.grid(row=32, column=5, sticky=S)

port_button = ttk.Button(root, text="Connect", command=set_serial_port)
port_button.grid(row=34, column=5)

refresh_button = ttk.Button(root, text="Refresh", command=refresh_serial_ports)
refresh_button.grid(row=35, column=5)

data_slider = Scale(root, from_=0, to=0, orient=HORIZONTAL)
data_slider.grid(row=44, column=5, columnspan=4, sticky=W+E)


dVar_pga = StringVar(root)
dVar_pga.set("Enabled")
pga_dropdown = OptionMenu(root, dVar_pga, "Enabled", "Bypassed", "IP_Buffer_Bypassed")
pga_dropdown.grid(row=5, column=6, sticky=E)
pgalabel = tk.Label(root, text="PGA", font=LARGE_FONT)
pgalabel.grid(row=4, column=6, sticky=S)
dVar_pga.trace("w", ADC_setpga)
reg_objects.append(pga_dropdown)

dVar_filter = StringVar(root)
dVar_filter.set("Low_Latency")
filter_dropdown = OptionMenu(root, dVar_filter, "Low_Latency", "SINC3")
filter_dropdown.grid(row=8, column=6, sticky=E)
filterlabel = tk.Label(root, text="Filter", font=LARGE_FONT)
filterlabel.grid(row=7, column=6, sticky=S)
dVar_filter.trace("w", ADC_setfilter)
reg_objects.append(filter_dropdown)

dVar_chop = StringVar(root)
dVar_chop.set("Off")
chop_dropdown = OptionMenu(root, dVar_chop, "On", "Off")
chop_dropdown.grid(row=11, column=7, sticky=E)
choplabel = tk.Label(root, text="Chop", font=LARGE_FONT)
choplabel.grid(row=10, column=7, sticky=S)
dVar_chop.trace("w", ADC_setchop)
reg_objects.append(chop_dropdown)

root.grid_columnconfigure(9, minsize=40)

dVar_vbias = StringVar(root)
dVar_vbias.set("1.65V")
vbias_dropdown = OptionMenu(root, dVar_vbias, "1.65V", "0.275V")
vbias_dropdown.grid(row=5, column=9, sticky=E)
vbiaslabel = tk.Label(root, text="Apply Vbias:", font=LARGE_FONT)
vbiaslabel.grid(row=4, column=9, sticky=S)
dVar_vbias.trace("w", ADC_setvbias)
reg_objects.append(vbias_dropdown)

c0 = IntVar();
check_0 = Checkbutton(root, text="AIN 0", variable=c0)
check_0.grid(row=6, column=9);
c0.trace("w", ADC_setvbias)
reg_objects.append(check_0)

c1 = IntVar();
check_1 = Checkbutton(root, text="AIN 1", variable=c1)
check_1.grid(row=7, column=9);
c1.trace("w", ADC_setvbias)
reg_objects.append(check_1)

c2 = IntVar();
check_2 = Checkbutton(root, text="AIN 2", variable=c2)
check_2.grid(row=8, column=9);
c2.trace("w", ADC_setvbias)
reg_objects.append(check_2)

c3 = IntVar();
check_3 = Checkbutton(root, text="AIN 3", variable=c3)
check_3.grid(row=9, column=9);
c3.trace("w", ADC_setvbias)
reg_objects.append(check_3)

c4 = IntVar();
check_4 = Checkbutton(root, text="AIN 4", variable=c4)
check_4.grid(row=10, column=9);
c4.trace("w", ADC_setvbias)
reg_objects.append(check_4)

c5 = IntVar();
check_5 = Checkbutton(root, text="AIN 5", variable=c5)
check_5.grid(row=11, column=9);
c5.trace("w", ADC_setvbias)
reg_objects.append(check_5)


# + current spinning

csmode = IntVar();
cs_mode_button = Checkbutton(root, text="Current Spinning Pins", font=LARGE_FONT, variable=csmode)
cs_mode_button.grid(row=5, column=11, columnspan=3, sticky=S);
reg_objects.append(cs_mode_button)
csmode.trace("w", ADC_cs_vars)

dVar_csN = StringVar(root)
dVar_csN.set("0")
csN_dropdown = OptionMenu(root, dVar_csN, "0", "1", "2", "3", "4", "5")
csN_dropdown.grid(row=6, column=12)
dVar_csN.trace("w", ADC_set_cs_pins)
reg_objects.append(csN_dropdown)

dVar_csE = StringVar(root)
dVar_csE.set("0")
csE_dropdown = OptionMenu(root, dVar_csE, "0", "1", "2", "3", "4", "5")
csE_dropdown.grid(row=7, column=13, sticky=W)
dVar_csE.trace("w", ADC_set_cs_pins)
reg_objects.append(csE_dropdown)

dVar_csW = StringVar(root)
dVar_csW.set("0")
csW_dropdown = OptionMenu(root, dVar_csW, "0", "1", "2", "3", "4", "5")
csW_dropdown.grid(row=7, column=11, sticky=E)
dVar_csW.trace("w", ADC_set_cs_pins)
reg_objects.append(csW_dropdown)

dVar_csS = StringVar(root)
dVar_csS.set("0")
csS_dropdown = OptionMenu(root, dVar_csS, "0", "1", "2", "3", "4", "5")
csS_dropdown.grid(row=8, column=12)
dVar_csS.trace("w", ADC_set_cs_pins)
reg_objects.append(csS_dropdown)

#========================== ANIMATED PLOT ==========================#
disable_regs()
start_button.config(state=DISABLED)

LARGE_FONT= ("Verdana", 12)
style.use("ggplot")

f = Figure(figsize=(5,5), dpi=100)
a = f.add_subplot(111)

yList = []
saved_yList = []

MAX_LIST_SIZE = 100
num_samples = 0;

def animate(i):
	global frame_active
	if not (frame_active):
		xList = list(range(max(data_slider.get()-MAX_LIST_SIZE, 0), data_slider.get()))
		a.clear()
		view_yList = []

		for n in range(max(data_slider.get()-MAX_LIST_SIZE, 0), data_slider.get()):
			view_yList.append(saved_yList[n])
		a.plot(xList, view_yList)

	else:
		if (ser.inWaiting() > 0):
			line = ser.readline().decode("utf-8")
			dataFile.write(line)
			text = line.split(',')
			yList.append(float(text[1]))
			if (len(yList) > MAX_LIST_SIZE):
				yList.pop(0)


			global num_samples
			num_samples += 1;
			xList = list(range(max(num_samples-MAX_LIST_SIZE, 0), num_samples))
			a.clear()
			a.plot(xList, yList)
			data_slider.config(from_=min(num_samples, MAX_LIST_SIZE), to=num_samples)
			data_slider.set(num_samples)

canvas = FigureCanvasTkAgg(f, root)
canvas.draw()
canvas.get_tk_widget().grid(row=0, column=0, rowspan=48, columnspan=4)


#========================== MAIN EXECUTION ==========================#
ani = animation.FuncAnimation(f, animate, interval=1)
root.mainloop()
