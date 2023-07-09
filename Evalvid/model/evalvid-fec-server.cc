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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/boolean.h"

#include "evalvid-fec-server.h"

#include "ns3/gnuplot.h"

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalvidFecServer");
NS_OBJECT_ENSURE_REGISTERED (EvalvidFecServer);

TypeId
EvalvidFecServer::GetTypeId (void)
{
  static TypeId
      tid =
          TypeId ("ns3::EvalvidFecServer")
              .SetParent<Application> ()
              .AddConstructor<EvalvidFecServer> ()
              .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                             UintegerValue (100), MakeUintegerAccessor (&EvalvidFecServer::m_port),
                             MakeUintegerChecker<uint16_t> ())
              .AddAttribute ("StaticFec", "If this variable is larger than 0, the number of FEC redundancy is equal to this percentage. Default value = 0 ",
                             UintegerValue (0), MakeUintegerAccessor (&EvalvidFecServer::m_staticFEC),
                             MakeUintegerChecker<uint16_t> ())
              .AddAttribute ("SenderDumpFilename", "Sender Dump Filename", StringValue (""),      // sd_a01 = "sd_a01"
                             MakeStringAccessor (&EvalvidFecServer::m_senderTraceFileName),
                             MakeStringChecker ())
              .AddAttribute ("SenderTraceFilename", "Sender trace Filename", StringValue (""),      // SenderTraceFilename = "st_highway_cif.st"
                             MakeStringAccessor (&EvalvidFecServer::m_videoTraceFileName),
                             MakeStringChecker ())
              .AddAttribute ("SenderTraceFilenameLowQuality", "Sender trace Filename with low quality", StringValue (""),      // SenderTraceFilenameLowQuality = "st_highway_cif.st"
                             MakeStringAccessor (&EvalvidFecServer::m_lowVideoTraceFileName),
                             MakeStringChecker ())
              .AddAttribute ("GOPInfoFileName", "GOP info Filename", StringValue (""),      // m_gopInfoMapFileName = "GopInfoFile"
                             MakeStringAccessor (&EvalvidFecServer::m_gopInfoMapFileName),
                             MakeStringChecker ())
              .AddAttribute ("SentFecTraceFileName", "Sent FEC packets info", StringValue (""),      // m_sentFecTraceFileName = "sentFECInfo"
                             MakeStringAccessor (&EvalvidFecServer::m_sentFecTraceFileName),
                             MakeStringChecker ())
              .AddAttribute ("UsingRTT", "using RTT for predicting FEC level", 
                             BooleanValue (true), MakeBooleanAccessor (&EvalvidFecServer::m_usingRTT),                  
                             MakeBooleanChecker ())
              .AddAttribute ("BandwidthComparison", "Doing a bandwith comparison on low and high quality videos.", 
                             BooleanValue (true), MakeBooleanAccessor (&EvalvidFecServer::m_usingBandwidth),             
                             MakeBooleanChecker ())
              .AddAttribute ("AppThreshold", "Max acceptable delay for each GOP from App view", 
                             DoubleValue(1.5), MakeDoubleAccessor (&EvalvidFecServer::m_applicationThreshold),                
                             MakeDoubleChecker<double> ())
              .AddAttribute ("StaticQuality", "Send all frames based on one quality.", StringValue ("No"),          // No, High , Low
                             MakeStringAccessor (&EvalvidFecServer::m_staticQuality),
                             MakeStringChecker ())
              .AddAttribute ("LocationEcho", "the location of sending eco packet in each GOP", UintegerValue (15),          // afterFec, middle, end
                             MakeUintegerAccessor (&EvalvidFecServer::m_locationEcho),
                             MakeUintegerChecker<uint32_t> ())
              .AddAttribute ("AAT_Factor", "A safety factor for application threshold", DoubleValue(0.2), 
                             MakeDoubleAccessor(&EvalvidFecServer::m_aat_factor), 
                             MakeDoubleChecker<double> ())
              .AddAttribute ("AAT_Constant", "A safety factor for application threshold", DoubleValue(0.2), 
                             MakeDoubleAccessor(&EvalvidFecServer::m_aat_constant), 
                             MakeDoubleChecker<double> ())
              .AddAttribute ("changing_value_PLR", "The changing value for predicting PLR", UintegerValue (2),          // 1, 2, 3, 4
                             MakeUintegerAccessor (&EvalvidFecServer::m_changing_value_PLR),
                             MakeUintegerChecker<uint32_t> ())
              .AddAttribute (
                  "PacketPayload",
                  "Packet Payload, i.e. MTU - (SEQ_HEADER + UDP_HEADER + IP_HEADER). " // SeqRTPHeader = 30 + UDP_header = 8 + IP_header=20  -> 58
                  "This is the same value used to hint video with MP4Box (1500 - 58). Default: "
                  "1442.", // IEEE 802.11 ->  MTU=2304 ,    2304 - 58 ==> default: 2246
                  UintegerValue (1442), // UintegerValue (2246),
                  MakeUintegerAccessor (&EvalvidFecServer::m_packetPayload),
                  MakeUintegerChecker<uint16_t> ());
  return tid;
}

EvalvidFecServer::EvalvidFecServer ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_port = 0;
  m_packetPayload = 0;
  m_packetId = 0;
  m_sendEvent = EventId ();

  m_predictedFecPkts = 0;
  m_firstSeqGop = 0;
  m_numIPackets = 0;

  m_predictedPLR = 0;
  m_videoQualityStatus = "Low";     // we always start to send low quality video
  m_applicationThreshold = 1.5;   // application threshold = 1.5 second     there is a same variable in receiver side and both of them should be same number.
  m_latestRecvPLR = 0;
  m_totalSentIFrame = 0;
  m_latestRecvTT = 0.0;
  m_staticFEC = 0 ;    
  m_usingRTT = true;
  m_usingBandwidth = true;
  m_staticQuality = "No";
  m_locationEcho = 15;

  // todo: dynamic array size based on the number of GOP in input files
  m_lowVideoGopID = new uint32_t[1000]{0};
  m_highVideoGopID = new uint32_t[1000]{0};

  MAX_QUEUE_LEN = 10240;
  m_lastInputQue = 0 ;
  m_lastOutputQue = 0;
  m_finishingAdjustedPrePLR = false;      // default value = false bacause there is no run for adjusted module yet. we need to set as default when we finish I frame

  m_counter_B_P = 0;
  m_flagEcho = false;

  m_aat_factor = 0.8;
  m_aat_constant = 1.0;
  m_changing_value_PLR = 2;
}

EvalvidFecServer::~EvalvidFecServer ()
{
  NS_LOG_FUNCTION (this);
}

void
EvalvidFecServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
EvalvidFecServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  Ptr<Socket> socket = 0;
  Ptr<Socket> socket6 = 0;

  if (socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      socket->Bind (local);
      socket->SetRecvCallback (MakeCallback (&EvalvidFecServer::HandleRead, this));
    }

  if (socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      socket6->Bind (local);
      socket6->SetRecvCallback (MakeCallback (&EvalvidFecServer::HandleRead, this));
    }

  //Load video trace file
  Setup ();

}

