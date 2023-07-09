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
 *
 */

#ifndef __EVALVID_FEC_SERVER_H__
#define __EVALVID_FEC_SERVER_H__

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/seq-my-rtp-header.h"
#include "ns3/seq-my-rtcp-header.h"
#include "ns3/seq-ts-echo-header.h"
#include "ns3/socket.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

using std::ifstream;
using std::ofstream;
using std::ostream;

using std::endl;
using std::ios;

using namespace std;

namespace ns3 {

/**
 * \ingroup applications
 * \defgroup Evalvid Evalvid
 */

/**
 * \ingroup Evalvid
 * \class EvalvidFecServer
 * \brief A Udp server. Receives UDP packets from a remote host. UDP packets
 * carry a 32bits sequence number followed by a 64bits time stamp in their
 * payloads. The application uses, the sequence number to determine if a packet
 * is lost, and the time stamp to compute the delay
 */
  class EvalvidFecServer : public Application
  {
    public:
      static TypeId GetTypeId (void);
      EvalvidFecServer ();
      virtual ~EvalvidFecServer ();

    protected:
      virtual void DoDispose (void);

    private:
      virtual void StartApplication (void);
      virtual void StopApplication (void);

      void Setup (void);
      void HandleRead (Ptr<Socket> socket);
      void Send ();

      string m_videoTraceFileName; //File from mp4trace tool of Evalvid.
      string m_lowVideoTraceFileName; //File from mp4trace tool of Evalvid.
      string m_senderTraceFileName; //File with information of packets transmitted by EvalvidFecServer.
      string m_gopInfoMapFileName; 
      string m_sentFecTraceFileName;
      
      fstream m_videoTraceFile;
      // string      m_RTTLogFileName;           // RTT_log.txt
      // ofstream m_RTTLogFile;
      ofstream m_sentFecTraceFile; // saving sent FEC packets
      ofstream m_lostLogFile; // saving loss rate from RTCP packets
      ofstream m_gopInfoMapFile; //saving Gop Info Map File
      
      uint16_t m_packetPayload;
      ofstream m_senderTraceFile;
      uint32_t m_packetId;
      uint16_t m_port;
      Ptr<Socket> m_socket;
      Address m_peerAddress;
      EventId m_sendEvent;

      struct m_videoInfoStruct_t
      {
        string frameType;
        uint32_t frameSize;
        uint32_t numOfUdpPackets;
        Time packetInterval;
      };

      map<uint32_t, m_videoInfoStruct_t *> m_videoInfoMap;
      map<uint32_t, m_videoInfoStruct_t *>::iterator m_videoInfoMapIt;

      map<uint32_t, m_videoInfoStruct_t *> m_lowVideoInfoMap;

      uint32_t m_predictedFecPkts;  // the number of Predicted FEC packet for a gop
      uint32_t m_firstSeqGop;       // first seq number of a gop
      uint32_t m_numIPackets;       // number of I packets in a gop

      uint32_t m_predictedPLR;      //predicted PLR for the current GOP and initialize value is 0
      string   m_videoQualityStatus;   //  High , Low   ,  we always start to send low quality video
      double   m_applicationThreshold; // acceptable application threshold = 1.5 second     there is a same variable in receiver side and both of them should be same number.
      uint32_t m_latestRecvPLR;     // latest received actual PLR
      uint32_t m_totalSentIFrame;   // total number of I-frame that we sent
      double   m_latestRecvTT;      // latest received TT 

      uint16_t m_staticFEC;         // static FEC
      bool     m_usingRTT;          // if we want to consider the TT value in prediction process, we set it as true.
      bool     m_usingBandwidth;    // if we want to use bandwidth comparison for low and high video quality, we should set this variable as a true. default = true
      string   m_staticQuality;     // if this variable is "No", we will not run modules that change the video quality (e.g. switchup(), adjustingPLR()). 
                                    // If it is "High", we send all in high.    If it is "Low", we send all in "Low".
      uint32_t   m_locationEcho;    // it should be a number >=1 and <=30  ---- if it is 30, it means that we send Echo packet and the end of the GOP, it it is 1, send Echo after I frame

      uint32_t *m_lowVideoGopID;    // contains the seq ID of I frames from LOW Quality input file for having the start seq number of frame that it is first seq of each GOP 
      uint32_t *m_highVideoGopID;   // contains the seq ID of I frames from HIGH Quality input file for having the start seq number of frame that it is first seq of each GOP 

      uint32_t MAX_QUEUE_LEN;       // the current capacity of queue from queue.h file is 10240.
      uint32_t m_lastInputQue;      // the last seq that we put into the queue for sending
      uint32_t m_lastOutputQue;     // the last seq that we receive an ack for this  

      bool m_finishingAdjustedPrePLR;       // when it is true, it means that we ran adjusted module on predicted PLR for the current GOP. So we need to set it as a false when we want to send a new GOP.

      struct m_gopInfoStruct_t
      {
        uint32_t firstSeq;
        uint32_t PLR; // actual PLR
        Time sentTimeEcho; // sent time for Echo packet
        uint32_t echoSeq; // seq for Echo packet ( we can detect the last packet for GOP)
        double TT; // transmission time    (Receivetime - sentTime)/2  from echoReply packet
        uint32_t fecLevel; // predicted PLR  %
        uint32_t numIPackets; //the number of I packets in a GOP
        uint32_t numFECPkts; // the number of FEC packets in a GOP
        string quality;       //  High , Low 
        double bandwidthRTCP;     // bandwith based on the RTCP packet if it is sent in low quality, otherwise = 0
        double bandwidthEchoReply;     // bandwith based on the Echo reply packet if it is sent in low quality, otherwise = 0
        double safetyFactor ;        // When we have a fresh TT, we calculate Safety factor for this previous GOP 
        Time firstSeqTime;     // start Time of GOP ( send time of first packet of the current GOP) 
      };

      map<uint32_t, m_gopInfoStruct_t *> m_gopInfoMap;
      map<uint32_t, m_gopInfoStruct_t *>::iterator m_gopInfoMapIt;

      void CalculatingFECLevel ();
      void DetermineFECLevel ();
      void ReplaceWithLowQuality ();
      void SendVideoPackets ();
      void SendFECPackets ();
      bool IsNormalData (double, uint32_t);
      uint32_t CalculateRTTLostPackets (uint32_t lastGopNo);
      void SwitchUpQuality ();
      void ReactionToCongestion ();
      void NormalizePLR ();
      void AdjustingPredictedPLR ();
      void SendEchoPacket();

      uint32_t m_counter_B_P ;
      bool m_flagEcho;

      double m_aat_factor ; 
      double m_aat_constant ; 
      uint32_t m_changing_value_PLR ; 
  };

} // namespace ns3

#endif // __EVALVID_FEC_SERVER_H__
