#include "nes_tkpwrapper.hxx"

namespace TKPEmu::NES {
    NES::NES() {}

    NES::~NES() {}

    void NES::HandleKeyDown(uint32_t) {}

    void NES::HandleKeyUp(uint32_t) {}

    void* NES::GetScreenData() {
        return nullptr;
    }

    bool& NES::IsReadyToDraw() {
        return should_draw_;
    }

    void NES::start() {
        std::lock_guard<std::mutex> lg(ThreadStartedMutex);
        Reset();
        Paused.store(true);
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

    void NES::reset() {
        cpu_.Reset();
    }

    void NES::update() {
        while (MessageQueue->PollRequests()) {
            auto request = MessageQueue->PopRequest();
            poll_request(request);
        }
        v_log();
        cpu_.Tick();
    }

    void NES::v_log() {
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

    bool NES::poll_uncommon_request(const Request& req) {
        return false;
    }

    bool NES::load_file(std::string path) {
        return cpu_.bus_.LoadCartridge(path);
    }
}