void
EvalvidFecServer::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_DEBUG ("Evalvid FEC Server : Stop application ");

  m_gopInfoMapFile  << std::fixed << std::setprecision (4) 
                  << "Gop No. " << std::setfill (' ') << std::setw (16) 
                  << "FirstSeqGOP" << std::setfill (' ') << std::setw (16) 
                  << "Actual-PLR" << " %" << std::setfill (' ') << std::setw (16) 
                  << "SentTime Echo" << std::setfill (' ') << std::setw (16)
                  << "TT" << std::setfill (' ') << std::setw (16)
                  << "Predicted-PLR"  << " %" << std::setw (16)
                  << "Num I-Pkts"  << std::setw (16)
                  << "Num FEC-Pkts"  << std::setw (16)
                  << "seq for echo" << std::setw (16)
                  << "High=1-Low=0" << std::setw (16)
                  <<std::endl;

  for (auto it = m_gopInfoMap.begin(); it != m_gopInfoMap.end(); ++it)
    {
      m_gopInfoMapFile  << std::fixed << std::setprecision (4) 
                        << it->first << std::setfill (' ') << std::setw (16) 
                        << it->second->firstSeq << std::setfill (' ') << std::setw (16) 
                        << it->second->PLR << std::setfill (' ') << std::setw (16) 
                        << it->second->sentTimeEcho.GetSeconds() << std::setfill (' ') << std::setw (16)
                        << it->second->TT << std::setfill (' ') << std::setw (16)
                        << it->second->fecLevel  << std::setw (16)
                        << it->second->numIPackets << std::setw (16)
                        << it->second->numFECPkts << std::setw (16)
                        << it->second->echoSeq << std::setw (16)
                        << it->second->quality << std::setw (16)
                        << std::endl; 
    }  

  m_senderTraceFile.close ();
  // m_RTTLogFile.close ();
  m_sentFecTraceFile.close ();
  // m_lostLogFile.close ();
  m_gopInfoMapFile.close ();
  // Simulator::Cancel (m_sendEvent);
}

void
EvalvidFecServer::Setup ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_videoInfoStruct_t *videoInfoStruct;
  m_videoInfoStruct_t *lowVideoInfoStruct;
  uint32_t frameId;
  string frameType;
  uint32_t frameSize;
  uint16_t numOfUdpPackets;
  double sendTime;
  double lastSendTime = 0.0;

  //Open file from mp4trace tool of EvalVid.
  ifstream m_videoTraceFile (m_videoTraceFileName.c_str (), ios::in);
  if (m_videoTraceFile.fail ())
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: Error while opening video trace file: "
                      << m_videoTraceFileName.c_str () << "\n"
                      << "Error info: " << std::strerror (errno) << "\n");
      return;
    }
  
  ifstream m_lowVideoTraceFile (m_lowVideoTraceFileName.c_str (), ios::in);
  if (m_lowVideoTraceFile.fail ())
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: Error while opening video trace file: "
                      << m_lowVideoTraceFileName.c_str () << "\n"
                      << "Error info: " << std::strerror (errno) << "\n");
      return;
    }
    

  uint16_t index = 0 ;
  if (m_staticQuality != "Low" )
  {    
    //Store video trace information on the struct
    while (m_videoTraceFile >> frameId >> frameType >> frameSize >> numOfUdpPackets >> sendTime)
      {
        videoInfoStruct = new m_videoInfoStruct_t;
        videoInfoStruct->frameType = frameType;
        videoInfoStruct->frameSize = frameSize;
        videoInfoStruct->numOfUdpPackets = numOfUdpPackets;
        videoInfoStruct->packetInterval = Seconds (sendTime - lastSendTime); 
        m_videoInfoMap.insert (pair<uint32_t, m_videoInfoStruct_t *> (frameId, videoInfoStruct));
        NS_LOG_LOGIC (">> EvalvidFecServer: " << frameId << "\t" << frameType << "\t" << frameSize
                                          << "\t" << numOfUdpPackets << "\t" << sendTime);
        lastSendTime = sendTime;

        if (frameType == "I")
          {
            m_highVideoGopID[index++] = frameId;
            NS_LOG_LOGIC ("highVideo I frame Index :  " << m_highVideoGopID[index-1] << std::endl );
          }
      }
    NS_LOG_INFO ("length of m_highVideoGopID :  " << index << std::endl );
    index = 0 ;

    m_videoInfoMapIt = m_videoInfoMap.begin ();
  }
  else if (m_staticQuality == "Low")
  {
    //Store video trace information on the struct
    while (m_lowVideoTraceFile >> frameId >> frameType >> frameSize >> numOfUdpPackets >> sendTime)
      {
        videoInfoStruct = new m_videoInfoStruct_t;
        videoInfoStruct->frameType = frameType;
        videoInfoStruct->frameSize = frameSize;
        videoInfoStruct->numOfUdpPackets = numOfUdpPackets;
        videoInfoStruct->packetInterval = Seconds (sendTime - lastSendTime); 
        m_videoInfoMap.insert (pair<uint32_t, m_videoInfoStruct_t *> (frameId, videoInfoStruct));
        NS_LOG_LOGIC (">> EvalvidFecServer: " << frameId << "\t" << frameType << "\t" << frameSize
                                          << "\t" << numOfUdpPackets << "\t" << sendTime);
        lastSendTime = sendTime;

        if (frameType == "I")
          {
            m_highVideoGopID[index++] = frameId;    // bayad moteghayer ro hamin High negah daram ke baghie jaha beham nakhore.
            NS_LOG_LOGIC ("low Video I frame Index :  " << m_highVideoGopID[index-1] << std::endl );
          }
      }
    NS_LOG_INFO ("length of m_highVideoGopID :  " << index << std::endl );
    index = 0 ;

    m_videoInfoMapIt = m_videoInfoMap.begin ();
  }
  
  m_lowVideoTraceFile.clear();  // clear any flags that might be set
  m_lowVideoTraceFile.seekg(0, ios::beg);

  lastSendTime = 0 ;
  //Store Low quality video trace information on the struct
  while (m_lowVideoTraceFile >> frameId >> frameType >> frameSize >> numOfUdpPackets >> sendTime)
    {
      lowVideoInfoStruct = new m_videoInfoStruct_t;
      lowVideoInfoStruct->frameType = frameType;
      lowVideoInfoStruct->frameSize = frameSize;
      lowVideoInfoStruct->numOfUdpPackets = numOfUdpPackets;
      lowVideoInfoStruct->packetInterval = Seconds (sendTime - lastSendTime); 
      m_lowVideoInfoMap.insert (pair<uint32_t, m_videoInfoStruct_t *> (frameId, lowVideoInfoStruct));
      NS_LOG_LOGIC (">> EvalvidFecServer: Low Quality Video Frames : " << frameId << "\t" << frameType << "\t" << frameSize
                                         << "\t" << numOfUdpPackets << "\t" << sendTime);
      lastSendTime = sendTime;
      if (frameType == "I")
        {
          m_lowVideoGopID[index++] = frameId;
          NS_LOG_LOGIC ("Low Video I frame Index :  " << m_lowVideoGopID[index-1] << std::endl );
        }
    }
  NS_LOG_INFO ("length of m_lowVideoGopID :  " << index << std::endl );
  index = 0 ;

  //Open file to store information of packets transmitted by EvalvidFecServer.
  m_senderTraceFile.open (m_senderTraceFileName.c_str (), ios::out);
  if (m_senderTraceFile.fail ())
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: Error while opening sender trace file: "
                      << m_senderTraceFileName.c_str () << std::endl );
      return;
    }

  // open the file for logging FEC packets 
  m_sentFecTraceFile.open(m_sentFecTraceFileName.c_str (), ios::out);
  if (m_sentFecTraceFile.fail ())
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: Error while opening sent FEC log file: "
                      <<m_sentFecTraceFileName.c_str () << "\n"
                      << "Error info: " << std::strerror (errno) << "\n");
      return;
    }
  
  //open the file for logging GOP info map 
  m_gopInfoMapFile.open(m_gopInfoMapFileName.c_str (), ios::out);
  if (m_gopInfoMapFile.fail ())
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: Error while opening GopInfoMapFile file: "
                      << m_gopInfoMapFileName.c_str () << "\n"
                      << "Error info: " << std::strerror (errno) << "\n");
      return;
    }
}

