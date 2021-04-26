# *tim* (Terminal Instant Messenger)
## Rudimentary P2P Instant Messenger for the *NIX Terminal
Written for the article [Socket Programming in C Part II: Writing an Instant Messenger for the Terminal](https://www.nospecialcharacters.com)

# How to build *tim*
*tim* is written using the BSD sockets API and works (as so far as it's been tested) on 
Linux and MacOS. *tim* can be built using GCC:

    gcc main.c -o tim
    
# How to use *tim*
*tim* can either be run to connect to another user or listen for an incoming connection. For instance, a 
user attempting to connect to another user at the IP address 42.91.6.1 on port 49094 would run *tim* using the 
following arguments :

    ./tim connect 42.91.6.1:49094
    
 A user running *tim* listening for a connection at that address would have to have run *tim* using the following 
 arguments before the connecting user attempts a connection:
 
    ./tim listen 49094
    
 *tim* works best when both users are on the same network as otherwise the user listening for a connection will need 
 to set their port forwarding options to be able to accept a connection. *tim* is P2P and does not have any kind of 
 functionality to get past firewalls or manage NAT Traversal.
