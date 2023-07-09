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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "evalvid-fec-client.h"

#include <stdlib.h>
#include <stdio.h>
#include "ns3/string.h"
#include "ns3/double.h"
// #include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalvidFecClient");
NS_OBJECT_ENSURE_REGISTERED (EvalvidFecClient);

TypeId
EvalvidFecClient::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::EvalvidFecClient")
          .SetParent<Application> ()
          .AddConstructor<EvalvidFecClient> ()
          .AddAttribute ("RemoteAddress", "The destination Ipv4Address of the outbound packets",
                         Ipv4AddressValue (),
                         MakeIpv4AddressAccessor (&EvalvidFecClient::m_peerAddress),
                         MakeIpv4AddressChecker ())
          .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                         UintegerValue (100), MakeUintegerAccessor (&EvalvidFecClient::m_peerPort),
                         MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("ReceiverDumpFilename", "Receiver Dump Filename", StringValue (""),
                         MakeStringAccessor (&EvalvidFecClient::receiverDumpFileName),
                         MakeStringChecker ())
          .AddAttribute ("CorrectedIframeFileName", "Corrected I Frames Filename", StringValue (""),      // m_correctedIframeFileName = "correctedIframe"
                         MakeStringAccessor (&EvalvidFecClient::m_correctedIframeFileName),
                         MakeStringChecker ())
          .AddAttribute ("RecvFECPktsFileName", "Received FEC Packets Filename", StringValue (""),      // m_recvFECPktsFileName = "recvFECPktsFile"
                         MakeStringAccessor (&EvalvidFecClient::m_recvFECPktsFileName),
                         MakeStringChecker ())
          .AddAttribute ("RecvIPktsFileName", "Received I Packets Filename", StringValue (""),      // m_recvIPktsFileName = "recvIPktsFile"   ---- before correction.
                         MakeStringAccessor (&EvalvidFecClient::m_recvIPktsFileName),
                         MakeStringChecker ())                                                  
          .AddAttribute ("AppThreshold", "Max acceptable delay for each GOP from App view", 
                          DoubleValue(1.5), MakeDoubleAccessor (&EvalvidFecClient::m_applicationThreshold),                
                          MakeDoubleChecker<double> ());
  return tid;
}

EvalvidFecClient::EvalvidFecClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sendEvent = EventId ();
  m_firstSeqGop = 0;
  m_numIPktsGop = 0;
  m_recvIPktGop = 0;
  m_recvFecPktGop = 0;
  m_recvBPPktGop = 0;
  m_lostPktGop = 0;
  m_correctionAbilityGop = 0;
  m_packetId = 0;
  m_applicationThreshold = 1.5;         // application threshold = 1.5 second     there is a same variable in sender side and both of them should be same number.
  // m_gopNumber = 0 ;
}

EvalvidFecClient::~EvalvidFecClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
EvalvidFecClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  m_peerAddress = ip;
  m_peerPort = port;
}

void
EvalvidFecClient::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
EvalvidFecClient::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      m_socket->Bind ();
      m_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));
    }

  receiverDumpFile.open (receiverDumpFileName.c_str (), ios::out);
  if (receiverDumpFile.fail ())
    {
      NS_FATAL_ERROR (
          ">> EvalvidFecClient: Error while opening output file: " << receiverDumpFileName.c_str ());
      return;
    }
  
  correctionFile.open (m_correctedIframeFileName.c_str(), ios::out);
  if (correctionFile.fail())
    {
      NS_FATAL_ERROR (
          ">> EvalvidFecClient: Error while opening output file: " << m_correctedIframeFileName.c_str() << std::endl);
      return;
    } 
  
  m_recvFecFile.open (m_recvFECPktsFileName.c_str (), ios::out);
  if (m_recvFecFile.fail())
    {
      NS_FATAL_ERROR (
          ">> EvalvidFecClient: Error while opening output file:" << m_recvFECPktsFileName.c_str() << std::endl);
      return;
    }

  m_recvIpktFile.open (m_recvIPktsFileName.c_str (), ios::out);
  if (m_recvIpktFile.fail())
    {
      NS_FATAL_ERROR (
          ">> EvalvidFecClient: Error while opening output file:" << m_recvIPktsFileName.c_str () << std::endl);
      return;
    }

  m_socket->SetRecvCallback (MakeCallback (&EvalvidFecClient::HandleRead, this));

  //Delay requesting to get server on line.
  m_sendEvent = Simulator::Schedule (
      Seconds (0.1), &EvalvidFecClient::Send,
      this); // SARA++ : need to change the time, based on the time of start app
}

