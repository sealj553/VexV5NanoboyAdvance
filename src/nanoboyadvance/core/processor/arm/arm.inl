/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#define Z_FLAG (ctx.cpsr & MASK_ZFLAG)
#define C_FLAG (ctx.cpsr & MASK_CFLAG)
#define N_FLAG (ctx.cpsr & MASK_NFLAG)
#define V_FLAG (ctx.cpsr & MASK_VFLAG)

inline void ARM::step() {
    auto& pipe = ctx.pipe;

    if (ctx.cpsr & MASK_THUMB) {
        ctx.r15 &= ~1;
        executeThumb(pipe[0]);
    } else {
        ctx.r15 &= ~3;

        u32 instruction = pipe[0];

        pipe[0] = pipe[1];
        pipe[1] = busRead32(ctx.r15, M_SEQ);

        executeARM(instruction);
    }
}

inline bool ARM::checkCondition(Condition condition) {
    if (condition == COND_AL) {
        return true;
    }
    switch (condition) {
        case COND_EQ: return  Z_FLAG;
        case COND_NE: return !Z_FLAG;
        case COND_CS: return  C_FLAG;
        case COND_CC: return !C_FLAG;
        case COND_MI: return  N_FLAG;
        case COND_PL: return !N_FLAG;
        case COND_VS: return  V_FLAG;
        case COND_VC: return !V_FLAG;
        case COND_HI: return  C_FLAG && !Z_FLAG;
        case COND_LS: return !C_FLAG ||  Z_FLAG;
        case COND_GE: return  N_FLAG ==  V_FLAG;
        case COND_LT: return  N_FLAG !=  V_FLAG;
        case COND_GT: return !Z_FLAG && (N_FLAG == V_FLAG);
        case COND_LE: return  Z_FLAG || (N_FLAG != V_FLAG);
        case COND_AL: return true;
        case COND_NV: return false;
    }
    return false;
}

inline void ARM::updateSignFlag(u32 result) {
    if (result >> 31) {
        ctx.cpsr |= MASK_NFLAG;
    } else {
        ctx.cpsr &= ~MASK_NFLAG;
    }
}

inline void ARM::updateZeroFlag(u64 result) {
    if (result == 0) {
        ctx.cpsr |= MASK_ZFLAG;
    } else {
        ctx.cpsr &= ~MASK_ZFLAG;
    }
}

inline void ARM::updateCarryFlag(bool carry) {
    if (carry) {
        ctx.cpsr |= MASK_CFLAG;
    } else {
        ctx.cpsr &= ~MASK_CFLAG;
    }
}

inline void ARM::refillPipeline() {
    if (ctx.cpsr & MASK_THUMB) {
        ctx.pipe[0] = busRead16(ctx.r15,     M_NONSEQ);
        ctx.pipe[1] = busRead16(ctx.r15 + 2, M_SEQ);
        ctx.r15 += 4;
    } else {
        ctx.pipe[0] = busRead32(ctx.r15,     M_NONSEQ);
        ctx.pipe[1] = busRead32(ctx.r15 + 4, M_SEQ);
        ctx.r15 += 8;
    }
}
