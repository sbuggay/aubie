# group 4
# CliendUDP.py
# run with python CliendUDP.py [hostname] [port] [operation] [string]

import sys, socket, struct, time

def modified_unpack(fmt, dat): #custom unpacking method dealing with ending strings
    non_str_len = struct.calcsize(fmt[:-1]) #get len of everything but the string
    str_len = len(dat) - non_str_len #get len of string
    str_fmt = "{0}s".format(str_len) #get new format
    new_fmt = fmt[:-1] + str_fmt #add it to the old format
    return struct.unpack(new_fmt, dat) #unpack it with new format

hostname = sys.argv[1] #set hostname
port = int(sys.argv[2]) #set port
operation = int(sys.argv[3]) #set operation
message = sys.argv[4] #set message
 
requestid = 1; #set the requestid to whatever

print "UDP target hostname:", hostname
print "UDP target port:", port
print "requestid:", requestid
print "operation:", operation
print "message:", message

print "Client: setting socket"
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #set up socket

start = time.clock()
msglength = 5 + len(message); #length is 5 + the length of the string
msgbuf = struct.pack("h h B", msglength, 1, operation) + message #pack the message buffer

print "Client: sending data"
sock.sendto(msgbuf, (hostname, port)) #send the data

#wait
data, addr = sock.recvfrom(1024) #recieve data from server
print "Client: response from ", addr

elapsed = (time.clock() - start) #get elapsed time
print "Time elapsed: ", elapsed

if operation == 85: #if operation 55
	print "Operation 85: Number of vowels"
	data = struct.unpack("h h h", data)
	print "tml:", data[0]
	print "requestid:", data[1]
	print "vowels:", data[2]
else: #else must be operation 170 (otherwise server would of thrown error)
	print "Operation 170: Disemvowel"
	data = modified_unpack("hhs", data)
	print "tml:", data[0]
	print "requestid:", data[1]
	print "string:", data[2]
