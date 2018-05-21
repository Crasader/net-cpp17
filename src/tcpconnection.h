#pragma once
#include "all.h"
#include "buffer.h"
#include "callback.h"
#include "channel.h"

class EventLoop;
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	TcpConnection(EventLoop *loop,int32_t sockfd,const std::any &context);
	~TcpConnection();

	EventLoop *getLoop() { return loop; }
	int32_t getSockfd() { return sockfd; }
	void setState(StateE s) { state  = s; }

	void setConnectionCallback(const ConnectionCallback &&cb)
	{ connectionCallback = std::move(cb); }

	void setMessageCallback(const MessageCallback &&cb)
	{ messageCallback = std::move(cb); }

	void setWriteCompleteCallback(const WriteCompleteCallback &&cb)
	{ writeCompleteCallback = std::move(cb); }

	void setHighWaterMarkCallback(const HighWaterMarkCallback &&cb,size_t highWaterMark)
	{ highWaterMarkCallback = std::move(cb); this->highWaterMark  = highWaterMark; }

	void setCloseCallback(const CloseCallback &cb)
	{ closeCallback = cb; }

	void sendInLoop(const void *message,size_t len);
	void sendInLoop(const StringPiece &message);
	void sendPipeInLoop(const void *message,size_t len);
	void sendPipeInLoop(const StringPiece &message);

	static void bindSendInLoop(TcpConnection *conn,const StringPiece &message);
	static void bindSendPipeInLoop(TcpConnection *conn,const StringPiece &message);
	
	void sendPipe(const StringPiece &message);
	void sendPipe(Buffer *message);
	void sendPipe(const void *message,int32_t len);

  	void send(const void *message,int32_t len);
	void send(Buffer *message);
	void send(const StringPiece &message);

	bool disconnected() const { return state == kDisconnected; }
	bool connected() { return state == kConnected; }
	void forceCloseInLoop();
	void connectEstablished();
	void forceCloseWithDelay(double seconds);
	void forceCloseDelay();

	void connectDestroyed();
	void shutdown();
	void shutdownInLoop();
	void forceClose();
	
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();

	void startReadInLoop();
	void stopReadInLoop();

	void startRead();
	void stopRead();

	std::any *getMutableContext() { return &context; }
	const std::any &getContext() const { return context; }
	void setContext(const std::any &context) { this->context = context; }

	Buffer *outputBuffer() { return &sendBuff; }
	Buffer *intputBuffer() { return &recvBuff; }

	const char *getip() { return ip; }
	uint16_t getport() { return port; }

	void setip(const char *ip) { this->ip = ip; }
	void setport(uint16_t port) { this->port = port; }

private:
	TcpConnection(const TcpConnection&);
	void operator=(const TcpConnection&);

	EventLoop *loop;
	int32_t sockfd;
	bool reading;

	Buffer recvBuff;
	Buffer sendBuff;
	ConnectionCallback connectionCallback;
	MessageCallback messageCallback;
	WriteCompleteCallback writeCompleteCallback;
	HighWaterMarkCallback highWaterMarkCallback;
	CloseCallback closeCallback;

	size_t highWaterMark;
	StateE state;
	ChannelPtr channel;
	std::any context;
	const char *ip;
	uint16_t port;
};
