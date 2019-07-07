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

#include <cstring>
#include <iostream>
#include "BoundaryValuesCpp.hpp"
#include "UvwCpp.hpp"
#include "SorSolverCpp.hpp"
#include "CfdSolverCpp.hpp"

void CfdSolverCpp::initialize(const std::string &scenarioName,
        Real Re, Real Pr, Real omg, Real eps, int itermax, Real alpha, Real beta, Real dt, Real tau,
        Real GX, Real GY, Real GZ, bool useTemperature, Real T_h, Real T_c,
        int imax, int jmax, int kmax, Real dx, Real dy, Real dz,
        Real *U, Real *V, Real *W, Real *P, Real *T, uint32_t *Flag) {
    this->scenarioName = scenarioName;
    this->Re = Re;
    this->Pr = Pr;
    this->omg = omg;
    this->eps = eps;
    this->itermax = itermax;
    this->alpha = alpha;
    this->beta = beta;
    this->dt = dt;
    this->tau = tau;
    this->GX = GX;
    this->GY = GY;
    this->GZ = GZ;
    this->useTemperature = useTemperature;
    this->T_h = T_h;
    this->T_c = T_c;
    this->imax = imax;
    this->jmax = jmax;
    this->kmax = kmax;
    this->dx = dx;
    this->dy = dy;
    this->dz = dz;

    // Create all arrays for the simulation.
    this->U = new Real[(imax+1)*(jmax+2)*(kmax+2)];
    this->V = new Real[(imax+2)*(jmax+1)*(kmax+2)];
    this->W = new Real[(imax+2)*(jmax+2)*(kmax+1)];
    this->P = new Real[(imax+2)*(jmax+2)*(kmax+2)];
    this->P_temp = new Real[(imax+2)*(jmax+2)*(kmax+2)];
    this->T = new Real[(imax+2)*(jmax+2)*(kmax+2)];
    this->T_temp = new Real[(imax+2)*(jmax+2)*(kmax+2)];
    this->F = new Real[(imax+1)*(jmax+1)*(kmax+1)];
    this->G = new Real[(imax+1)*(jmax+1)*(kmax+1)];
    this->H = new Real[(imax+1)*(jmax+1)*(kmax+1)];
    this->RS = new Real[(imax+1)*(jmax+1)*(kmax+1)];
    this->Flag = new FlagType[(imax+2)*(jmax+2)*(kmax+2)];

    memset(this->U, 0, sizeof(Real)*(imax+1)*(jmax+2)*(kmax+2));
    memset(this->V, 0, sizeof(Real)*(imax+2)*(jmax+1)*(kmax+2));
    memset(this->W, 0, sizeof(Real)*(imax+1)*(jmax+2)*(kmax+1));
    memset(this->P, 0, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memset(this->P_temp, 0, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memset(this->T, 0, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memset(this->T_temp, 0, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memset(this->F, 0, sizeof(Real)*(imax+1)*(jmax+1)*(kmax+1));
    memset(this->G, 0, sizeof(Real)*(imax+1)*(jmax+1)*(kmax+1));
    memset(this->H, 0, sizeof(Real)*(imax+1)*(jmax+1)*(kmax+1));
    memset(this->RS, 0, sizeof(Real)*(imax+1)*(jmax+1)*(kmax+1));
    memset(this->Flag, 0, sizeof(FlagType)*(imax+2)*(jmax+2)*(kmax+2));

    // Copy the content of U, V, W, P, T and Flag to the internal representation.
    memcpy(this->U, U, sizeof(Real)*(imax+1)*(jmax+2)*(kmax+2));
    memcpy(this->V, V, sizeof(Real)*(imax+2)*(jmax+1)*(kmax+2));
    memcpy(this->W, W, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+1));
    memcpy(this->P, P, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memcpy(this->T, T, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memcpy(this->Flag, Flag, sizeof(unsigned int)*(imax+2)*(jmax+2)*(kmax+2));
}

CfdSolverCpp::~CfdSolverCpp() {
    delete[] U;
    delete[] V;
    delete[] W;
    delete[] P;
    delete[] P_temp;
    delete[] T;
    delete[] T_temp;
    delete[] F;
    delete[] G;
    delete[] H;
    delete[] RS;
    delete[] Flag;
}

#include <iostream>

void CfdSolverCpp::setBoundaryValues() {
    setBoundaryValuesCpp(T_h, T_c, imax, jmax, kmax, U, V, W, T, Flag);
}

void CfdSolverCpp::setBoundaryValuesScenarioSpecific() {
    setBoundaryValuesScenarioSpecificCpp(scenarioName, imax, jmax, kmax, U, V, W, Flag);

    /*std::cout << std::endl;
    for (int j = jmax+1; j >= 0; j--) {
        for (int i = 0; i <= imax+1; i++) {
            std::cout << U[IDXU(i,j,kmax/2)] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;*/
}

Real CfdSolverCpp::calculateDt() {
    calculateDtCpp(Re, Pr, tau, dt, dx, dy, dz, imax, jmax, kmax, U, V, W, useTemperature);
    return dt;
}


void CfdSolverCpp::calculateTemperature() {
    Real *temp = T;
    T = T_temp;
    T_temp = temp;
    calculateTemperatureCpp(Re, Pr, alpha, dt, dx, dy, dz, imax, jmax, kmax, U, V, W, T, T_temp, Flag);
}

void CfdSolverCpp::calculateFgh() {
    calculateFghCpp(Re, GX, GY, GZ, alpha, beta, dt, dx, dy, dz, imax, jmax, kmax, U, V, W, T, F, G, H, Flag);
}

void CfdSolverCpp::calculateRs() {
    calculateRsCpp(dt, dx, dy, dz, imax, jmax, kmax, F, G, H, RS);

    if (jmax == 1) {
        std::cout << std::endl;
        for (int k = kmax; k >= 1; k--) {
            for (int i = 1; i <= imax; i++) {
                std::cout << RS[IDXRS(i,jmax/2+1,k)] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    } else {
        std::cout << std::endl;
        for (int j = jmax; j >= 1; j--) {
            for (int i = 1; i <= imax; i++) {
                std::cout << RS[IDXRS(i,j,kmax/2+1)] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}


void CfdSolverCpp::executeSorSolver() {
    sorSolverCpp(omg, eps, itermax, dx, dy, dz, imax, jmax, kmax, P, P_temp, RS, Flag);
}

void CfdSolverCpp::calculateUvw() {
    calculateUvwCpp(dt, dx, dy, dz, imax, jmax, kmax, U, V, W, F, G, H, P, Flag);
}

void CfdSolverCpp::getDataForOutput(Real *U, Real *V, Real *W, Real *P, Real *T) {
    // Copy the content of U, V, W, P, T in the internal representation to the specified output arrays.
    memcpy(U, this->U, sizeof(Real)*(imax+1)*(jmax+2)*(kmax+2));
    memcpy(V, this->V, sizeof(Real)*(imax+2)*(jmax+1)*(kmax+2));
    memcpy(W, this->W, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+1));
    memcpy(P, this->P, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memcpy(T, this->T, sizeof(Real)*(imax+2)*(jmax+2)*(kmax+2));
    memcpy(Flag, this->Flag, sizeof(unsigned int)*(imax+2)*(jmax+2)*(kmax+2));
}
