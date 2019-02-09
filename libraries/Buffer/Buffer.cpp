#include "Buffer.h"

/*------------------------------Constructor Methods------------------------------*/

Buffer::Buffer(const int size)
{
	store_ = new char[size]();
	size_ = size;
	front_ = 0;
	back_ = 0;
}

Buffer::~Buffer()
{

}

/*------------------------------Public Methods------------------------------*/

bool Buffer::push(char c)
{
	store_[front_] = c;

	front_++;

	if(front_ >= size_)
	{
		front_ = 0;
	}

	if(front_ == back_)
	{
		back_++;
	}

	if(back_ >= size_)
	{
		back_ = 0;
	}

	return true;
}

char Buffer::pop()
{
	char c;

	if(available())
	{
		c = store_[back_];

		back_++;

		if(back_ >= size_)
		{
			back_ = 0;
		}
	}
	else
	{
		c = '\0';
	}

	return c;
}

char Buffer::peek()
{
	return store_[back_];
}

bool Buffer::available()
{
	if(back_ != front_)
	{
		return true;
	}
	else
	{
		return false;
	}
}