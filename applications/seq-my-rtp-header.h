
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

#ifndef SEQ_MY_RTP_HEADER_H
#define SEQ_MY_RTP_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
/**
 * \ingroup applications
 *
 * \brief My RTP Packet header to carry sequence number, timestamp, first sequence number of a gop,
 *  Num of I packets, Frame_type, correctionAbility
 *
 * The header is used as a payload in applications (typically UDP) to convey
 * a 32 bit sequence number followed by a 64 bit timestamp 
 * and a 32 bit first sequence number of a gop and 
 * a 32 bit number of I packets and a 16 bit frame type, and 32 bit frame seq 
 * and a 32 bit correction ability (30 bytes total).
 * 
 * The timestamp is not set explicitly but automatically set to the
 * simulation time upon creation. 
 *
 */
class SeqMyRTPHeader : public Header
{
public:

  // enum for video frame types 
  enum FRAME_TYPE
  {
    I_FRAME = 1,
    FEC_FRAME = 2,
    B_FRAME = 3,
    P_FRAME = 4
  };

  SeqMyRTPHeader ();
  
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

  /******************** First Seq Num **********************/
  /**
   * \param fisrt_seq the first sequence number of a gop
   */
  void SetFirstSeq (uint32_t fisrt_seq);
  /**
   * \return the first sequence number of a gop
   */
  uint32_t GetFirstSeq (void) const;

  /******************** Num of I packets **********************/
  /**
   * \param num_Ipkts the number of I packets in a gop
   */
  void SetNumIpkts(uint32_t num_Ipkts);
  /**
   * \return the number of I packets in a gop
   */
  uint32_t GetNumIpkts (void) const;

  /******************** Frame Type **********************/
  /**
   * \param frame_type the type of the video frame
   */
  void SetFrameType (uint16_t frame_type);
  /**
   * \return the type of the video frame
   */
  FRAME_TYPE GetFrameType (void) const;

  /******************** Frame Seq **********************/
  /**
   * \param frame_seq the frame seq of the video frame
   */
  void SetFrameSeq (uint32_t frame_seq);
  /**
   * \return the type of the video frame
   */
  uint32_t GetFrameSeq (void) const;

  /******************** Correction ability field **********************/
  /**
   * \param correctionAbility the maximum number of packet loss that we can recover them for a gop
   */
  void SetCorrectionAbility (uint32_t correction_ability);
  /**
   * \return the maximum number of packet loss that we can recover them for a gop
   */
  uint32_t GetCorrectionAbility (void) const;


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
  uint32_t m_seq;               //!< Sequence number
  uint64_t m_ts;                //!< Timestamp
  uint32_t m_firstSeq;          //!< First sequence number of gop
  uint32_t m_numIPkt;           //!< The number of I packets in a gop
  uint16_t m_frameType;         //!< Type of the video frame in Packet
  uint32_t m_frameSeq;          //!< sequence number of frames in input video file.  we want to know that each packet belongs which frame.
  uint32_t m_correctionAbility; //!< m_correctionAbility = FEC_LEVEL  based on Reed-solomon code in a gop      
                                //!!!!! In RS code, when the locations of the errors are already known (when it is being used as an erasure code), up to N-K errors can be corrected.
};

} // namespace ns3

#endif /* SEQ_MY_RTP_HEADER_H */