void
EvalvidFecClient::Send (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<Packet> p = Create<Packet> ();

  SeqTsHeader seqTs;
  seqTs.SetSeq (m_packetId);
  p->AddHeader (seqTs);
  m_packetId++;
  m_socket->Send (p);

  NS_LOG_INFO (">> EvalvidFecClient: Sending request for video streaming to EvalvidServer at "
               << m_peerAddress << ":" << m_peerPort);
}

void
EvalvidFecClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  receiverDumpFile.close ();
  correctionFile.close ();
  m_recvFecFile.close ();
  m_recvIpktFile.close ();
  Simulator::Cancel (m_sendEvent);
}

void
EvalvidFecClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          if (packet->GetSize () == 20) // packet is a ECHO 
            {
              SeqTsEchoHeader seqEcho;
              packet->RemoveHeader (seqEcho);

              NS_LOG_INFO (" Received a ECHO packet with Sender's time = " 
                            << seqEcho.GetTsValue ().GetSeconds () << std::endl);

              SeqTsEchoHeader seqEchoReply;
              seqEchoReply.SetTsValue (seqEcho.GetTsValue ());
              seqEchoReply.SetTsEchoReply(Simulator::Now ());
              seqEchoReply.SetSeq(m_packetId);
              
              m_packetId++;
              Ptr<Packet> p = Create<Packet> ();
              p->AddHeader (seqEchoReply);
              m_socket->Send (p);
              NS_LOG_DEBUG ( "Sent RTT from receiver ----- TsValue: " << seqEchoReply.GetTsValue() << "  TsEchoReply: " << seqEchoReply.GetTsEchoReply() << std::endl);
            }
          else if (packet->GetSize () > 0)
            {
              SeqMyRTPHeader seqMyRTP;
              packet->RemoveHeader (seqMyRTP);
              uint32_t packetId = seqMyRTP.GetSeq ();
              uint32_t firstSeq = seqMyRTP.GetFirstSeq ();
              uint32_t numIPackets = seqMyRTP.GetNumIpkts ();
              SeqMyRTPHeader::FRAME_TYPE frameType = seqMyRTP.GetFrameType ();
              uint32_t maxAcceptedLoss = seqMyRTP.GetCorrectionAbility ();
              // uint32_t frameId_packet = seqMyRTP.GetFrameSeq ();
              double transmitionTime =  Simulator::Now ().ToDouble (ns3::Time::S) - seqMyRTP.GetTs().ToDouble(ns3::Time::S);

              // NS_LOG_INFO (">> EvalvidFecClient: Received packet at "
              //                 << Simulator::Now ().GetSeconds () << "s\tid: " << packetId << "\tudp\t" << packet->GetSize () << 
              //                 "\tFrame_type\t" << frameType << "\tfirstSeq\t" << firstSeq << "\ttransmitionTime\t" << transmitionTime << std::endl); 
                                                       

              m_gopPacketInfo currentPacket;

              if (m_firstSeqGop == firstSeq and transmitionTime <= m_applicationThreshold )      // the packet is belong to current gop  with acceptable delay
                {
                  if (frameType == SeqMyRTPHeader::I_FRAME)
                    {
                      m_recvIPktGop++;
                      m_numIPktsGop = numIPackets;
                      m_correctionAbilityGop = maxAcceptedLoss;

                      currentPacket.m_packetSeq = packetId;
                      currentPacket.m_packetSize = packet->GetSize ();
                      currentPacket.m_packetTime = Simulator::Now ().ToDouble (ns3::Time::S);
                      currentPacket.m_packetSentTime = seqMyRTP.GetTs().ToDouble(ns3::Time::S);
                      currentPacket.m_frameId = seqMyRTP.GetFrameSeq ();
                      m_I_PktsCurrGOP.push_back (currentPacket);

                      m_recvIpktFile << std::fixed << std::setprecision (4)
                                    << currentPacket.m_packetTime 
                                    << std::setfill (' ') << std::setw (16) << "id " << currentPacket.m_packetSeq 
                                    << std::setfill (' ') << std::setw (16) << "udp "
                                    << currentPacket.m_packetSize << std::endl;
                    }
                  else if (frameType == SeqMyRTPHeader::FEC_FRAME)
                    {
                      m_recvFecPktGop++;
                      m_numIPktsGop = numIPackets;
                      m_correctionAbilityGop = maxAcceptedLoss;

                      currentPacket.m_packetSeq = packetId;
                      currentPacket.m_packetSize = packet->GetSize ();
                      currentPacket.m_packetTime = Simulator::Now ().ToDouble (ns3::Time::S);
                      currentPacket.m_packetSentTime = seqMyRTP.GetTs().ToDouble(ns3::Time::S);
                      currentPacket.m_frameId = seqMyRTP.GetFrameSeq ();
                      m_FEC_PktsCurrGOP.push_back (currentPacket);

                      m_recvFecFile << std::fixed << std::setprecision (4)
                                    << currentPacket.m_packetTime 
                                    << std::setfill (' ') << std::setw (16) << "id " << currentPacket.m_packetSeq 
                                    << std::setfill (' ') << std::setw (16) << "udp "
                                    << currentPacket.m_packetSize << std::endl;
                    }
                  else if (frameType == SeqMyRTPHeader::B_FRAME || frameType == SeqMyRTPHeader::P_FRAME)
                    {
                      // when we receive a B/P packet, we are in 100% sure that we will not receive any I and FEC packet belong to current gop
                      // So we can go for correcting process.

                      // correction function if I and FEC packet were received
                      if (m_recvIPktGop + m_recvFecPktGop > 0 )
                      {
                        DoCorrectionGop();
                      }
                      m_recvIPktGop = 0;
                      m_recvFecPktGop = 0;
                      m_correctionAbilityGop = 0;
                      
                      // just log into file
                      m_recvBPPktGop++;

                      receiverDumpFile << std::fixed << std::setprecision (4)
                                      << Simulator::Now ().ToDouble (ns3::Time::S) << std::setfill (' ') << std::setw (16) 
                                      << "id " << packetId << std::setfill (' ') << std::setw (16) 
                                      << "udp " << packet->GetSize () << std::setfill (' ') << std::setw (16) 
                                      << "latency " << transmitionTime << std::setfill (' ') << std::setw (16)
                                      << "frameId " << seqMyRTP.GetFrameSeq () << std::setfill (' ') << std::setw (16) <<std::endl; 
                                      // << "gop " << m_gopNumber << std::endl;
                      }
                }
              else if ( m_firstSeqGop < firstSeq and transmitionTime <= m_applicationThreshold )    // the packet is belong to next gop , it is a first packet of next gop  with acceptable delay
                {
                  if ( m_recvIPktGop + m_recvFecPktGop > 0 )     // so, we could not receive any B or P packets of previous gop, so DoCorrectionGop function was not called.
                    {
                      DoCorrectionGop();
                    }
                
                  // we need to update m_first_seq and last seq.
                  // clear counters
                  m_firstSeqGop = firstSeq;
                  m_numIPktsGop = numIPackets;
                  m_correctionAbilityGop = maxAcceptedLoss;
                  m_recvIPktGop = 0;
                  m_recvFecPktGop = 0;
                  m_recvBPPktGop = 0;
                  m_correctionAbilityGop = 0;
                  // m_gopNumber++;
                  
                  if (frameType == SeqMyRTPHeader::I_FRAME)
                    {
                      m_recvIPktGop++;

                      currentPacket.m_packetSeq = packetId;
                      currentPacket.m_packetSize = packet->GetSize ();
                      currentPacket.m_packetTime = Simulator::Now ().ToDouble (ns3::Time::S);
                      currentPacket.m_packetSentTime = seqMyRTP.GetTs().ToDouble(ns3::Time::S);
                      currentPacket.m_frameId = seqMyRTP.GetFrameSeq ();
                      m_I_PktsCurrGOP.push_back (currentPacket);

                      m_recvIpktFile << std::fixed << std::setprecision (4)
                                    << currentPacket.m_packetTime 
                                    << std::setfill (' ') << std::setw (16) << "id " << currentPacket.m_packetSeq 
                                    << std::setfill (' ') << std::setw (16) << "udp "
                                    << currentPacket.m_packetSize << std::endl;
                    }
                  else if (frameType == SeqMyRTPHeader::FEC_FRAME)
                    {
                      m_recvFecPktGop++;

                      currentPacket.m_packetSeq = packetId;
                      currentPacket.m_packetSize = packet->GetSize ();
                      currentPacket.m_packetTime = Simulator::Now ().ToDouble (ns3::Time::S);
                      currentPacket.m_packetSentTime = seqMyRTP.GetTs().ToDouble(ns3::Time::S);
                      currentPacket.m_frameId = seqMyRTP.GetFrameSeq ();
                      m_FEC_PktsCurrGOP.push_back (currentPacket);

                      m_recvFecFile << std::fixed << std::setprecision (4)
                                    << currentPacket.m_packetTime 
                                    << std::setfill (' ') << std::setw (16) << "id " << currentPacket.m_packetSeq 
                                    << std::setfill (' ') << std::setw (16) << "udp "
                                    << currentPacket.m_packetSize << std::endl;
                    }
                  else if (frameType == SeqMyRTPHeader::B_FRAME || frameType == SeqMyRTPHeader::P_FRAME)
                    {                    
                      // just log into file
                      m_recvBPPktGop++;

                      receiverDumpFile << std::fixed << std::setprecision (4)
                                      << Simulator::Now ().ToDouble (ns3::Time::S) << std::setfill (' ') << std::setw (16) 
                                      << "id " << packetId << std::setfill (' ') << std::setw (16) 
                                      << "udp " << packet->GetSize () << std::setfill (' ') << std::setw (16) 
                                      << "latency " << transmitionTime << std::setfill (' ') << std::setw (16)
                                      << "frameId " << seqMyRTP.GetFrameSeq () << std::setfill (' ') << std::setw (16) << std::endl; 
                                      // << "gop " << m_gopNumber << std::endl;
                    }        
                }
              else if (m_firstSeqGop > firstSeq or transmitionTime > m_applicationThreshold )     
                {
                  // the packet is belong to previous gop nd discard received packet    or  we received it more than acceptable delay.

                  NS_LOG_DEBUG (">> EvalvidFecClient: Received a delayed packet at "
                              << Simulator::Now ().GetSeconds () << "s\tid: " << packetId << "\tudp\t" << packet->GetSize () << 
                              "\tFrame_type\t" << frameType << "\tfirstSeq\t" << firstSeq << "\ttrans_time\t" << Simulator::Now () - seqMyRTP.GetTs() << std::endl);   
                }
            }
          else
            {
              // packet is not a RTP or echo packet ...
              NS_LOG_DEBUG (">> EvalvidFecClient: Receiving a packet that is not RTP or echo packet at "
                            << Simulator::Now ().GetSeconds ());
            }
        }
    }
}

