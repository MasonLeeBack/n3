#ifndef _SINGLETON_H_
#define _SINGLETON_H_


#pragma once

// GPG �� ������ �ڵ� ����ü ���Ͽ� ������ �ҽ� �״����. ����ؼ� ������.
// by bs
// ���� �Ҵ翡 ���� ������ ����ġ �ʾ�. GPG 1.3 section �� �����µ��� ������ �մϴ�.
// by ev

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// singleton ��ü
#ifdef _PREFIX_
   #if __cplusplus
   extern "C" void __pfx_assume(bool, const char *);
   #else
   void __pfx_assume(int, const char *);
   #endif
#else
#define __pfx_assume(condition, message)
#endif

template <class T > class CSingleton
{
protected:
	static T* ms_obj;		// member static

public:
	CSingleton()
	{
#pragma warning(disable:4311 4312)
		assert(!ms_obj && "�ߺ� ���� ����������.");

		int offset = (int)(T*)1-(int)(CSingleton<T>*)(T*)1; // ���߻���� ����
		ms_obj = (T*)((int)this+offset);
#pragma warning(default:4311 4312)
	}

	virtual ~CSingleton()
	{
		assert(ms_obj);
		ms_obj = NULL;
	}
	
	static T& GetInstance()
	{
		assert(ms_obj);
#ifdef _PREFIX_
		__pfx_assume(ms_obj != 0, ""); // Helps with PREfix analysis.
#endif
		return (*ms_obj);
	}

	static T* GetInstancePtr()
	{
		return ms_obj;
	}

	static bool CreateInstance()
	{
		assert(!ms_obj);
		T* pTemp = new T;
		assert(pTemp);
		return true;
	}

	static void DestroyInstance()
	{
		assert(ms_obj);
		delete ms_obj;
		ms_obj = NULL;
	}
};

// ���� ��ü ����
template <class T > T* CSingleton <T >::ms_obj = 0;

#endif