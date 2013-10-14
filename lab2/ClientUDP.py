# group 4
# ClientUDP.py
# run with python ClientUDP.py [hostname] [port] [operation] [string]

import sys, socket, struct, time, random

BUFFER_SIZE = 1000 # set buffer size
debug = 1 # set to print out debug info

if len(sys.argv) < 6:
	print "usage: [hostname] [port] [operation] [hostnames...]"

hostname = sys.argv[1] #set hostname
port = int(sys.argv[2]) #set port
requestid = int(sys.argv[3]) # set the requestid to whatever
gid = 4 # set group number

# initilize hostname variables
hostname_list = []
hostname_size = 0
for x in range (4, len(sys.argv)):
	hostname_list.append(sys.argv[x])
	hostname_size += len(sys.argv[x])

hostname_size += len(hostname_list) # add space for each delimeter

tml = 5 + hostname_size # 5 for header plus the size of hostnames

hostname_string = "" # initilize the string that will hold every hostname
for x in range (0, len(hostname_list)): # create string
		hostname_string += "~" + hostname_list[x] # add delimter only before, not one on the end

# print out some debug information
if debug == 1:
	print "[", tml, "|", gid, "|", requestid, "|", hostname_string, "]"
	print "sending tml:", tml # 2 bytes
	print "sending checksum:", 0
	print "sending gid:", gid # 1 byte
	print "sending requestid:", requestid # 1 byte
	print "sending hostnames:", hostname_list # x bytes

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #set up socket

tml = socket.htons(tml) # set network byte order for TML
message = struct.pack("HBBB", tml, 0, gid, requestid) + hostname_string

print message[0], message[1];

start = time.clock() # start timing clock
sock.sendto(message, (hostname, port)) # send the data
print "[CLIENT] sent packet"

data, addr = sock.recvfrom(BUFFER_SIZE) # recieve data from server
print "[CLIENT] recieved packet"

# print time elapsed
elapsed = (time.clock() - start) # get elapsed time
print "[CLIENT] Time elapsed: ", elapsed

# print ":".join("{0:x}".format(ord(c)) for c in data)

# unpack data

iplen = (len(data) - struct.calcsize("HBBB")) / 4
data = struct.unpack("=HBBB{0}I".format(iplen), data)

# print debug info
if debug == 1:
	print "recieved tml:", socket.ntohs(data[0])
	print "recieved checksum:", data[1]
	print "recieved gid:", data[2]
	print "recieved requestid", data[3]

for x in range (0, iplen):
	ip1 = struct.pack("!I", data[x + 4]) # pack it into network order
	text = socket.inet_ntoa(ip1) # must be packed to translate to 4 byte . format
	print hostname_list[x], "resolved to", text


