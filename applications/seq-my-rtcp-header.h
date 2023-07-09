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

#ifndef SEQ_MY_RTCP_HEADER_H
#define SEQ_MY_RTCP_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
/**
 * \ingroup applications
 *
 * \brief Packet header to carry sequence number and timestamp
 *
 * The header is used as a payload in applications (typically UDP) to convey
 * a 32 bit sequence number followed by a 64 bit timestamp 
 * and a 32 bit number of lost packets and 64 bits first seq of gop (24 bytes total).
 *
 * The timestamp is not set explicitly but automatically set to the
 * simulation time upon creation.
 *
 */
class SeqMyRTCPHeader : public Header
{
public:
  SeqMyRTCPHeader ();

  /******************** Seq Num **********************/
  /**
   * \param seq the sequence number
   */
  void SetSeq (uint32_t seq);
  /**
   * \return the sequence number
   */
  uint32_t GetSeq (void) const;

  /******************** Time Stamp **********************/
  /**
   * \return the time stamp
   */
  Time GetTs (void) const;

  /******************** Loss Rate **********************/
  /**
   * \param numPacketLoss the loss rate for a gop    ( a percentage)
   */
  void SetLostPackets (uint32_t numPacketLoss);
  /**
   * \return the loss rate for a gop   ( a percentage)
   */
  uint32_t GetLostPackets (void) const;

  /******************** First Seq **********************/
  /**
   * \param firstSeqGop the first sequence for a gop   
   */
  void SetFirstSeq(uint64_t firstSeqGop);
  /**
   * \return the first sequence for a gop
   */
  uint64_t GetFirstSeq (void) const;

  /******************** General Func for New Header **********************/
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_seq; //!< Sequence number
  uint64_t m_ts; //!< Timestamp
  uint32_t m_lossPackets; //!< the loss rate for the previous gop based on percentage
  uint64_t m_firstSeq;   //!< First sequence number of gop     
};

} // namespace ns3

#endif /* SEQ_MY_RTCP_HEADER_H */