void
EvalvidFecServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<Packet> packet;
  Address from;
  m_socket = socket;
  
  while ((packet = socket->RecvFrom (from)))
    {
      // separating differnt type of packets    ---- recognizing RTCP
      //?????????    size of header   or type of the header
      // NS_LOG_DEBUG (">> EvalvidFecServer: Client sent a packet with size ******  =    " << packet->GetSize() << "  &&&&&");

      m_peerAddress = from;

      if (packet->GetSize () == 24) // packet is a RTCP
        {
          SeqMyRTCPHeader seqMyRTCP;
          packet->RemoveHeader (seqMyRTCP);
          // m_predictedPLR = seqMyRTCP.GetLostPackets ();
          Time currentTime = Simulator::Now ();

          // m_lostLogFile  << std::fixed << std::setprecision (4) 
          //               << currentTime.ToDouble (Time::S) << std::setfill (' ') << std::setw (16) 
          //               << seqMyRTCP.GetLostPackets () << std::setfill (' ') << std::setw (16) 
          //               << seqMyRTCP.GetFirstSeq () << std::endl; 

          // set the PLR for related GOP in matrix info based on firstSeq of GOP
          for (auto it = m_gopInfoMap.rbegin(); it != m_gopInfoMap.rend(); ++it)
          {
            if ( it->second->firstSeq == seqMyRTCP.GetFirstSeq () )
            {
              it->second->PLR = seqMyRTCP.GetLostPackets ();
              m_latestRecvPLR = seqMyRTCP.GetLostPackets ();
              NS_LOG_DEBUG ("Received a RTCP at sender ----- seqId: " << seqMyRTCP.GetSeq() << "  GOP No. " << it->first << " , Gop.firstSeq: " << it->second->firstSeq 
                            << " , PLR: " << it->second->PLR << " , SentTime Echo: " << it->second->sentTimeEcho << " , TT: " << it->second->TT << std::endl );

              m_lastOutputQue = it->second->firstSeq + it->second->numIPackets + it->second->numFECPkts-1;

              NS_LOG_DEBUG ("m_lastOutputQue =  " << m_lastOutputQue  << std::endl );

              if (it->second->quality == "Low")   // this gop was sent in low quality
              {
                it->second->bandwidthRTCP = (double)(it->second->numIPackets + it->second->numFECPkts) / (currentTime - it->second->firstSeqTime).GetSeconds() ;
                NS_LOG_DEBUG ("bandwidthRTCP - low quality - based on RTCP  ...  seqId: " << seqMyRTCP.GetSeq() << "  GOP No. " << it->first << " , Gop.firstSeq: " << it->second->firstSeq 
                                << " , I-packets: " << it->second->numIPackets  << " , FEC-packets: " << it->second->numFECPkts 
                                << " , currentTime: " << currentTime.GetSeconds() << " , SendTime: " << it->second->firstSeqTime.GetSeconds() 
                                << " , bandwidthRTCP = " << it->second->bandwidthRTCP << std::endl );
              }
             
              break;
            }
          }                      
        }
      else if (packet->GetSize () == 20)
        {
          SeqTsEchoHeader seqEchoRely;
          packet->RemoveHeader (seqEchoRely);
          Time currentTime = Simulator::Now ();
          double RTT = currentTime.GetSeconds () - seqEchoRely.GetTsValue ().GetSeconds (); 
          //Queueingdelay = seqEchoRely.GetTsEchoReply().GetSeconds () - seqEchoRely.GetTsValue ().GetSeconds () - Size od echo packet / bandwidth

          // m_RTTLogFile  << std::fixed << std::setprecision (4) 
          //               << currentTime.ToDouble (Time::S) << std::setfill (' ') << std::setw (16) 
          //               << "RecvEcho " << std::setfill (' ') << std::setw (16) 
          //               << "SentEchoTime " << seqEchoRely.GetTsValue ().GetSeconds () << std::setfill (' ') << std::setw (16) 
          //               << "RTT " << RTT << std::endl; 
          
          //record TT based on sentTime of Echo Pkt in m_gopInfoMap
          
          for (auto it = m_gopInfoMap.rbegin(); it != m_gopInfoMap.rend(); ++it)
          {
            if ( it->second->sentTimeEcho == seqEchoRely.GetTsValue () )
            {
              // it->second->TT = currentTT.GetSeconds();
              it->second->TT = RTT/(double)(2);
              m_latestRecvTT = it->second->TT ;

              NS_LOG_DEBUG ("Received a RTT at sender ----- GOP No. " << it->first << " , Gop.firstSeq: " << it->second->firstSeq << " , PLR: " 
                              << it->second->PLR << " , SentTime of Echo: " << it->second->sentTimeEcho << " , TT: " << it->second->TT << std::endl );

              m_lastOutputQue = it->second->echoSeq;            
              NS_LOG_DEBUG ("m_lastOutputQue =  " << m_lastOutputQue  << std::endl );

              // wrong idea : record bandwith but it is wrong based on our goal. we need to calculate bandwith just based on one frame not all of the frame of GOP
              
              // keep another value for bandwidth usage 
              if (it->second->quality == "Low")   // this gop was sent in low quality
              {
                it->second->bandwidthEchoReply = (double)((it->second->echoSeq - it->second->firstSeq) +1 ) / (currentTime - it->second->firstSeqTime).GetSeconds() ;    //packet/sec
                NS_LOG_DEBUG ("bandwidthEchoReply - low quality - based on Echo reply  ...  GOP No. " << it->first << " , Gop.firstSeq: " << it->second->firstSeq 
                                << " , echoSeq: " << it->second->echoSeq  << " , firstSeq: " << it->second->firstSeq 
                                << " , currentTime: " << currentTime.GetSeconds() << " , SendTimeof first seq: " << it->second->firstSeqTime.GetSeconds() 
                                << " , bandwidthEchoReply = " << it->second->bandwidthEchoReply << std::endl );
              }
              

              break;
            }
          }      
        }
      else
        {
          if (InetSocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO (">> EvalvidFecServer: Client at "
                           << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                           << " is requesting a video streaming.");
            }
          else if (Inet6SocketAddress::IsMatchingType (from))
            {
              NS_LOG_INFO (">> EvalvidFecServer: Client at "
                           << Inet6SocketAddress::ConvertFrom (from).GetIpv6 ()
                           << " is requesting a video streaming.");
            }
          
          NS_LOG_INFO (">> EvalvidFecServer: Starting  video streaming...");
          m_sendEvent = Simulator::ScheduleNow (&EvalvidFecServer::Send, this);
        }
    }
}

