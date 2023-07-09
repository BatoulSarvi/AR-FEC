/* 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation
 *
 * Author: Batoul Sarvi <batoul.sarvi@gmail.com>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "seq-my-rtcp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SeqMyRTCPHeader");

NS_OBJECT_ENSURE_REGISTERED (SeqMyRTCPHeader);

SeqMyRTCPHeader::SeqMyRTCPHeader ()
  : m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ()),
    m_lossPackets (0),
    m_firstSeq (0)
{
  NS_LOG_FUNCTION (this);
}

/******************** Seq Num **********************/
void
SeqMyRTCPHeader::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}
uint32_t
SeqMyRTCPHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

/******************** Time Stamp **********************/
Time
SeqMyRTCPHeader::GetTs (void) const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

/******************** Loss Rate **********************/
void 
SeqMyRTCPHeader::SetLostPackets (uint32_t numPacketLoss)
{
  NS_LOG_FUNCTION (this << numPacketLoss);
  m_lossPackets = numPacketLoss;
}
uint32_t
SeqMyRTCPHeader::GetLostPackets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lossPackets;
}

/******************** First Seq **********************/
void 
SeqMyRTCPHeader::SetFirstSeq (uint64_t firstSeqGop)
{
  NS_LOG_FUNCTION (this << firstSeqGop);
  m_firstSeq = firstSeqGop;
}

uint64_t
SeqMyRTCPHeader::GetFirstSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_firstSeq;
}

/******************** General Func for New Header **********************/
TypeId
SeqMyRTCPHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SeqMyRTCPHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<SeqMyRTCPHeader> ()
  ;
  return tid;
}
TypeId
SeqMyRTCPHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
SeqMyRTCPHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(seq=" << m_seq << " time=" << TimeStep (m_ts).As (Time::S) 
     << "lossPacket = " << m_lossPackets << "FisrtSeqGop = " << m_firstSeq << ")";
}
uint32_t
SeqMyRTCPHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 4+8+4+8;
}

void
SeqMyRTCPHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU64 (m_ts);
  i.WriteHtonU32 (m_lossPackets);
  i.WriteHtonU64 (m_firstSeq);
}

uint32_t
SeqMyRTCPHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  m_lossPackets = i.ReadNtohU32 ();
  m_firstSeq = i.ReadNtohU64 ();
  return GetSerializedSize ();
}

} // namespace ns3
