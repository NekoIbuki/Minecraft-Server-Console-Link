#define OUTPUTMSG_EXPORTS
#include "outputMSG_API.h"
#include <memory>
#include <string>

//#pragma data_seg(".shared")
//SharedMessage g_sharedMsg;
//#pragma data_seg()
//#pragma comment(linker, "/SECTION:.shared,RWS")
//
//BOOL InitializeSharedMemory() {
//    memset(&g_sharedMsg, 0, sizeof(SharedMessage));
//    return TRUE;
//}
//
//SharedMessage* GetSharedMessage() {
//    return &g_sharedMsg;
//}

namespace {
    std::unique_ptr<OutputMessageImpl> outputInstance;
}

struct OutputMessageImpl::Impl {
    std::mutex mutex_;
    std::string playerChat;
    std::string playerConnect;
    std::string playerCommand;
    std::string blockCommand;
    std::string serverERROR;
    
};

// 实现OutputMessageImpl的构造函数和析构函数
//OutputMessageImpl::~OutputMessageImpl() = default;
OutputMessageImpl::OutputMessageImpl() : pImpl(new Impl()) {}
OutputMessageImpl::~OutputMessageImpl() { delete pImpl; }

// 实现工厂函数
extern "C" {
    OUTPUTMSG_API IOutputMessage* CreateOutputMessage() {
        return new OutputMessageImpl();
    }

    OUTPUTMSG_API void DestroyOutputMessage(IOutputMessage* instance) {
        delete instance;
    }

    OUTPUTMSG_API const IOutputMessage& GetGlobalOutputMessage() {
        if (!outputInstance) {
            outputInstance = std::make_unique<OutputMessageImpl>();
        }
        return *outputInstance;
    }
}

void ProcessOutput(const std::string& line) {
    static OutputMessageImpl instance;
    instance.SetPChat(line);  // 可访问私有方法
}

// 实现OUTMSG接口
const IOutputMessage& OUTMSG() {
    if (!outputInstance) {
        outputInstance = std::make_unique<OutputMessageImpl>();
    }
    return *outputInstance;
}

// 实现所有虚函数
std::string OutputMessageImpl::GetPChat() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    return pImpl->playerChat;
}

std::string OutputMessageImpl::GetPCMD() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    return pImpl->playerCommand;
}

std::string OutputMessageImpl::GetPCONN() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    return pImpl->playerConnect;
}

std::string OutputMessageImpl::GetBCMD() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    return pImpl->blockCommand;
}

std::string OutputMessageImpl::GetSERROR() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    return pImpl->serverERROR;
}
//OUTMSG设置
void OutputMessageImpl::SetPChat(const std::string& msg) {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->playerChat = msg;
}
void OutputMessageImpl::SetPCMD(const std::string& msg) {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->playerCommand = msg;
}
void OutputMessageImpl::SetPCONN(const std::string& msg) {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->playerConnect = msg;
}
void OutputMessageImpl::SetBCMD(const std::string& msg) {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->blockCommand = msg;
}
void OutputMessageImpl::SetSERROR(const std::string& msg) {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->serverERROR = msg;
}