void
EvalvidFecClient::DoCorrectionGop (void)
{
  uint32_t totalRecvIandFECPktsGop = m_recvIPktGop + m_recvFecPktGop;
  // NS_LOG_DEBUG("DoCorrectionGop :  m_recvIPktGop  : " << m_recvIPktGop << "  m_recvFecPktGop : " << m_recvFecPktGop);
  uint32_t totalExpectIandFECPktsGop = m_numIPktsGop + m_correctionAbilityGop;
  m_lostPktGop = ceil(((double)(totalExpectIandFECPktsGop - totalRecvIandFECPktsGop) / totalExpectIandFECPktsGop) * 100) ;  // [(N - x)/N]* 100 

  if (m_lostPktGop == 0)
    {
      NS_LOG_DEBUG (" DoCorrectionGop: Loss rate = 0% for seq = " << m_firstSeqGop << " to seq = " << m_firstSeqGop + m_numIPktsGop - 1 << std::endl);                          

      // there is 0% packet lost so we can put I packets in file 
      for (uint32_t i = 0 ; i < m_I_PktsCurrGOP.size(); i ++)
        {
          // receiverDumpFile << std::fixed << std::setprecision (4) << m_I_PktsCurrGOP[i].m_packetTime
          //                               << std::setfill (' ') << std::setw (16) << "id "
          //                               << m_I_PktsCurrGOP[i].m_packetSeq << std::setfill (' ')
          //                               << std::setw (16) << "udp " << m_I_PktsCurrGOP[i].m_packetSize
          //                               << std::endl;

          receiverDumpFile << std::fixed << std::setprecision (4)
                           << m_I_PktsCurrGOP[i].m_packetTime << std::setfill (' ') << std::setw (16) 
                           << "id " << m_I_PktsCurrGOP[i].m_packetSeq << std::setfill (' ') << std::setw (16) 
                           << "udp " << m_I_PktsCurrGOP[i].m_packetSize << std::setfill (' ') << std::setw (16) 
                           << "latency " << m_I_PktsCurrGOP[i].m_packetTime - m_I_PktsCurrGOP[i].m_packetSentTime << std::setfill (' ') << std::setw (16)
                           << "frameId " << m_I_PktsCurrGOP[i].m_frameId << std::setfill (' ') << std::setw (16) << std::endl; 
                           // << "gop " << m_gopNumber << std::endl;                                        
        }
      // saving correction in a file
      correctionFile << std::fixed << std::setprecision (4)
                        << Simulator::Now ().ToDouble (Time::S) << std::setfill (' ')
                        << std::setw (16) << "IPacketsID " << m_firstSeqGop << "-" << m_firstSeqGop + m_numIPktsGop - 1 << std::endl;
    }
  // else if (0 < m_lostPktGop && m_lostPktGop <= m_correctionAbilityGop)
  else if (totalRecvIandFECPktsGop >= m_numIPktsGop)
    {
      NS_LOG_DEBUG (">> DoCorrectionGop: Recover loss packets, Loss rate = " << m_lostPktGop << "% for seq = " << m_firstSeqGop << " to seq = " << m_firstSeqGop + m_numIPktsGop - 1  << " totalExpectIandFECPktsGop = " << totalExpectIandFECPktsGop << " totalRecvIandFECPktsGop = " << totalRecvIandFECPktsGop<< std::endl);
      CorrectionGop (); //we can recover lost packets and put all packets in receiver dump file

      // saving correction in a file
      correctionFile << std::fixed << std::setprecision (4)
                     << Simulator::Now ().ToDouble (Time::S) << std::setfill (' ')
                     << std::setw (16) << "IPacketsID " << m_firstSeqGop << "-" << m_firstSeqGop + m_numIPktsGop - 1 << std::endl;
    }
  // else if (m_lostPktGop > m_correctionAbilityGop)
  else if (totalRecvIandFECPktsGop < m_numIPktsGop)
    {
      NS_LOG_DEBUG (">> DoCorrectionGop: Can't recover loss packets. Loss rate = " << m_lostPktGop << "% for seq = " << m_firstSeqGop << " to seq = " << m_firstSeqGop + m_numIPktsGop - 1 << " totalExpectIandFECPktsGop = " << totalExpectIandFECPktsGop << " totalRecvIandFECPktsGop = " << totalRecvIandFECPktsGop<< std::endl);
      // discard all recieved packets      ??????? e.g.  total = 16   correc_ability = 3     received = 12   ===> discard all 12 packets just for one
    }
  
  //after each situation , we need to clear received arraies of I and FEC
  while (m_I_PktsCurrGOP.size () != 0)
    {
      m_I_PktsCurrGOP.pop_back ();
    }
  while (m_FEC_PktsCurrGOP.size () != 0)
    {
      m_FEC_PktsCurrGOP.pop_back ();
    }

  // send RTCP packet with the number of lost packets (m_lost_counter_gop)
  SendRTCP();
}

