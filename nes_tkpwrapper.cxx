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
        while (!Stopped.load()) {
			if (!Paused.load()) {
                update();
                while (MessageQueue->PollRequests()) {
					auto request = MessageQueue->PopRequest();
					poll_request(request);
				}
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
        cpu_.Tick();
    }

    bool NES::poll_uncommon_request(const Request& req) {
        return false;
    }

    bool NES::load_file(std::string path) {
        return cpu_.bus_.LoadCartridge(path);
    }
}