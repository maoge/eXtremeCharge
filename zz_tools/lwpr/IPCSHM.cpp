#include "IPCSHM.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace ZZTools
{

	IPCID_T IPCSHM::Create(const IPCKEY_T key,
	                             int size,
	                             IPC_CREATE_TYPE_E type /*= CREATE_NEW_ONE*/,
	                             int perm /*= IPC_DEFAULT_PERM*/)
	{
		int retcode = 0;
		int flg = perm;

		switch(type)
		{
		case CREATE_NEW_ONE:
			flg |= IPC_CREAT;
			break;
		case CREATE_NEW_ONE_EXCL:
			flg |= IPC_CREAT;
			flg |= IPC_EXCL;
			break;
		case GET_EXIST_ONE:
			break;
		default:
			assert(0);
		}

		if(type != GET_EXIST_ONE && size <= 0)
			throw LWPR_IPC_SHM_SIZE_ERR(EXCEPTION_TRACE, "share memory size is zero");

		retcode = ::shmget(key, size, flg);
		if(retcode == -1)
		{
			throw LWPR_IPC_SHM_GET_ERR(EXCEPTION_TRACE, "shmget error");
		}

		return retcode;
	}

	IPCID_T IPCSHM::GetID(const IPCKEY_T key)
	{
		int retcode = ::shmget(key, 0, IPC_DEFAULT_PERM);

		if(retcode == IPC_INVALID_ID)
		{
			throw LWPR_IPC_SHM_NOT_EXIST(EXCEPTION_TRACE, "shm not exist");
		}

		return retcode;
	}

	void IPCSHM::Destroy(const IPCID_T id)
	{
		::shmctl(id, IPC_RMID, 0);
	}

	void* IPCSHM::Mount(const IPCID_T id, PERMISSION_E prms /*= RW*/)
	{
		void *addr = NULL;
		int flg = 0;

		if(prms == READ)
			flg |= SHM_RDONLY;

		addr = ::shmat(id, 0, flg);

		if((void *)(-1) == addr)
			throw LWPR_IPC_SHM_SHMAT_ERR(EXCEPTION_TRACE, "shmat error");

		return addr;
	}

	void IPCSHM::Unmount(void *addr)
	{
		int retcode = 0;

		retcode = ::shmdt(addr);
		if(retcode == -1)
		{
			throw LWPR_IPC_SHM_SHMDT_ERR(EXCEPTION_TRACE, "shmdt error");
		}
	}

	int IPCSHM::FetchSHMID(IPCIDList& ids, UINT32 magic_code)
	{
		// ����ipcs -m �����ȡIPCID
		FILE* fp = popen("ipcs -m 2> /dev/null", "r");
		if(fp == NULL) return 0;

		std::vector < std::string > cachedLine;

		while(!feof(fp))
		{
			char bufLine[256] = {0};
			char *pRet = fgets(bufLine, 255, fp);
			if(pRet != NULL)
			{
				cachedLine.push_back(pRet);
			}
		}

		pclose(fp);

		for(size_t i = 0; i < cachedLine.size(); i++)
		{
			// �����ı�
			char tmpBuf[128][7];
			memset(tmpBuf, 0, sizeof(tmpBuf));
			int nRet = sscanf(cachedLine[i].c_str(), "%s%s%s%s%s%s%s",
			                  tmpBuf[0],
			                  tmpBuf[1],
			                  tmpBuf[2],
			                  tmpBuf[3],
			                  tmpBuf[4],
			                  tmpBuf[5],
			                  tmpBuf[6]);
			if(nRet >= 5)
			{
				int ipcid = atoi(tmpBuf[1]);
				if(ipcid >= 0)
				{
					try
					{
						UINT32* pCode = (UINT32*)IPCSHM::Mount(ipcid);

						if(*pCode == magic_code)
						{
							ids.push_back(ipcid);
						}

						IPCSHM::Unmount(pCode);
					}
					catch(...)
					{
					}
				}
			}
		}

		return ids.size();
	}
};
