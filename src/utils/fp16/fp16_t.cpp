/*
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
 * This file is a part of the CANN Open Software.
 * Licensed under CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "mki/utils/fp16/fp16_t.h"

#define FP16_EXTRAC_SIGN(x) (((x) >> 15) & 1)
#define FP16_EXTRAC_EXP(x) (((x) >> 10) & FP16_MAX_EXP)
#define FP16_EXTRAC_MAN(x) ((((x) >> 0) & 0x3FF) | (((((x) >> 10) & 0x1F) > 0 ? 1 : 0) * 0x400))
#define FP16_CONSTRUCTOR(s, e, m) \
    (static_cast<uint16_t>(((s) << FP16_SIGN_INDEX) | ((e) << FP16_MAN_LEN) | ((m) & FP16_MAX_MAN)))
#define FP16_IS_ZERO(x) (((x) & FP16_ABS_MAX) == 0)
#define FP16_IS_DENORM(x) ((((x) & FP16_EXP_MASK) == 0))
#define FP16_IS_INVALID(x) (((x) & FP16_EXP_MASK) == FP16_EXP_MASK)
#define FP32_CONSTRUCTOR(s, e, m) (((s) << FP32_SIGN_INDEX) | ((e) << FP32_MAN_LEN) | ((m) & FP32_MAX_MAN))

namespace {
const uint16_t CONST_1 = 1;
const uint16_t CONST_2 = 2;
const uint16_t CONST_3 = 3;
const uint16_t CONST_7 = 7;
const uint16_t CONST_10 = 10;
const uint16_t CONST_11 = 11;
const uint16_t CONST_15 = 15;
const uint16_t CONST_31 = 31;
const uint16_t CONST_32 = 32;

const uint16_t FP16_EXP_BIAS = 15;
const uint16_t FP16_EXP_MASK = 0x7C00;
const uint16_t FP16_MAN_LEN = 10;
const uint16_t FP16_MAN_MASK = 0x03FF;
const uint16_t FP16_MAN_HIDE_BIT = 0x0400;
const uint16_t FP16_SIGN_INDEX = 15;
const uint16_t FP16_SIGN_MASK = 0x8000;
const uint16_t FP16_MAX = 0x7BFF;
const uint16_t FP16_MIN = 0xFBFF;
const uint16_t FP16_ABS_MAX = 0x7FFF;
const uint16_t FP16_MAX_EXP = 0x001F;
const uint16_t FP16_MAX_MAN = 0x03FF;
const uint16_t FP32_EXP_BIAS = 127;
const uint16_t FP32_MAN_LEN = 23;
const uint16_t FP32_SIGN_INDEX = 31;
const uint32_t FP32_SIGN_MASK = 0x80000000u;
const uint32_t FP32_EXP_MASK = 0x7F800000u;
const uint32_t FP32_MAN_MASK = 0x007FFFFFu;
const uint32_t FP32_MAN_HIDE_BIT = 0x00800000u;
const uint32_t FP32_ABS_MAX = 0x7FFFFFFFu;
const uint32_t FP32_MAX_MAN = 0x7FFFFF;
const uint16_t FP64_EXP_BIAS = 1023;
const uint16_t FP64_MAN_LEN = 52;
const uint16_t FP64_SIGN_INDEX = 63;
const uint64_t FP64_SIGN_MASK = 0x8000000000000000LLu;
const uint64_t FP64_EXP_MASK = 0x7FF0000000000000LLu;
const uint64_t FP64_MAN_MASK = 0x000FFFFFFFFFFFFFLLu;
const uint64_t FP64_MAN_HIDE_BIT = 0x0010000000000000LLu;
const uint8_t INT8_T_MAX = 0x7F;
const uint8_t BIT_LEN8_MAX = 0xFF;
const uint16_t INT16_T_MAX = 0x7FFF;
const uint16_t BIT_LEN16_MAX = 0xFFFF;
const uint32_t INT32_T_MAX = 0x7FFFFFFFu;
const uint32_t BIT_LEN32_MAX = 0xFFFFFFFFu;
const int TRANS_TYPE_INT8 = 0;
const int TRANS_TYPE_INT16 = 1;
const int TRANS_TYPE_NUM = 2;

enum class Fp16RoundMode {
    ROUND_TO_NEAREST = 0, /**< round to nearest even */
    ROUND_BY_TRUNCATED,   /**< round by Truncated    */
    ROUND_MODE_RESERVED,
};

const Fp16RoundMode FP16_ROUND_MODE = Fp16RoundMode::ROUND_TO_NEAREST;
}

