#pragma once
#ifndef TKPEMU_NES_CPU_H
#define TKPEMU_NES_CPU_H
#include <cstdint>
#include <limits>
#include <type_traits>
namespace TKPEmu::NES::Devices {
    union FlagUnion {
    public:
        struct {
            uint8_t Carry        : 1;
            uint8_t Zero         : 1;
            uint8_t InterruptDis : 1;
            uint8_t Decimal      : 1;
            uint8_t Unused       : 2;
            uint8_t Overflow     : 1;
            uint8_t Negative     : 1;
        } Flags;
        operator uint8_t&() {
            return value_;
        }
        uint8_t& operator=(const uint8_t& rhs) {
            return value_ = rhs;
        }
    private:
        uint8_t value_;
    };
    template<typename T>
    struct Register {
    public:
        Register(FlagUnion& P_ref) : P_ref_(P_ref) {
            static_assert(std::is_unsigned<T>::value, "Register takes unsigned parameters only");
        }
        operator T&() const {
            return value_;
        }
        T& operator=(const T& rhs) {
            return value_ = rhs;
        }
        // Postfix operator need not be implemented, since this
        // operator does computation and it would only complicate things
        T& operator++() {
            ++value_;
            P_ref_.Flags.Zero = !static_cast<bool>(value_);
            P_ref_.Flags.Negative = (value_ >> 7);
            return value_;
        }
        T& operator--() {
            --value_;
            P_ref_.Flags.Zero = !static_cast<bool>(value_);
            P_ref_.Flags.Negative = (value_ >> 7);
            return value_;
        }
    private:
        T value_;
        FlagUnion& P_ref_;
    };
    class NES {
    public:
        FlagUnion P;
        Register<uint8_t> A, X, Y, SP;
        Register<uint16_t> PC;
    private:

    };
}
#endif