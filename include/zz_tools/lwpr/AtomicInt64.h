#pragma once

#include "LWPRType.h"

namespace ZZTools
{
	class AtomicInt64Impl;
	
	class AtomicInt64
	{
	public:
		/**
		 * ���캯��
		 */
		AtomicInt64(INT64 initialValue = 0);
		
		/**
		 * ��������
		 */
		~AtomicInt64();
		
		/**
		 * ��ȡ��ǰֵ
		 */
		INT64 Get() const;

		/**
		 * ����Ϊ����ֵ
		 */
		void Set(INT64 newValue);

		/**
		 * �����ǰֵ == Ԥ��ֵ������ԭ�ӷ�ʽ����ֵ����Ϊ�����ĸ���ֵ
		 * @return ����ɹ����򷵻� true������ False ָʾʵ��ֵ��Ԥ��ֵ�����
		 */
		bool CompareAndSet(INT64 expect, INT64 update);

		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ��ǰ��ֵ
		 */
		INT64 GetAndIncrement();

		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ��ǰ��ֵ
		 */
		INT64 GetAndDecrement();
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @return ��ǰ��ֵ
		 */
		INT64 GetAndAdd(int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @param delta Ҫ���ϵ�ֵ
		 * @return ���µ�ֵ
		 */
		INT64 AddAndGet(int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @return ��ǰ��ֵ
		 */
		INT64 GetAndSub(int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @param delta Ҫ���ϵ�ֵ
		 * @return ���µ�ֵ
		 */
		INT64 SubAndGet(int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ���µ�ֵ
		 */
		INT64 IncrementAndGet();
		
		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ���µ�ֵ
		 */
		INT64 DecrementAndGet();
		
		/**
		 * i = 2
		 */
		INT64 operator=(INT64 newValue);
		
		/**
		 * ++i
		 */
		INT64 operator++();
		
		/**
		 * i++
		 */
		INT64 operator++(int);
		
		/**
		 * --i
		 */
		INT64 operator--();
		
		/**
		 * i--
		 */
		INT64 operator--(int);
		
		/**
		 * ����ת��������
		 */
		operator INT64 () const;
		
		//////////////////////////////////////////////////////////////////////////

		/**
		 * �����ǰֵ == Ԥ��ֵ������ԭ�ӷ�ʽ����ֵ����Ϊ�����ĸ���ֵ
		 * @return ����ɹ����򷵻� true������ False ָʾʵ��ֵ��Ԥ��ֵ�����
		 */
		static bool CompareAndSet(volatile INT64 *ptr, INT64 expect, INT64 update);
		
		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ��ǰ��ֵ
		 */
		static INT64 GetAndIncrement(volatile INT64 *ptr);
		
		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ��ǰ��ֵ
		 */
		static INT64 GetAndDecrement(volatile INT64 *ptr);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @return ��ǰ��ֵ
		 */
		static INT64 GetAndAdd(volatile INT64 *ptr, int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @param delta Ҫ���ϵ�ֵ
		 * @return ���µ�ֵ
		 */
		static INT64 AddAndGet(volatile INT64 *ptr, int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @return ��ǰ��ֵ
		 */
		static INT64 GetAndSub(volatile INT64 *ptr, int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ������ֵ�뵱ǰֵ���
		 * @param delta Ҫ���ϵ�ֵ
		 * @return ���µ�ֵ
		 */
		static INT64 SubAndGet(volatile INT64 *ptr, int delta);
		
		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ���µ�ֵ
		 */
		static INT64 IncrementAndGet(volatile INT64 *ptr);
		
		/**
		 * ��ԭ�ӷ�ʽ����ǰֵ�� 1
		 * @return ���µ�ֵ
		 */
		static INT64 DecrementAndGet(volatile INT64 *ptr);

	private:
		volatile INT64 m_nValue;
	};
};
