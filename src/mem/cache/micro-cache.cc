 /*
 * Author: Caden Chou
 * Copyright (c) 2025 Brown University
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

#include "mem/cache/micro-cache.hh"
#include "params/MicroCache.hh"
#include <vector>
#include <iostream>

namespace gem5 {

MicroCache::MicroCache(const MicroCacheParams *p) :
    SimObject(*p),
    cpu_side_port(p->name + ".cpu_side_port", this),
    mem_side_port(p->name + ".mem_side_port", this),
    memSendEvent([this] { sendToMem(); }, name()),
    cpuSendEvent([this] { sendToCpu(); }, name()),
    writebackEvent([this] { writebackToMem(); }, name()),
    unblockEvent([this] { unblock(); }, name()),
    toMem(nullptr),
    toCPU(nullptr),
    toWriteback(nullptr),
    latency(p->latency * 1000),
    assoc(p->assoc),
    blocked(false),
    writingback(false),
    pending(nullptr),
    stats(this)
    // TODO: Add any additional fields (e.g., replacement policy data for multi-block caches)
{
    assert(p->size >= 64);


    // Assume the block size is 64 bytes.
    const int blockSize = 64;
    // Number of ways is given by the associativity.
    numWays = p->assoc;
    // Compute number of sets: total cache size divided by (block size * number of ways)
    numSets = (p->size / blockSize) / numWays;
    
    DPRINTF(MicroCache, "numSets: %d, numWays: %d\n", numSets, numWays);

    // Resize the 2D vector: numSets rows, each with numWays Blocks.
    blks.resize(numSets, std::vector<Block>(numWays));

    // Initialize each block in every set.
    for (int i = 0; i < numSets; i++) {
        for (int j = 0; j < numWays; j++) {
            blks[i][j].valid = false;
            blks[i][j].dirty = false;
            blks[i][j].last_access_time = 0;
            memset(blks[i][j].data, 0, sizeof(blks[i][j].data));
            blks[i][j].tag = 0;
        }
    }

}

/**
 * Notes:
 * - malloc p->size
 * - blks is now 2D array
 *  - Rows: numSets; Cols: numWays
 * - Indexing into set
 *  - setIdx = tag % numSets
 * - Sequentially scan sets
 * - Block eviction
 * - WHENEVER YOU ACCESS A BLOCK 
 * - HandleRequest
 *  - loop through all sets, loop all blocks if invalid then break
 *      -  see screenshot
 *      - You get index of chosen way
 */

/* Port registration: connects CPU and Memory sides */
Port&
MicroCache::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "cpu_side") {
        return cpu_side_port;
    } else if (if_name == "mem_side") {
        return mem_side_port;
    }
    return SimObject::getPort(if_name, idx);
}

/* 
 * handleRequest: Called when the CPU (or upper-level cache) issues a read/write request.
 * This function should implement the "Idle" and "Compare Tag" states of our FSM.
 */
