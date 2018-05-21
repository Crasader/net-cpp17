#include "acceptor.h"
#include "log.h"

 Acceptor::Acceptor(EventLoop* loop,const char *ip,int16_t port)
 :loop(loop),
  socket(ip,port),
  channel(loop,socket.getListenFd()),
  listenfd(socket.getListenFd()),
  listenning(false)
 {
	 channel.setReadCallback(std::bind(&Acceptor::handleRead,this));
 }

 Acceptor::~Acceptor()
 {
	 channel.disableAll();
	 channel.remove();
	::close(listenfd);
 }

 void Acceptor::handleRead()
 {
	loop->assertInLoopThread();
	struct sockaddr_in6 address;
	socklen_t len = sizeof(address);
#ifdef __linux__
	int32_t connfd = ::accept4(listenfd,(struct sockaddr*)&address,
	                         &len,SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif

#ifdef __APPLE__
	int32_t connfd = ::accept(listenfd,(struct sockaddr*)&address,&len);
#endif

	if (connfd >= 0)
	{
		if (newConnectionCallback)
		{
			socket.setSocketNonBlock(connfd);
			newConnectionCallback(connfd);
		}
		else
		{
			::close(connfd);
			LOG_WARN<<"handleRead";
		}
	}
	else
	{
		LOG_SYSERR << "in Acceptor::handleRead";
	}
 }

 void Acceptor::listen()
 {
	loop->assertInLoopThread();
	listenning = true;
	channel.enableReading();
 }
