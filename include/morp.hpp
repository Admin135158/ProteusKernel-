#pragma once
#include <openssl/hmac.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <arpa/inet.h>
#include <vector>

namespace morp {
inline constexpr const char* SECRET = "MORPHEUS_DEV_KEY_CHANGE_IN_PRODUCTION";
inline constexpr size_t HEADER_LEN = 12;
inline constexpr size_t SIG_LEN    = 32;
inline constexpr size_t FRAME_MIN  = HEADER_LEN + SIG_LEN;

inline constexpr uint8_t MORP_MSG_PROBE     = 0x01;
inline constexpr uint8_t MORP_MSG_HEARTBEAT = 0x02;
inline constexpr uint8_t MORP_MSG_ACK       = 0x08;
inline constexpr uint8_t MORP_MSG_COMMAND   = 0x10;

inline std::vector<uint8_t> hmac_sha256(const uint8_t* data, size_t len) {
    std::vector<uint8_t> out(32);
    unsigned int outlen = 32;
    HMAC(EVP_sha256(), SECRET, (int)strlen(SECRET), data, len, out.data(), &outlen);
    return out;
}

inline std::vector<uint8_t> encode(uint8_t type, const std::vector<uint8_t>& payload = {}) {
    size_t plen = payload.size();
    std::vector<uint8_t> frame(FRAME_MIN + plen, 0);
    memcpy(frame.data(), "MORP", 4);
    frame[4] = 2; frame[5] = type; frame[6] = 0; frame[7] = 0;
    uint32_t np = htonl((uint32_t)plen);
    memcpy(frame.data() + 8, &np, 4);
    std::vector<uint8_t> hmac_in(HEADER_LEN + 32 + plen, 0);
    memcpy(hmac_in.data(), frame.data(), HEADER_LEN);
    if (plen) memcpy(hmac_in.data() + HEADER_LEN + 32, payload.data(), plen);
    auto sig = hmac_sha256(hmac_in.data(), hmac_in.size());
    memcpy(frame.data() + HEADER_LEN, sig.data(), 32);
    if (plen) memcpy(frame.data() + FRAME_MIN, payload.data(), plen);
    return frame;
}
}
