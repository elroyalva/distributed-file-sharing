# Distributed File Sharing

Workflow of server
1. Server starts listening on the port specified by argument for connections.
2. It gets its own public IP and hostname using methods written for it.
3. It builds a server-IP-List and adds its own entry to it.
4. It waits for connections.
5. When a client connects to it, it will update the server-IP-List and send the same to all connected clients.
6. When a server leaves the system, it’ll take out its entry from the list and update all other remaining clients
7. If help command is typed, the help page is displayed.
8. Similarly for ‘display’ and ‘creator’ command.
Workflow of client
1. A client will first start listening on the specified port. It can register to a server using the ‘register’ command
2. The client will also maintain a server-IP-List.
3. It can connect to any client on the server-IP-List except server.
4. All the clients on the system can be viewed using ‘list’ command.
5. A client can quit the system using ‘quit’ command.
6. ‘help’, ‘creator’ and ’display’ perform the same functions as server.


Can be started in 2 modes.
client mode and server mode.

Part of the code has been removed to discourage plagiarism.
Connection and disconnection of clients from server is implemented.

moved from http://github.com/epalva/DistFS
