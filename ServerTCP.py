# group 4
# ServerTCP.py
# run with python ServerTCP.py [port]

import socket, sys, struct

port = int(sys.argv[1]) # set port

print "Port #:", port

BUFFER_SIZE = 20
 
def disemvoweling(str): # disemvowel
	for char in str:
		if char in "aeiouAEIOU":
			str = str.replace(char,'')
	return str
	
def vlength(str): # vowel count
	count = 0
	for char in str:
		if char in "aeiouAEIOU":
			count = count + 1
	return count
	
def modified_unpack(fmt, dat):
    non_str_len = struct.calcsize(fmt[:-1])
    str_len = len(dat) - non_str_len
    str_fmt = "{0}s".format(str_len)
    new_fmt = fmt[:-1] + str_fmt
    return struct.unpack(new_fmt, dat)
		
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # set socket
s.bind(('', port)) # bind on port
s.listen(1) # listen

# infinite loop
while 1:
	conn, addr = s.accept()
	print 'Connection address:', addr
	data = conn.recv(BUFFER_SIZE)
	if not data: break
	tup = modified_unpack("h h B s", data)
	
	tml = tup[0] # extract tml
	rid = tup[1] # extract rid
	operation = tup[2] # extract operation
	message = tup[3] # extract message
	
	print "Tml:", tml
	print "RID:", rid
	print "Operation:", operation
	print "Message:", message
	
	if operation == 85:
		message = vlength(message)
		tml = 6
		out = struct.pack("h h h", tml, rid, message)
		conn.send(out)
		
	if operation == 170:
		message = disemvoweling(message)
		tml = 4 + len(message)
		out = struct.pack("h h", tml, rid) + message
		conn.send(out)

	
conn.close()
