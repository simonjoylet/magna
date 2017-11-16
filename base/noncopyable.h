#ifndef NON_COPYABLE_H
#define NON_COPYABLE_H

namespace base
{
class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:  // emphasize the following members are private  
	noncopyable(const noncopyable&);
	const noncopyable& operator=(const noncopyable&);
};


} // namespace base

#endif//NON_COPYABLE_H
