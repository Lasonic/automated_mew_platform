import sys
import os

import serial
import time
#################
### IMPORTANT ###
# Selecte the correct COM port where Arduino Due is connected !
due_COM_port = 'COM14'
#################

if os.name == 'nt':
    from serial.tools.list_ports_windows import comports
elif os.name == 'posix':
    from serial.tools.list_ports_posix import comports
else:
    raise ImportError("Sorry: no implementation for your platform ('{}') available".format(os.name))

def parse_port_descriptors(portname, name, desc):
    result = {
        'portname' : portname,
        'name' : name
    }
    if desc:
        # split the description by the spaces
        desc = desc.split()
        for d in desc:
            # if a portion of the description has a '=', we use it
            tlist = d.split('=')
            if len(tlist)==2:
                # split by ':' incase its the 'VID:PID=1D50:6015' portion
                keys = [tlist[0]] if ':' not in tlist[0] else tlist[0].split(':')
                vals = [tlist[1]] if ':' not in tlist[1] else tlist[1].split(':')
                for i in range(len(keys)):
                    if i<len(vals):
                        # save the key and value pair to this port's descriptor
                        tkey = keys[i].lower()
                        result[tkey] = vals[i]

    return result

def find_smoothie():

    iterator = sorted(comports())

    for n, (port, name, desc) in enumerate(iterator, 1):
        # parse the USB serial port's description string
        port_data = parse_port_descriptors(port, name, desc)
        # all smoothieboards have a 'name' attribute
        if port_data.get('pid')=='6015':
            return port_data

def create_connection(port_desc):
    if port_desc:
        try:
            return serial.Serial(port_desc.get('portname'), 115200, timeout=0.2)

        except (OSError, serial.SerialException):
            pass

def write_string_to_port(port, data):
    data = data+'\r\n'
    return port.write(str.encode(data))

connected = False

print('not connected')
run = True
while True and run == True:
    # connect to Arduino Due
    #try:
    #    smoothie2 = serial.Serial('COM7',115200,timeout=0.2)
    #    print("connected to smooothie via UART!")
    #except:
    #    print("Ooops! Can't connect to smoothie via UART...") 
    try:
        due = serial.Serial(due_COM_port,115200,timeout=0.2)
        print("Connected to Due!")
    except:
        print("Ooops! Can't connect to Due.") 
        pass
    
    # connect to smoothie
    smoothie_desc = find_smoothie()
    if smoothie_desc:
        print('attemping connection to {}...'.format(smoothie_desc['portname']))
        this_port = create_connection(smoothie_desc)
        while this_port and this_port.is_open and run == True:
            if not connected:
                print('Connnected to Smoothieboard!')    
            connected = True

            # Select the board
            input_board = input("Select smoothie(s), smoothie_UART(s2) or due(d):")
            if input_board == 's':
                try:
                    input_gcode = input("Input command to smoothie:")
                    write_string_to_port(this_port,input_gcode)
                    data = ''
                    #data = this_port.readline().decode('UTF-8')
                    while data != 'ok\r\n':
                        #print(data) 
                        data = this_port.readline().decode('UTF-8')
                        time.sleep(.2)
                        print(data)
                except (OSError, serial.SerialException):
                    break
            if input_board == 's2':
                try:    
                    input_gcode = input("Input command to smoothie:")
                    write_string_to_port(smoothie2,input_gcode)
                    data = ''
                    #data = this_port.readline().decode('UTF-8')
                    while data != 'ok\r\n':
                    #print(data) 
                        data = smoothie2.readline().decode('UTF-8')
                        time.sleep(.2)
                        print(data)
                except (OSError, serial.SerialException):
                    break                
            if input_board  == 'd':
                try:
                    input_due = input("Input command to due:")
                    print (input_due)
                    #due.reset_output_buffer()
                    due.write(input_due.encode()) # send as a string.
                    data_due = ''
                    
                    while "\r\n" not in data_due:
                        data_due = due.readline().decode('UTF-8')
                        time.sleep(.2)
                        print(data_due)
                        
                    #due.write(str.encode(input_due))
                except:
                    print ("Failed to send the command")
                    pass
            if input_board == "flush":
                this_port.flushInput()
                try:
                    due.flushInput()
                except:
                   print ("Failed to flush")
                   pass
            if input_board == "exit":
                run = False
                try:
                    this_port.close()
                    due.close()
                except:
                    pass
    if connected:
        print('not connected')
    connected = False
    time.sleep(.2)
