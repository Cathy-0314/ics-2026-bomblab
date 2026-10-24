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

    uint32_t low  = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid  = (uint32_t)(challenge >> 16);

    uint32_t step2 = (3u * high + low) ^ 0xd3dcu;
    uint32_t step0 = 3u * (low - mid) + 0x748u;
    uint32_t ans   = (5u * step0) ^ (step2 + 0x318au);

    values[count++] = ans;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    values[0] = 3u * ((uint32_t)(challenge >> 13) & 0x3fu);
    count = 1;

    for (uint32_t i = 1; i <= 5; i++) {
        uint32_t shift = 15 + 4 * (i - 1);
        uint32_t t = ((uint32_t)(challenge >> shift) & 0x1fu) + i;
        values[count] = 5u * (t ^ values[count - 1]);
        count++;
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t idx = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);

    uint32_t r;
    switch (idx) {
        case 0: r = 5u * x + 0x3807cu;          break;  
        case 1: r = (x ^ 0x55a1u) + (x >> 1);   break;  
        case 2: r = 3u * x + 0x3a1u;            break;  
        case 3: r = (4u * x) ^ 0xeee0u;         break;  
        case 4: r = (2u * x) ^ 0xc8e8u;         break;  
        case 5: r = 5u * x + 0x40191u;          break;  
        case 6: r = (x ^ 0xdbe8u) + (x >> 2);   break;  
        case 7: r = 3u * x + 0x1f8fu;           break;  
        default: r = 0u;                        break;
    }

    values[count++] = idx;
    values[count++] = r;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t a, uint32_t b, uint32_t d, uint32_t c) {
    uint32_t mid = b + ((d - b) >> 1);
    if (a == mid) {
        return (mid + 0xa7u) ^ c;
    }
    uint32_t ebx = mid + c;
    if (a < mid) {
        return (3u * p4_walk(a, b, mid - 1, c)) ^ ebx ^ 0x14u;
    } else {
        return (3u * p4_walk(a, mid + 1, d, c)) ^ ebx ^ 0x7u;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t a = (uint32_t)challenge & 0x7fu;
    uint32_t c = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);

    values[count++] = p4_walk(a, 0u, 0x7fu, c);
    return write_answer(out, cap, values, count);
}

static const int      p5_next[12]  = {1, 6, 3, 0, 8, 9, 11, 5, -1, 10, 2, 4};
static const uint32_t p5_f8[12]    = {0x17d, 0xb78, 0x519, 0x825, 0x461, 0x369,
                                      0x35,  0xa72, 0x611, 0x932, 0x738, 0x279};
static const uint32_t p5_fc[12]    = {0xa830, 0x3b51, 0x1955, 0xfa8, 0x4e10, 0xc185,
                                      0xa43c, 0x15b7, 0x9f78, 0xfe17, 0x822f, 0x4185};
static const int      p5_heads[8]  = {7, 5, 9, 10, 2, 3, 0, 1};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t idx   = (uint32_t)challenge & 7u;
    uint32_t steps = ((uint32_t)(challenge >> 3)) & 3u;
    uint32_t a = ((uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32)) ^ 0xe017u;

    int n = p5_heads[idx];
    uint32_t r    = (3u * a + p5_fc[n]) ^ p5_f8[n];
    uint32_t last = p5_f8[n];

    for (uint32_t i = 0; i < steps; i++) {
        n = p5_next[n];
        r    = (3u * r + p5_fc[n]) ^ p5_f8[n];
        last = p5_f8[n];
    }

    values[count++] = r;
    values[count++] = last;
    return write_answer(out, cap, values, count);
}

static uint32_t p6_op0(uint32_t x) { return (x ^ 0xc507u) + (x << 7); }
static uint32_t p6_op1(uint32_t x) { return (x >> 3) | (x << 29); }        /* ror 3 */
static uint32_t p6_op2(uint32_t x) { return (8u * x + 0x9f24u) ^ (7u * x); }
static uint32_t p6_op3(uint32_t x) { return ((x << 3) | (x >> 29)) ^ 0x65cau; } /* rol 3 */
static uint32_t p6_op4(uint32_t x) { return ((x >> 1) ^ x) + 0xdc94u; }
static uint32_t p6_op5(uint32_t x) { return (x + 0xcef8u) ^ (x >> 3); }

static uint32_t p6_call(uint32_t idx, uint32_t x) {
    switch (idx) {
        case 0: return p6_op3(x);
        case 1: return p6_op1(x);
        case 2: return p6_op2(x);
        case 3: return p6_op4(x);
        case 4: return p6_op4(x);
        case 5: return p6_op5(x);
        case 6: return p6_op0(x);
        case 7: return p6_op1(x);
        default: return 0u;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t t = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t r = p6_call((uint32_t)challenge & 7u, t) ^ 0x530fu;
    r = p6_call(((uint32_t)(challenge >> 3)) & 7u, r) + 0x1d60u;

    values[count++] = r;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t idx = (uint32_t)context & 3u;
    uint32_t B   = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t C   = (uint32_t)(context >> 32) ^ (uint32_t)context;

    uint32_t A = B ^ C;
    uint32_t M = ((A >> 5) ^ 0xe758u) + (A ^ 0xd0c7u);
    uint32_t T = M ^ ((idx * 0xd0c7u) ^ (uint32_t)(context >> 16));

    int n = snprintf(out, cap, "%32sOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                     "", idx, T, T ^ (C + idx + 0xe758u + (B >> 5)));
    return (n < 0 || (size_t)n >= cap) ? 1 : 0;
}