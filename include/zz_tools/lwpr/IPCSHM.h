#pragma once

#include "LWPRType.h"
#include "Exception.h"

namespace ZZTools
{
	class IPCSHM
	{
	public:
		/**
		 * ����һ�������ڴ����
		 */
		static IPCID_T Create(const IPCKEY_T key,
		                      int size,
		                      IPC_CREATE_TYPE_E type = CREATE_NEW_ONE,
		                      int perm = IPC_DEFAULT_PERM);

		/**
		 * ����IPC Key������Ӧ��IPC ID
		 */
		static IPCID_T GetID(const IPCKEY_T key);

		/**
		 * ����һ�������ڴ����
		 */
		static void Destroy(const IPCID_T id);

		/**
		 * ���ں˽������ڴ��ַ�������������
		 */
		static void* Mount(const IPCID_T id, PERMISSION_E prms = RW);

		/**
		 * ж�ر����̹��صĽ��̵�ַ
		 */
		static void Unmount(void *addr);

		/**
		 * ����MagicCode��ȡϵͳ�пɷ��ʵĹ����ڴ�ID
		 */
		static int FetchSHMID(IPCIDList& ids, UINT32 magic_code);

	private:
	};

	DEFINE_EXCEPTION(LWPR_IPC_SHM_GET_ERR);
	DEFINE_EXCEPTION(LWPR_IPC_SHM_SIZE_ERR);
	DEFINE_EXCEPTION(LWPR_IPC_SHM_SHMAT_ERR);
	DEFINE_EXCEPTION(LWPR_IPC_SHM_SHMDT_ERR);

	DEFINE_EXCEPTION(LWPR_IPC_SHM_NOT_EXIST);
};
