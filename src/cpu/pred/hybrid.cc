/*
 * Copyright (c) 2011, 2014 ARM Limited
 * Copyright (c) 2022-2023 The University of Edinburgh
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 #include "cpu/pred/hybrid.hh"

// #include "base/bitfield.hh"
// #include "base/intmath.hh"
 
 namespace gem5
 {
 
 namespace branch_prediction
 {
 
 HybridBP::HybridBP(const HybridBPParams &params)
     : BPredUnit(params),
     localBP(params.localBP),
     tournamentBP(params.tournamentBP),
     bimodeBP(params.bimodeBP),
     perceptronBP(params.perceptronBP),
     tageBP(params.tageBP),
     stats(this)
 {

 }
 
 bool
 HybridBP::lookup(ThreadID tid, Addr pc, void * &bp_history)
 {
    // Create a new Prediction History container to hold the metadata
    // of this prediction. Each branch predictor uses this data in its own way
    // to track speculative predictionas

    PredHistPtrContainer *phpc = new PredHistPtrContainer;
    
    
    // Call the local predictor to see what it predict
    // (the predictor will also update its internal state)
    // ! Do this also for all 5 bps
    bool localPred = localBP->lookup(tid, pc, phpc->localHist);
    bool tournamentPred = tournamentBP->lookup(tid, pc, phpc->tournamentHist);
    bool bimodePred = bimodeBP->lookup(tid, pc, phpc->bimodeHist);
    bool tagePred = tageBP->lookup(tid, pc, phpc->tageHist);
    bool perceptronPred = perceptronBP->lookup(tid, pc, phpc->perceptronHist);
    phpc->localPred = localPred;
    phpc->tournamentPred = tournamentPred;
    phpc->bimodePred = bimodePred;
    phpc->tagePred = tagePred;
    phpc->perceptronPred = perceptronPred;
    

    // Set pointer that will get passed back in subsequent calls for this branch
    bp_history = phpc;

    // Find best predictor using accuracyHistory
    // Get the accuracy histories for each bp for this branch
    BranchAccuracy* curBranchAcc = &accuracyHistory[pc];
    // Create an array of deque pointers that we can loop over
    std::deque<bool>* histories[NumBps] = {
        &curBranchAcc->local,
        &curBranchAcc->tournament,
        &curBranchAcc->bimode,
        &curBranchAcc->tage,
        &curBranchAcc->perceptron
    };

    bool finalPred;

    int curInitCounter = initCounters[pc];

    // Warm up period for 
    if (curInitCounter < HistoryWindow) {
        int predictor = curInitCounter % NumBps;
        initCounters[pc]++;
        switch (predictor) {
            case 0: finalPred = phpc->localPred; stats.localPreds++; break;
            case 1: finalPred = phpc->tournamentPred; stats.tournamentPreds++; break;
            case 2: finalPred = phpc->bimodePred; stats.bimodePreds++; break;
            case 3: finalPred = phpc->tagePred; stats.tagePreds++; break;
            case 4: finalPred = phpc->perceptronPred; stats.perceptronPreds++; break;
            default: finalPred = phpc->tagePred; stats.tagePreds++; break;
        }
    } else {
        // For each bp, calculate its accuracy based on recent history
        int bestPredictor = 0;
        double bestAcc = 0.0;
        for (int i = 0; i < NumBps; i++) {
            auto curDq = histories[i];
            int correct = 0;
            // For each outcome in *curDq
            for (bool outcome : *curDq) {
                if (outcome) correct++;
            }
            // Avoid dividing by 0
            double curAcc = curDq->empty() ? 0.0 : (double)correct / curDq->size();
            if (curAcc > bestAcc) {
                bestAcc = curAcc;
                bestPredictor = i;
            }
        }

        switch (bestPredictor) {
            case 0: finalPred = phpc->localPred; stats.localPreds++; break;
            case 1: finalPred = phpc->tournamentPred; stats.tournamentPreds++; break;
            case 2: finalPred = phpc->bimodePred; stats.bimodePreds++; break;
            case 3: finalPred = phpc->tagePred; stats.tagePreds++; break;
            case 4: finalPred = phpc->perceptronPred; stats.perceptronPreds++; break;
            default: finalPred = phpc->tagePred; stats.tagePreds++; break;
        }
    }
    
    
    
    return finalPred;

 }
 
 void
 HybridBP::updateHistories(ThreadID tid, Addr pc, bool uncond,
                          bool taken, Addr target, void * &bp_history)
 {
    PredHistPtrContainer *phpc;
    // If this is a conditional branch, pointer to bp_history was created before
    assert(uncond || bp_history);

    if (!bp_history) {
        // Some of the branch predictors update state on non-conditional insts,
        // so we need a container to hold them
        phpc = new PredHistPtrContainer;
        bp_history = phpc;
    }
    if (!uncond) {
        phpc = static_cast<PredHistPtrContainer *>(bp_history);
        // TODO: speculatively update decision state if needed
        // (see tournament predictor for example)
    }

    localBP->updateHistories(tid, pc, uncond, taken, target, phpc->localHist);
    tournamentBP->updateHistories(tid, pc, uncond, taken, target, phpc->tournamentHist);
    bimodeBP->updateHistories(tid, pc, uncond, taken, target, phpc->bimodeHist);
    tageBP->updateHistories(tid, pc, uncond, taken, target, phpc->tageHist);
    perceptronBP->updateHistories(tid, pc, uncond, taken, target, phpc->perceptronHist);
}
 
 
 void
 HybridBP::update(ThreadID tid, Addr pc, bool taken,
                      void * &bp_history, bool squashed,
                      const StaticInstPtr & inst, Addr target)
 {
    // This is an update for the outcome of a previously predicted instruction
    // assert(bp_history);

    if (!bp_history) {
        return;
    }


    PredHistPtrContainer *phpc = static_cast<PredHistPtrContainer *>(bp_history);

    localBP->update(tid, pc, taken, phpc->localHist, squashed, inst, target);
    tournamentBP->update(tid, pc, taken, phpc->tournamentHist, squashed, inst, target);
    bimodeBP->update(tid, pc, taken, phpc->bimodeHist, squashed, inst, target);
    tageBP->update(tid, pc, taken, phpc->tageHist, squashed, inst, target);
    perceptronBP->update(tid, pc, taken, phpc->perceptronHist, squashed, inst, target);

    if (squashed) {
        // TODO: restore decision state if needed
        return;
    }

    BranchAccuracy* curBranchAcc = &accuracyHistory[pc];

    // For each bp, get rid of oldest history if history window is exceeded
    if (curBranchAcc->local.size() >= HistoryWindow) curBranchAcc->local.pop_front();
    if (curBranchAcc->tournament.size() >= HistoryWindow) curBranchAcc->tournament.pop_front();
    if (curBranchAcc->bimode.size() >= HistoryWindow) curBranchAcc->bimode.pop_front();
    if (curBranchAcc->tage.size() >= HistoryWindow) curBranchAcc->tage.pop_front();
    if (curBranchAcc->perceptron.size() >= HistoryWindow) curBranchAcc->perceptron.pop_front();

    // For each bp, push most recent prediction history
    // See whether prediction for each bp was correct
    curBranchAcc->local.push_back(phpc->localPred == taken);
    curBranchAcc->tournament.push_back(phpc->tournamentPred == taken);
    curBranchAcc->bimode.push_back(phpc->bimodePred == taken);
    curBranchAcc->tage.push_back(phpc->tagePred == taken);
    curBranchAcc->perceptron.push_back(phpc->perceptronPred == taken);

    // predictor should have deleted this history; safe to delete container
    assert(!phpc->localHist);
    delete phpc;
    bp_history = nullptr;

}
 
 void
 HybridBP::squash(ThreadID tid, void * &bp_history)
 {
    // Add a check for null bp_history
    if (!bp_history) {
        return;  // Nothing to squash if bp_history is null
    }
    PredHistPtrContainer *phpc = static_cast<PredHistPtrContainer *>(bp_history);
    
    // TODO: restore decision state if needed
    
    localBP->squash(tid, phpc->localHist);
    tournamentBP->squash(tid, phpc->tournamentHist);
    bimodeBP->squash(tid, phpc->bimodeHist);
    tageBP->squash(tid, phpc->tageHist);
    perceptronBP->squash(tid, phpc->perceptronHist);
    
    // predictor should have deleted this history; safe to delete container
    assert(!phpc->localHist);
    assert(!phpc->tournamentHist);
    assert(!phpc->bimodeHist);
    assert(!phpc->tageHist);
    assert(!phpc->perceptronHist);
    delete phpc;
    bp_history = nullptr;
 }

 HybridBP::HybridBPStats::HybridBPStats(statistics::Group *parent)
 : statistics::Group(parent),
   ADD_STAT(localPreds, statistics::units::Count::get(), "Number of times local predictor was used"),
   ADD_STAT(tournamentPreds, statistics::units::Count::get(), "Number of times tournament predictor was used"),
   ADD_STAT(bimodePreds, statistics::units::Count::get(), "Number of times BiMode predictor was used"),
   ADD_STAT(tagePreds, statistics::units::Count::get(), "Number of times TAGE predictor was used"),
   ADD_STAT(perceptronPreds, statistics::units::Count::get(), "Number of times perceptron predictor was used")
   {
}
 
 } // namespace branch_prediction
 } // namespace gem5
 
