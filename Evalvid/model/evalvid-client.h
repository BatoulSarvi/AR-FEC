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

#ifndef __EVALVID_CLIENT_H__
#define __EVALVID_CLIENT_H__

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/seq-ts-header.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <fstream>
#include <iomanip>

using std::ifstream;
using std::ofstream;
using std::ostream;

using std::ios;
using std::endl;

using namespace std;

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup evalvid
 * \class EvalvidClient
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class EvalvidClient : public Application
{
public:

  static TypeId GetTypeId (void);

  EvalvidClient ();

  virtual ~EvalvidClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Ipv4Address ip, uint16_t port);

    /**
   * \brief Returns the number of lost packets
   * \return the number of lost packets
   */
  uint32_t GetLost (void) const;

  /**
   * \brief Returns the number of received packets
   * \return the number of received packets
   */
  uint64_t GetReceived (void) const;

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void Send (void);
  void HandleRead (Ptr<Socket> socket);

  ofstream    receiverDumpFile;
  string      receiverDumpFileName;

  Ptr<Socket> m_socket;
  Ipv4Address m_peerAddress;
  uint16_t    m_peerPort;
  EventId     m_sendEvent;

  uint64_t    m_received;     // the number of received packets
  uint64_t    m_preId;    // the seq number of previous received packet
  uint64_t    m_lossCounter; //!< Lost packet counter
  
  uint32_t    m_recvIframe;   // counter for sent I frame
  uint32_t    m_recvPBframe;   // counter for sent P and B frame
  uint32_t    m_recvIpacket;  // counter for I packets 

  double m_applicationThreshold;  // application threshold = 1.5 second    

};

} // namespace ns3

#endif // __EVALVID_CLIENT_H__