bool
MicroCache::handleRequest(PacketPtr pkt)
{ 
    // DPRINTF(MicroCache, "\t\t*** REQUEST called ***\n");
    // If the cache is currently blocked (waiting on a memory response), 
    // then signal that the CPU must retry later.
    if (blocked) {
        // DPRINTF(MicroCache, "\t\t*** REQUEST end (blocked) ***\n\n");
        return false;
    }
    blocked = true;


    // ! Compute the block index and tag from the packet's address.
    assert(pkt != nullptr);

    

    // Block size is 64, so block offset should be 6
    // The convention that I am using is to store only the tag bits in blks[0].tag
    Addr pktTag = pkt->getAddr() >> 6;

    // ! TODO: Calculate the set, then scan set's ways for matching tag
    // Find the block that is being requested by the pkt from cpu
    bool hit = false;
    int set = pktTag % numSets;
    int hitWay = -1;
    for (int way = 0; way < numWays; way++) {
        if ((blks[set][way].tag == pktTag) && blks[set][way].valid) {
            hitWay = way;
            hit = true;
            break;
        }
    }

    

    DPRINTF(MicroCache, "handleRequest: pkt address: 0x%x; read: %d; write: %d\n", pkt->getAddr(), pkt->isRead(), pkt->isWrite());

    // DPRINTF(MicroCache, "BEGIN REQ block - valid: %d; dirty: %d; tag: 0x%x\n", currBlock.valid, currBlock.dirty, currBlock.tag);

    // For a one-block cache, this may simply be a comparison with blks[0].tag.
    // For multi-block caches (Part 3), use index and tag extraction based on block size, cache size, and associativity.
    // ! Check that packet's address matches the block tag and block is valid.
    if ((hit) && (pkt->isRead() || pkt->isWrite())) { 
        DPRINTF(MicroCache, "cache HIT, set: %d, way: %d\n", set, hitWay);
        // **Cache Hit Path** (FSM: Compare Tag → Idle)
        stats.hits++; // Increment hit count
        
        // ! TODO: Update block's LRU timestamp if using LRU replacement (Part 3).
        blks[set][hitWay].last_access_time = curTick();

        // DPRINTF(MicroCache, "pkt is write: %d\n", pkt->isWrite());
        // ! For write requests, mark the block dirty and write into block
        if (pkt->isWrite()) {
            DPRINTF(MicroCache, "marking set: %d, way: %d dirty\n", set, hitWay);
            blks[set][hitWay].dirty = true;
            assert(blks[set][hitWay].valid);
            assert(blks[set][hitWay].data != nullptr);
            pkt->writeData(blks[set][hitWay].data);
        }
        // DPRINTF(MicroCache, "pkt needs response: %d\n", pkt->needsResponse());
        if (pkt->needsResponse()) {
            // ! Populate the packet with data from the cache block.
            assert(blks[set][hitWay].data != nullptr);
            pkt->setData(blks[set][hitWay].data);
            pkt->makeTimingResponse();
            toCPU = pkt;
            // Schedule an event to send the response back to the CPU.
            schedule(cpuSendEvent, curTick() + latency);
        } else {
            // No response is needed; just unblock after appropriate latency.
            schedule(unblockEvent, curTick() + latency);
        }
    } 
    // ! the packet's address is a miss (either tag mismatch or block invalid).
    else if ((!hit) && (pkt->isRead() || pkt->isWrite())) { 
        DPRINTF(MicroCache, "cache MISS, set: %d, way: %d\n", set, hitWay);
        // **Cache Miss Path** (FSM: Compare Tag → [Write-Back or Allocate])
        // Must handle both read and write requests on a miss.
        assert(pkt->isRead() || pkt->isWrite());
        stats.misses++; // Increment miss count

        // ! Issue a read request to memory for the new block.
        // Clear lower 6 bits to align with 64 byte block
        requestFromMem(pkt->getAddr() & ~(Addr)0x3F);

        pending = pkt; // Save the original CPU request for later completion in handleResponse.

        
    } else {
        // DPRINTF(MicroCache, "Non-read or non-write packet \n");
        // Non-read, non-write packets are ignored; unblock the cache.
        blocked = false;
    }
    // DPRINTF(MicroCache, "END REQ block - valid: %d; dirty: %d; tag: 0x%x\n", currBlock.valid, currBlock.dirty, currBlock.tag);
    // DPRINTF(MicroCache, "\t\t*** REQUEST end (normal) ***\n\n");
    return true;
}

/*
 * handleResponse: Called when a response is received from memory.
 * This function implements the "Write-Back" and "Allocate" states of our FSM.
 */
