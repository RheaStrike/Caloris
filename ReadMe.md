# Caloris #

![](http://github.com/RheaStrike/Caloris/raw/master/PubImage/calroislogo.PNG)

## What is Caloris? ##

**Caloris** is a simple Win32 library for Socket.IO, without Boost::Asio. Boost::Asio is most powerful library, but Asio's IOCP model have too many thread.
So, Caloris select **WSAEventSelect** model.

## Websocket protocol ##

Socket.IO is a module of node.js that base on RFC6455, but modified.
I was analyzed Socket.IO's TCP packet and write packet code on C/C++.
See more [http://rhea.pe.kr/515](http://rhea.pe.kr/515).

![](http://github.com/RheaStrike/Caloris/raw/master/PubImage/websocket_framing.PNG)

## Socket.IO protocol ##

![](http://github.com/RheaStrike/Caloris/raw/master/PubImage/ws.png)

## WSAEventSelect ##

Caloris's socket model is WSAEventSelect. This is an asynchronous I/O and very fast. I like WSAEventSelect on client side. See more 
[http://msdn.microsoft.com/en-us/library/windows/desktop/ms741576(v=vs.85).aspx](http://msdn.microsoft.com/en-us/library/windows/desktop/ms741576(v=vs.85).
![](http://github.com/RheaStrike/Caloris/raw/master/PubImage/EventCaloris.PNG)

## Why Caloris? ##

I am developing an MMORPG. When I need community module, I select the Socket.IO.
But there is not Socket.IO's C++ client that make write Caloris.

## Enjoy WebScoket: node.js + Socket.IO + Caloris ##

To be continued...
