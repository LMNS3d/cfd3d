/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2019, Christoph Neuhauser, Stefan Haas, Paul Ng
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BoundaryValuesMpi.hpp"
#include "../Flag.hpp"
#include "DefinesMpi.hpp"

void setLeftRightBoundariesMpi(
        Real T_h, Real T_c,
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *U, Real *V, Real *W, Real *T,
        FlagType *Flag) {
    if (il == 1) {
    #pragma omp parallel for
        for (int j = jl; j <= ju; j++) {
            for (int k = kl; k <= ku; k++) {
                // Left wall
                if (isNoSlip(Flag[IDXFLAG(0,j,k)])) {
                    U[IDXU(0,j,k)] = 0.0;
                } else if (isFreeSlip(Flag[IDXFLAG(0,j,k)])) {
                    U[IDXU(0,j,k)] = 0.0;
                } else if (isOutflow(Flag[IDXFLAG(0,j,k)])) {
                    U[IDXU(0,j,k)] = U[IDXU(1,j,k)];
                }

                // Left boundary T
                if (isHot(Flag[IDXFLAG(0,j,k)])) {
                    T[IDXT(0,j,k)] = 2 * T_h - T[IDXT(1,j,k)];
                } else if (isCold(Flag[IDXFLAG(0,j,k)])) {
                    T[IDXT(0,j,k)] = 2 * T_c - T[IDXT(1,j,k)];
                } else {
                    T[IDXT(0,j,k)] = T[IDXT(1,j,k)];
                }
            }
        }
        #pragma omp parallel for
        for (int j = jl-1; j <= ju; j++) {
            for (int k = kl-1; k <= ku; k++) {
                // Left wall
                if (isNoSlip(Flag[IDXFLAG(0,j,k)])) {
                    V[IDXV(0,j,k)] = -V[IDXV(1,j,k)];
                    W[IDXW(0,j,k)] = -W[IDXW(1,j,k)];
                }
                else if (isFreeSlip(Flag[IDXFLAG(0,j,k)])) {
                    V[IDXV(0,j,k)] = V[IDXV(1,j,k)];
                    W[IDXW(0,j,k)] = W[IDXW(1,j,k)];
                }
                else if (isOutflow(Flag[IDXFLAG(0,j,k)])) {
                    V[IDXV(0,j,k)] = V[IDXV(1,j,k)];
                    W[IDXW(0,j,k)] = W[IDXW(1,j,k)];
                }
            }
        }
    }

    if (iu == imax) {
        #pragma omp parallel for
        for (int j = jl; j <= ju; j++) {
            for (int k = kl; k <= ku; k++) {
                // Right wall
                if (isNoSlip(Flag[IDXFLAG(imax+1,j,k)])) {
                    U[IDXU(imax,j,k)] = 0.0;
                } else if (isFreeSlip(Flag[IDXFLAG(imax+1,j,k)])) {
                    U[IDXU(imax,j,k)] = 0.0;
                } else if (isOutflow(Flag[IDXFLAG(imax+1,j,k)])) {
                    U[IDXU(imax,j,k)] = U[IDXU(imax-1,j,k)];
                }

                // Right boundary T
                if (isHot(Flag[IDXFLAG(imax+1,j,k)])) {
                    T[IDXT(imax+1,j,k)] = 2 * T_h - T[IDXT(imax,j,k)];
                }  else if (isCold(Flag[IDXFLAG(imax+1,j,k)])) {
                    T[IDXT(imax+1,j,k)] = 2 * T_c - T[IDXT(imax,j,k)];
                } else {
                    T[IDXT(imax+1,j,k)] = T[IDXT(imax,j,k)];
                }
            }
        }
        #pragma omp parallel for
        for (int j = jl-1; j <= ju; j++) {
            for (int k = kl-1; k <= ku; k++) {
                // Right wall
                if (isNoSlip(Flag[IDXFLAG(imax+1,j,k)])) {
                    V[IDXV(imax+1,j,k)] = -V[IDXV(imax,j,k)];
                    W[IDXW(imax+1,j,k)] = -W[IDXW(imax,j,k)];
                }
                else if (isFreeSlip(Flag[IDXFLAG(imax+1,j,k)])) {
                    V[IDXV(imax+1,j,k)] = V[IDXV(imax,j,k)];
                    W[IDXW(imax+1,j,k)] = W[IDXW(imax,j,k)];
                }
                else if (isOutflow(Flag[IDXFLAG(imax+1,j,k)])) {
                    V[IDXV(imax+1,j,k)] = V[IDXV(imax,j,k)];
                    W[IDXW(imax+1,j,k)] = W[IDXW(imax,j,k)];
                }
            }
        }
    }
}

