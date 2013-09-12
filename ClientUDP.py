# group 4
# ClientUDP.py
# run with python ClientUDP.py [hostname] [port] [operation] [string]

import sys, socket, struct, time, random

BUFFER_SIZE = 100 # set buffer size

def modified_unpack(fmt, dat): # custom unpacking method dealing with ending strings
    non_str_len = struct.calcsize(fmt[:-1]) # get len of everything but the string
    str_len = len(dat) - non_str_len # get len of string
    str_fmt = "{0}s".format(str_len) # get new format
    new_fmt = fmt[:-1] + str_fmt # add it to the old format
    return struct.unpack(new_fmt, dat) # unpack it with new format

hostname = sys.argv[1] #set hostname
port = int(sys.argv[2]) #set port
operation = int(sys.argv[3]) #set operation
message = sys.argv[4] #set message
 
requestid = random.randint(0, 65535); #set the requestid to whatever

print "sending tml:", 5 + len(message)
print "sending requestid:", requestid
print "sending operation:", operation
print "sending message:", message

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #set up socket

start = time.clock()
msglength = socket.htons(5 + len(message)); #length is 5 + the length of the string
requestid = socket.htons(requestid);
msgbuf = struct.pack("H H B", msglength, requestid, operation) + message #pack the message buffer


sock.sendto(msgbuf, (hostname, port)) #send the data
print "Client: sent packet"

#wait
data, addr = sock.recvfrom(BUFFER_SIZE) #recieve data from server
print "Client: recieved packet"

elapsed = (time.clock() - start) #get elapsed time
print "Time elapsed: ", elapsed

if operation == 85: #if operation 55
	print "Operation 85: Number of vowels"
	data = struct.unpack("H H H", data)
	print "recieved tml:", socket.ntohs(data[0])
	print "recieved requestid:", socket.ntohs(data[1])
	print "recieved vowels:", socket.ntohs(data[2])
else: #else must be operation 170 (otherwise server would of thrown error)
	print "Operation 170: Disemvowel"
	data = modified_unpack("HHs", data)
	print "recieved tml:", socket.ntohs(data[0])
	print "recieved requestid:", socket.htons(data[1])
	print "recieved message:", data[2]
