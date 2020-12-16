# OpenMVG_grpc


Directory Working on is grpcsrc.

Features: 

- Working AsyncBidirectional Streaming Data between client and server. 

Todo: 

-  Unary Streams 
-  File Sents 
-  Library functions for the computing





To run: 

- mkdir build
- cmake ..
- make -j4

Folder working on in grpcsrc folder. 

- Client functions working on the nodeclient.cc 
- Main Leader file is called nodeserver.cc 

- protobuff file is masterworker.proto

- Rest of the file in folder is for testing. 

End Goal is have one file combined both nodeclient and nodeserver. 

- node.cc will be the end file which represents one node entity.