void setDownUpBoundariesMpi(
        Real T_h, Real T_c,
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *U, Real *V, Real *W, Real *T,
        FlagType *Flag) {
    if (jl == 1) {
        #pragma omp parallel for
        for (int i = il; i <= iu; i++) {
            for (int k = kl; k <= ku; k++) {
                // Down wall
                if (isNoSlip(Flag[IDXFLAG(i,0,k)])) {
                    V[IDXV(i,0,k)] = 0.0;
                } else if (isFreeSlip(Flag[IDXFLAG(i,0,k)])) {
                    V[IDXV(i,0,k)] = 0.0;
                } else if (isOutflow(Flag[IDXFLAG(i,0,k)])) {
                    V[IDXV(i,0,k)] = V[IDXV(i,1,k)];
                }

                // Down boundary T
                if (isHot(Flag[IDXFLAG(i,0,k)])) {
                    T[IDXT(i,0,k)] = 2 * T_h - T[IDXT(i,1,k)];
                } else if (isCold(Flag[IDXFLAG(i,0,k)])) {
                    T[IDXT(i,0,k)] = 2 * T_c - T[IDXT(i,1,k)];
                } else {
                    T[IDXT(i,0,k)] = T[IDXT(i,1,k)];
                }
            }
        }
        #pragma omp parallel for
        for (int i = il-1; i <= iu; i++) {
            for (int k = kl-1; k <= ku; k++) {
                // Down wall
                if (isNoSlip(Flag[IDXFLAG(i,0,k)])) {
                    U[IDXU(i,0,k)] = -U[IDXU(i,1,k)];
                    W[IDXW(i,0,k)] = -W[IDXW(i,1,k)];
                } else if (isFreeSlip(Flag[IDXFLAG(i,0,k)])) {
                    U[IDXU(i,0,k)] = U[IDXU(i,1,k)];
                    W[IDXW(i,0,k)] = W[IDXW(i,1,k)];
                } else if (isOutflow(Flag[IDXFLAG(i,0,k)])) {
                    U[IDXU(i,0,k)] = U[IDXU(i,1,k)];
                    W[IDXW(i,0,k)] = W[IDXW(i,1,k)];
                }
            }
        }
    }

    if (ju == jmax) {
        #pragma omp parallel for
        for (int i = il; i <= iu; i++) {
            for (int k = kl; k <= ku; k++) {
                // Up wall
                if (isNoSlip(Flag[IDXFLAG(i,jmax+1,k)])) {
                    V[IDXV(i,jmax,k)] = 0.0;
                } else if (isFreeSlip(Flag[IDXFLAG(i,jmax+1,k)])) {
                    V[IDXV(i,jmax,k)] = 0.0;
                } else if (isOutflow(Flag[IDXFLAG(i,jmax+1,k)])) {
                    V[IDXV(i,jmax,k)] = V[IDXV(i,jmax-1,k)];
                }

                // Up boundary T
                if (isHot(Flag[IDXFLAG(i,jmax+1,k)])) {
                    T[IDXT(i,jmax+1,k)] = 2 * T_h - T[IDXT(i,jmax,k)];
                } else if (isCold(Flag[IDXFLAG(i,jmax+1,k)])) {
                    T[IDXT(i,jmax+1,k)] = 2 * T_c - T[IDXT(i,jmax,k)];
                } else {
                    T[IDXT(i,jmax+1,k)] = T[IDXT(i,jmax,k)];
                }
            }
        }
        #pragma omp parallel for
        for (int i = il-1; i <= iu; i++) {
            for (int k = kl-1; k <= ku; k++) {
                // Up wall
                if (isNoSlip(Flag[IDXFLAG(i,jmax+1,k)])) {
                    U[IDXU(i,jmax+1,k)] = -U[IDXU(i,jmax,k)];
                    W[IDXW(i,jmax+1,k)] = -W[IDXW(i,jmax,k)];
                } else if (isFreeSlip(Flag[IDXFLAG(i,jmax+1,k)])) {
                    U[IDXU(i,jmax+1,k)] = U[IDXU(i,jmax,k)];
                    W[IDXW(i,jmax+1,k)] = W[IDXW(i,jmax,k)];
                } else if (isOutflow(Flag[IDXFLAG(i,jmax+1,k)])) {
                    U[IDXU(i,jmax+1,k)] = U[IDXU(i,jmax,k)];
                    V[IDXV(i,jmax,k)] = V[IDXV(i,jmax-1,k)];
                    W[IDXW(i,jmax+1,k)] = W[IDXW(i,jmax,k)];
                }
            }
        }
    }
}

