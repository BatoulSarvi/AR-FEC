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
#include "seq-my-rtp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SeqMyRTPHeader");

NS_OBJECT_ENSURE_REGISTERED (SeqMyRTPHeader);

SeqMyRTPHeader::SeqMyRTPHeader ()
  : m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ()),
    m_firstSeq (0),
    m_numIPkt (0),
    m_frameType (I_FRAME),
    m_frameSeq (0),
    m_correctionAbility(0)
{
  NS_LOG_FUNCTION (this);
}

/******************** Seq Num **********************/

void
SeqMyRTPHeader::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}
uint32_t
SeqMyRTPHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

/******************** Time Stamp **********************/
Time
SeqMyRTPHeader::GetTs (void) const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

/******************** First Seq Num **********************/
void 
SeqMyRTPHeader::SetFirstSeq (uint32_t first_seq)
{
  NS_LOG_FUNCTION (this << first_seq);
  m_firstSeq = first_seq;
}
uint32_t
SeqMyRTPHeader::GetFirstSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_firstSeq;
}

/******************** Num of I packets **********************/
void 
SeqMyRTPHeader::SetNumIpkts (uint32_t num_Ipkts)
{
  NS_LOG_FUNCTION (this << num_Ipkts);
  m_numIPkt = num_Ipkts;
}
uint32_t
SeqMyRTPHeader::GetNumIpkts (void) const
{
  NS_LOG_FUNCTION (this);
  return m_numIPkt;
}

/******************** Frame Type **********************/
void
SeqMyRTPHeader::SetFrameType (uint16_t frame_type)
{
  NS_LOG_FUNCTION (this << frame_type);
  m_frameType = frame_type;
}
SeqMyRTPHeader::FRAME_TYPE
SeqMyRTPHeader::GetFrameType (void) const
{
  NS_LOG_FUNCTION (this);
  return FRAME_TYPE (m_frameType);
}

/******************** Frame Seq **********************/
void
SeqMyRTPHeader::SetFrameSeq (uint32_t frame_seq)
{
  NS_LOG_FUNCTION (this << frame_seq);
  m_frameSeq = frame_seq;
}
uint32_t
SeqMyRTPHeader::GetFrameSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_frameSeq;
}

/******************** Correction ability field **********************/
void 
SeqMyRTPHeader::SetCorrectionAbility (uint32_t correction_ability)
{
  NS_LOG_FUNCTION (this << correction_ability);
  m_correctionAbility = correction_ability;
}
uint32_t
SeqMyRTPHeader::GetCorrectionAbility (void) const
{
  NS_LOG_FUNCTION (this);
  return m_correctionAbility;
}

/******************** General Func for New Header **********************/
TypeId
SeqMyRTPHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SeqMyRTPHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<SeqMyRTPHeader> ()
  ;
  return tid;
}
TypeId
SeqMyRTPHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
SeqMyRTPHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(seq = " << m_seq << " time =" << TimeStep (m_ts).As (Time::S) <<
  " first seq = " << m_firstSeq << " number of I packets = " << m_numIPkt <<  
  " frame type = " << m_frameType << " frame seq = " << m_frameSeq << " max correction ability =" << m_correctionAbility << " )";
}
uint32_t
SeqMyRTPHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  //    m_seq   +  m_ts  +  m_firstSeq + m_numIPkt  +  m_frameType + m_frameSeq + m_correctionAbility
  //     4      +  8     +  4          + 4          +  2           +  4         + 4 
  return 4+8+4+4+2+4+4;         // 30 bytes for header
}

void
SeqMyRTPHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU64 (m_ts);
  i.WriteHtonU32 (m_firstSeq);
  i.WriteHtonU32 (m_numIPkt);
  i.WriteHtonU16 (m_frameType);
  i.WriteHtonU32 (m_frameSeq);
  i.WriteHtonU32 (m_correctionAbility);
}
uint32_t
SeqMyRTPHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  m_firstSeq = i.ReadNtohU32 ();
  m_numIPkt = i.ReadNtohU32 ();
  m_frameType = i.ReadNtohU16 ();
  m_frameSeq = i.ReadNtohU32 ();
  m_correctionAbility = i.ReadNtohU32();
  return GetSerializedSize ();
}

} // namespace ns3