void
EvalvidFecServer::Send ()
{
  NS_LOG_INFO(">> m_changing_value_PLR =  " << m_changing_value_PLR << std::endl);
  if (m_videoInfoMapIt != m_videoInfoMap.end ())
    {
      if (m_videoInfoMapIt->second->frameType == "I")
        {
          NS_LOG_INFO(">> EvalvidFecServer: Starting a new GOP!");
          m_totalSentIFrame++;

          //Set the first seq of the gop
          m_firstSeqGop = m_packetId + 1;

          //calculating FEC packets for this I frame based on packet lost         **************************
          CalculatingFECLevel ();

          while (!m_finishingAdjustedPrePLR)
          {
            if ( m_staticQuality == "No")
            {
              if (m_videoQualityStatus == "Low" )    // we need to send low quality
              {
                ReplaceWithLowQuality ();
              }
            }
            else if (m_staticQuality == "Low" )
            {
              m_videoQualityStatus = "Low";
              ReplaceWithLowQuality ();
            }
            else if (m_staticQuality == "High" )
            {
              m_videoQualityStatus = "High";
            }

            //Calculating the number of I-frame packets based on the m_packetPayload
            if (m_videoInfoMapIt->second->frameSize % m_packetPayload)
              m_numIPackets = (m_videoInfoMapIt->second->frameSize / m_packetPayload) + 1;
            else
              m_numIPackets = m_videoInfoMapIt->second->frameSize / m_packetPayload;
            
            DetermineFECLevel ();

            if ( m_staticQuality == "No")
            {
              AdjustingPredictedPLR();
            }
            else
            {
              //TODO: add a AdjustedPredictedPLR for staticQuality which is equal Low or High 
              if ( m_staticQuality == "High" || m_staticQuality == "Low" )
              {
                NS_LOG_DEBUG ("static Quality ---- m_staticQuality = " << m_staticQuality << std::endl);
                m_finishingAdjustedPrePLR = true;
              }
              else
              {
                NS_FATAL_ERROR ("m_staticQuality variable has not a suitable value.");
              }
            }
          }

          //generating a row in a matrix of the information for each GOP
          m_gopInfoStruct_t *gopInfoStruct = new m_gopInfoStruct_t;
          gopInfoStruct->firstSeq = m_firstSeqGop;
          gopInfoStruct->PLR = 222;
          gopInfoStruct->sentTimeEcho = Simulator::Now () ;     // as a defualt value, I can not set 0 for Time variable.
          gopInfoStruct->TT = 0.0; 
          gopInfoStruct->fecLevel = m_predictedPLR;
          gopInfoStruct->numIPackets = m_numIPackets;
          gopInfoStruct->numFECPkts = m_predictedFecPkts;
          gopInfoStruct->quality = m_videoQualityStatus;     
          gopInfoStruct->echoSeq = 0;
          gopInfoStruct->bandwidthRTCP = 0.0 ;    //default value for bandwidthRTCP
          gopInfoStruct->bandwidthEchoReply = 0.0 ;   //default value for bandwidth based on Echo reply packet
          gopInfoStruct->safetyFactor = 0.0 ;  
          gopInfoStruct->firstSeqTime = Simulator::Now () ;    // as a defualt value, I can not set 0 for Time variable.

          m_gopInfoMap.insert (pair<uint32_t, m_gopInfoStruct_t *> (m_totalSentIFrame, gopInfoStruct));    

          SendVideoPackets ();

          if (m_numIPackets != 0)
          {
            SendFECPackets ();
          }
          else
          {
            NS_FATAL_ERROR (">> EvalvidFecServer: I-Frame size is 0!");
          }
          m_finishingAdjustedPrePLR = false;

          if (m_locationEcho == 1)  //send echo after I and FEC packets
          {                  
            //send echo packet for calculating RTT 
            SendEchoPacket();
            NS_LOG_DEBUG ("Send Echo packet after I frame, m_locationEcho = " << m_locationEcho << std::endl);
          }
        }
      else if (m_videoInfoMapIt->second->frameType == "P" ||
               m_videoInfoMapIt->second->frameType == "B")
        {
          //Sending the P-frame or B-frame in multiples segments

          SendVideoPackets ();  

          ///*  
          //Send Echo in the middle of GOP 
          m_counter_B_P += 1 ;

          if (m_counter_B_P >= m_locationEcho and m_flagEcho == false and m_locationEcho != 1)     //and m_locationEcho == "middle")
          {
            SendEchoPacket();
            m_flagEcho = true;
            NS_LOG_DEBUG ("Send Echo packet in the middle of GOP, m_locationEcho = " << m_locationEcho  << " m_counter_B_P = " << m_counter_B_P << std::endl);
          }
          //*/
        }
      else
        {
          NS_FATAL_ERROR (">> EvalvidFecServer: Frame Type is not Valid!");
        }

      m_videoInfoMapIt++;
      if (m_videoInfoMapIt == m_videoInfoMap.end ())
        {
          NS_LOG_INFO (">> EvalvidFecServer: Video streaming successfully completed!");
        }
      else 
        {
          if (m_videoInfoMapIt->second->packetInterval.GetSeconds () == 0)
            {
              m_sendEvent = Simulator::ScheduleNow (&EvalvidFecServer::Send, this);
            }
          else
            {
              m_sendEvent = Simulator::Schedule (m_videoInfoMapIt->second->packetInterval, &EvalvidFecServer::Send, this);  
            }

          /*   logging IDs in gop file if we want to keep the seq number of each GOP  */

          // if the next frame is a I-frame, we are at the end of the current GOP
          if (m_videoInfoMapIt->second->frameType == "I") 
            {
              // having ability for correcting P-frame and B-frame is not important.
              // So, I skip to add fec packet for them.

              if (m_locationEcho == 30)
              {                  
                //send echo packet for calculating RTT 
                SendEchoPacket();
                NS_LOG_DEBUG ("Send Echo packet End of GOP, m_locationEcho = " << m_locationEcho << std::endl);
              }

              if (m_locationEcho < 30 and m_locationEcho > 1)
              {
                // Send Echo in the middle of GOP
                if (m_flagEcho == false)
                {
                  SendEchoPacket();
                  NS_LOG_DEBUG ("Send Echo packet in the middle (end) GOP, m_locationEcho = " << m_locationEcho << " m_counter_B_P = " << m_counter_B_P << std::endl);
                }
                // m_flagEcho = false ;
                // m_counter_B_P = 0 ;
              }

              m_flagEcho = false ;
              m_counter_B_P = 0 ;
              m_firstSeqGop = 0;
              m_numIPackets = 0;
            }
        }
    }
  else
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: Send function was called but the Frame does not exist!");
    }
}

void
EvalvidFecServer::SendEchoPacket ()
{
  NS_LOG_INFO (">> EvalvidFecServer: SendEcho Packet function" << std::endl);

  Ptr<Packet> p = Create<Packet> ();
  m_packetId++;
  
  SeqTsEchoHeader seqEcho;
  Time currTime = Simulator::Now ();
  seqEcho.SetSeq (m_packetId);
  seqEcho.SetTsValue(currTime);
  p->AddHeader (seqEcho);
  m_socket->SendTo (p, 0, m_peerAddress);

  m_lastInputQue = m_packetId;

  // m_RTTLogFile  << std::fixed << std::setprecision (4)
  //               << currTime.ToDouble (Time::S) << std::setfill (' ') << std::setw (16)
  //               << "SentECHO " << std::setfill (' ') << std::setw (16)
  //               << "I_counter " << m_totalSentIFrame << std::endl;
  
  NS_LOG_INFO (">> EvalvidFecServer: send Echo packet _ GopNumber " << m_totalSentIFrame << std::endl);
  
  for (auto it = m_gopInfoMap.rbegin(); it != m_gopInfoMap.rend(); ++it)
  {
    if ( it->second->firstSeq == m_firstSeqGop )
    {
      it->second->sentTimeEcho = currTime;
      it->second->echoSeq = m_packetId;
      break;
    }
  }

}