void setFrontBackBoundariesMpi(
        Real T_h, Real T_c,
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *U, Real *V, Real *W, Real *T,
        FlagType *Flag) {
    if (kl == 1) {
        #pragma omp parallel for
        for (int i = il; i <= iu; i++) {
            for (int j = jl; j <= ju; j++) {
                // Back wall
                if (isNoSlip(Flag[IDXFLAG(i,j,0)])) {
                    W[IDXW(i,j,0)] = 0.0;
                }
                else if (isFreeSlip(Flag[IDXFLAG(i,j,0)])) {
                    W[IDXW(i,j,0)] = 0.0;
                }
                else if (isOutflow(Flag[IDXFLAG(i,j,0)])) {
                    W[IDXW(i,j,0)] = W[IDXW(i,j,1)];
                }

                // Back boundary T
                if (isHot(Flag[IDXFLAG(i,j,0)])) {
                    T[IDXT(i,j,0)] = 2 * T_h - T[IDXT(i,j,1)];
                }
                else if (isCold(Flag[IDXFLAG(i,j,0)])) {
                    T[IDXT(i,j,0)] = 2 * T_c - T[IDXT(i,j,1)];
                }
                else {
                    T[IDXT(i,j,0)] = T[IDXT(i,j,1)];
                }
            }
        }
        #pragma omp parallel for
        for (int i = il-1; i <= iu; i++) {
            for (int j = jl-1; j <= ju; j++) {
                // Back wall
                if (isNoSlip(Flag[IDXFLAG(i,j,0)])) {
                    U[IDXU(i,j,0)] = -U[IDXU(i,j,1)];
                    V[IDXV(i,j,0)] = -V[IDXV(i,j,1)];
                }
                else if (isFreeSlip(Flag[IDXFLAG(i,j,0)])) {
                    U[IDXU(i,j,0)] = U[IDXU(i,j,1)];
                    V[IDXV(i,j,0)] = V[IDXV(i,j,1)];
                }
                else if (isOutflow(Flag[IDXFLAG(i,j,0)])) {
                    U[IDXU(i,j,0)] = U[IDXU(i,j,1)];
                    V[IDXV(i,j,0)] = V[IDXV(i,j,1)];
                }
            }
        }
    }

    if (ku == kmax) {
        #pragma omp parallel for
        for (int i = il; i <= iu; i++) {
            for (int j = jl; j <= ju; j++) {
                // Front wall
                if (isNoSlip(Flag[IDXFLAG(i,j,kmax+1)])) {
                    W[IDXW(i,j,kmax)] = 0.0;
                }
                else if (isFreeSlip(Flag[IDXFLAG(i,j,kmax+1)])) {
                    W[IDXW(i,j,kmax)] = 0.0;
                }
                else if (isOutflow(Flag[IDXFLAG(i,j,kmax+1)])) {
                    W[IDXW(i,j,kmax)] = W[IDXW(i,j,kmax-1)];
                }

                // Front boundary T
                if (isHot(Flag[IDXFLAG(i,j,kmax+1)])) {
                    T[IDXT(i,j,kmax+1)] = 2 * T_h - T[IDXT(i,j,kmax)];
                }
                else if (isCold(Flag[IDXFLAG(i,j,kmax+1)])) {
                    T[IDXT(i,j,kmax+1)] = 2 * T_c - T[IDXT(i,j,kmax)];
                }
                else {
                    T[IDXT(i,j,kmax+1)] = T[IDXT(i,j,kmax)];
                }
            }
        }
        #pragma omp parallel for
        for (int i = il-1; i <= iu; i++) {
            for (int j = jl-1; j <= ju; j++) {
                // Front wall
                if (isNoSlip(Flag[IDXFLAG(i,j,kmax+1)])) {
                    U[IDXU(i,j,kmax+1)] = -U[IDXU(i,j,kmax)];
                    V[IDXV(i,j,kmax+1)] = -V[IDXV(i,j,kmax)];
                }
                else if (isFreeSlip(Flag[IDXFLAG(i,j,kmax+1)])) {
                    U[IDXU(i,j,kmax+1)] = U[IDXU(i,j,kmax)];
                    V[IDXV(i,j,kmax+1)] = V[IDXV(i,j,kmax)];
                }
                else if (isOutflow(Flag[IDXFLAG(i,j,kmax+1)])) {
                    U[IDXU(i,j,kmax+1)] = U[IDXU(i,j,kmax)];
                    V[IDXV(i,j,kmax+1)] = V[IDXV(i,j,kmax)];
                }
            }
        }
    }
}


