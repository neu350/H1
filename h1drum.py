from tkinter import *
from tkinter import font
from tkinter import ttk

import serial
import serial.tools.list_ports
import time

arduinoPort = "/dev/cu.usbmodem1421"

all_ports = list(serial.tools.list_ports.comports()) # list of a 3-tuple for each port
ports = [(port, desc) for port, desc, hwid in all_ports if "Arduino" in desc]
if len(ports) > 0:
    arduinoPort, arduinoDesc = ports[0]
    print("Detected port: %s (%s)" % (arduinoPort, arduinoDesc))
else:
    print("Could not auto-detect port. Using %s as default." % arduinoPort)
    print("Ports:")
    print(all_ports)




ser = serial.Serial(arduinoPort, 115200)

def go(*args):
    try:
        ia = float(mode.get())
        ib = speed.get()*100
        ic = polarity.get()
        ie = max(500,(step.get())*1000)
        ii = format("6,%d;0,%s;2,%d;3,%d;4;" % (ia,ib,ic,ie))
        command.set(ii)
        iibyte=bytearray(ii,'ascii')
        ser.write(iibyte)
        print(ii)
        print(iibyte)
    except ValueError:
        pass

def stop(*args):
    try:
     #   value = float(feet.get())
     #   meters.set(speed)
        command.set("5;")
        stopbyte=bytearray("5;",'ascii')
        ser.write(stopbyte)
        print(stopbyte)
    except ValueError:
        pass
    
root = Tk()
root.title("H1 carousel controller")

command = StringVar()

mainframe = ttk.Frame(root, padding="3 3 12 12")
mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
mainframe.columnconfigure(0, weight=1)
mainframe.rowconfigure(0, weight=1)



headFont = font.Font(family='Helvetica', size=16, weight='bold')
smallFont = font.Font(family='Helvetica', size=8)

porty = StringVar()
porty = "Arduino on port: %s" % arduinoPort
ttk.Label(mainframe, text=porty).grid(column=1, row=8, columnspan=3)

#ttk.Label(mainframe, text="Mode", font=headFont).grid(column=1, row=1, sticky=S)
#ttk.Label(mainframe, text="Speed", font=headFont).grid(column=2, row=1, sticky=S)
#ttk.Label(mainframe, text="step length (s)", font=headFont).grid(column=3, row=1, sticky=S)

ttk.Label(mainframe, textvariable=command).grid(column=1, row=6, sticky=S, columnspan=3)


mode = IntVar()
modeframe = ttk.Labelframe(mainframe, text='Mode')
m1 = ttk.Radiobutton(modeframe, text='oscillate', variable=mode, value='1').grid(column=1, row=2, sticky=(S,W))
m2 = ttk.Radiobutton(modeframe, text='continuous', variable=mode, value='2').grid(column=1, row=3, sticky=W)
m3 = ttk.Radiobutton(modeframe, text='steps', variable=mode, value='3').grid(column=1, row=4, sticky=W)
mode.set('1')

modeframe.grid(column=1,row=1)

polarity = IntVar()
polframe = ttk.Labelframe(modeframe, text='Polarity')
m4 = ttk.Radiobutton(polframe, text='CCW', variable=polarity, value='1', ).grid(column=2, row=1, sticky=E)
m5 = ttk.Radiobutton(polframe, text='CW', variable=polarity, value='-1', ).grid(column=1, row=1, sticky=W)
polarity.set(1)
spacer=ttk.Frame(modeframe, width=10, height=20).grid(column=1,row=5)
polframe.grid(column=1,row=6)

speed = DoubleVar()
speedframe = ttk.Labelframe(mainframe, text='Speed')
slider = Scale(speedframe, orient=HORIZONTAL, variable=speed, length=100, from_=1, to=20, resolution=0.5).grid(column=2, row=2, rowspan=2)
speed.set(3.5)
speedframe.grid(column=2,row=1)

step = DoubleVar()
stepframe = ttk.Labelframe(mainframe, text='Step/Osc Duration')
step_entry = ttk.Entry(stepframe, width=7, textvariable=step)
step_entry.grid(column=1, row=1, sticky=(W, E))
ttk.Label(stepframe, text="(min 0.5s)").grid(column=1, row=2, sticky=N)
step.set(1)
stepframe.grid(column=3,row=1)



#ttk.Label(mainframe, textvariable=round(speed.get(),2)).grid(column=2, row=2, sticky=(W, E))
#ttk.Label(mainframe, textvariable=meters).grid(column=2, row=2, sticky=(W, E))


ttk.Button(mainframe, text="go", command=go).grid(column=3, row=9, sticky=W)
ttk.Button(mainframe, text="stop", command=stop).grid(column=2, row=9, sticky=W)

#ttk.Label(mainframe, text="feet").grid(column=3, row=1, sticky=W)
#ttk.Label(mainframe, text="is equivalent to").grid(column=1, row=2, sticky=E)
#ttk.Label(mainframe, text="meters").grid(column=3, row=2, sticky=W)

for child in mainframe.winfo_children(): child.grid_configure(padx=5, pady=5)
step_entry.grid_configure(padx=25)
step_entry.focus()
root.bind('<Return>', go)
root.bind('<Escape>', stop)

root.mainloop()