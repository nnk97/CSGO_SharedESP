# CSGO_SharedESP
Proof of concept for sharing ESP data over network.  
Both server and client use boost asio for communication over UDP.  
  
**Video showcase**:  
[![YouTube link](https://img.youtube.com/vi/6fVcXVIlHRo/0.jpg)](https://www.youtube.com/watch?v=6fVcXVIlHRo)  
  
## Note:  
This is a cheat, and using it on VAC protected servers will most likely lead to VAC ban.  
Use at your own risk, I don't recommend running that on valuable accounts without `-insecure`/`sv_lan 1`.  
  
## Used:  
* [boost library](http://www.boost.org/)  
* P4TR!CK - VMT hooking class  
* Marcus Throman - [Netvars class for Source Engine](http://marcusthorman.blogspot.com/2012/06/networked-variables-in-source-engine.html)  