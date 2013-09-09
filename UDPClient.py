import socket
import sys
import struct

hostname = sys.argv[1]
port = int(sys.argv[2])
operation = int(sys.argv[3])
message = sys.argv[4]
 
print "UDP target hostname:", hostname
print "UDP target port:", port
print "operation:", operation
print "message:", message

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

start = time.clock()

sock.sendto(message, (hostname, port))

data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes

elapsed = (time.clock() - start)

print "data is this: ", data

print "addr is this: ", addr
