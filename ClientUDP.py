import socket
import sys
import struct
import time

hostname = sys.argv[1]
port = int(sys.argv[2])
operation = int(sys.argv[3])
message = sys.argv[4]
 
requestid = 1;

print "UDP target hostname:", hostname
print "UDP target port:", port
print "requestid:", requestid
print "operation:", operation
print "message:", message

print "Client: setting socket"
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

start = time.clock()
msglength = 5 + len(message);
msgbuf = struct.pack("h h b", msglength, 1, operation) + message #pack the message
# print ":".join("{0:x}".format(ord(c)) for c in msgbuf)

print "Client: sending data"
sock.sendto(msgbuf, (hostname, port));

data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
print "Client: response from ", addr

elapsed = (time.clock() - start)
print "Time elapsed: ", elapsed

if operation == 85:
	print "Operation 85: Number of vowels"
	data = struct.unpack("h", data);
	print "Recieved data: ", data[0]
else: 
	print "Operation 107: Disemvowel"
	print "Recieved data: ", data
