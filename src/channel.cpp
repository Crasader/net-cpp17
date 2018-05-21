#include "channel.h"
#include "eventloop.h"

const int32_t Channel::kNoneEvent = 0;
const int32_t Channel::kReadEvent = POLLIN | POLLPRI;
const int32_t Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop,int32_t fd)
:loop(loop),
 fd(fd),
 events(0),
 revents(0),
 index(-1),
 tied(false),
 eventHandling(false),
 addedToLoop(false)
{

}

Channel::~Channel()
{
	assert(!eventHandling);
	assert(!addedToLoop);
	if (loop->isInLoopThread())
	{
		assert(!loop->hasChannel(this));
	}
}

void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop = false;
	loop->removeChannel(this);
}

void Channel::update()
{
	addedToLoop = true;
	loop->updateChannel(this);
}

void Channel::handleEventWithGuard()
{
	eventHandling = true;
#ifdef __linux__
	if ((revents & EPOLLHUP) && !(revents & EPOLLIN))
	{
		if (closeCallback) closeCallback();
	}

	if (revents & EPOLLERR)
	{
		if (errorCallback) errorCallback();
	}

	if (revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (readCallback) readCallback();
	}

	if (revents & EPOLLOUT)
	{
		if (writeCallback) writeCallback();
	}

#endif

#ifdef __APPLE__
	if ((revents & POLLHUP) && !(revents & POLLIN))
	{
		if (closeCallback) closeCallback();
	}

	if (revents & POLLERR)
	{
		if (errorCallback) errorCallback();
	}

	if (revents & (POLLIN | POLLPRI | POLLHUP))
	{
		if (readCallback) readCallback();
	}

	if (revents & POLLOUT)
	{
		if (writeCallback) writeCallback();
	}
#endif
	eventHandling = false;
}

void Channel::handleEvent()
{
	std::shared_ptr<void> guard;
	if (tied)
	{
		guard = tie.lock();
		if (guard)
		{
			handleEventWithGuard();
		}
	}
	else
	{
		handleEventWithGuard();
	}
}

void Channel::setTie(const std::shared_ptr<void> &obj)
{
	tie = obj;
	tied = true;
}
