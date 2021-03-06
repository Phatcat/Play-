#pragma once

#include "Iop_Module.h"
#include "Iop_SifMan.h"
#include "Iop_SifDynamic.h"
#include "Iop_Sysmem.h"
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipArchiveReader.h"

class CIopBios;

namespace Iop
{
	class CSifCmd : public CModule
	{
	public:
								CSifCmd(CIopBios&, CSifMan&, CSysmem&, uint8*);
		virtual					~CSifCmd();

		std::string				GetId() const override;
		std::string				GetFunctionName(unsigned int) const override;
		void					Invoke(CMIPS&, unsigned int) override;

		void					ProcessInvocation(uint32, uint32, uint32*, uint32);

		void					LoadState(Framework::CZipArchiveReader&);
		void					SaveState(Framework::CZipArchiveWriter&);

	private:
		typedef std::list<CSifDynamic*> DynamicModuleList;

		struct SIFRPCQUEUEDATA
		{
			uint32		threadId;
			uint32		active;
			uint32		serverDataLink;    //Set when there's a pending request
			uint32		serverDataStart;   //Set when a RPC server has been registered on the queue
			uint32		serverDataEnd;
			uint32		queueNext;
		};

		struct SIFRPCSERVERDATA
		{
			uint32		serverId;

			uint32		function;
			uint32		buffer;
			uint32		size;

			uint32		cfunction;
			uint32		cbuffer;
			uint32		csize;

			uint32		rsize;
			uint32		rid;

			uint32		queueAddr;
		};
		static_assert(sizeof(SIFRPCSERVERDATA) <= 0x44, "Size of SIFRPCSERVERDATA must be less or equal to 68 bytes.");

		struct SIFCMDDATA
		{
			uint32		sifCmdHandler;
			uint32		data;
			uint32		gp;
		};
		
		enum
		{
			MAX_SYSTEM_COMMAND = 0x20,
			MAX_SREG = 0x20,
			PENDING_CMD_BUFFER_SIZE = 0x400,
		};

		struct MODULEDATA
		{
			uint8      trampoline[0x800];
			uint8      sendCmdExtraStruct[0x10];
			uint32     sreg[MAX_SREG];
			SIFCMDDATA sysCmdBuffer[MAX_SYSTEM_COMMAND];
			uint8      pendingCmdBuffer[PENDING_CMD_BUFFER_SIZE];
			uint32     pendingCmdBufferSize;
		};

		void					ClearServers();
		void					BuildExportTable();

		void					ProcessCustomCommand(uint32);
		void					ProcessSetSreg(uint32);
		void					ProcessRpcRequestEnd(uint32);
		void					ProcessDynamicCommand(uint32);
		void					ProcessNextDynamicCommand();

		int32					SifGetSreg(uint32);
		uint32					SifSetCmdBuffer(uint32, uint32);
		void					SifAddCmdHandler(uint32, uint32, uint32);
		uint32					SifSendCmd(uint32, uint32, uint32, uint32, uint32, uint32);
		uint32					SifBindRpc(uint32, uint32, uint32);
		void					SifCallRpc(CMIPS&);
		void					SifRegisterRpc(CMIPS&);
		uint32					SifCheckStatRpc(uint32);
		void					SifSetRpcQueue(uint32, uint32);
		uint32					SifGetNextRequest(uint32);
		void					SifExecRequest(CMIPS&);
		void					SifRpcLoop(CMIPS&);
		uint32					SifGetOtherData(uint32, uint32, uint32, uint32, uint32);
		void					FinishExecRequest(uint32, uint32);
		void					FinishExecCmd();
		void					SleepThread();

		uint32					m_usrCmdBuffer = 0;
		uint32					m_usrCmdBufferLen = 0;
		uint32					m_sysCmdBuffer = 0;

		CIopBios&				m_bios;
		CSifMan&				m_sifMan;
		CSysmem&				m_sysMem;
		uint8*					m_ram = nullptr;
		uint32					m_moduleDataAddr = 0;
		uint32					m_trampolineAddr = 0;
		uint32					m_sendCmdExtraStructAddr = 0;
		uint32					m_sregAddr = 0;
		uint32					m_sifRpcLoopAddr = 0;
		uint32					m_sifExecRequestAddr = 0;
		uint32					m_sifExecCmdHandlerAddr = 0;
		uint32					m_pendingCmdBufferAddr = 0;
		uint32					m_pendingCmdBufferSizeAddr = 0;
		bool					m_executingCmd = false;
		DynamicModuleList		m_servers;
	};

	typedef std::shared_ptr<CSifCmd> SifCmdPtr;
}
