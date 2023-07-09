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

#ifndef __EVALVID_FEC_CLIENT_H__
#define __EVALVID_FEC_CLIENT_H__

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/seq-ts-header.h"

#include "ns3/seq-my-rtp-header.h"
#include "ns3/seq-my-rtcp-header.h"
#include "ns3/seq-ts-echo-header.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <vector>

using std::ifstream;
using std::ofstream;
using std::ostream;

using std::endl;
using std::ios;

using namespace std;

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup evalvid
 * \class EvalvidFecClient
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class EvalvidFecClient : public Application
{
public:
  static TypeId GetTypeId (void);

  EvalvidFecClient ();

  virtual ~EvalvidFecClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Ipv4Address ip, uint16_t port);

protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void Send (void);
  void HandleRead (Ptr<Socket> socket);
  
  void DoCorrectionGop (void);
  void CorrectionGop (void);
  void SendRTCP(void);


  ofstream receiverDumpFile;
  string receiverDumpFileName;
  string m_correctedIframeFileName;
  string m_recvFECPktsFileName;
  string m_recvIPktsFileName;
  ofstream correctionFile;
  ofstream m_recvFecFile;
  ofstream m_recvIpktFile;

  Ptr<Socket> m_socket;
  Ipv4Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;

  uint32_t m_firstSeqGop;   // first seq of current gop
  uint32_t m_numIPktsGop;   // number of expected I packets for current gop
  uint32_t m_recvIPktGop;   // number of received I packets of current gop
  uint32_t m_recvFecPktGop;   // number of received FEC packets of current gop
  uint32_t m_recvBPPktGop;    // number of received B and P packets of current gop
  uint32_t m_lostPktGop;      // loss rate for current gop ( a percentage)
  uint32_t m_correctionAbilityGop;  // the max correction ability or the max expected loss packets or (expected number of fec packets)
  uint32_t m_packetId;    // sequence number for packets which are sent from client to server
  double m_applicationThreshold;  // application threshold = 1.5 second     there is a same variable in sender side and both of them should be same number.
  // uint32_t m_gopNumber;       // the number of current GOP

    
  struct m_gopPacketInfo
  {
    double m_packetTime;
    uint32_t m_packetSeq;
    uint32_t m_packetSize;
    double m_packetSentTime;
    uint32_t m_frameId;
  };

  vector <m_gopPacketInfo> m_I_PktsCurrGOP;
  vector <m_gopPacketInfo> m_FEC_PktsCurrGOP;

};

} // namespace ns3

#endif // __EVALVID_FEC_CLIENT_H__