void
EvalvidFecClient::CorrectionGop (void)
{
  uint32_t lastSeqIPkt = m_firstSeqGop + m_numIPktsGop - 1;
  uint32_t iteratorGop = m_firstSeqGop;                                  // we expect maximum number of I packets in m_current_gop should be 10.
  uint32_t indexI = 0;         // indexI is a iteratore for Received I Pkts of Current Gop array
  uint32_t indexFec = 0;         // indexFec is a iteratore for Received FEC Pkts of Current Gop array
  uint32_t lenIArray = m_I_PktsCurrGOP.size();
  uint32_t lenFECArray = m_FEC_PktsCurrGOP.size();

  while ( iteratorGop <= lastSeqIPkt)
    {
      if ( lenIArray != 0 )     // we received at least one I packet.
        {
          if (indexI < lenIArray)   // if we lost the last packets we need copy of prevoius packet.
            {                       // so we need different IF conditions.
              if ( m_I_PktsCurrGOP[indexI].m_packetSeq == iteratorGop )
                {
                  // receiverDumpFile << std::fixed << std::setprecision (4) << m_I_PktsCurrGOP[indexI].m_packetTime
                  //                   << std::setfill (' ') << std::setw (16) << "id "
                  //                   << m_I_PktsCurrGOP[indexI].m_packetSeq << std::setfill (' ')
                  //                   << std::setw (16) << "udp " << m_I_PktsCurrGOP[indexI].m_packetSize
                  //                   << std::endl;

                  receiverDumpFile << std::fixed << std::setprecision (4)
                                      << m_I_PktsCurrGOP[indexI].m_packetTime << std::setfill (' ') << std::setw (16) 
                                      << "id " << m_I_PktsCurrGOP[indexI].m_packetSeq << std::setfill (' ') << std::setw (16) 
                                      << "udp " << m_I_PktsCurrGOP[indexI].m_packetSize << std::setfill (' ') << std::setw (16) 
                                      << "latency " << m_I_PktsCurrGOP[indexI].m_packetTime - m_I_PktsCurrGOP[indexI].m_packetSentTime << std::setfill (' ') << std::setw (16)
                                      << "frameId " << m_I_PktsCurrGOP[indexI].m_frameId << std::setfill (' ') << std::setw (16) << std::endl; 
                                      // << "gop " << m_gopNumber << std::endl; 
                  iteratorGop++;
                  indexI++;
                }
              else        // we found a lost packet and recover it
                {
                  // receiverDumpFile << std::fixed << std::setprecision (4) << m_I_PktsCurrGOP[indexI].m_packetTime << std::setfill (' ')
                  //                 << std::setw (16) << "id " << iteratorGop << std::setfill (' ')
                  //                 << std::setw (16) << "udp " << m_I_PktsCurrGOP[indexI].m_packetSize
                  //                 << std::endl;

                  receiverDumpFile << std::fixed << std::setprecision (4)
                                      << m_I_PktsCurrGOP[indexI].m_packetTime << std::setfill (' ') << std::setw (16) 
                                      << "id " << iteratorGop << std::setfill (' ') << std::setw (16) 
                                      << "udp " << m_I_PktsCurrGOP[indexI].m_packetSize << std::setfill (' ') << std::setw (16) 
                                      << "latency " << m_I_PktsCurrGOP[indexI].m_packetTime - m_I_PktsCurrGOP[indexI].m_packetSentTime << std::setfill (' ') << std::setw (16)
                                      << "frameId " << m_I_PktsCurrGOP[indexI].m_frameId << std::setfill (' ') << std::setw (16) << std::endl;  
                                      // << "gop " << m_gopNumber << std::endl;
                  iteratorGop++;
                }
            }
          else    // there are some lost packet at the end of the gop that we need to recover them
            {
              // receiverDumpFile << std::fixed << std::setprecision (4) << m_I_PktsCurrGOP[indexI-1].m_packetTime << std::setfill (' ') 
              //                 << std::setw (16) << "id " << iteratorGop << std::setfill (' ')
              //                 << std::setw (16) << "udp " << m_I_PktsCurrGOP[indexI-1].m_packetSize
              //                 << std::endl;

              receiverDumpFile << std::fixed << std::setprecision (4)
                              << m_I_PktsCurrGOP[indexI-1].m_packetTime << std::setfill (' ') << std::setw (16) 
                              << "id " << iteratorGop << std::setfill (' ') << std::setw (16) 
                              << "udp " << m_I_PktsCurrGOP[indexI-1].m_packetSize << std::setfill (' ') << std::setw (16) 
                              << "latency " << m_I_PktsCurrGOP[indexI-1].m_packetTime - m_I_PktsCurrGOP[indexI-1].m_packetSentTime << std::setfill (' ') << std::setw (16) 
                              << "frameId " << m_I_PktsCurrGOP[indexI-1].m_frameId << std::setfill (' ') << std::setw (16) << std::endl; 
                              // << "gop " << m_gopNumber << std::endl;
              iteratorGop++;
            }
        }
      else if (lenFECArray != 0)      // we did not receive I packet but we received at least 1 FEC packet.
        {
          // pas hameye I haro az dast dadim va alan faghat FEC darim, pas bbayad ye array az FEC ha negah darim
                      
          // dar halati ke I daryaft nakarde bashim , hamishe fec seq > iterator khahad bood.
          // pas ye for niaz darim ke be tedade numIpackts tekrar beshe 

          // receiverDumpFile << std::fixed << std::setprecision (4) << m_FEC_PktsCurrGOP[indexFec].m_packetTime << std::setfill (' ')    /// we alway use the fisrt FEC packet for recovery
          //                   << std::setw (16) << "id " << iteratorGop << std::setfill (' ')
          //                   << std::setw (16) << "udp " << m_FEC_PktsCurrGOP[indexFec].m_packetSize
          //                   << std::endl;

          receiverDumpFile << std::fixed << std::setprecision (4)
                            << m_FEC_PktsCurrGOP[indexFec].m_packetTime << std::setfill (' ') << std::setw (16) 
                            << "id " << iteratorGop << std::setfill (' ') << std::setw (16) 
                            << "udp " << m_FEC_PktsCurrGOP[indexFec].m_packetSize << std::setfill (' ') << std::setw (16) 
                            << "latency " << m_FEC_PktsCurrGOP[indexFec].m_packetTime - m_FEC_PktsCurrGOP[indexFec].m_packetSentTime << std::setfill (' ') << std::setw (16)
                            << "frameId " << m_FEC_PktsCurrGOP[indexFec].m_frameId << std::setfill (' ') << std::setw (16) << std::endl;  
                            // << "gop " << m_gopNumber << std::endl;
          iteratorGop++;
          indexFec++;
        }
      else    // agar vared in else shavad eshtebahan varede correction shode, BUG!!!!!
        {
          NS_FATAL_ERROR(">> EvalvidFecClient: Error in correcting function. We don't received any packets. 100% loss rate " );
          return;
        }
    }
}

void
EvalvidFecClient::SendRTCP (void)
{
  // creat packet for sending lost  that it should be a percentage from 0 to 100 
  // reset the m_lostpackets 

  Ptr<Packet> p = Create<Packet> ();
  SeqMyRTCPHeader seqMyRTCP;
  seqMyRTCP.SetSeq (m_packetId);
  seqMyRTCP.SetLostPackets (m_lostPktGop);
  seqMyRTCP.SetFirstSeq (m_firstSeqGop);
  p->AddHeader (seqMyRTCP);
  m_packetId++;

  m_socket->Send (p);
  m_lostPktGop = 0;

  NS_LOG_DEBUG ("  SendRTCP: Sent RTCP packet from receiver ---- seqId =  " << seqMyRTCP.GetSeq() << " Gop.firstSeq = " << seqMyRTCP.GetFirstSeq()  << " PLR = " << seqMyRTCP.GetLostPackets() << " % " << std::endl);
}

} // Namespace ns3
