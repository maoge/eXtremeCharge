#pragma once

#include <cassert>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>


namespace ZZTools
{
	typedef char                         BOOL;      // bool
	typedef unsigned    char            UINT8;      // Unsigned  8 bit quantity
	typedef signed      char             INT8;      // Signed    8 bit quantity
	typedef unsigned    short           UINT16;     // Unsigned 16 bit quantity
	typedef signed      short            INT16;     // Signed   16 bit quantity
	typedef unsigned    int             UINT32;     // Unsigned 32 bit quantity
	typedef signed      int              INT32;     // Signed   32 bit quantity
	typedef unsigned    long long       UINT64;     // Unsigned 64 bit quantity
	typedef signed      long long        INT64;     // Signed   64 bit quantity
	typedef float                        FP32;      // Single precision floating point
	typedef double                       FP64;      // Double precision floating point

#define LWPR_TRUE			1
#define LWPR_FALSE			0
};

namespace ZZTools
{
	typedef key_t		IPCKEY_T;
	typedef int			IPCID_T;
	typedef pid_t		PID_T;
	typedef msqid_ds	MSQ_INFO_T;

	const int IPC_INVALID_ID = -1;

	typedef enum ipc_create_type
	{
		CREATE_NEW_ONE,         // ����һ����IPC������Ѿ����ڣ��򷵻سɹ�
		CREATE_NEW_ONE_EXCL,    // ����һ����IPC������Ѿ����ڣ��򷵻�ʧ��
		GET_EXIST_ONE,          // ȡ���Ѿ����ڵ�IPC����������ڣ��򷵻�ʧ��
	} IPC_CREATE_TYPE_E;

	static const int IPC_DEFAULT_PERM = 0666;

	typedef enum permission_e
	{
		RW,
		READ,
		WRITE,
	} PERMISSION_E;

	typedef void (*SIGHANDLER_PFT)(int);

	typedef pthread_t		THREAD_ID_T;
	typedef pthread_key_t	THREAD_KEY_T;
	typedef pid_t			PID_T;
	typedef int				SOCKET_FD_T;
	typedef fd_set			SOCKET_SET_T;
	typedef std::vector <PID_T> PIDList;
	typedef std::vector <IPCID_T> IPCIDList;
	typedef std::vector <SOCKET_FD_T> SOCKETFDList;
};
