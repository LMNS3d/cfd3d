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

#include <iostream>
#include <chrono>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <omp.h>
#include "CfdSolver/Init.hpp"
#include "CfdSolver/Flag.hpp"
#include "CfdSolver/CfdSolver.hpp"
#include "CfdSolver/Cpp/CfdSolverCpp.hpp"
#ifdef USE_MPI
#include "CfdSolver/Mpi/CfdSolverMpi.hpp"
#include "CfdSolver/Mpi/MpiHelpers.hpp"
#endif
#ifdef USE_CUDA
#include "CfdSolver/Cuda/CfdSolverCuda.hpp"
#endif
#ifdef USE_OPENCL
#include "CfdSolver/Opencl/CfdSolverOpencl.hpp"
#endif
#include "IO/IOUtils.hpp"
#include "IO/ArgumentParser.hpp"
#include "IO/ProgressBar.hpp"
#include "IO/ScenarioFile.hpp"
#include "IO/NetCdfWriter.hpp"
#include "IO/VtkWriter.hpp"
#include "IO/TrajectoriesFile.hpp"
#include "IO/GeometryCreator.hpp"
#include "ParticleTracer/StreamlineTracer.hpp"

const std::string scenarioDirectory = "../scenarios/";
const std::string outputDirectory = "output/";
const std::string geometryDirectory = "geometry/";
const std::string lineDirectory = "lines/";

