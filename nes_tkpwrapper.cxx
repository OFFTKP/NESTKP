#include "nes_tkpwrapper.hxx"

namespace TKPEmu::NES {
    NES_TKPWrapper::NES_TKPWrapper() {
        ppu_.SetNMI(std::bind(&Devices::CPU::NMI, &cpu_));
    }

    NES_TKPWrapper::~NES_TKPWrapper() {}

    void NES_TKPWrapper::HandleKeyDown(uint32_t) {}

    void NES_TKPWrapper::HandleKeyUp(uint32_t) {}

    void* NES_TKPWrapper::GetScreenData() {
        return ppu_.GetScreenData();
    }

    bool& NES_TKPWrapper::IsReadyToDraw() {
        return ppu_.IsReadyToDraw();
    }

    void NES_TKPWrapper::start() {
        std::lock_guard<std::mutex> lg(ThreadStartedMutex);
        Reset();
        while (!Stopped.load()) {
			if (!Paused.load()) {
                update();
            } else {
				Step.wait(false);
				Step.store(false);
				update();
			}
        }
    }

    void NES_TKPWrapper::reset() {
        cpu_.Reset();
        ppu_.Reset();
    }

    void NES_TKPWrapper::update() {
        while (MessageQueue->PollRequests()) {
            auto request = MessageQueue->PopRequest();
            poll_request(request);
        }
        v_log();
        cpu_.Tick();
    }

    void NES_TKPWrapper::v_log() {
		if (logging_) {
            if (log_flags_.test(0)) {
                *log_file_ptr_ << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << cpu_.PC << " A:" << std::setw(2) << (uint16_t)cpu_.A << 
                    " X:" << std::setw(2) << (uint16_t)cpu_.X << " Y:" << std::setw(2) << (uint16_t)cpu_.Y << " P:" << std::setw(2) << cpu_.P.to_ullong() <<
                    " SP:" << std::setw(2) << (uint16_t)cpu_.SP;
            }
            if (log_flags_.test(1)) {
                *log_file_ptr_ << " CYC:" << std::dec << cpu_.cycles_;
            }
            *log_file_ptr_ << std::endl;
        }
    }

    bool NES_TKPWrapper::poll_uncommon_request(const Request& req) {
        return false;
    }

    bool NES_TKPWrapper::load_file(std::string path) {
        return cpu_.bus_.LoadCartridge(path);
    }
}