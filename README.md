# ECEN-602-Machine-Problems

## ECEN 602 Network Programming Assignment 2
### TCP Simple Broadcast Chat Server and Client

Test case 1: Normal operation of the chat client with three clients connected

This was tested by starting the server, connecting three clients and then each client sent a welcome message to be received by all other clients. Below is the resulting terminal output after running the scripts and giving messages to each client. 

Server side terminal output: 


./server 12345
Socket created successfully.
Socket binding successfull.
Listening for incoming connections...
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (4)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (5)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (6)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (4)
Forwarding message from User {testuser1} to all other users.
Sending data to client (5)
Sending data to client (6)
Incoming from descriptor = (5)
Forwarding message from User {testuser2} to all other users.
Sending data to client (4)
Sending data to client (6)
Incoming from descriptor = (6)
Forwarding message from User {testuser3} to all other users.
Sending data to client (4)
Sending data to client (5)

Client 1 terminal output:


./client testuser1 127.0.0.1 12345
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser1
hello from testuser1
Message from testuser2: hello from testuser2
Message from testuser3: hello from testuser3


Client 2 terminal output:


./client testuser2 127.0.0.1 12345
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser2
Message from testuser1: hello from testuser1
hello from testuser2
Message from testuser3: hello from testuser3

Client 3 terminal output: 


./client testuser3 127.0.0.1 12345
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser3
Message from testuser1: hello from testuser1
Message from testuser2: hello from testuser2
hello from testuser3



Test case 2: server rejects a client with a duplicate username

The server is started, and a client named ‘user’ joins successfully. Then, another user with the same username attempts to join, and is blocked by the server. 

Server output:


./server 12346
Socket created successfully.
Socket binding successfull.
Listening for incoming connections...
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (4)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (5)
Username already exists! Take another one!
Please choose another user name. {user} is already taken.

First client with name user joins


./client user 127.0.0.1 12346
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, user

Second client with name user tries to join


./client user 127.0.0.1 12346
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, user
Received EOF from client. Terminating connection!
Disconnected from server.





Test case 4: server rejects the client because it exceeds the maximum number of clients allowed

The maximum number of clients is 3, so the fourth client that joins is rejected. 

Server console output


./server 1234
Socket created successfully.
Socket binding successfull.
Listening for incoming connections...
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (4)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (5)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (6)
New user has joined the chat!
Acnowledged user joined!
Incoming from descriptor = (3)
Got a new connection!
Successfully established a connection with the client!
Waiting for data...
Incoming from descriptor = (7)
Server has reached maximum capacity. You cannot join!

Client 1 console output


./client testuser1 127.0.0.1 1234
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser1

Client 2 console output


./client testuser2 127.0.0.1 1234
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser2

Client 3 console output


./client testuser3 127.0.0.1 1234
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser3

Client 4 console output


./client testuser4 127.0.0.1 1234
Starting client.
Socket created successfully
Successfully connected to server.
Welcome, testuser4
Received EOF from client. Terminating connection!
Disconnected from server.