namespace Mki {

void ExtractFP16(const uint16_t &val, uint16_t *sVal, int16_t *eVal, uint16_t *mVal)
{
    // 1.Extract
    *sVal = FP16_EXTRAC_SIGN(val);
    *eVal = FP16_EXTRAC_EXP(val);
    *mVal = FP16_EXTRAC_MAN(val);

    // Denormal
    if ((*eVal) == 0) {
        *eVal = 1;
    }
}

template <typename T>
static void ReverseMan(bool negative, T &manVal)
{
    if (negative) {
        manVal = (~(manVal)) + 1;
    }
}

template <typename T>
T RightShift(T manVal, int16_t shift)
{
    int bits = sizeof(T) * 8;
    T mask = ((static_cast<T>(1u)) << (static_cast<unsigned int>(bits - 1)));
    for (int i = 0; i < shift; i++) {
        manVal = ((manVal & mask) | (manVal >> 1));
    }
    return manVal;
}

template <typename T>
T GetManSum(int16_t e1, const T &m1, int16_t e2, const T &m2)
{
    T sum = 0;
    T mTmp = 0;
    if (e1 != e2) {
        int16_t eTmp = std::abs(e1 - e2);
        if (e1 > e2) {
            mTmp = m2;
            mTmp = RightShift(mTmp, eTmp);
            sum = m1 + mTmp;
        } else {
            mTmp = m1;
            mTmp = RightShift(mTmp, eTmp);
            sum = mTmp + m2;
        }
    } else {
        sum = m1 + m2;
    }
    return sum;
}

template <typename T>
T ManRoundToNearest(bool bit0, bool bit1, bool bitLeft, T manVal, uint16_t shift = 0)
{
    manVal = (manVal >> shift) + ((bit1 && (bitLeft || bit0)) ? 1 : 0);
    return manVal;
}

template <typename T>
int16_t GetManBitLength(T manVal)
{
    int16_t len = 0;
    while (manVal) {
        manVal >>= 1;
        len += 1;
    }
    return len;
}

static bool IsRoundOne(uint64_t manValue, uint16_t trctLen)
{
    bool trctLeft = false;
    bool trctHigh = false;
    uint16_t shiftOut = trctLen - CONST_2;
    uint64_t maskA = 0x4;
    maskA = maskA << shiftOut;
    uint64_t maskB = 0x2;
    maskB = maskB << shiftOut;
    uint64_t maskC = maskB - 1;

    bool lastBit = ((manValue & maskA) > 0);
    if (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) {
        trctLeft = ((manValue & maskC) > 0);
        trctHigh = ((manValue & maskB) > 0);
    }
    return ((trctLeft || lastBit) && trctHigh);
}

/**
 * @param [in] exp    exponent of Fp16T value
 * @param [in] manVal exponent of Fp16T value
 * @brief   normalize Fp16T value
 */
static void Fp16Normalize(int16_t &exp, uint16_t &manVal)
{
    if (exp >= FP16_MAX_EXP) {
        manVal = FP16_MAX_MAN;
        exp = FP16_MAX_EXP - 1;
    } else if ((exp == 0) && (manVal == FP16_MAN_HIDE_BIT)) {
        manVal = 0;
        exp += 1;
    }
}

// Convert Fp16T to float/fp32
static float Fp16ToFloat(const uint16_t &fpValue)
{
    uint32_t sVal = 0;
    uint32_t eVal = 0;
    uint32_t mVal = 0;
    uint32_t fVal = 0;

    uint16_t hfSign = 0;
    uint16_t hfMan = 0;
    int16_t hfExp = 0;
    ExtractFP16(fpValue, &hfSign, &hfExp, &hfMan);
    while ((hfMan != 0) && ((hfMan & FP16_MAN_HIDE_BIT) == 0)) {
        hfMan <<= 1;
        hfExp -= 1;
    }

    sVal = hfSign;
    if (hfMan != 0) {
        eVal = static_cast<uint32_t>(hfExp - FP16_EXP_BIAS + FP32_EXP_BIAS);
        mVal = hfMan & FP16_MAN_MASK;
        mVal = mVal << (FP32_MAN_LEN - FP16_MAN_LEN);
    }
    fVal = FP32_CONSTRUCTOR(sVal, eVal, mVal);
    uint32_t* ptrFVal = &fVal;
    float* ptrRet = reinterpret_cast<float *>(ptrFVal);
    float ret = *ptrRet;
    return ret;
}

// Convert Fp16T to double/fp64
static double Fp16ToDouble(const uint16_t &fpValue)
{
    uint64_t sVal = 0;
    uint64_t eVal = 0;
    uint64_t mVal = 0;
    uint64_t fVal = 0;
    uint16_t hfSign = 0;
    uint16_t hfMan = 0;
    int16_t hfExp = 0;
    ExtractFP16(fpValue, &hfSign, &hfExp, &hfMan);

    while ((hfMan != 0) && ((hfMan & FP16_MAN_HIDE_BIT) == 0)) {
        hfMan <<= 1;
        hfExp -= 1;
    }

    sVal = hfSign;
    if (hfMan != 0) {
        eVal = static_cast<uint64_t>(hfExp - FP16_EXP_BIAS + FP64_EXP_BIAS);
        mVal = hfMan & FP16_MAN_MASK;
        mVal = mVal << (FP64_MAN_LEN - FP16_MAN_LEN);
    }
    fVal = (sVal << FP64_SIGN_INDEX) | (eVal << FP64_MAN_LEN) | (mVal);
    uint64_t* ptrFVal = &fVal;
    double* ptrRet = reinterpret_cast<double *>(ptrFVal);
    double ret = *ptrRet;

    return ret;
}

static uint8_t FP16CheckIntOverflow(uint16_t sVal, uint16_t &hfExp, uint64_t &longIntM,
                                    uint16_t &shiftOut, uint8_t type)
{
    uint64_t maxValueMap[TRANS_TYPE_NUM][2] = {  // 2: negative and positive
        {0x20000u,    0x1FFFFu},    // MAX negative/positive int8, 0x20000u: 0x1FFFFu
        {0x2000000Lu, 0x1FFFFFFLu}, // MAX negative/positive int16, 0x2000000Lu: 0x1FFFFFFLu
    };

    if (type >= TRANS_TYPE_NUM) {
        return 1;
    }
    uint8_t overflowFlag = 0;
    uint64_t maxNegetaiveValue = maxValueMap[type][0];
    uint64_t maxPositiveValue = maxValueMap[type][1];
    while (hfExp != FP16_EXP_BIAS) {
        if (hfExp > FP16_EXP_BIAS) {
            hfExp -= 1;
            longIntM = longIntM << 1;
            // sign=1,negative number(<0)
            if (sVal == 1 && longIntM >= maxNegetaiveValue) {
                longIntM = maxNegetaiveValue;
                overflowFlag = 1;
                break;
            } else if (sVal != 1 && longIntM >= maxPositiveValue) {
                longIntM = maxPositiveValue;
                overflowFlag = 1;
                break;
            }
        } else {
            hfExp += 1;
            shiftOut += 1;
        }
    }
    return overflowFlag;
}

// Convert Fp16T to int8_t
static int8_t Fp16ToInt8(const uint16_t &fpValue)
{
    if (FP16_IS_DENORM(fpValue)) { // Denormalized number
        return 0;
    }

    // 1.get sVal and shift it to bit0.
    uint8_t sVal = FP16_EXTRAC_SIGN(fpValue);

    // 2.get hfExp and hfMan
    uint16_t hfExp = FP16_EXTRAC_EXP(fpValue);
    uint16_t hfMan = FP16_EXTRAC_MAN(fpValue);

    uint16_t shiftOut = 0;
    uint8_t overflowFlag = 0;
    uint64_t longIntM = hfMan;

    if (FP16_IS_INVALID(fpValue)) { // Inf or NaN
        overflowFlag = 1;
    } else {
        overflowFlag = FP16CheckIntOverflow(sVal, hfExp, longIntM, shiftOut, TRANS_TYPE_INT8);
    }

    uint8_t mVal = 0;
    uint8_t retV = 0;
    if (overflowFlag != 0) {
        retV = INT8_T_MAX + sVal;
    } else {
        bool needRound = IsRoundOne(longIntM, shiftOut + FP16_MAN_LEN);
        mVal = static_cast<uint8_t>((longIntM >> (FP16_MAN_LEN + shiftOut)) & BIT_LEN8_MAX);
        needRound = needRound && ((sVal == 0 && mVal < INT8_T_MAX) || (sVal == 1 && mVal <= INT8_T_MAX));
        if (needRound) {
            mVal += 1;
        }
        if (sVal != 0) {
            mVal = (~mVal) + CONST_1;
        }
        if (mVal == 0) {
            sVal = 0;
        }
        // Generate final result
        retV = (sVal << CONST_7) | (mVal);
    }

    int8_t ret = *(reinterpret_cast<int8_t *>(&retV));
    return ret;
}

static uint8_t FP16CheckU8Overflow(uint8_t &mVal, uint16_t &hfExp, uint64_t &longIntM, uint16_t &shiftOut)
{
    uint8_t overflowFlag = 0;
    while (hfExp != FP16_EXP_BIAS) {
        if (hfExp > FP16_EXP_BIAS) {
            hfExp -= 1;
            longIntM = longIntM << 1;
            if (longIntM >= 0x40000Lu) { // overflow 0x40000
                longIntM = 0x3FFFFLu; // (Fp16T-manVal)+8(uint8)=18bit
                overflowFlag = 1;
                mVal = ~0;
                break;
            }
        } else {
            hfExp += 1;
            shiftOut += 1;
        }
    }
    return overflowFlag;
}

// Convert Fp16T to uint8_t
static uint8_t Fp16ToUInt8(const uint16_t &fpValue)
{
    if (FP16_IS_DENORM(fpValue)) { // Denormalized number
        return 0;
    }

     // 1.get sVal and shift it to bit0.
    uint8_t sVal = FP16_EXTRAC_SIGN(fpValue);

    // 2.get hfExp and hfMan
    uint16_t hfExp = FP16_EXTRAC_EXP(fpValue);
    uint16_t hfMan = FP16_EXTRAC_MAN(fpValue);

    uint8_t mVal = 0;
    if (FP16_IS_INVALID(fpValue)) {
        mVal = ~0;
    } else {
        uint16_t shiftOut = 0;
        uint64_t longIntM = hfMan;
        uint8_t overFlow = FP16CheckU8Overflow(mVal, hfExp, longIntM, shiftOut);
        if (overFlow == 0) {
            bool needRound = IsRoundOne(longIntM, shiftOut + FP16_MAN_LEN);
            mVal = static_cast<uint8_t>((longIntM >> (FP16_MAN_LEN + shiftOut)) & BIT_LEN8_MAX);
            if (needRound && mVal != BIT_LEN8_MAX) {
                mVal += 1;
            }
        }
    }

    if (sVal == 1) { // Negative number
        mVal = 0;
    }
    // Generate final result
    uint8_t ret = mVal;

    return ret;
}

// Convert Fp16T to int16_t
static int16_t Fp16ToInt16(const uint16_t &fpValue)
{
    // 1.get sVal and shift it to bit0.
    uint16_t sVal = FP16_EXTRAC_SIGN(fpValue);

    // 2.get hfExp and hfMan
    uint16_t hfExp = FP16_EXTRAC_EXP(fpValue);
    uint16_t hfMan = FP16_EXTRAC_MAN(fpValue);

    int16_t ret = 0;
    uint16_t retV = 0;
    if (FP16_IS_DENORM(fpValue)) { // Denormalized number
        retV = 0;
        ret = *(reinterpret_cast<uint8_t *>(&retV));
        return ret;
    }

    uint16_t shiftOut = 0;
    uint64_t longIntM = hfMan;
    uint8_t overflowFlag = 0;

    if (FP16_IS_INVALID(fpValue)) {
        overflowFlag = 1;
    } else {
        overflowFlag = FP16CheckIntOverflow(sVal, hfExp, longIntM, shiftOut, TRANS_TYPE_INT16);
    }
    uint16_t mVal = 0;
    if (overflowFlag != 0) {
        retV = INT16_T_MAX + sVal;
    } else {
        bool needRound = IsRoundOne(longIntM, shiftOut + FP16_MAN_LEN);
        mVal = static_cast<uint16_t>((longIntM >> (FP16_MAN_LEN + shiftOut)) & BIT_LEN16_MAX);
        if (needRound && mVal < INT16_T_MAX) {
            mVal += 1;
        }
        if (sVal != 0) {
            mVal = (~mVal) + CONST_1;
        }
        if (mVal == 0) {
            sVal = 0;
        }
        // Generate final result
        retV = (sVal << CONST_15) | (mVal);
    }

    ret = *(reinterpret_cast<int16_t *>(&retV));
    return ret;
}

// Convert Fp16T to uint16_t
static uint16_t Fp16ToUInt16(const uint16_t &fpValue)
{
    uint16_t ret = 0;

    // 1.get sVal and shift it to bit0.
    uint16_t sVal = FP16_EXTRAC_SIGN(fpValue);

    // 2.get hfExp and hfMan
    uint16_t hfMan = FP16_EXTRAC_MAN(fpValue);
    uint16_t hfExp = FP16_EXTRAC_EXP(fpValue);

    if (FP16_IS_DENORM(fpValue)) { // Denormalized number
        return 0;
    }

    uint16_t mVal = 0;
    if (FP16_IS_INVALID(fpValue)) { // Inf or NaN
        mVal = ~0;
    } else {
        uint64_t longIntM = hfMan;
        uint16_t shiftOut = 0;

        while (hfExp != FP16_EXP_BIAS) {
            if (hfExp > FP16_EXP_BIAS) {
                hfExp -= 1;
                longIntM = longIntM << 1;
            } else {
                hfExp += 1;
                shiftOut += 1;
            }
        }
        bool needRound = IsRoundOne(longIntM, shiftOut + FP16_MAN_LEN);
        mVal = static_cast<uint16_t>((longIntM >> (FP16_MAN_LEN + shiftOut)) & BIT_LEN16_MAX);
        if (needRound && mVal != BIT_LEN16_MAX) {
            mVal += 1;
        }
    }

    if (sVal == 1) { // Negative number
        mVal = 0;
    }
    // Generate final result
    ret = mVal;

    return ret;
}

// Convert Fp16T to int32_t
static int32_t Fp16ToInt32(const uint16_t &fpValue)
{
    int32_t ret = 0;
    uint32_t retV = 0;
    uint32_t mVal = 0;

    // 1.get sVal and shift it to bit0.
    uint32_t sVal = FP16_EXTRAC_SIGN(fpValue);

    // 2.get hfExp and hfMan
    uint16_t hfExp = FP16_EXTRAC_EXP(fpValue);
    uint16_t hfMan = FP16_EXTRAC_MAN(fpValue);

    if (FP16_IS_INVALID(fpValue)) { // Inf or NaN
        retV = INT32_T_MAX + sVal;
    } else {
        uint16_t shiftOut = 0;
        uint64_t longIntM = hfMan;
        while (hfExp != FP16_EXP_BIAS) {
            if (hfExp > FP16_EXP_BIAS) {
                longIntM = longIntM << 1;
                hfExp -= 1;
            } else {
                shiftOut += 1;
                hfExp += 1;
            }
        }
        bool needRound = IsRoundOne(longIntM, shiftOut + FP16_MAN_LEN);
        mVal = static_cast<uint32_t>((longIntM >> (FP16_MAN_LEN + shiftOut)) & BIT_LEN32_MAX);
        if (needRound && mVal < INT32_T_MAX) {
            mVal += 1;
        }

        if (sVal == 1) {
            mVal = (~mVal) + 1;
        }
        if (mVal == 0) {
            sVal = 0;
        }
    // Generate final result
        retV = (sVal << CONST_31) | (mVal);
    }

    ret = *(reinterpret_cast<int32_t *>(&retV));
    return ret;
}

// Convert Fp16T to uint32_t
static uint32_t Fp16ToUInt32(const uint16_t &fpValue)
{
    uint32_t ret = 0;
    uint32_t mVal = 0;

    // 1.get sVal and shift it to bit0.
    uint32_t sVal = FP16_EXTRAC_SIGN(fpValue);

    // 2.get hfExp and hfMan
    uint16_t hfExp = FP16_EXTRAC_EXP(fpValue);
    uint16_t hfMan = FP16_EXTRAC_MAN(fpValue);

    if (FP16_IS_DENORM(fpValue)) { // Denormalized number
        return 0u;
    }

    if (FP16_IS_INVALID(fpValue)) { // Inf or NaN
        mVal = ~0u;
    } else {
        uint64_t longIntM = hfMan;
        uint16_t shiftOut = 0;
        while (hfExp != FP16_EXP_BIAS) {
            if (hfExp > FP16_EXP_BIAS) {
                longIntM = longIntM << 1;
                hfExp -= 1;
            } else {
                shiftOut += 1;
                hfExp += 1;
            }
        }
        bool needRound = IsRoundOne(longIntM, shiftOut + FP16_MAN_LEN);
        mVal = static_cast<uint32_t>((longIntM >> (FP16_MAN_LEN + shiftOut)) & BIT_LEN32_MAX);
        if (needRound && mVal != BIT_LEN32_MAX) {
            mVal += 1;
        }
    }

    if (sVal == 1) { // Negative number
        mVal = 0;
    }
    // Generate final result
    ret = mVal;

    return ret;
}

// Performing Fp16T addition
static uint16_t Fp16Add(uint16_t v1, uint16_t v2)
{
    uint16_t ret = 0;
    uint16_t s1 = 0;
    uint16_t s2 = 0;
    int16_t e1 = 0;
    int16_t e2 = 0;
    uint32_t m1 = 0;
    uint32_t m2 = 0;
    uint16_t sVal = 0;
    uint16_t mVal = 0;
    int16_t eVal = 0;
    uint16_t m1Tmp = 0;
    uint16_t m2Tmp = 0;

    uint16_t shiftOut = 0;
    // 1.Extract
    ExtractFP16(v1, &s1, &e1, &m1Tmp);
    ExtractFP16(v2, &s2, &e2, &m2Tmp);
    m1 = m1Tmp;
    m2 = m2Tmp;

    uint16_t sum;
    if (s1 != s2) {
        ReverseMan(s1 > 0, m1);
        ReverseMan(s2 > 0, m2);
        sum = static_cast<uint16_t>(GetManSum(e1, m1, e2, m2));
        sVal = (sum & FP16_SIGN_MASK) >> FP16_SIGN_INDEX;
        ReverseMan(sVal > 0, m1);
        ReverseMan(sVal > 0, m2);
    } else {
        sum = static_cast<uint16_t>(GetManSum(e1, m1, e2, m2));
        sVal = s1;
    }

    if (sum == 0) {
        shiftOut = CONST_3;
        m1 = m1 << shiftOut;
        m2 = m2 << shiftOut;
    }

    int16_t eTmp = 0;
    uint32_t mTrct = 0;

    eVal = std::max(e1, e2);
    eTmp = std::abs(e1 - e2);
    if (e1 > e2) {
        mTrct = (m2 << (CONST_32 - static_cast<uint16_t>(eTmp)));
        m2 = RightShift(m2, eTmp);
    } else if (e1 < e2) {
        mTrct = (m1 << (CONST_32 - static_cast<uint16_t>(eTmp)));
        m1 = RightShift(m1, eTmp);
    }
    // calculate mantissa
    mVal = static_cast<uint16_t>(m1 + m2);

    uint16_t mMin = static_cast<uint16_t>(FP16_MAN_HIDE_BIT << shiftOut);
    uint16_t mMax = mMin << 1;
    // Denormal
    while (mVal < mMin && eVal > 0) { // the value of mVal should not be smaller than 2^23
        mVal = mVal << 1;
        mVal += (FP32_SIGN_MASK & mTrct) >> FP32_SIGN_INDEX;
        mTrct = mTrct << 1;
        eVal = eVal - 1;
    }
    while (mVal >= mMax) { // the value of mVal should be smaller than 2^24
        mTrct = mTrct >> 1;
        mTrct = mTrct | (FP32_SIGN_MASK * (mVal & 1));
        mVal = mVal >> 1;
        eVal = eVal + 1;
    }

    bool isLastBit = ((mVal & 1) > 0);
    bool isTrctHigh = 0;
    bool isTrctLeft = 0;
    isTrctHigh = (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) && ((mTrct & FP32_SIGN_MASK) > 0);
    isTrctLeft = (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) && ((mTrct & FP32_ABS_MAX) > 0);
    mVal = ManRoundToNearest(isLastBit, isTrctHigh, isTrctLeft, mVal, shiftOut);
    while (mVal >= mMax) {
        mVal = mVal >> 1;
        eVal = eVal + 1;
    }

    if (eVal == 0 && mVal <= mMax) {
        mVal = mVal >> 1;
    }
    Fp16Normalize(eVal, mVal);

    ret = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
    return ret;
}

// Performing Fp16T subtraction
static uint16_t Fp16Sub(uint16_t v1, uint16_t v2)
{
    // Reverse
    uint16_t tmp = ((~(v2)) & FP16_SIGN_MASK) | (v2 & FP16_ABS_MAX);
    uint16_t ret = Fp16Add(v1, tmp);
    return ret;
}

// Performing Fp16T multiplication
static uint16_t Fp16Mul(uint16_t v1, uint16_t v2)
{
    uint16_t s1 = 0;
    uint16_t s2 = 0;
    int16_t e1 = 0;
    int16_t e2 = 0;
    uint32_t m1 = 0;
    uint32_t m2 = 0;
    uint16_t m1Tmp = 0;
    uint16_t m2Tmp = 0;

    // 1.Extract
    ExtractFP16(v1, &s1, &e1, &m1Tmp);
    ExtractFP16(v2, &s2, &e2, &m2Tmp);
    m1 = m1Tmp;
    m2 = m2Tmp;

    uint16_t sVal = 0;
    uint16_t mVal = 0;
    int16_t eVal = 0;
    uint32_t mulM = 0;
    eVal = e1 + e2 - FP16_EXP_BIAS - CONST_10;
    mulM = m1 * m2;
    sVal = s1 ^ s2;

    uint32_t mMin = FP16_MAN_HIDE_BIT;
    uint32_t mMax = mMin << 1;
    uint32_t mTrct = 0;
    // the value of mVal should not be smaller than 2^23
    while (mulM < mMin && eVal > 1) {
        mulM = mulM << 1;
        eVal = eVal - 1;
    }
    while (mulM >= mMax || eVal < 1) {
        mTrct = mTrct >> 1;
        mTrct = mTrct | (FP32_SIGN_MASK * (mulM & 1));
        mulM = mulM >> 1;
        eVal = eVal + 1;
    }
    bool bLastBit = ((mulM & 1) > 0);
    bool bTrctHigh = (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) && ((mTrct & FP32_SIGN_MASK) > 0);
    bool bTrctLeft = (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) && ((mTrct & FP32_ABS_MAX) > 0);
    mulM = ManRoundToNearest(bLastBit, bTrctHigh, bTrctLeft, mulM);

    while (mulM >= mMax || eVal < 0) {
        mulM = mulM >> 1;
        eVal = eVal + 1;
    }

    if (eVal == 1 && mulM < FP16_MAN_HIDE_BIT) {
        eVal = 0;
    }
    mVal = static_cast<uint16_t>(mulM);

    Fp16Normalize(eVal, mVal);

    uint16_t ret = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
    return ret;
}

// Performing Fp16T division
static uint16_t Fp16Div(uint16_t v1, uint16_t v2)
{
    uint16_t ret;
    if (FP16_IS_ZERO(v2)) { // result is inf
    // throw "Fp16T division by zero.";
        uint16_t s1 = FP16_EXTRAC_SIGN(v1);
        uint16_t s2 = FP16_EXTRAC_SIGN(v2);
        uint16_t sVal = s1 ^ s2;
        ret = FP16_CONSTRUCTOR(sVal, FP16_MAX_EXP - 1, FP16_MAX_MAN);
    } else if (FP16_IS_ZERO(v1)) {
        ret = 0u;
    } else {
        uint16_t s1 = 0;
        uint16_t s2 = 0;
        int16_t e1 = 0;
        int16_t e2 = 0;
        uint64_t m1 = 0;
        uint64_t m2 = 0;
        uint16_t m1Tmp = 0;
        uint16_t m2Tmp = 0;
        // 1.Extract
        ExtractFP16(v1, &s1, &e1, &m1Tmp);
        ExtractFP16(v2, &s2, &e2, &m2Tmp);
        m1 = m1Tmp;
        m2 = m2Tmp;

        uint64_t mTmp = 0;
        float mDiv = 0;
        if (e1 > e2) {
            mTmp = m1;
            uint16_t tmp = static_cast<uint16_t>(e1 - e2);
            for (int i = 0; i < tmp; i++) {
                mTmp = mTmp << 1;
            }
            m1 = mTmp;
        } else if (e1 < e2) {
            mTmp = m2;
            uint16_t tmp = static_cast<uint16_t>(e2 - e1);
            for (int i = 0; i < tmp; i++) {
                mTmp = mTmp << 1;
            }
            m2 = mTmp;
        }
        mDiv = static_cast<float>(m1 * 1.0f / m2);
        Fp16T fpDiv = static_cast<Fp16T>(mDiv); /*lint !e524*/
        ret = fpDiv.val;
        if (s1 != s2) {
            ret |= FP16_SIGN_MASK;
        }
    }
    return ret;
}

// operate
Fp16T Fp16T::operator+(const Fp16T fp16) const
{
    uint16_t retVal = Fp16Add(val, fp16.val);
    Fp16T ret(retVal);
    return ret;
}

Fp16T Fp16T::operator+=(const Fp16T fp16)
{
    val = Fp16Add(val, fp16.val);
    return *this;
}

Fp16T Fp16T::operator-(const Fp16T fp16) const
{
    uint16_t retVal = Fp16Sub(val, fp16.val);
    Fp16T ret(retVal);
    return ret;
}

Fp16T Fp16T::operator-=(const Fp16T fp16)
{
    val = Fp16Sub(val, fp16.val);
    return *this;
}

Fp16T Fp16T::operator*(const Fp16T fp16) const
{
    uint16_t retVal = Fp16Mul(val, fp16.val);
    Fp16T ret(retVal);
    return ret;
}

Fp16T Fp16T::operator*=(const Fp16T fp16)
{
    val = Fp16Mul(val, fp16.val);
    return *this;
}

Fp16T Fp16T::operator/(const Fp16T fp16) const
{
    uint16_t retVal = Fp16Div(val, fp16.val);
    Fp16T ret(retVal);
    return ret;
}

Fp16T Fp16T::operator/=(const Fp16T fp16)
{
    val = Fp16Div(val, fp16.val);
    return *this;
}

// compare
bool Fp16T::operator==(const Fp16T &fp16) const
{
    if (FP16_IS_ZERO(val) && FP16_IS_ZERO(fp16.val)) {
        return true;
    } else {
        return ((val & BIT_LEN16_MAX) == (fp16.val & BIT_LEN16_MAX)); // bit compare
    }
}

bool Fp16T::operator!=(const Fp16T &fp16) const
{
    if (FP16_IS_ZERO(val) && FP16_IS_ZERO(fp16.val)) {
        return false;
    } else {
        return ((val & BIT_LEN16_MAX) != (fp16.val & BIT_LEN16_MAX)); // bit compare
    }
}

bool Fp16T::operator>(const Fp16T &fp16) const
{
    uint16_t s1 = 0;
    uint16_t s2 = 0;
    uint16_t e1 = 0;
    uint16_t e2 = 0;
    uint16_t m1 = 0;
    uint16_t m2 = 0;

    // 1.Extract
    s1 = FP16_EXTRAC_SIGN(val);
    s2 = FP16_EXTRAC_SIGN(fp16.val);
    e1 = FP16_EXTRAC_EXP(val);
    e2 = FP16_EXTRAC_EXP(fp16.val);
    m1 = FP16_EXTRAC_MAN(val);
    m2 = FP16_EXTRAC_MAN(fp16.val);

    // Compare
    if ((s1 == 0) && (s2 > 0)) { // +  -
        // -0=0
        return !(FP16_IS_ZERO(val) && FP16_IS_ZERO(fp16.val));
    } else if ((s1 == 0) && (s2 == 0)) { // + +
        if (e1 > e2) { // e1 - e2 >= 1; Va always larger than Vb
            return true;
        } else if (e1 == e2) {
            return m1 > m2;
        } else {
            return false;
        }
    } else if ((s1 > 0) && (s2 > 0)) { // - -    opposite to  + +
        if (e1 < e2) {
            return true;
        } else if (e1 == e2) {
            return m1 < m2;
        } else {
            return false;
        }
    } else { // -  +
        return false;
    }
}

bool Fp16T::operator>=(const Fp16T &fp16) const
{
    if ((*this) > fp16) {
        return true;
    } else if ((*this) == fp16) {
        return true;
    } else {
        return false;
    }
}

bool Fp16T::operator<(const Fp16T &fp16) const
{
    if ((*this) >= fp16) {
        return false;
    } else {
        return true;
    }
}

bool Fp16T::operator<=(const Fp16T &fp16) const
{
    if ((*this) > fp16) {
        return false;
    } else {
        return true;
    }
}

// evaluation
Fp16T& Fp16T::operator=(const Fp16T &fp16)
{
    if (this == &fp16) {
        return *this;
    }
    val = fp16.val;
    return *this;
}

Fp16T &Fp16T::operator=(const float &fVal)
{
    uint16_t sVal = 0;
    uint16_t mVal = 0;
    int16_t eVal = 0;
    uint32_t eF = 0;
    uint32_t mF = 0;
    uint32_t ui32V = *(reinterpret_cast<const uint32_t *>(&fVal)); // 1:8:23bit sign:exp:manVal
    uint32_t mLenDelta = 0;

    sVal = static_cast<uint16_t>((ui32V & FP32_SIGN_MASK) >> FP32_SIGN_INDEX); // 4Byte->2Byte
    eF = (ui32V & FP32_EXP_MASK) >> FP32_MAN_LEN; // 8 bit exponent
    mF = (ui32V & FP32_MAN_MASK); // 23 bit mantissa dont't need to care about denormal
    mLenDelta = FP32_MAN_LEN - FP16_MAN_LEN;

    bool needRound = false;
    // Exponent overflow/NaN converts to signed inf/NaN
    if (eF > 0x8Fu) { // 0x8Fu:142=127+15
        eVal = FP16_MAX_EXP - 1;
        mVal = FP16_MAX_MAN;
    } else if (eF <= 0x70u) { // 0x70u:112=127-15 Exponent underflow converts to denormalized half or signed zero
        eVal = 0;
        if (eF >= 0x67) { // 0x67:103=127-24 Denormal
            mF = (mF | FP32_MAN_HIDE_BIT);
            uint16_t shiftOut = FP32_MAN_LEN;
            uint64_t mTmp = (static_cast<uint64_t>(mF)) << (eF - 0x67);

            needRound = IsRoundOne(mTmp, shiftOut);
            mVal = static_cast<uint16_t>(mTmp >> shiftOut);
            if (needRound) {
                mVal += 1;
            }
        } else if (eF == 0x66 && mF > 0) { // 0x66:102 Denormal 0<f_v<min(Denormal)
            mVal = 1;
        } else {
            mVal = 0;
        }
    } else { // Regular case with no overflow or underflow
        needRound = IsRoundOne(mF, mLenDelta);
        mVal = static_cast<uint16_t>(mF >> mLenDelta);
        if (needRound) {
            mVal += 1;
        }
        eVal = static_cast<int16_t>(eF - 0x70u);
        if ((mVal & FP16_MAN_HIDE_BIT) != 0) {
            eVal += 1;
        }
    }

    Fp16Normalize(eVal, mVal);
    val = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
    return *this;
}

Fp16T &Fp16T::operator=(const int8_t &iVal)
{
    uint16_t sVal = 0;
    uint16_t eVal = 0;
    uint16_t mVal = 0;

    sVal = static_cast<uint16_t>(((static_cast<uint8_t>(iVal)) & 0x80) >> CONST_7);
    mVal = static_cast<uint16_t>(((static_cast<uint8_t>(iVal)) & INT8_T_MAX));

    if (mVal == 0) {
        eVal = 0;
    } else {
        if (sVal != 0) { // negative number(<0)
            mVal = static_cast<uint16_t>(std::abs(iVal)); // complement
        }

        eVal = FP16_MAN_LEN;
        while ((mVal & FP16_MAN_HIDE_BIT) == 0) {
            eVal = eVal - CONST_1;
            mVal = mVal << CONST_1;
        }
        eVal = eVal + FP16_EXP_BIAS;
    }

    val = FP16_CONSTRUCTOR(sVal, eVal, mVal);
    return *this;
}

Fp16T &Fp16T::operator=(const uint8_t &uiVal)
{
    uint16_t sVal = 0;
    uint16_t eVal = 0;
    uint16_t mVal = uiVal;
    if (mVal != 0) {
        eVal = FP16_MAN_LEN;
        while ((mVal & FP16_MAN_HIDE_BIT) == 0) {
            mVal = mVal << CONST_1;
            eVal = eVal - CONST_1;
        }
        eVal = eVal + FP16_EXP_BIAS;
    }

    val = FP16_CONSTRUCTOR(sVal, eVal, mVal);
    return *this;
}

static int16_t GetExpValueformInt16(uint32_t &mTmp, uint16_t mMax, uint16_t len)
{
    int16_t eVal = FP16_EXP_BIAS + FP16_MAN_LEN;
    uint16_t eTmp = len - CONST_11;
    uint32_t trctMask = 1;
    for (int i = 1; i < eTmp; i++) {
        trctMask = (trctMask << 1) + 1;
    }
    uint32_t mTrct = (mTmp & trctMask) << (CONST_32 - eTmp);
    for (int i = 0; i < eTmp; i++) {
        mTmp = (mTmp >> 1);
        eVal = eVal + 1;
    }
    bool bLastBit = ((mTmp & 1) > 0);
    bool bTrctHigh = false;
    bool bTrctLeft = false;
    if (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) { // Truncate
        bTrctHigh = ((mTrct & FP32_SIGN_MASK) > 0);
        bTrctLeft = ((mTrct & FP32_ABS_MAX) > 0);
    }
    mTmp = ManRoundToNearest(bLastBit, bTrctHigh, bTrctLeft, mTmp);
    while (mTmp >= mMax || eVal < 0) {
        mTmp = mTmp >> 1;
        eVal = eVal + 1;
    }
    return eVal;
}

Fp16T &Fp16T::operator=(const int16_t &iVal)
{
    if (iVal == 0) {
        val = 0;
    } else {
        uint16_t sVal;
        uint16_t uiVal = *(reinterpret_cast<const uint16_t *>(&iVal));
        sVal = static_cast<uint16_t>(uiVal >> CONST_15);
        if (sVal != 0) {
            int16_t iValM = -iVal;
            uiVal = *(reinterpret_cast<uint16_t *>(&iValM));
        }
        uint32_t mTmp = (uiVal & FP32_ABS_MAX);

        uint16_t mMin = FP16_MAN_HIDE_BIT;
        uint16_t mMax = mMin << 1;
        uint16_t len = static_cast<uint16_t>(GetManBitLength(mTmp));
        if (mTmp != 0) {
            int16_t eVal;
            if (len > CONST_11) {
                eVal = GetExpValueformInt16(mTmp, mMax, len);
            } else {
                eVal = FP16_EXP_BIAS;
                mTmp = mTmp << (CONST_11 - len);
                eVal = eVal + (len - 1);
            }
            uint16_t mVal = static_cast<uint16_t>(mTmp);
            val = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
        } else {
        }
    }
    return *this;
}

Fp16T &Fp16T::operator=(const uint16_t &uiVal)
{
    if (uiVal == 0) {
        val = 0;
    } else {
        int16_t eVal;
        uint16_t mVal = uiVal;
        uint16_t mMin = FP16_MAN_HIDE_BIT;
        uint16_t mMax = mMin << 1;
        uint16_t len = static_cast<uint16_t>(GetManBitLength(mVal));
        if (len > CONST_11) {
            eVal = FP16_EXP_BIAS + FP16_MAN_LEN;
            uint32_t mTrct;
            uint32_t trctMask = 1;
            uint16_t eTmp = len - CONST_11;
            for (int i = 1; i < eTmp; i++) {
                trctMask = (trctMask << 1) + 1;
            }
            mTrct = (mVal & trctMask) << (CONST_32 - eTmp);
            for (int i = 0; i < eTmp; i++) {
                mVal = (mVal >> 1);
                eVal = eVal + 1;
            }
            bool bLastBit = ((mVal & 1) > 0);
            bool bTrctHigh = false;
            bool bTrctLeft = false;
            if (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) { // Truncate
                bTrctHigh = ((mTrct & FP32_SIGN_MASK) > 0);
                bTrctLeft = ((mTrct & FP32_ABS_MAX) > 0);
            }
            mVal = ManRoundToNearest(bLastBit, bTrctHigh, bTrctLeft, mVal);
            while (mVal >= mMax || eVal < 0) {
                mVal = mVal >> 1;
                eVal = eVal + 1;
            }
            if (FP16_IS_INVALID(val)) {
                val = FP16_MAX;
            }
        } else {
            eVal = FP16_EXP_BIAS;
            mVal = mVal << (CONST_11 - len);
            eVal = eVal + (len - 1);
        }
        val = FP16_CONSTRUCTOR(0u, static_cast<uint16_t>(eVal), mVal);
    }
    return *this;
}

Fp16T &Fp16T::operator=(const int32_t &iVal)
{
    if (iVal == 0) {
        val = 0;
    } else {
        uint32_t uiVal = *(reinterpret_cast<const uint32_t *>(&iVal));
        uint16_t sVal = static_cast<uint16_t>(uiVal >> CONST_31);
        if (sVal != 0) {
            int32_t iValM = -iVal;
            uiVal = *(reinterpret_cast<uint32_t *>(&iValM));
        }
        int16_t eVal;
        uint32_t mTmp = (uiVal & FP32_ABS_MAX);

        uint32_t mMin = FP16_MAN_HIDE_BIT;
        uint32_t mMax = mMin << 1;
        uint16_t len = static_cast<uint16_t>(GetManBitLength(mTmp));
        if (len > CONST_11) {
            eVal = FP16_EXP_BIAS + FP16_MAN_LEN;
            uint32_t trctMask = 1;
            uint16_t eTmp = len - CONST_11;
            for (int i = 1; i < eTmp; i++) {
                trctMask = (trctMask << 1) + 1;
            }
            uint32_t mTrct = (mTmp & trctMask) << (CONST_32 - eTmp);
            for (int i = 0; i < eTmp; i++) {
                eVal = eVal + 1;
                mTmp = (mTmp >> 1);
            }
            bool bLastBit = ((mTmp & 1) > 0);
            bool bTrctHigh = false;
            bool bTrctLeft = false;
            if (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) { // Truncate
                bTrctLeft = ((mTrct & FP32_ABS_MAX) > 0);
                bTrctHigh = ((mTrct & FP32_SIGN_MASK) > 0);
            }
            mTmp = ManRoundToNearest(bLastBit, bTrctHigh, bTrctLeft, mTmp);
            while (mTmp >= mMax || eVal < 0) {
                eVal = eVal + 1;
                mTmp = mTmp >> 1;
            }
            if (eVal >= FP16_MAX_EXP) {
                mTmp = FP16_MAX_MAN;
                eVal = FP16_MAX_EXP - 1;
            }
        } else {
            eVal = FP16_EXP_BIAS;
            mTmp = mTmp << (CONST_11 - len);
            eVal = eVal + (len - 1);
        }
        uint16_t mVal = static_cast<uint16_t>(mTmp);
        val = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
    }
    return *this;
}

Fp16T &Fp16T::operator=(const uint32_t &uiVal)
{
    if (uiVal == 0) {
        val = 0;
    } else {
        int16_t eVal;
        uint32_t mTmp = uiVal;

        uint32_t mMin = FP16_MAN_HIDE_BIT;
        uint32_t mMax = mMin << 1;
        uint16_t len = static_cast<uint16_t>(GetManBitLength(mTmp));
        if (len > CONST_11) {
            eVal = FP16_EXP_BIAS + FP16_MAN_LEN;
            uint32_t mTrct = 0;
            uint32_t trctMask = 1;
            uint16_t eTmp = len - CONST_11;
            for (int i = 1; i < eTmp; i++) {
                trctMask = (trctMask << 1) + 1;
            }
            mTrct = (mTmp & trctMask) << (CONST_32 - eTmp);
            for (int i = 0; i < eTmp; i++) {
                mTmp = (mTmp >> 1);
                eVal = eVal + 1;
            }
            bool bLastBit = ((mTmp & 1) > 0);
            bool bTrctHigh = false;
            bool bTrctLeft = false;
            if (Fp16RoundMode::ROUND_TO_NEAREST == FP16_ROUND_MODE) { // Truncate
                bTrctHigh = ((mTrct & FP32_SIGN_MASK) > 0);
                bTrctLeft = ((mTrct & FP32_ABS_MAX) > 0);
            }
            mTmp = ManRoundToNearest(bLastBit, bTrctHigh, bTrctLeft, mTmp);
            while (mTmp >= mMax || eVal < 0) {
                mTmp = mTmp >> 1;
                eVal = eVal + 1;
            }
            if (eVal >= FP16_MAX_EXP) {
                eVal = FP16_MAX_EXP - 1;
                mTmp = FP16_MAX_MAN;
            }
        } else {
            eVal = FP16_EXP_BIAS;
            mTmp = mTmp << (CONST_11 - len);
            eVal = eVal + (len - 1);
        }
        uint16_t mVal = static_cast<uint16_t>(mTmp);
        val = FP16_CONSTRUCTOR(0u, static_cast<uint16_t>(eVal), mVal);
    }
    return *this;
}

Fp16T &Fp16T::operator=(const double &dValue)
{
    uint16_t sVal = 0;
    uint16_t mVal = 0;
    int16_t eVal = 0;
    uint32_t mLenDelta = 0;
    uint64_t eD = 0;
    uint64_t mD = 0;
    uint64_t ui64V = *(reinterpret_cast<const uint64_t *>(&dValue)); // 1:11:52bit sign:exp:manVal

    sVal = static_cast<uint16_t>((ui64V & FP64_SIGN_MASK) >> FP64_SIGN_INDEX); // 4Byte
    eD = (ui64V & FP64_EXP_MASK) >> FP64_MAN_LEN; // 10 bit exponent
    mD = (ui64V & FP64_MAN_MASK); // 52 bit mantissa
    mLenDelta = FP64_MAN_LEN - FP16_MAN_LEN;

    bool needRound = false;
    // Exponent overflow/NaN converts to signed inf/NaN
    if (eD >= 0x410u) {
        // 0x410:1040=1023+16
        eVal = FP16_MAX_EXP - 1;
        mVal = FP16_MAX_MAN;
        val = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
    } else if (eD <= 0x3F0u) {
        // 0x3F0:1008=1023-15
        // Exponent underflow converts to denormalized half or signed zero
        eVal = 0;
        if (eD >= 0x3E7u) {
            // 0x3E7u:999=1023-24 Denormal
            mD = (FP64_MAN_HIDE_BIT | mD);
            uint16_t shiftOut = FP64_MAN_LEN;
            uint64_t mTmp = (static_cast<uint64_t>(mD)) << (eD - 0x3E7u);

            needRound = IsRoundOne(mTmp, shiftOut);
            mVal = static_cast<uint16_t>(mTmp >> shiftOut);
            if (needRound) {
                mVal += 1;
            }
        } else if (eD == 0x3E6u && mD > 0) {
            mVal = 1;
        } else {
            mVal = 0;
        }
    } else {
        // Regular case with no overflow or underflow
        // 0x3F0:1008=1023-15
        eVal = static_cast<int16_t>(eD - 0x3F0u);
        needRound = IsRoundOne(mD, mLenDelta);
        mVal = static_cast<uint16_t>(mD >> mLenDelta);
        if (needRound) {
            mVal += 1;
        }
        if ((mVal & FP16_MAN_HIDE_BIT) != 0) {
            eVal += 1;
        }
    }

    Fp16Normalize(eVal, mVal);
    val = FP16_CONSTRUCTOR(sVal, static_cast<uint16_t>(eVal), mVal);
    return *this;
}

// convert
Fp16T::operator int8_t() const { return Fp16ToInt8(val); }

Fp16T::operator int16_t() const { return Fp16ToInt16(val); }

Fp16T::operator int32_t() const { return Fp16ToInt32(val); }

Fp16T::operator uint8_t() const { return Fp16ToUInt8(val); }

Fp16T::operator uint16_t() const { return Fp16ToUInt16(val); }

Fp16T::operator uint32_t() const { return Fp16ToUInt32(val); }

Fp16T::operator float() const { return Fp16ToFloat(val); }

Fp16T::operator double() const { return Fp16ToDouble(val); }

int8_t Fp16T::ToInt8() { return Fp16ToInt8(val); }

int16_t Fp16T::ToInt16() { return Fp16ToInt16(val); }

int32_t Fp16T::ToInt32() { return Fp16ToInt32(val); }

uint8_t Fp16T::ToUInt8() { return Fp16ToUInt8(val); }

uint16_t Fp16T::ToUInt16() { return Fp16ToUInt16(val); }

uint32_t Fp16T::ToUInt32() { return Fp16ToUInt32(val); }

float Fp16T::ToFloat() { return Fp16ToFloat(val); }

double Fp16T::ToDouble() { return Fp16ToDouble(val); }

int Fp16T::IsInf() const
{
    if ((val & FP16_ABS_MAX) == FP16_EXP_MASK) {
        if ((val & FP16_SIGN_MASK) != 0) {
            return -1;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

std::ostream &operator<<(std::ostream &os, const Fp16T &fp16)
{
    os << static_cast<float>(fp16);
    return os;
}
} // namespace Mki
