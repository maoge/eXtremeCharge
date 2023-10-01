#include "Object.h"

namespace ZZTools
{
	Object* Object::Duplicate(Object *ref)
	{
		if(ref)
		{
			ref->IncRef();
		}
		return ref;
	}

	void Object::Release(Object *ref)
	{
		if(ref)
		{
			ref->DecRef();
		}
	}

	Object::~Object()
	{
		// ���û�������жϣ�����������ջ�϶���
		//assert(m_nRefCount.Get() == 0);
	}

	void Object::IncRef()
	{
		// assert(m_nRefCount >= 1);
		m_nRefCount++;
	}

	void Object::DecRef()
	{
		// assert(m_nRefCount > 0);
		if(--m_nRefCount == 0)
		{
			delete this;
		}
	}
};