void setInternalUBoundariesMpi(
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *U,
        FlagType *Flag) {
    #pragma omp parallel for
    for (int i = il; i <= iu-1; i++) {
        for (int j = jl; j <= ju; j++) {
            for (int k = kl; k <= ku; k++) {
                int R_check = 0;
                int L_check = 0;
                int R1_check = 0;
                int L1_check = 0;

                if (!isFluid(Flag[IDXFLAG(i, j, k)])) {
                    if (B_R(Flag[IDXFLAG(i, j, k)])) {
                        U[IDXU(i, j, k)] = Real(0);
                        R_check = 1;
                    }

                    if (B_L(Flag[IDXFLAG(i, j, k)])) {
                        U[IDXU(i - 1, j, k)] = Real(0);
                        L_check = 1;
                    }

                    if (B_U(Flag[IDXFLAG(i, j, k)])) {
                        if (L_check == 0) {
                            U[IDXU(i - 1, j, k)] = -U[IDXU(i - 1, j + 1, k)];
                            L1_check = 1;
                        }
                        if (R_check == 0) {
                            U[IDXU(i, j, k)] = -U[IDXU(i, j + 1, k)];
                            R1_check = 1;
                        }
                    }

                    if (B_D(Flag[IDXFLAG(i, j, k)])) {
                        if (L_check == 0) {
                            U[IDXU(i - 1, j, k)] = -U[IDXU(i - 1, j - 1, k)];
                            L1_check = 1;
                        }
                        if (R_check == 0) {
                            U[IDXU(i, j, k)] = -U[IDXU(i, j - 1, k)];
                            R1_check = 1;
                        }
                    }

                    if (B_B(Flag[IDXFLAG(i, j, k)])) {
                        if (L_check == 0 && L1_check == 0) {
                            U[IDXU(i - 1, j, k)] = -U[IDXU(i - 1, j, k - 1)];
                        }
                        if (R_check == 0 && R1_check == 0) {
                            U[IDXU(i, j, k)] = -U[IDXU(i, j, k - 1)];
                        }
                    }

                    if (B_F(Flag[IDXFLAG(i, j, k)])) {
                        if (L_check == 0 && L1_check == 0) {
                            U[IDXU(i - 1, j, k)] = -U[IDXU(i - 1, j, k + 1)];
                        }
                        if (R_check == 0 && R1_check == 0) {
                            U[IDXU(i, j, k)] = -U[IDXU(i, j, k + 1)];
                        }
                    }
                }
            }
        }
    }     
}

