import socket
import sys
import struct
import time

def unpack_with_final_asciiz(fmt, dat):
    if fmt[-1] not in ('z', 's') or (fmt[-1] == 's' and fmt[-2].isdigit()):
        return struct.unpack(fmt, dat)

    non_str_len = struct.calcsize(fmt[:-1])
    str_len = len(dat) - non_str_len

    if fmt[-1] == 'z':
        str_fmt = "{0}sx".format(str_len - 1)
    else:
        str_fmt = "{0}s".format(str_len)
    new_fmt = fmt[:-1] + str_fmt

    return struct.unpack(new_fmt, dat)

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
print "packed hex:", ":".join("{0:x}".format(ord(c)) for c in msgbuf)
# print ":".join("{0:x}".format(ord(c)) for c in msgbuf)

print "Client: sending data"
sock.sendto(msgbuf, (hostname, port));

data, addr = sock.recvfrom(1024)
print "Client: response from ", addr

elapsed = (time.clock() - start)
print "Time elapsed: ", elapsed

if operation == 85:
	print "Operation 85: Number of vowels"
	print "Recieved data:", data
	data = struct.unpack("h h h", data);
	print "Recieved data:", data
	print "tml:", data[0]
	print "requestid:", data[1]
	print "vowels:", data[2]
else: 
	print "Operation 107: Disemvowel"
	print "Recieved data: ", data
	data = unpack_with_final_asciiz("hhs", data)
	print "tml:", data[0]
	print "requestid:", data[1]
	print "string:", data[2]
