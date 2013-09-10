import socket, sys, struct

port = int(sys.argv[1])

print "Port #:", port

BUFFER_SIZE = 20
 
def disemvoweling(str):
	for char in str:
		if char in "aeiouAEIOU":
			str = str.replace(char,'')
	return str
	
def vlength(str):
	count = 0
	for char in str:
		if char in "aeiouAEIOU":
			count = count + 1
	return count
	
def modified_unpack(fmt, dat):
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
		
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', port))
s.listen(1)

conn, addr = s.accept()
print 'Connection address:', addr
while 1:
	data = conn.recv(BUFFER_SIZE)
	if not data: break
	print "received data:", data
	tup = modified_unpack("h h B s", data)
	
	tml = tup[0]
	rid = tup[1]
	operation = tup[2]
	message = tup[3]
	
	print "Tml:", tml
	print "RID:", rid
	print "Operation:", operation
	print "Message:", message
	
	if operation == 85:
		message = vlength(message)
		out = struct.pack("h h h", tml, rid, message)
		conn.send(out)
		
	if operation == 170:
		message = disemvoweling(message)
		out = struct.pack("h h", tml, rid) + message
		conn.send(out)
	
conn.close()
