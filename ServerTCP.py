# group 4
# ServerTCP.py
# run with python ServerTCP.py [port]

import socket, sys, struct

port = int(sys.argv[1]) # set port

BUFFER_SIZE = 100 # set buffer size
 
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
	
def modified_unpack(fmt, dat): # custom unpacking method dealing with ending strings
    non_str_len = struct.calcsize(fmt[:-1]) # get len of everything but the string
    str_len = len(dat) - non_str_len # get len of string
    str_fmt = "{0}s".format(str_len) # get new format
    new_fmt = fmt[:-1] + str_fmt # add it to the old format
    return struct.unpack(new_fmt, dat) # unpack it with new format

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # set socket
s.bind(('', port)) # bind on port
s.listen(1) # listen

# infinite loop
while 1:
	conn, addr = s.accept() # accept connection
	print 'Connection address:', addr
	data = conn.recv(BUFFER_SIZE) # recieve data
	if not data: break
	tup = modified_unpack("h h B s", data) # unpack data
	
	tml = tup[0] # extract tml
	rid = tup[1] # extract rid
	operation = tup[2] # extract operation
	message = tup[3] # extract message
	
	# display some info about the packet
	print "Tml:", tml
	print "RID:", rid
	print "Operation:", operation
	print "Message:", message
	
	# vowel length
	if operation == 85:
		message = vlength(message) # get vowel count
		tml = 6 #set tml (will always be 6)
		out = struct.pack("h h h", tml, rid, message) # pack packet
		conn.send(out) # send packet
		
	# disemvowel
	if operation == 170:
		message = disemvoweling(message) # get disemvoweled message
		tml = 4 + len(message) # recalculate tml
		out = struct.pack("h h", tml, rid) + message # pack packet
		conn.send(out) # send packet

conn.close() # close connection	
