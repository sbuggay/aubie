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

print "ServerTCP.py group 4"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # set socket
s.bind(('', port)) # bind on port
s.listen(1) # listen

# infinite loop
while 1:
	print "Server: waiting to recieve..."
	conn, addr = s.accept() # accept connection
	print 'Connection address:', addr
	data = conn.recv(BUFFER_SIZE) # recieve data
	if not data: break

	print "Server: recieved packet"

	tup = modified_unpack("HHBs", data) # unpack data
	
	tml = socket.ntohs(tup[0]) # extract tml (this is a short so must use htons)
	rid = socket.ntohs(tup[1]) # extract rid (this is also a short)
	operation = tup[2] # extract operation
	message = tup[3] # extract message
	
	# display some info about the packet
	print "recieved tml:", tml
	print "recieved requestid:", rid
	print "recieved operation:", operation
	print "recieved message:", message

	# vowel length
	if operation == 85:
		message = vlength(message) # get vowel count

		tml = 6

		print "sending tml:", tml
		print "sending requestid", rid
		print "sending message:", message

		#convert host to network byte order
		tml = socket.htons(tml) #set tml (will always be 6)
		rid = socket.htons(rid)
		message = socket.htons(message)

		out = struct.pack("H H H", tml, rid, message) # pack packet
		conn.send(out) # send packet
		print "Server: sent packet"
		
	# disemvowel
	if operation == 170:
		message = disemvoweling(message) # get disemvoweled message
		
		tml = 4 + len(message)

		print "sending tml:", tml
		print "sending requestid", rid
		print "sending message:", message

		tml = socket.htons(tml) # recalculate tml
		rid = socket.htons(rid)

		out = struct.pack("H H", tml, rid) + message # pack packet
		conn.send(out) # send packet

		print "Server: sent packet"

conn.close() # close connection	