void
EvalvidFecServer::CalculatingFECLevel ()
{
  NS_LOG_DEBUG ("  CalculatingFECLevel, last predicted PLR = " << m_predictedPLR << "  Current GOP No. = " << m_totalSentIFrame << std::endl);

  if (m_predictedPLR == 0 && m_totalSentIFrame == 1) // for the first GOP
  {
    m_predictedPLR = 5;       // 5% as a first value for first GOP because there is no history for PLR.
  }
  else if (m_gopInfoMap[m_totalSentIFrame-1]->PLR != 222  )     // we have a fresh PLR for the previous GOP.
  {
    if (m_gopInfoMap[m_totalSentIFrame-1]->PLR == 0  ) // Actual PLR for the previous GOP = 0%
    { 
      m_predictedPLR = ceil ((double)(m_predictedPLR) / pow(2,m_changing_value_PLR));         //  1/2 * previous predicted PLR 
      NormalizePLR();
      if (m_staticQuality == "No")
        SwitchUpQuality();  
    }
    else if (m_gopInfoMap[m_totalSentIFrame-1]->PLR > 0 and m_gopInfoMap[m_totalSentIFrame-1]->PLR <= 50  )
    {
      m_predictedPLR = m_gopInfoMap[m_totalSentIFrame-1]->PLR;    //Predicted PLR = Actual previous PLR      
      if (m_staticQuality == "No")
        SwitchUpQuality();  
    }
    else  // Actual PLR > 50 % --> we need to change the quality to Low if it is not equal to low and put the Predicted PLR  = 50%
    {
      // Reaction to Congestion 
      ReactionToCongestion();
      //return 
    }
  } 
  else     // no fresh PLR  
  {       // we need to increase predicted PLR and after that Switch up quality Module.
    if ( m_predictedPLR < m_latestRecvPLR )
      m_predictedPLR = m_latestRecvPLR + m_changing_value_PLR;
    else
      m_predictedPLR += m_changing_value_PLR;

    NormalizePLR();
    if (m_staticQuality == "No")
      SwitchUpQuality(); 
  }    
  
  NS_LOG_DEBUG ("  CalculatingFECLevel,  predicted PLR  (after fresh PLR) =  " << m_predictedPLR << " % " <<std::endl);

  if (m_usingRTT == true )      // we want to consider RTT packet 
  {
    if ( m_totalSentIFrame > 1 )    // if it is not first GOP
    {
      if ( m_gopInfoMap[m_totalSentIFrame-1]->TT  != 0.0 )      // we have a fresh TT 
      {
        //return of function

        // if we have a increase in the TT and also we have a fresh TT , we can predict that we are going into the worse condition.
        // or 
        // if we have a fresh TT while the previous one was a lost TT , we can consider that we are in a better consition.
        // S = f( t1, t2)      t1 is a TT for 2 previous GOP,    t2 is a TT for the prefious GOP  , and now we are in the current GOP which we want to sent.

        if ( m_totalSentIFrame > 2 )    // we can do this comparision if we have at least 2 sentGOP
        {
          if (m_gopInfoMap[m_totalSentIFrame-2]->TT != 0.0)      // we have a value for t1 
          {
            NS_LOG_DEBUG ("  calculating safetyFactor current : "<< m_gopInfoMap[m_totalSentIFrame-1]->TT << "  previous : " <<  m_gopInfoMap[m_totalSentIFrame-2]->TT <<std::endl);
            if ( m_gopInfoMap[m_totalSentIFrame-1]->TT > (1.1 * m_gopInfoMap[m_totalSentIFrame-2]->TT ))
            {
              m_gopInfoMap[m_totalSentIFrame - 1]->safetyFactor = m_gopInfoMap[m_totalSentIFrame-2]->TT / m_gopInfoMap[m_totalSentIFrame-1]->TT;
              SwitchUpQuality();
            }
            
          }
        }
      }
      else      // we don't have fresh TT
      {
        uint32_t consecutiveDelayedRTT = 0 ;
        consecutiveDelayedRTT = CalculateRTTLostPackets(m_totalSentIFrame);
        NS_LOG_DEBUG (" CalculatingFECLevel: consecutive delayed EchoReply packets (RTT) = " << consecutiveDelayedRTT << std::endl);

        double AdaptiveAppDelayThreshold = m_applicationThreshold * (m_aat_constant - (m_aat_factor * (double)consecutiveDelayedRTT));
        if (AdaptiveAppDelayThreshold < 0)
          AdaptiveAppDelayThreshold = 0 ;

        NS_LOG_ERROR (" CalculatingFECLevel: AdaptiveAppDelayThreshold = " << AdaptiveAppDelayThreshold << " m_aat_factor =  " << m_aat_factor  << " m_aat_constant =  " << m_aat_constant << std::endl);
        
        if ( m_latestRecvTT > AdaptiveAppDelayThreshold )
        {
          // Reaction to Congestion 
          ReactionToCongestion();
          //return
        }
        else if ( IsNormalData(m_latestRecvTT, consecutiveDelayedRTT ) == false ) // last_TT which is recieved is abnormal data  and we need to increase predicted PLR
        {
          m_predictedPLR += m_changing_value_PLR;         //pow(2,consecutiveDelayedRTT);
          NormalizePLR();
          if (m_staticQuality == "No")
            SwitchUpQuality(); 
        }
        else    // if ( IsNormalData(m_latestRecvTT) == true )  //  last_TT which is recieved is normal data
        {
          // we dont have any change on Predicted PLR and return 
          // return
          // m_predictedPLR += consecutiveDelayedRTT;
          // NormalizePLR();
        }
      }
    }
    NS_LOG_DEBUG ("  CalculatingFECLevel,  predicted PLR  (after considering status EchoReply packets) =  " << m_predictedPLR << " % " <<std::endl);
  }

  NS_LOG_DEBUG ("  CalculatingFECLevel,  predicted PLR  (END) =  " << m_predictedPLR << " % -- m_videoQualityStatus = "<< m_videoQualityStatus <<std::endl);
}

void
EvalvidFecServer::DetermineFECLevel()
{
  // we have a prediction on PLR for current GOP in m_predictedPLR variable.
  //now, we need to calculate the Packet Safe Rate:  PSR = 100 - PLR
  // calculate the FEC level for having PSR % for next GOP
  //for example. PLR = 16 %  ----> we will recieved 84% of protectedGOP 
  //  we want to calculate that how many packet we need to add to this GOP for receiveing 676 safe at the receiver
  // so     I-Packets = 676      676/x = 84/100    ---> x = 805  ---> fec level = 805-676 = 129

  // implementing static redundancy for all GOP
  if (m_staticFEC != 0)
  {
    m_predictedFecPkts = ceil ((double) (100 * m_numIPackets) / (100 - m_staticFEC)) - m_numIPackets ;
    m_predictedPLR = m_staticFEC;
  }
  else 
  {
    m_predictedFecPkts = ceil ((double) (100 * m_numIPackets) / (100 - m_predictedPLR)) - m_numIPackets ;
  }

  // The max FECLEVEL could be I_counter (based on ability of correction in RS code)
  // For example: I_counter= 10 , MaxFECptk = 10 --->  RS(20,10)---> ability for correction = (20-10)= 10 ptk
  // because we have n-k correction ability in RS when we know the location of lost(error) packets
  if (m_predictedFecPkts > m_numIPackets)
  {
    NS_FATAL_ERROR ("THE NUMBER OF PREDICTED FEC > num I-Frame packet " << std::endl);
    m_predictedFecPkts = m_numIPackets;
  }
}

