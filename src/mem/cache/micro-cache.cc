// /*
//  * Author: Caden Chou
//  * Copyright (c) 2023 Brown University
//  * All rights reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions are
//  * met: redistributions of source code must retain the above copyright
//  * notice, this list of conditions and the following disclaimer;
//  * redistributions in binary form must reproduce the above copyright
//  * notice, this list of conditions and the following disclaimer in the
//  * documentation and/or other materials provided with the distribution;
//  * neither the name of the copyright holders nor the names of its
//  * contributors may be used to endorse or promote products derived from
//  * this software without specific prior written permission.
//  *
//  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  */

//  #include "mem/cache/micro-cache.hh"

//  #include "params/MicroCache.hh"
 
//  namespace gem5 {
 
//  MicroCache::MicroCache(const MicroCacheParams *p) :
//      SimObject(*p),
//      cpu_side_port(p->name + ".cpu_side_port", this),
//      mem_side_port(p->name + ".mem_side_port", this),
//      memSendEvent([this] { sendToMem(); }, name()),
//      cpuSendEvent([this] { sendToCpu(); }, name()),
//      writebackEvent([this] { writebackToMem(); }, name()),
//      unblockEvent([this] { unblock(); }, name()),
//      toMem(nullptr),
//      toCPU(nullptr),
//      toWriteback(nullptr),
//      latency(p->latency * 1000),
//      assoc(p->assoc),
//      blocked(false),
//      writingback(false),
//      pending(nullptr),
//      stats(this)
//      // TODO: YOUR ADDITIONAL FIELDS HERE!
//  {
//      assert(p->size >= 64);

//      // allocate one block;
//      blks = (Block *) malloc(sizeof(Block));
//      blks[0].valid = false;
 
//  };
 
//  /* Method required for Cache object to communicate */
//  Port&
//  MicroCache::getPort(const std::string &if_name, PortID idx)
//  {
//      if (if_name == "cpu_side") {
//          return cpu_side_port;
//      } else if (if_name == "mem_side") {
//          return mem_side_port;
//      }
 
//      return SimObject::getPort(if_name, idx);
//  }
 
//  /* Request from CPU side */
//  bool
//  MicroCache::handleRequest(PacketPtr pkt)
//  { 
//      if (blocked) {
//          return false;
//      }
//      blocked = true;
 
 
//      if (false) { // TODO: check if address is in cache
//          // hit case!
//          stats.hits++; // scaff

//          // todo: check information about pkt
 
//          if (pkt->needsResponse()) {
//             // todo: populate pkt with appropriate data
//              pkt->makeTimingResponse();
//              toCPU = pkt;
 
//              // schedule response event to be sent
//              schedule(cpuSendEvent, curTick() + latency);
//          } else {
//              schedule(unblockEvent, curTick() + latency);
//          }
//      } else if (false) { // TODO: check if address is not in cache
//          assert(pkt->isRead() || pkt->isWrite());
//          // miss case!
//          stats.misses++;
 
//          // TODO: send a request to memory using requestFromMem
 
//          pending = pkt; // make sure we track the request we sent
//      } else {
//          // Ignore non-read, non-write packets
//          blocked = false;
//      }
 
//      return true;
//  }
 
 
//  void
//  MicroCache::handleResponse(PacketPtr pkt)
//  {
//     assert(pending != nullptr);
 
//     Addr pendingAddr = pending->getAddr();
 
//      if (writingback) {
//         // this is an acknowledgement that writeback is complete!
//          writingback = false;
//          assert(pkt->isWrite() && pkt->isResponse());
//      } else {
//         // TODO: where does the data go?

//          if (false) { // TODO: do we need to evict a block?
//             // TODO: call writebackData

//              writingback = true;
//          }
 
//          // TODO: fill block using data in pkt

//      }
 
//      // respond to CPU if necessary and unblock
//      if (!writingback) {
//          assert(blocked);
//          if (pending->needsResponse()) {
//             // make sure pending data is set here or above!
//              pending->makeTimingResponse();
//              toCPU = pending;
//              pending = nullptr;
 