void setInternalVBoundariesMpi(
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *V,
        FlagType *Flag) {
    #pragma omp parallel for
    for (int i = il; i <= iu; i++) {
        for (int j = jl; j <= ju-1; j++) {
            for (int k = kl; k <= ku; k++) {
                int U_check = 0;
                int D_check = 0;
                int U1_check = 0;
                int D1_check = 0;

                if (!isFluid(Flag[IDXFLAG(i, j, k)])) {
                    if (B_U(Flag[IDXFLAG(i, j, k)])) {
                        V[IDXV(i, j, k)] = Real(0);
                        U_check = 1;
                    }

                    if (B_D(Flag[IDXFLAG(i, j, k)])) {
                        V[IDXV(i, j - 1, k)] = Real(0);
                        D_check = 1;
                    }

                    if (B_R(Flag[IDXFLAG(i, j, k)])) {
                        if (D_check == 0) {
                            V[IDXV(i, j - 1, k)] = -V[IDXV(i + 1, j - 1, k)];
                            D1_check = 0;
                        }
                        if (U_check == 0) {
                            V[IDXV(i, j, k)] = -V[IDXV(i + 1, j, k)];
                            U1_check = 0;
                        }
                    }

                    if (B_L(Flag[IDXFLAG(i, j, k)])) {
                        if (D_check == 0) {
                            V[IDXV(i, j - 1, k)] = -V[IDXV(i - 1, j - 1, k)];
                            D1_check = 0;
                        }
                        if (U_check == 0) {
                            V[IDXV(i, j, k)] = -V[IDXV(i - 1, j, k)];
                            U1_check = 0;
                        }
                    }

                    if (B_B(Flag[IDXFLAG(i, j, k)])) {
                        if (D_check == 0 && D1_check == 0) {
                            V[IDXV(i, j - 1, k)] = -V[IDXV(i, j - 1, k - 1)];
                        }
                        if (U_check == 0 && U1_check == 0) {
                            V[IDXV(i, j, k)] = -V[IDXV(i, j, k - 1)];
                        }
                    }

                    if (B_F(Flag[IDXFLAG(i, j, k)])) {
                        if (D_check == 0 && D1_check == 0) {
                            V[IDXV(i, j - 1, k)] = -V[IDXV(i, j - 1, k + 1)];
                        }
                        if (U_check == 0 && U1_check == 0) {
                            V[IDXV(i, j, k)] = -V[IDXV(i, j, k + 1)];
                        }
                    }
                }
            }
        }
    }   
}

void setInternalWBoundariesMpi(
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *W,
        FlagType *Flag) {
    #pragma omp parallel for
    for (int i = il; i <= iu; i++) {
        for (int j = jl; j <= ju; j++) {
            for (int k = kl; k <= ku-1; k++) {
                int F_check = 0;
                int B_check = 0;
                int F1_check = 0;
                int B1_check = 0;

                if (!isFluid(Flag[IDXFLAG(i, j, k)])) {
                    if (B_B(Flag[IDXFLAG(i, j, k)])) {
                        W[IDXW(i, j, k - 1)] = Real(0);
                        B_check = 1;
                    }

                    if (B_F(Flag[IDXFLAG(i, j, k)])) {
                        W[IDXW(i, j, k)] = Real(0);
                        F_check = 1;
                    }

                    if (B_R(Flag[IDXFLAG(i, j, k)])) {
                        if (B_check == 0) {
                            W[IDXW(i, j, k - 1)] = -W[IDXW(i + 1, j, k - 1)];
                            B1_check = 1;
                        }
                        if (F_check == 0) {
                            W[IDXW(i, j, k)] = -W[IDXW(i + 1, j, k)];
                            F1_check = 1;
                        }
                    }

                    if (B_L(Flag[IDXFLAG(i, j, k)])) {
                        if (B_check == 0) {
                            W[IDXW(i, j, k - 1)] = -W[IDXW(i - 1, j, k - 1)];
                            B1_check = 1;
                        }
                        if (F_check == 0) {
                            W[IDXW(i, j, k)] = -W[IDXW(i - 1, j, k)];
                            F1_check = 1;
                        }
                    }

                    if (B_U(Flag[IDXFLAG(i, j, k)])) {
                        if (B_check == 0 && B1_check == 0) {
                            W[IDXW(i, j, k - 1)] = -W[IDXW(i, j + 1, k - 1)];
                        }
                        if (F_check == 0 && F1_check == 0) {
                            W[IDXW(i, j, k)] = -W[IDXW(i, j + 1, k)];
                        }
                    }

                    if (B_D(Flag[IDXFLAG(i, j, k)])) {
                        if (B_check == 0 && B1_check == 0) {
                            W[IDXW(i, j, k - 1)] = -W[IDXW(i, j - 1, k - 1)];
                        }
                        if (F_check == 0 && F1_check == 0) {
                            W[IDXW(i, j, k)] = -W[IDXW(i, j - 1, k)];
                        }
                    }
                }
            }     
        }
    }      
}