void
EvalvidFecServer::AdjustingPredictedPLR()
{
  NS_LOG_DEBUG ("num Of Pkt In Queue = m_lastInputQue - m_lastOutputQue =  " << m_lastInputQue << " - " <<  m_lastOutputQue << " = " << m_lastInputQue - m_lastOutputQue 
                << "  m_numIPackets = " << m_numIPackets << "   m_predictedFecPkts =  " << m_predictedFecPkts 
                << "  0.7 * MAX_QUEUE_LEN = " << ceil(0.7 * MAX_QUEUE_LEN) << std::endl);
                
  if ( (m_lastInputQue - m_lastOutputQue) + m_numIPackets + m_predictedFecPkts > ceil(0.7 * MAX_QUEUE_LEN))
  {
    NS_LOG_DEBUG ("THE NUMBER OF PREDICTED FEC + num I-Frame packet > 0.7*available capacity of queue ..." << std::endl);
    
    if (m_videoQualityStatus == "Low")
    {
      NS_LOG_DEBUG (" Quality is low..." << std::endl);

      if ((m_lastInputQue - m_lastOutputQue) + m_numIPackets + m_predictedFecPkts < MAX_QUEUE_LEN)
      {
        // the current values for FEC level and Predicted PLR are accurate
      }
      else 
      {
        if ( MAX_QUEUE_LEN > ((m_lastInputQue - m_lastOutputQue) + m_numIPackets) )
        {
          m_predictedFecPkts = (MAX_QUEUE_LEN) - (m_lastInputQue - m_lastOutputQue) - m_numIPackets;
          m_predictedPLR = 100 - floor((double)(100 * m_numIPackets) / (m_numIPackets + m_predictedFecPkts));
        }
        else
        {          
          // we can not do anything, we should send low quality video with min pred_PLR = 5%
          m_predictedPLR = 5 ; 
          m_predictedFecPkts = ceil ((double) (100 * m_numIPackets) / (100 - m_predictedPLR)) - m_numIPackets ;
        }
      }

      m_finishingAdjustedPrePLR = true;

      NS_LOG_DEBUG (" Predicted PLR =  " << m_predictedPLR << " %  Current Gop No. " << m_totalSentIFrame
                    << "  Num of FEC packets = " << m_predictedFecPkts << "  Num of I Packets = " << m_numIPackets << " IframeIDs " 
                    << m_firstSeqGop << "-" << m_firstSeqGop + m_numIPackets-1 <<std::endl);
    }
    else if (m_videoQualityStatus == "High")
    {
      NS_LOG_DEBUG ("Quality is high --->  Change quality to Low .." << std::endl);
      m_videoQualityStatus = "Low";
    }
  }
  else
  {
    m_finishingAdjustedPrePLR = true;
    NS_LOG_DEBUG ("  DeterminFECLevel,  Predicted PLR =  " << m_predictedPLR << " %  Current Gop No. " << m_totalSentIFrame
                << "  Num of FEC packets = " << m_predictedFecPkts << "  Num of I Packets = " << m_numIPackets << " IframeIDs " 
                << m_firstSeqGop << "-" << m_firstSeqGop + m_numIPackets-1 <<std::endl);
  }
}

void 
EvalvidFecServer::ReplaceWithLowQuality ()
{            
  //This I belongs to which GOP in High video ?

  NS_LOG_DEBUG (">> EvalvidFecServer: ReplaceWithLowQuality ...frame_Id_first_high:  " << m_videoInfoMapIt->first << std::endl);
  uint32_t frame_Id_first_high = m_videoInfoMapIt->first;

  // uint16_t index = 0 ;
  uint16_t num_GOP = m_totalSentIFrame - 1 ;
  NS_LOG_DEBUG (">> EvalvidFecServer: ReplaceWithLowQuality ...num_GOP:  " << num_GOP << std::endl);
  // while ( m_highVideoGopID[index] != 0)
  // {
  //   if (m_highVideoGopID[index] == frame_Id_first_high)
  //   {
  //     num_GOP = index;
  //     break;
  //   }
  //   index++;
  // }
  //be careful about second gop with congestion because of first gop with 100%
  uint32_t frame_Id_last_high = m_highVideoGopID[num_GOP + 1]-1;       // we have first and last frame id of the current GOP in high quality
  NS_LOG_DEBUG (">> EvalvidFecServer: ReplaceWithLowQuality ...frame_Id_last_high:  " << frame_Id_last_high << std::endl);

  uint32_t frame_Id_first_low = 0;
  uint32_t frame_Id_last_low = 0 ;
  // if (num_GOP != 0)
  //{
    frame_Id_first_low = m_lowVideoGopID[num_GOP];
    if (frame_Id_first_low == 0 )
    {
      NS_FATAL_ERROR (">> EvalvidFecServer: the number of GOP in LOW and High videos is not same. \n");
      return;
    }
    frame_Id_last_low = m_lowVideoGopID[num_GOP + 1]-1;
  //}

  for ( uint32_t i = frame_Id_first_high , j = frame_Id_first_low ; i <= frame_Id_last_high; i++)
  {
    m_videoInfoMap[i]->frameSize = m_lowVideoInfoMap[j]->frameSize;
    m_videoInfoMap[i]->frameType = m_lowVideoInfoMap[j]->frameType;
    m_videoInfoMap[i]->numOfUdpPackets = m_lowVideoInfoMap[j]->numOfUdpPackets;
    m_videoInfoMap[i]->packetInterval = m_lowVideoInfoMap[j]->packetInterval;
    j++;
    if (j > frame_Id_last_low)
      j = frame_Id_last_low;
    NS_LOG_DEBUG (">> EvalvidFecServer: ReplaceWithLowQuality ... i:" << i 
                    << "   m_videoInfoMap[i]->frameSize: " << m_videoInfoMap[i]->frameSize 
                    << "   m_videoInfoMap[i]->frameType: " << m_videoInfoMap[i]->frameType 
                    << "   m_videoInfoMap[i]->numOfUdpPackets: " << m_videoInfoMap[i]->numOfUdpPackets 
                    << "   m_videoInfoMap[i]->packetInterval: " << m_videoInfoMap[i]->packetInterval 
                    << std::endl);
  }
  
  NS_LOG_DEBUG (">> EvalvidFecServer: ReplaceWithLowQuality ...finish:  " << std::endl);
}