void
MicroCache::handleResponse(PacketPtr pkt)
{
    // 'pending' holds the original CPU request that triggered this memory operation.
    // We must still satisfy or finalize that CPU request (read/write) now that memory responded.
    assert(pending != nullptr);
    assert(pkt != nullptr);

    // DPRINTF(MicroCache, "\t\t*** RESPONSE called ***\n");
    DPRINTF(MicroCache, "handleResponse: pkt address: 0x%x; read: %d; write: %d\n", pkt->getAddr(), pkt->isRead(), pkt->isWrite());

    // The CPU address that caused this memory request (or writeback).
    Addr pendingAddr = pending->getAddr();

    // DPRINTF(MicroCache, "pending pkt tag: 0x%x\n", (pendingAddr >> 6));
    // DPRINTF(MicroCache, "writingback: %d\n", writingback);
    // DPRINTF(MicroCache, "BEGIN RES block - valid: %d; dirty: %d; tag: 0x%x\n", blks[0].valid, blks[0].dirty, blks[0].tag);

    // ! TODO: FIND CHOSEN WAY, replace blks[0]s
    Addr fromMemTag = pkt->getAddr() >> 6;
    // DPRINTF(MicroCache, "pendingAddr: %lx; pktAddr: %lx\n", pendingAddr, pkt->getAddr());
    
    int set = fromMemTag % numSets;
    // We need a way of persisting the state of this chosen way from the call to this function handling the original memory response
    // to the call handling the writeback response (see diagram in hw assignment). chosenWay is a class variable
    if (!writingback) {
        // Just default to the first block in this set (we can evict if necessary)
        for (int way = 0; way < numWays; way++) {
            if (!blks[set][way].valid) {
                chosenWay = way;
                break;
            } else if (blks[set][way].last_access_time < blks[set][chosenWay].last_access_time) {
                chosenWay = way;
            }
        }
    }
    
    
    // Check if this response is acknowledging a previously issued writeback.
    if (writingback) {
        // DPRINTF(MicroCache, "\t\t 1\n");
        // If 'writingback' is true, we expect this to be a write response
        // indicating that the writeback of a dirty block has completed.
        writingback = false;
        assert(pkt->isWrite() && pkt->isResponse());
    } else {
        // DPRINTF(MicroCache, "\t\t 2\n");
        // Otherwise, this is a response containing data from memory (i.e., a read response).
        // Typically, we fetch new data to fill our cache block after a miss.

        // ! Decide if you need to writeback data
        if (blks[set][chosenWay].valid && blks[set][chosenWay].dirty) { 
            // ! Need to reconstruct an address from the tag!
            Addr writebackAddr = (blks[set][chosenWay].tag << 6);
            DPRINTF(MicroCache, "Writing back data for set: %d, way: %d; writebackAddr: %lx\n", set, chosenWay, writebackAddr);
            assert(blks[set][chosenWay].data != nullptr);
            writebackData(true, writebackAddr, blks[set][chosenWay].data);
            // Basically block sending data to CPU until we receive response from memory indicating writeback is complete
            // Then we will go into first if-statement and consequently execute bottom if-statement (send data to CPU)
            writingback = true; 
        }

        // ! Fill the block with the data from 'pkt'.

        pkt->writeData(blks[set][chosenWay].data);
        // Need to write data from original CPU request write
        if (pending->isWrite()) {
            pending->writeData(blks[set][chosenWay].data);
            blks[set][chosenWay].dirty = true;
            DPRINTF(MicroCache, "Filling data into set: %d, way: %d for write req\n", set, chosenWay);
        } else {
            blks[set][chosenWay].dirty = false;
            DPRINTF(MicroCache, "Filling data into set: %d, way: %d for read req\n", set, chosenWay);
        }
        assert(pendingAddr >> 6 == fromMemTag);
        blks[set][chosenWay].valid = true;
        blks[set][chosenWay].tag = fromMemTag;
        // ! TODO: Set last_access_time
        blks[set][chosenWay].last_access_time = curTick();
    }

    // Once we finish any writeback logic or block filling,
    // we can respond to the CPU if needed.
    if (!writingback) {
        // DPRINTF(MicroCache, "\t\t 4\n");
        // If we're not still waiting on a writeback, we can finalize the CPU request.
        assert(blocked);

        if (pending->needsResponse()) {
            // DPRINTF(MicroCache, "\t\t 5\n");
            // If the CPU is waiting for data (e.g., read),
            // convert the pending request into a timing response.
            assert(blks[set][chosenWay].data != nullptr);
            // ! Set the data of the pending request packet from the cpu with the data of the cache block
            pending->setData(blks[set][chosenWay].data);
            pending->makeTimingResponse();
            toCPU = pending;
            pending = nullptr;

            // Schedule an event to send the response up to the CPU side.
            schedule(cpuSendEvent, curTick() + latency);
        } else {
            // DPRINTF(MicroCache, "\t\t 6\n");
            // If no response is needed (e.g., a write that doesn't require data back),
            // just unblock the cache after applying latency.
            pending = nullptr;
            schedule(unblockEvent, curTick() + latency);
        }
    }
    // DPRINTF(MicroCache, "END RES block - valid: %d; dirty: %d; tag: 0x%x\n", blks[0].valid, blks[0].dirty, blks[0].tag);

    // 'pkt' is the dynamically allocated packet from memory. We must delete it to avoid leaks.
    delete pkt; // scaff

    // DPRINTF(MicroCache, "\t\t*** RESPONSE end ***\n\n");
}


MicroCache::MicroCacheStats::MicroCacheStats(statistics::Group *parent)
  : statistics::Group(parent),
    ADD_STAT(hits, statistics::units::Count::get(), "Number of hits"),
    ADD_STAT(misses, statistics::units::Count::get(), "Number of misses"),
    ADD_STAT(hitRate, statistics::units::Ratio::get(), "Number of hits/ (hits + misses)", hits / (hits + misses))
{
}

gem5::MicroCache*
MicroCacheParams::create() const
{
    return new gem5::MicroCache(this);
}

} // namespace gem5




