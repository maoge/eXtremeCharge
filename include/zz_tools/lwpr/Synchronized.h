#pragma once

#include "LWPRType.h"
#include "IPCSEM.h"
#include "Mutex.h"
#include "RWMutex.h"
#include "Semph.h"
#include "Object.h"
#include "Var.h"

namespace ZZTools
{
	class Synchronized : public Object
	{
		typedef enum syn_type_e
		{
			SYN_IPCSEM,
			SYN_MUTEX,
			SYN_SEMPH,
		} SYN_TYPE_E;

	public:
		Synchronized(const IPCID_T id);
		Synchronized(Mutex &lock);
		Synchronized(Semph &lock);
		~Synchronized();

	private:
		SYN_TYPE_E m_synType;

		IPCID_T m_id;
		Mutex* m_pMutex;
		Semph* m_pSemph;
	};

	class SynchronizedRead : public Object
	{
	public:
		SynchronizedRead(RWMutex &lock);
		~SynchronizedRead();

	private:
		RWMutex* m_pRWMutex;
	};

	class SynchronizedWrite : public Object
	{
	public:
		SynchronizedWrite(RWMutex &lock);
		~SynchronizedWrite();

	private:
		RWMutex* m_pRWMutex;
	};

	DECLAREVAR(Synchronized);
	DECLAREVAR(SynchronizedRead);
	DECLAREVAR(SynchronizedWrite);
};
