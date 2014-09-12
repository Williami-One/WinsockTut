# WinsockExamples

Winsock programming examples with 7 different I/O strategies


### Build    

To build via Visual Studio simply execute:

    premake vs2013
    

### Introduction

I/O Model       | Description
----------------|------------
socket          | basic BSD socket
select          | with select()
async_select    | with WSAAsyncSelect()
async_event     | with WSAAsyncSelect()
complete_routine| with alertable I/O
overlap         | with overlapped I/O
iocp            | with Completion Port, both server and client usage


`test_tcp_client.py` is a simple python script to test server performance