int main(int argc, char *argv[]) {
    CfdSolver *cfdSolver;
    ProgressBar progressBar;
    std::string outputFileWriterType;
    OutputFileWriter *outputFileWriter = nullptr;
    LinearSystemSolverType linearSystemSolverType;
    bool traceStreamlines = false;
    std::vector<rvec3> particleSeedingLocations;
    bool dataIsUpToDate = true;
    bool shallWriteOutput = true;

#ifdef USE_MPI
    int myrank = 0, nproc = 1, rankL, rankR, rankD, rankU, rankB, rankF, threadIdxI, threadIdxJ, threadIdxK,
            il, iu, jl, ju, kl, ku;
#else
    int nproc = 1;
    int myrank = 0;
#endif
    // MPI data
    int iproc = 1, jproc = 1, kproc = 1;
    // For hybrid MPI solver
    int numOmpHybridThreads = 1;

    // CUDA & OpenCL data
    int blockSizeX, blockSizeY, blockSizeZ, blockSize1D;

    // OpenCL data
    int openclPlatformId = 0;

    int imax, jmax, kmax, itermax, numParticles;
    Real Re, Pr, UI, VI, WI, PI, TI, GX, GY, GZ, tEnd, dtWrite, xLength, yLength, zLength, xOrigin, yOrigin, zOrigin,
            dt, dx, dy, dz, alpha, omg, tau, eps, beta, T_h, T_c;
    bool useTemperature = true;
    std::string scenarioName, geometryName, scenarioFilename, geometryFilename, outputFilename, solverName;
    parseArguments(
            argc, argv, scenarioName, solverName, outputFileWriterType, shallWriteOutput, linearSystemSolverType,
            numParticles, traceStreamlines, iproc, jproc, kproc, numOmpHybridThreads,
            blockSizeX, blockSizeY, blockSizeZ, blockSize1D, openclPlatformId);
    scenarioFilename = scenarioDirectory + scenarioName + ".dat";

#ifdef USE_MPI
    if (solverName == "mpi") {
        mpiInit(argc, argv, iproc, jproc, kproc, imax, jmax, kmax,
                myrank, il, iu, jl, ju, kl, ku,
                rankL, rankR, rankD, rankU, rankB, rankF,
                threadIdxI, threadIdxJ, threadIdxK, nproc);

        omp_set_num_threads(numOmpHybridThreads);
    }
#endif

    if (outputFileWriterType == "netcdf") {
        outputFileWriter = new NetCdfWriter(nproc, myrank);
    } else if (outputFileWriterType == "vtk") {
        outputFileWriter = new VtkWriter(nproc, myrank);
    } else if (outputFileWriterType == "vtk-binary") {
        outputFileWriter = new VtkWriter(nproc, myrank, true);
    } else if (outputFileWriterType == "vtk-ascii") {
        outputFileWriter = new VtkWriter(nproc, myrank, false);
    } else if (outputFileWriterType.length() == 0) {
        outputFileWriter = new VtkWriter(nproc, myrank);
    } else {
        if (myrank == 0) {
            std::cerr << "Invalid output format." << std::endl;
        }
        exit(1);
    }

    readScenarioConfigurationFromFile(
            scenarioFilename, scenarioName, geometryName,
            tEnd, dtWrite, xLength, yLength, zLength, xOrigin, yOrigin, zOrigin,
            UI, VI, WI, PI, TI, GX, GY, GZ,
            Re, Pr, omg, eps, itermax, alpha, beta, dt, tau, useTemperature,
            T_h, T_c, imax, jmax, kmax, dx, dy, dz);
    rvec3 gridOrigin = rvec3(xOrigin, yOrigin, zOrigin);
    rvec3 gridSize = rvec3(xLength, yLength, zLength);
    StreamlineTracer streamlineTracer;
    particleSeedingLocations = getParticleSeedingLocationsForScenario(scenarioName, numParticles, gridOrigin, gridSize);

    if (!useTemperature){
        T_c = 0.0;
        T_h = 0.0;
        beta = 0.0;
        Pr = 0.0;
        TI = 0.0;
    }

#ifdef REAL_FLOAT
    eps *= 2.0f;
#endif

    geometryFilename = geometryDirectory + geometryName;
    outputFilename = outputDirectory + scenarioName;
    if (myrank == 0) {
        std::cout << "Scenario name: " << scenarioName << std::endl;
        std::cout << "Scenario file: " << scenarioFilename << std::endl;
        std::cout << "Geometry file: " << geometryFilename << std::endl;
        std::cout << "Output file: " << outputFilename << std::endl;
        std::cout << "Solver name: " << solverName << std::endl;
    }

    std::string outputFormatEnding = outputFileWriter->getOutputFormatEnding();

    prepareOutputDirectory(
            outputDirectory, outputFilename, outputFormatEnding, lineDirectory, geometryDirectory, shallWriteOutput);

    Real n = 0;
    Real t = 0;
    Real tWrite = 0;

    Real *U, *V, *W, *P, *T;
    FlagType *Flag, *FlagAll;

#ifdef USE_MPI
    if (solverName == "mpi") {
        // Set the range of the domain.
        mpiDomainDecompositionScheduling(imax, iproc, threadIdxI, il, iu);
        mpiDomainDecompositionScheduling(jmax, jproc, threadIdxJ, jl, ju);
        mpiDomainDecompositionScheduling(kmax, kproc, threadIdxK, kl, ku);

        // For debugging: Print the domain decomposition.
        /*for (int i = 0; i < nproc; i++) {
            if (myrank == i) {
                std::cout << "idx: " << threadIdxI << ", il: " << il << ", iu: " << iu << std::endl;
                std::cout << "idx: " << threadIdxJ << ", jl: " << jl << ", ju: " << ju << std::endl;
                std::cout << "idx: " << threadIdxK << ", kl: " << kl << ", ku: " << ku << std::endl;
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }*/

        // Create all arrays for the simulation.
        U = new Real[(iu - il + 4)*(ju - jl + 3)*(ku - kl + 3)];
        V = new Real[(iu - il + 3)*(ju - jl + 4)*(ku - kl + 3)];
        W = new Real[(iu - il + 3)*(ju - jl + 3)*(ku - kl + 4)];
        P = new Real[(iu - il + 3)*(ju - jl + 3)*(ku - kl + 3)];
        T = new Real[(iu - il + 3)*(ju - jl + 3)*(ku - kl + 3)];
        Flag = new FlagType[(iu - il + 3)*(ju - jl + 3)*(ku - kl + 3)];
        FlagAll = new FlagType[(imax+2)*(jmax+2)*(kmax+2)];
    } else
#endif
    {
        // Create all arrays for the simulation.
        U = new Real[(imax+1)*(jmax+2)*(kmax+2)];
        V = new Real[(imax+2)*(jmax+1)*(kmax+2)];
        W = new Real[(imax+2)*(jmax+2)*(kmax+1)];
        P = new Real[(imax+2)*(jmax+2)*(kmax+2)];
        T = new Real[(imax+2)*(jmax+2)*(kmax+2)];
        Flag = new FlagType[(imax+2)*(jmax+2)*(kmax+2)];
        FlagAll = Flag;
    }

    if (geometryName == "none") {
        initFlagNoObstacles(scenarioName, imax, jmax, kmax, FlagAll);
    } else {
        if (!boost::filesystem::exists(geometryFilename)) {
            generateScenario(scenarioName, geometryFilename, imax, jmax, kmax);
        }
        initFlagFromGeometryFile(scenarioName, geometryFilename, imax, jmax, kmax, FlagAll);
    }

#ifdef USE_MPI
    if (solverName == "mpi") {
        outputFileWriter->setMpiData(il, iu, jl, ju, kl, ku);

        // Copy the relevant part of the geometry.
        for (int i = il - 1; i <= iu + 1; i++) {
            for (int j = jl - 1; j <= ju + 1; j++) {
                for (int k = kl - 1; k <= ku + 1; k++) {
                    Flag[(((i) - (il-1))*(ju - jl + 3)*(ku - kl + 3) + ((j) - (jl-1))*(ku - kl + 3) + ((k) - (kl-1)))]
                            = FlagAll[IDXFLAG(i, j, k)];
                }
            }
        }

        initArraysMpi(UI, VI, WI, PI, TI, il, iu, jl, ju, kl, ku, U, V, W, P, T, FlagAll);
    } else
#endif
    {
        initArrays(UI, VI, WI, PI, TI, imax, jmax, kmax, U, V, W, P, T, FlagAll);
    }

    auto startTime = std::chrono::system_clock::now();

    if (shallWriteOutput) {
        outputFileWriter->initializeWriter(outputFilename, imax, jmax, kmax, dx, dy, dz, xOrigin, yOrigin, zOrigin);
        outputFileWriter->writeTimestep(0, t, U, V, W, P, T, Flag);
    }


    if (solverName == "cpp") {
        cfdSolver = new CfdSolverCpp();
    }
#ifdef USE_MPI
    else if (solverName == "mpi") {
        cfdSolver = new CfdSolverMpi(il, iu, jl, ju, kl, ku, myrank, rankL, rankR, rankD, rankU, rankB, rankF);
    }
#endif
#ifdef USE_CUDA
    else if (solverName == "cuda") {
        if (linearSystemSolverType != LINEAR_SOLVER_JACOBI) {
            std::cerr << "Warning: CUDA solver was selected, but a linear solver different from Jacobi. "
                    << "Only the Jacobi solver is supported for CUDA." << std::endl;
        }
        cfdSolver = new CfdSolverCuda(blockSizeX, blockSizeY, blockSizeZ, blockSize1D);
    }
#endif
#ifdef USE_OPENCL
    else if (solverName == "opencl") {
        if (linearSystemSolverType != LINEAR_SOLVER_JACOBI) {
            std::cerr << "Warning: OpenCL solver was selected, but a linear solver different from Jacobi. "
                      << "Only the Jacobi solver is supported for OpenCL." << std::endl;
        }
        cfdSolver = new CfdSolverOpencl(openclPlatformId, blockSizeX, blockSizeY, blockSizeZ, blockSize1D);
    }
#endif
    else {
        std::cerr << "Fatal error: Unsupported solver name \"" << solverName << "\"." << std::endl;
        exit(1);
    }
    cfdSolver->initialize(scenarioName, linearSystemSolverType, shallWriteOutput,
            Re, Pr, omg, eps, itermax, alpha, beta, dt, tau, GX, GY, GZ, useTemperature,
            T_h, T_c, imax, jmax, kmax, dx, dy, dz, U, V, W, P, T, Flag);

    while (t < tEnd) {
        if (myrank == 0) {
            progressBar.printProgress(t, tEnd, 50);
        }

        cfdSolver->setBoundaryValues();
        cfdSolver->setBoundaryValuesScenarioSpecific();
        dt = cfdSolver->calculateDt();
        if (useTemperature) {
            cfdSolver->calculateTemperature();
        }
        cfdSolver->calculateFgh();
        cfdSolver->calculateRs();

        cfdSolver->executeSorSolver();
        cfdSolver->calculateUvw();
        dataIsUpToDate = false;

        t += dt;
        tWrite += dt;
        n++;
        if (tWrite - dtWrite > 0) {
            if (shallWriteOutput) {
                if (!dataIsUpToDate) {
                    cfdSolver->getDataForOutput(U, V, W, P, T);
                    dataIsUpToDate = true;
                }
                outputFileWriter->writeTimestep(n, t, U, V, W, P, T, Flag);
                if (myrank == 0) {
                    progressBar.printOutput(n, t, 50);
                }
            }
            tWrite -= dtWrite;
        }
    }

    if (traceStreamlines) {
        if (!dataIsUpToDate) {
            cfdSolver->getDataForOutput(U, V, W, P, T);
            dataIsUpToDate = true;
        }
        Real traceDt = dt * Real(5.0);
        if (boost::starts_with(scenarioName, "rayleigh_benard")) {
            traceDt *= Real(1000.0);
        }
        Trajectories streamlines = streamlineTracer.trace(
                particleSeedingLocations, gridOrigin, gridSize, traceDt, imax, jmax, kmax, dx, dy, dz, U, V, W, P, T);
        writeTrajectoriesToObjFile(lineDirectory + scenarioName + "-streamlines.obj", streamlines);
        writeTrajectoriesToBinLinesFile(lineDirectory + scenarioName + "-streamlines.binlines", streamlines);
    }


    auto endTime = std::chrono::system_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    if (myrank == 0) {
        std::cout << "System time elapsed: " << (elapsedTime.count() * 1e-6) << "s" << std::endl;
    }

    delete cfdSolver;
    delete outputFileWriter;
    delete[] U;
    delete[] V;
    delete[] W;
    delete[] P;
    delete[] T;
    if (Flag != FlagAll) {
        delete[] FlagAll;
    }
    delete[] Flag;

#ifdef USE_MPI
    if (solverName == "mpi") {
        mpiStop();
    }
#endif

    return 0;
}