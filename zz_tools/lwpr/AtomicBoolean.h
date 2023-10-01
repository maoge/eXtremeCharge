#pragma once

#include "LWPRType.h"

namespace ZZTools
{
	class AtomicBoolean
	{
	public:
		/**
		 * ���캯��
		 */
		AtomicBoolean(bool initialValue = false);

		/**
		 * �����ǰֵ == Ԥ��ֵ������ԭ�ӷ�ʽ����ֵ����Ϊ�����ĸ���ֵ
		 * @return ����ɹ����򷵻� true������ False ָʾʵ��ֵ��Ԥ��ֵ�����
		 */
		bool CompareAndSet(bool expect, bool update);

		/**
		 * ���ص�ǰֵ
		 */
		bool Get() const;

		/**
		 * ����Ϊ����ֵ����������ǰ��ֵ
		 * @return ��ǰ��ֵ
		 */
		bool GetAndSet(bool newValue);

		/**
		 * ������������Ϊ����ֵ��
		 */
		void Set(bool newValue);

		/**
		 * ����ת��������
		 */
		operator bool () const;

	private:
		volatile int m_nValue;
	};
};