void
EvalvidFecServer::SendVideoPackets ()
{
  //Sending the video-frame in multiples segments
  NS_LOG_DEBUG (">> EvalvidFecServer: ReplaceWithLowQuality ... m_videoInfoMapIt->first:" << m_videoInfoMapIt->first 
                    << "   m_videoInfoMapIt->second->frameSize: " << m_videoInfoMapIt->second->frameSize 
                    << "   m_videoInfoMapIt->second->frameType: " << m_videoInfoMapIt->second->frameType 
                    << "   m_videoInfoMapIt->second->numOfUdpPackets: " << m_videoInfoMapIt->second->numOfUdpPackets 
                    << "   m_videoInfoMapIt->second->packetInterval: " << m_videoInfoMapIt->second->packetInterval 
                    << std::endl);

  for (uint16_t i = 0; i < (m_videoInfoMapIt->second->frameSize / m_packetPayload); i++)
    {
      Ptr<Packet> p = Create<Packet> (m_packetPayload);
      m_packetId++;

      if (InetSocketAddress::IsMatchingType (m_peerAddress))
        {
          NS_LOG_INFO (">> EvalvidFecServer: Send "
                        << m_videoInfoMapIt->second->frameType << " frame packet at "
                        << Simulator::Now ().GetSeconds () << "s\tid: " << m_packetId
                        << "\t first seq gop\t" << m_firstSeqGop << "   + numIpackets    "
                        << m_numIPackets << "   correction ability    " << m_predictedFecPkts
                        << "\tudp\t" << p->GetSize () << " to "
                        << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << std::endl);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
        {
          // NS_LOG_INFO (">> EvalvidFecServer: Send "
          //               << m_videoInfoMapIt->second->frameType << " frame packet at "
          //               << Simulator::Now ().GetSeconds () << "s\tid: " << m_packetId
          //               << "\t first seq gop\t" << m_firstSeqGop << "   + numIpackets    "
          //               << m_numIPackets << "   correction ability    " << m_predictedFecPkts
          //               << "\tudp\t" << p->GetSize () << " to "
          //               << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << std::endl);
        }

      SeqMyRTPHeader seqRTP;
      seqRTP.SetSeq (m_packetId);
      seqRTP.SetFirstSeq (m_firstSeqGop);
      seqRTP.SetNumIpkts (m_numIPackets);

      if (m_videoInfoMapIt->second->frameType == "I")
        seqRTP.SetFrameType (SeqMyRTPHeader::I_FRAME);
      else if (m_videoInfoMapIt->second->frameType == "P")
        seqRTP.SetFrameType (SeqMyRTPHeader::P_FRAME);
      else if (m_videoInfoMapIt->second->frameType == "B")
        seqRTP.SetFrameType (SeqMyRTPHeader::B_FRAME);
// 
      seqRTP.SetFrameSeq (m_videoInfoMapIt->first);
      seqRTP.SetCorrectionAbility (m_predictedFecPkts);
      p->AddHeader (seqRTP);
      m_socket->SendTo (p, 0, m_peerAddress);

      m_senderTraceFile << std::fixed << std::setprecision (4)
                  << seqRTP.GetTs().ToDouble (Time::S) << std::setfill (' ')
                  << std::setw (16) << "id " << m_packetId << std::setfill (' ')
                  << std::setw (16) << "udp " << p->GetSize () << std::setfill (' ')
                  << std::setw (16) << "type " << m_videoInfoMapIt->second->frameType << std::setfill (' ')
                  << std::setw (16) << "frameId " << m_videoInfoMapIt->first << std::endl;

      m_lastInputQue = m_packetId;
    }

  //Sending the rest of the video-frame
  if (m_videoInfoMapIt->second->frameSize % m_packetPayload)
    {
      Ptr<Packet> p = Create<Packet> (m_videoInfoMapIt->second->frameSize % m_packetPayload);
      m_packetId++;

      if (InetSocketAddress::IsMatchingType (m_peerAddress))
        {
          // NS_LOG_INFO (">> EvalvidFecServer: Send "
          //               << m_videoInfoMapIt->second->frameType << " frame packet at "
          //               << Simulator::Now ().GetSeconds () << "s\tid: " << m_packetId
          //               << "\t first seq gop\t" << m_firstSeqGop << "   + numIpackets    "
          //               << m_numIPackets << "   correction ability    " << m_predictedFecPkts
          //               << "\tudp\t" << p->GetSize () << " to "
          //               << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << std::endl);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
        {
          // NS_LOG_INFO (">> EvalvidFecServer: Send "
          //               << m_videoInfoMapIt->second->frameType << " frame packet at "
          //               << Simulator::Now ().GetSeconds () << "s\tid: " << m_packetId
          //               << "\t first seq gop\t" << m_firstSeqGop << "   + numIpackets    "
          //               << m_numIPackets << "   correction ability    " << m_predictedFecPkts
          //               << "\tudp\t" << p->GetSize () << " to "
          //               << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << std::endl);
        }

      SeqMyRTPHeader seqRTP;
      seqRTP.SetSeq (m_packetId);
      seqRTP.SetFirstSeq (m_firstSeqGop);
      seqRTP.SetNumIpkts (m_numIPackets);

      if (m_videoInfoMapIt->second->frameType == "I")
        seqRTP.SetFrameType (SeqMyRTPHeader::I_FRAME);
      else if (m_videoInfoMapIt->second->frameType == "P")
        seqRTP.SetFrameType (SeqMyRTPHeader::P_FRAME);
      else if (m_videoInfoMapIt->second->frameType == "B")
        seqRTP.SetFrameType (SeqMyRTPHeader::B_FRAME);

      seqRTP.SetFrameSeq (m_videoInfoMapIt->first);
      seqRTP.SetCorrectionAbility (m_predictedFecPkts);
      p->AddHeader (seqRTP);
      m_socket->SendTo (p, 0, m_peerAddress);

      m_senderTraceFile << std::fixed << std::setprecision (4)
                        << seqRTP.GetTs().ToDouble (Time::S) << std::setfill (' ')
                        << std::setw (16) << "id " << m_packetId << std::setfill (' ')
                        << std::setw (16) << "udp " << p->GetSize () << std::setfill (' ')
                        << std::setw (16) << "type " << m_videoInfoMapIt->second->frameType << std::setfill (' ')
                        << std::setw (16) << "frameId " << m_videoInfoMapIt->first << std::endl;

      m_lastInputQue = m_packetId;
    }
}

void
EvalvidFecServer::SendFECPackets ()
{
  for (uint16_t i = 0; i < m_predictedFecPkts; i++)
    {
      //sending one FEC packet.
      Ptr<Packet> p = Create<Packet> (m_packetPayload);
      m_packetId++;

      if (InetSocketAddress::IsMatchingType (m_peerAddress))
        {
          // NS_LOG_INFO (">> EvalvidFecServer: Send FEC packet at "
          //               << Simulator::Now ().GetSeconds () << "s\tid: " << m_packetId
          //               << "\t first seq gop\t" << m_firstSeqGop << " to "
          //               << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << std::endl);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
        {
          // NS_LOG_INFO (">> EvalvidFecServer: Send FEC packet at "
          //               << Simulator::Now ().GetSeconds () << "s\tid: " << m_packetId
          //               << "\t first seq gop\t" << m_firstSeqGop << " to "
          //               << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << std::endl);
        }

      m_sentFecTraceFile << std::fixed << std::setprecision(4) << Simulator::Now().ToDouble(Time::S)
                     << std::setfill(' ') << std::setw(16) <<  "id " << m_packetId
                     << std::setfill(' ') << std::setw(16) <<  "udp " << p->GetSize()
                     << std::setfill(' ') << std::setw(16) <<  "IframeIDs " << m_firstSeqGop << "-" << m_firstSeqGop + m_numIPackets-1 
                     << std::endl;

      SeqMyRTPHeader seqRTP;
      seqRTP.SetSeq (m_packetId);
      seqRTP.SetFirstSeq (m_firstSeqGop);
      seqRTP.SetNumIpkts (m_numIPackets);
      seqRTP.SetFrameType (SeqMyRTPHeader::FEC_FRAME);
      seqRTP.SetFrameSeq (m_videoInfoMapIt->first);       // save the frame seq of I frame for FEC packets
      seqRTP.SetCorrectionAbility (m_predictedFecPkts);
      p->AddHeader (seqRTP);
      m_socket->SendTo (p, 0, m_peerAddress);

      m_lastInputQue = m_packetId;
    }
}
////////////////////////////////////////////////////////////////////

bool 
EvalvidFecServer::IsNormalData(double latestRecvTT, uint32_t numConsDelayedPkt) 
{
  NS_LOG_DEBUG ("IsNormalData()  .. latestRecvTT =  " << latestRecvTT << "  numConsDelayedPkt = " << numConsDelayedPkt << "  m_totalSentIFrame = " << m_totalSentIFrame << std::endl);

	double sumDiff = 0;
	double std_dev = 0;

  uint32_t firstIndex = 0;
  uint32_t lastIndex = 0;
  uint32_t history = 5;    //get a mean for last 5 secs,  and m_totalSentIFrame = current GOP that we want to send

  // if (m_totalSentIFrame <= history )   
  //   firstIndex = 1;  
  // else 
  //   firstIndex = m_totalSentIFrame - history;  

  if (m_totalSentIFrame-1 > numConsDelayedPkt+1 )
  {
    lastIndex = m_totalSentIFrame-1-numConsDelayedPkt-1 ;     // we want to reach last index before latest recv TT.
    if (lastIndex >= history)
      firstIndex = lastIndex - history + 1 ;
    else 
      firstIndex = 1 ;
  }
  else 
  {
    lastIndex = 1 ;
    firstIndex = 1;
  }

  NS_LOG_DEBUG ("IsNormalData() : first Index =  " << firstIndex << " ,  lastIndex .." << lastIndex << std::endl);

  double weightedMean = 0; 
  double a = 0.15;

  for (uint32_t i = firstIndex; i <= lastIndex ; i++)
  {
    if ( m_gopInfoMap[i]->TT != 0.0 )   // if tt == 0 , we work with a tt that we have not recieved a reply YET.
    {
      weightedMean = (1-a)*(weightedMean) + (a)*(m_gopInfoMap[i]->TT);
    }
  }  
	
	for (uint32_t i = firstIndex; i <= lastIndex; i++) 
  {
		sumDiff += pow(m_gopInfoMap[i]->TT - weightedMean, 2);
	}

	std_dev = sqrt((double)(sumDiff/ (double)(lastIndex - firstIndex + 1)));

  NS_LOG_DEBUG (">> EvalvidFecServer: IsNormalData .. [ " << firstIndex << " , " << lastIndex << "] ... weightedMean + 2*std_dev =  "
                        << weightedMean << " + " <<  2*std_dev << " =  " << weightedMean + 2*std_dev
                        << "   latestRecvTT = " << latestRecvTT << std::endl);

	if ( weightedMean + 2*std_dev < latestRecvTT ) 
  {
    return false;     // abnormal data based on standard deviation of last 5 received TTs.
  }
  else
  {
    return true;      // normal data based on standard deviation 
  }
}

uint32_t
EvalvidFecServer::CalculateRTTLostPackets(uint32_t lastGopNo)
{
  uint32_t counter = 0 ;
  for (auto it = m_gopInfoMap.rbegin(); it != m_gopInfoMap.rend(); ++it)
    {
      if ( it->first == lastGopNo)
      {
        continue;
      }
      else if ( it->first < lastGopNo && it->second->TT == 0.0 )
      {
        counter++;
      }
      else
      {
        break;
      }
    }  
  return counter; 
}

void
EvalvidFecServer::SwitchUpQuality(void)
{
  NS_LOG_DEBUG ("SwitchUpQuality() start ..." << std::endl);

  if ( m_videoQualityStatus == "Low")      //Low to High
  {
    if (m_usingBandwidth == true)
    {
      NS_LOG_DEBUG ("SwitchUpQuality() -> Using bandwidth ..." << std::endl);
      if ( m_gopInfoMap[m_totalSentIFrame-1]->bandwidthRTCP != 0.0 or m_gopInfoMap[m_totalSentIFrame-1]->bandwidthEchoReply != 0.0)      // we have fresh PLR or fresh TT
      { 
        uint32_t frame_Id_first_high = m_videoInfoMapIt->first;
        uint32_t frame_Id_last_high = m_highVideoGopID[(m_totalSentIFrame-1)+1]-1; 
        uint32_t num_of_video_pkt = 0 ;

        // calculating the number of I-frame packets
        if (m_videoInfoMap[frame_Id_first_high]->frameSize % m_packetPayload)
          num_of_video_pkt = (m_videoInfoMap[frame_Id_first_high]->frameSize / m_packetPayload) + 1;
        else
          num_of_video_pkt = m_videoInfoMap[frame_Id_first_high]->frameSize / m_packetPayload;
        
        // calculating the number of FEC packets
        num_of_video_pkt += ceil ((double) (100 * num_of_video_pkt) / (100 - m_predictedPLR)) - num_of_video_pkt ;

        // calculating the number of B/P frame packets
        for ( uint32_t i = frame_Id_first_high+1 ; i <= frame_Id_last_high; i++)
        {
          if (m_videoInfoMap[i]->frameSize % m_packetPayload)
            num_of_video_pkt += (m_videoInfoMap[i]->frameSize / m_packetPayload) + 1;
          else
            num_of_video_pkt += m_videoInfoMap[i]->frameSize / m_packetPayload;
        }
        double required_bandwith_high = num_of_video_pkt / 1 ;      // packet/seconds

        NS_LOG_DEBUG (">> SwitchUpQuality...required bandwidth for high quality :  " << required_bandwith_high << std::endl);
        NS_LOG_DEBUG (">> SwitchUpQuality...required bandwidth for low quality based on RTCP :  " << m_gopInfoMap[m_totalSentIFrame-1]->bandwidthRTCP << std::endl);
        NS_LOG_DEBUG (">> SwitchUpQuality...required bandwidth for low quality based on Echo Reply :  " << m_gopInfoMap[m_totalSentIFrame-1]->bandwidthEchoReply << std::endl);

        double used_Bandwidth = 0.0 ;
        if ( m_gopInfoMap[m_totalSentIFrame-1]->bandwidthRTCP != 0.0 ) 
        {
          if ( m_gopInfoMap[m_totalSentIFrame - 1]->safetyFactor != 0.0 )
          {
            used_Bandwidth = m_gopInfoMap[m_totalSentIFrame-1]->bandwidthRTCP * m_gopInfoMap[m_totalSentIFrame - 1]->safetyFactor;
            NS_LOG_DEBUG (">> SwitchUpQuality...  Using safety factor ...." << std::endl);
          }
          else
            used_Bandwidth = m_gopInfoMap[m_totalSentIFrame-1]->bandwidthRTCP; 
          
          if ( used_Bandwidth >= required_bandwith_high )
          {
            m_videoQualityStatus = "High";
            NS_LOG_DEBUG (">> SwitchUpQuality...  change quality based on Bandwidthfrom RTCP ...   m_videoQualityStatus =  " << m_videoQualityStatus << std::endl);
          }          
        }  
      }
      else
      {
        NS_LOG_DEBUG (">> SwitchUpQuality...Don't have a fresh PLR or Fresh TT ---  m_videoQualityStatus =  " << m_videoQualityStatus << std::endl);
      }
    }
    else
    {
      NS_LOG_DEBUG ("SwitchUpQuality() -> DONT Using bandwidth comparison ..." << std::endl);
      m_videoQualityStatus = "High";
    }
  }
}

void
EvalvidFecServer::ReactionToCongestion(void)
{
  NS_LOG_DEBUG ("ReactionToCongestion()  .." << std::endl);

  m_predictedPLR = 50;
  if (m_staticQuality == "No")
  {
    if ( m_videoQualityStatus == "High" )      //high to low
    {
      m_videoQualityStatus = "Low";      
    }
  }
  else
  {
    m_videoQualityStatus = m_staticQuality;
  }
}

void
EvalvidFecServer::NormalizePLR(void)
{
   //if predicted PLR more than 50% , it means that we want to protect the FEC packets. so it is not reseanable. 
  //e.g. we want to sned 100 I-packets, based on RS code, N-100 = the error correction ability. 
  //So, if we send 200 ( 100 I + 100 FEC), this means that if we can receive 100 packet from 200 , we can say that we recover all 100 I-packets.
  // if we send 300 ( 100 I + 200 FEC), based on RS , in this situation we protect FEC packets also. means: we send FEC for protecting FEC. it is not reseanable.
  NS_LOG_DEBUG ("NormalizePLR()  .." << std::endl);
  
  if ( m_predictedPLR < 1)
  {
    m_predictedPLR = 1;
  }
  else if (m_predictedPLR > 50 )
  {
    m_predictedPLR = 50;
  }
  NS_LOG_DEBUG ("  predicted PLR  (after normalizing ) =  " << m_predictedPLR 
                << " %  m_videoQualityStatus = " << m_videoQualityStatus << std::endl);
}

} // Namespace ns3
