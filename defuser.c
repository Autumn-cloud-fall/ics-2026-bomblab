#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t lo    = (uint32_t) challenge;
    uint32_t hi    = (uint32_t)(challenge >> 32);
    uint32_t mid16 = (uint32_t)(challenge >> 16);

    uint32_t s2 = ( hi + lo * 2 ) ^ 0x4587;
    uint32_t s0 = ( ( lo - mid16 ) * 5 ) + 0x9ed5;
    uint32_t res = ( s2 + 0xb2fb ) ^ s0;

    values[0] = res;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t prev = (uint32_t)(((challenge >> 10) & 0xfeu) + 20u); 
    values[0] = prev;
     for (uint32_t i = 1; i < 6; ++i) {
         uint32_t shift = 22u + 3u * (i - 1u);
         uint32_t nibble = (uint32_t)(challenge >> shift) & 0xfu;
         // 汇编顺序：nibble + i → 和prev异或 → ×3，更新prev
         prev = ( (nibble + i) ^ prev ) * 3u;
         values[i] = prev;
     }
    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t idx = (uint32_t)challenge & 0x7u;
    uint32_t arg = (uint32_t)((challenge >> 32) ^ (challenge >> 3));
    uint32_t target;
    switch (idx) {
        case 0: target = (arg * 4u) ^ 0xbd1u; break;
        case 1: target = (arg * 2u) ^ 0x6e72u; break;
        case 2: target = (arg ^ 0x633u) + (arg >> 2); break;
        case 3: target = arg * 5u + 0x7d5u; break;
        case 4: target = (arg ^ 0x8dbdu) + (arg >> 1); break;
        case 5: target = arg * 5u + 0x3a4du; break;
        case 6: target = arg * 5u + 0x4189du; break;
        default: target = arg * 5u + 0x181a5u; break;
    }
//idx 0 实际跳到 p3_case_0，不是 p3_case_7；完整映射为 0→case_0、1→case_7、2→case_3、3→case_2、4→case_5、5→case_6、6→case_1、7→case_4。
    values[0] = idx;
    values[1] = target;
    count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk_impl(uint32_t target, uint32_t low, uint32_t high, uint32_t salt) {
    uint32_t mid = low + (high - low) / 2u;
    if (mid == target) return (mid + 0xd6u) ^ salt;

    uint32_t mixed = mid + salt;
    uint32_t result;
    if (target < mid) {
        result = p4_walk_impl(target, low, mid - 1u, salt);
        return ((result * 5u) ^ mixed) ^ 0x8u;
    }

    result = p4_walk_impl(target, mid + 1u, high, salt);
    return ((result * 5u) ^ mixed) ^ 0x1du;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t target = (uint32_t)(challenge & 0x7fu);
    uint32_t salt = (uint32_t)((challenge >> 7u) ^ (challenge >> 32u));
    uint32_t ans = p4_walk_impl(target, 0u, 127u, salt);

    values[0] = ans;
    count = 1;
    return write_answer(out, cap, values, count);
}

struct p5_node {
    uint32_t low;
    uint32_t high;
    unsigned next;
};

static uint32_t p5_traverse(uint64_t challenge, uint32_t *last_high) {
    static const struct p5_node nodes[] = {
        {0xc777u, 0x096fu, 1u},
        {0x4179u, 0x0162u, 3u},
        {0xdf4eu, 0x084cu, 5u},
        {0x704fu, 0x0a22u, 9u},
        {0xf8c7u, 0x0b32u, 6u},
        {0xeabdu, 0x0576u, 7u},
        {0xdb4eu, 0x0308u, 2u},
        {0xad31u, 0x022au, 10u},
        {0xdd1cu, 0x0036u, 0u},
        {0x3e5fu, 0x064eu, 0u},
        {0x848eu, 0x040cu, 11u},
        {0x7d04u, 0x0756u, 4u}
    };
    static const unsigned heads[] = {7u, 10u, 11u, 4u, 6u, 2u, 5u, 8u};
    unsigned node = heads[challenge & 7u];
    uint32_t traversals = (uint32_t)(challenge >> 3u) & 3u;
    uint32_t value = (uint32_t)((challenge >> 6u) ^ (challenge >> 32u)) ^ 0xc96u;

    for (uint32_t i = 0; i <= traversals; ++i) {
        value = (value + nodes[node].low) * 5u ^ nodes[node].high;
        *last_high = nodes[node].high;
        if (i < traversals) node = nodes[node].next;
    }
    return value;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    // TODO: 根据 phase_05 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t last_high = 0;
    values[0] = p5_traverse(challenge, &last_high);
    values[1] = last_high;
    count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t p6_apply(unsigned op, uint32_t x) {
    switch (op) {
        case 0: return (x >> 2u) ^ x ^ 0xd7e9u;
        case 1: return (x ^ 0x8148u) + x * 4u;
        case 2: return (x + 0x5954u) ^ (x >> 2u);
        case 3: return (x * 5u) ^ (x * 2u + 0xbf26u);
        case 4: return ((x << 3u) | (x >> 29u)) ^ 0x0f36u;
        default: return (x >> 3u) | (x << 29u);
    }
}

static uint32_t p6_dispatch(uint64_t challenge) {
    static const unsigned op_for_index[8] = {2u, 1u, 0u, 5u, 2u, 3u, 3u, 4u};
    uint32_t x = (uint32_t)((challenge >> 6u) ^ (challenge >> 32u));
    unsigned first_index = (unsigned)(challenge & 7u);
    unsigned second_index = (unsigned)((challenge >> 3u) & 7u);

    x = p6_apply(op_for_index[first_index], x) ^ 0xe0b2u;
    x = p6_apply(op_for_index[second_index], x);
    return x + 0xf263u;
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
   // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = p6_dispatch(challenge);
    count = 1;
   return write_answer(out, cap, values, count);
}

static uint32_t secret_mix(uint32_t x) {
    uint32_t t = (x << 7) | (x >> 25);
    t += 0x935dU;
    t ^= 0x5de2U;
    return t;
}

//// TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap)
{
    uint32_t tl = (uint32_t)context;
    uint32_t th = (uint32_t)(context >> 32);
    uint32_t cl = (uint32_t)challenge;
    uint32_t ch = (uint32_t)(challenge >> 32);

    uint32_t a = cl ^ ch;
    uint32_t b = tl ^ th;

    uint32_t x = tl & 3U;
    uint32_t tmp1 = secret_mix(a ^ b);
    uint32_t tmp2 = (x * 0x935dU) ^ (uint32_t)(context >> 16);
    uint32_t y = tmp1 ^ tmp2;
    uint32_t z = y ^ ( (a >> 7) + b + x + 0x5de2U );

    int ret = snprintf(out, cap, "%32sOPEN %u %u %u", "", x, y, z);
    if (ret < 0 || (size_t)ret >= cap) {
        return 1;
    }
    return 0;
}
