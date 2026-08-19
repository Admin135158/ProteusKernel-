#pragma once
// proteus_holo_bridge.hpp
// Integration layer: ProteusKernel swarm node + HOLO-Invariant occurrence verification
// Co-authored convergence: Proteus mobilizer + HOLO stabilizer

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <chrono>
#include <mutex>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

namespace proteus {

static constexpr double PHI = 1.61803398874989484820;
static constexpr double PHI_CONJ = 0.61803398874989484820;

struct Vec3 {
    double x, y, z;
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    double mag() const { return std::sqrt(x*x + y*y + z*z); }
};

struct OccurrenceProof {
    uint64_t timestamp_ns;
    uint64_t sequence_id;
    std::array<uint8_t, 32> node_id;
    std::array<uint8_t, 64> signature;
    std::array<uint8_t, 32> state_hash;
    double phi_stability;
    bool bounded;
};

class HoloVerifier {
public:
    explicit HoloVerifier(const std::string& private_key_pem);
    ~HoloVerifier();
    HoloVerifier(const HoloVerifier&) = delete;
    HoloVerifier& operator=(const HoloVerifier&) = delete;

    OccurrenceProof sign_occurrence(
        uint64_t sequence,
        const std::vector<uint8_t>& state_blob,
        double measured_phi_deviation
    );
    bool verify_occurrence(
        const OccurrenceProof& proof,
        const std::vector<uint8_t>& state_blob,
        const std::array<uint8_t, 32>& expected_node_id
    );
    bool is_bounded(double local_phi, double peer_phi, double tolerance = 0.05) const;
    std::array<uint8_t, 32> derive_node_id() const;

private:
    EVP_PKEY* pkey_ = nullptr;
    std::mutex mtx_;
    uint64_t last_sequence_ = 0;
    std::vector<uint8_t> serialize_for_sign(uint64_t seq, uint64_t ts,
                                           const std::array<uint8_t,32>& hash);
    std::array<uint8_t, 32> hash_state(const std::vector<uint8_t>& blob);
};

struct DNABackupFrame {
    uint32_t magic = 0x504B444E;
    uint16_t version = 1;
    uint64_t epoch;
    uint32_t node_count;
    std::vector<uint8_t> compressed_state;
    OccurrenceProof proof;
    std::array<uint8_t, 32> prev_frame_hash;
};

class DNAEncoder {
public:
    static DNABackupFrame encode(
        uint64_t epoch,
        const std::vector<Vec3>& positions,
        const std::vector<double>& phi_fields,
        HoloVerifier& signer
    );
    static bool decode_and_verify(
        const DNABackupFrame& frame,
        const std::array<uint8_t, 32>& expected_node_id,
        HoloVerifier& verifier,
        std::vector<Vec3>& out_positions,
        std::vector<double>& out_phi_fields
    );
    static std::vector<uint8_t> compress_phi_aware(
        const std::vector<Vec3>& positions,
        const std::vector<double>& phi_fields
    );
};

struct VerifiedHeartbeat {
    uint32_t magic = 0x48525442;
    uint16_t proto_version = 1;
    uint64_t timestamp_ns;
    std::array<uint8_t, 32> node_id;
    Vec3 position;
    Vec3 velocity;
    double phi_local;
    OccurrenceProof proof;
};

class SwarmNode {
public:
    SwarmNode(const std::string& name, const std::string& key_pem);
    bool init_network(uint16_t bind_port);
    void tick(double dt);
    Vec3 compute_stabilizer_force(const std::vector<VerifiedHeartbeat>& peers);
    bool should_backup() const;
    DNABackupFrame generate_backup();
    Vec3 position() const { return pos_; }
    Vec3 velocity() const { return vel_; }
    uint64_t sequence() const { return seq_; }

private:
    std::string name_;
    std::unique_ptr<HoloVerifier> verifier_;
    Vec3 pos_{0,0,0};
    Vec3 vel_{0,0,0};
    double phi_local_ = PHI_CONJ;
    uint64_t seq_ = 0;
    uint64_t last_backup_epoch_ = 0;
    int sock_fd_ = -1;
    std::vector<VerifiedHeartbeat> peer_buffer_;
    mutable std::mutex peer_mtx_;
    void broadcast_heartbeat();
    void receive_peers();
    void update_phi_dynamics(double dt);
};

} // namespace proteus