void setInternalTBoundariesMpi(
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *T,
        FlagType *Flag) {
    #pragma omp parallel for
    for (int i = il; i <= iu; i++) {
        for (int j = jl; j <= ju; j++) {
            for (int k = kl; k <= ku; k++) {
                int numDirectFlag = 0;
                Real T_temp = Real(0);

                if (!isFluid(Flag[IDXFLAG(i, j, k)])) {
                    if (B_R(Flag[IDXFLAG(i, j, k)])) {
                        T_temp = T[IDXT(i + 1, j, k)];
                        numDirectFlag++;
                    }

                    if (B_L(Flag[IDXFLAG(i, j, k)])) {
                        T_temp = T[IDXT(i - 1, j, k)];
                        numDirectFlag++;
                    }

                    if (B_U(Flag[IDXFLAG(i, j, k)])) {
                        T_temp = T[IDXT(i, j + 1, k)];
                        numDirectFlag++;
                    }

                    if (B_D(Flag[IDXFLAG(i, j, k)])) {
                        T_temp = T[IDXT(i, j - 1, k)];
                        numDirectFlag++;
                    }

                    if (B_B(Flag[IDXFLAG(i, j, k)])) {
                        T_temp = T[IDXT(i, j, k - 1)];
                        numDirectFlag++;
                    }

                    if (B_F(Flag[IDXFLAG(i, j, k)])) {
                        T_temp = T[IDXT(i, j, k + 1)];
                        numDirectFlag++;
                    }

                    if (numDirectFlag == 0) {
                        T[IDXT(i,j,k)] = 0;
                    } else {
                        T[IDXT(i,j,k)] = T_temp / Real(numDirectFlag);
                    }
                }
            }     
        }
    }      
}

void setBoundaryValuesMpi(
        Real T_h, Real T_c,
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *U, Real *V, Real *W, Real *T,
        FlagType *Flag) {
    setLeftRightBoundariesMpi(T_h, T_c, imax, jmax, kmax, il, iu, jl, ju, kl, ku, U, V, W, T, Flag);
    setDownUpBoundariesMpi(T_h, T_c, imax, jmax, kmax, il, iu, jl, ju, kl, ku, U, V, W, T, Flag);
    setFrontBackBoundariesMpi(T_h, T_c, imax, jmax, kmax, il, iu, jl, ju, kl, ku, U, V, W, T, Flag);
    setInternalUBoundariesMpi(imax, jmax, kmax, il, iu, jl, ju, kl, ku, U,Flag);
    setInternalVBoundariesMpi(imax, jmax, kmax, il, iu, jl, ju, kl, ku, V,Flag);
    setInternalWBoundariesMpi(imax, jmax, kmax, il, iu, jl, ju, kl, ku, W,Flag);
    setInternalTBoundariesMpi(imax, jmax, kmax, il, iu, jl, ju, kl, ku, T,Flag);
}


void setBoundaryValuesScenarioSpecificMpi(
        const std::string &scenarioName,
        int imax, int jmax, int kmax,
        int il, int iu, int jl, int ju, int kl, int ku,
        Real *U, Real *V, Real *W,
        FlagType *Flag) {
    if (scenarioName == "driven_cavity") {
        if (ju == jmax) {
            #pragma omp parallel for
            for (int i = il-2; i <= iu+1; i++) {
                for (int k = kl-1; k <= ku+1; k++) {
                    // Upper wall
                    U[IDXU(i,jmax+1,k)] = 2.0 - U[IDXU(i,jmax,k)];
                }
            }
        }
    } else if (scenarioName == "flow_over_step") {
        if (il == 1) {
            #pragma omp parallel for
            for (int j = jmax / 2 + 1; j <= jmax; j++) {
                for (int k = kl; k <= ku; k++) {
                    // Left wall
                    U[IDXU(0, j, k)] = 1.0;
                    V[IDXV(0, j, k)] = 0.0;
                    W[IDXW(0, j, k)] = 0.0;
                }
            }
        }
    } else if (scenarioName == "single_tower") {
        if (il == 1) {
            #pragma omp parallel for
            for (int j = jl; j <= ju; j++) {
                for (int k = kl; k <= ku; k++) {
                    // Left wall
                    U[IDXU(0, j, k)] = 1.0;
                    V[IDXV(0, j, k)] = 0.0;
                    W[IDXW(0, j, k)] = 0.0;
                }
            }
        }
    } else if (scenarioName == "terrain_1" || scenarioName == "fuji_san" || scenarioName == "zugspitze") {
        if (il == 1) {
            #pragma omp parallel for
            for (int j = jl; j <= ju; j++) {
                for (int k = kl; k <= ku; k++) {
                    // Left wall
                    if (isInflow(Flag[IDXFLAG(0, j, k)])) {
                        U[IDXU(0, j, k)] = 1.0;
                        V[IDXV(0, j, k)] = 0.0;
                        W[IDXW(0, j, k)] = 0.0;
                    }
                }
            }
        }
    }
}