//              // schedule response event to be sent
//              schedule(cpuSendEvent, curTick() + latency);
//          } else {
//              pending = nullptr;
//              schedule(unblockEvent, curTick() + latency);
//          }
//      }
 
//      // this packet was dynamically created by us
//      delete pkt; // scaff
//  }
 
//  MicroCache::MicroCacheStats::MicroCacheStats(statistics::Group *parent)
//    : statistics::Group(parent),
//      ADD_STAT(hits, statistics::units::Count::get(), "Number of hits"),
//      ADD_STAT(misses, statistics::units::Count::get(), "Number of misses"),
//      ADD_STAT(hitRate, statistics::units::Ratio::get(), "Number of hits/ (hits + misses)", hits / (hits + misses))
 
//  {
//  }
 
//  gem5::MicroCache*
//  MicroCacheParams::create() const
//  {
//      return new gem5::MicroCache(this);
//  }
 
//  }
 







 /*
 * Author: Caden Chou
 * Copyright (c) 2023 Brown University
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
    // For a one-block cache, allocate one block; later, for multi-block caches,
    // you'll need to allocate an array of blocks based on cache size and associativity.
    blks = (Block *) malloc(sizeof(Block));
    blks[0].valid = false;
}

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
    DPRINTF(MicroCache, "*** handleRequest called ***\n");
    // If the cache is currently blocked (waiting on a memory response), 
    // then signal that the CPU must retry later.
    if (blocked) {
        DPRINTF(MicroCache, "*** handleRequest end (blocked) ***\n");
        return false;
    }
    blocked = true;

    

    // ! Compute the block index and tag from the packet's address.
    
    // Block size is 64, so block offset should be 6
    // The convention that I am using is to store only the tag bits in blks[0].tag
    Addr pktTag = pkt->getAddr() >> 6;

    DPRINTF(MicroCache, "pkt cpu request packet tag: %ld\n", pktTag);
    DPRINTF(MicroCache, "BEGIN REQ block - valid: %d; dirty: %d; tag: %x\n", blks[0].valid, blks[0].dirty, blks[0].tag);

    // For a one-block cache, this may simply be a comparison with blks[0].tag.
    // For multi-block caches (Part 3), use index and tag extraction based on block size, cache size, and associativity.
    // ! Check that packet's address matches the block tag and block is valid.
    if ((pktTag == blks[0].tag) && blks[0].valid) { 
        DPRINTF(MicroCache, "cache HIT\n");
        // **Cache Hit Path** (FSM: Compare Tag → Idle)
        stats.hits++; // Increment hit count
        
        // TODO: Update block's LRU timestamp if using LRU replacement (Part 3).
        DPRINTF(MicroCache, "pkt is write: %d\n", pkt->isWrite());
        // ! For write requests, mark the block dirty and write into block
        if (pkt->isWrite()) {
            blks[0].dirty = true;
            blks[0].valid = true;
            // TODO: PRINT PKT INFO LIKE IS IT MASKED OR NOT
            pkt->writeData(blks[0].data);
        }
        DPRINTF(MicroCache, "pkt needs response: %d\n", pkt->needsResponse());
        if (pkt->needsResponse()) {
            // ! Populate the packet with data from the cache block.
            pkt->setData(blks[0].data);
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
    else if ((pktTag != blks[0].tag) || !blks[0].valid) { 
        DPRINTF(MicroCache, "cache MISS\n");
        // **Cache Miss Path** (FSM: Compare Tag → [Write-Back or Allocate])
        // Must handle both read and write requests on a miss.
        assert(pkt->isRead() || pkt->isWrite());
        stats.misses++; // Increment miss count

        // ! Issue a read request to memory for the new block.
        // Clear lower 6 bits to align with 64 byte block
        requestFromMem(pkt->getAddr() & ~(Addr)0x3F);

        pending = pkt; // Save the original CPU request for later completion in handleResponse.

        
    } else {
        DPRINTF(MicroCache, "Non-read or non-write packet \n");
        // Non-read, non-write packets are ignored; unblock the cache.
        blocked = false;
    }
    DPRINTF(MicroCache, "END REQ block - valid: %d; dirty: %d; tag: %x\n", blks[0].valid, blks[0].dirty, blks[0].tag);
    DPRINTF(MicroCache, "*** handleRequest end (normal) ***\n");
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

    DPRINTF(MicroCache, "*** handleResponse called ***\n");


    // The CPU address that caused this memory request (or writeback).
    Addr pendingAddr = pending->getAddr();

    DPRINTF(MicroCache, "pending cpu request packet tag: %ld\n", pendingAddr >> 6);
    DPRINTF(MicroCache, "writingback: %d\n", writingback);
    DPRINTF(MicroCache, "BEGIN RES block - valid: %d; dirty: %d; tag: %x\n", blks[0].valid, blks[0].dirty, blks[0].tag);

    // Check if this response is acknowledging a previously issued writeback.
    if (writingback) {
        DPRINTF(MicroCache, "\t\t 1\n");
        // If 'writingback' is true, we expect this to be a write response
        // indicating that the writeback of a dirty block has completed.
        writingback = false;
        assert(pkt->isWrite() && pkt->isResponse());
        // TODO (Optional): If needed, now we can proceed to request a new block
        // if we haven't already (in a multi-block scenario). For a single-block cache,
        // you might already have requested the new block before the writeback was done.
    } else {
        DPRINTF(MicroCache, "\t\t 2\n");
        // Otherwise, this is a response containing data from memory (i.e., a read response).
        // Typically, we fetch new data to fill our cache block after a miss.

        // ! For multi-block caches, if you haven't already evicted a block, check here.
        // ! Decide if you need to evict a block. In a single-block cache, you likely did it earlier.
        if (blks[0].dirty) { 
            DPRINTF(MicroCache, "\t\t 3\n");
            // ! dirty implies valid (?)
            // assert(blks[0].valid);
            writebackData(true, blks[0].tag, blks[0].data);
            // Basically block sending data to CPU until we receive response from memory indicating writeback is complete
            // Then we will go into first if-statement and consequently execute bottom if-statement (send data to CPU)
            writingback = true; 
        }

        // ! Fill the single-block cache with the data from 'pkt'.
        pkt->writeData(blks[0].data);
        blks[0].valid = true;
        blks[0].dirty = false;
        blks[0].tag = (pendingAddr >> 6);
    }

    // Once we finish any writeback logic or block filling,
    // we can respond to the CPU if needed.
    if (!writingback) {
        DPRINTF(MicroCache, "\t\t 4\n");
        // If we're not still waiting on a writeback, we can finalize the CPU request.
        assert(blocked);

        if (pending->needsResponse()) {
            DPRINTF(MicroCache, "\t\t 5\n");
            // If the CPU is waiting for data (e.g., read),
            // convert the pending request into a timing response.
            // ! Set the data of the pending request packet from the cpu with the data of the cache block
            // TODO: Hopefully we send correct data to cpu (not whole block)
            pending->setData(blks[0].data);
            pending->makeTimingResponse();
            toCPU = pending;
            pending = nullptr;

            // Schedule an event to send the response up to the CPU side.
            schedule(cpuSendEvent, curTick() + latency);
        } else {
            DPRINTF(MicroCache, "\t\t 6\n");
            // If no response is needed (e.g., a write that doesn't require data back),
            // just unblock the cache after applying latency.
            pending = nullptr;
            schedule(unblockEvent, curTick() + latency);
        }
    }
    DPRINTF(MicroCache, "END RES block - valid: %d; dirty: %d; tag: %x\n", blks[0].valid, blks[0].dirty, blks[0].tag);

    // 'pkt' is the dynamically allocated packet from memory. We must delete it to avoid leaks.
    delete pkt; // scaff

    DPRINTF(MicroCache, "*** handleResponse end ***\n");
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
