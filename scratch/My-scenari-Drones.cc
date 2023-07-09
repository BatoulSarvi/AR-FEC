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

#include "ns3/core-module.h"

#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"

#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-error-rate-model.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/ssid.h"
#include "ns3/traffic-control-module.h"

#include "ns3/aodv-helper.h"
#include "ns3/netanim-module.h"

#include "ns3/wifi-net-device.h"
#include "ns3/config-store.h"

#include "ns3/evalvid-client-server-helper.h"
#include "ns3/evalvid-fec-client-server-helper.h"

// #include "ns3/core-module.h"
// #include "ns3/netsimulyzer-module.h"

#include "ns3/test.h"

// infrastructure Network Topology
//
//    sample for runing :
//    ./waf --run "scratch/My-scenari-Drones.cc --withFEC=true --myErrorModel=Table --inputPropLoss=2" > 1_result/myScenarioDrone.out 2>&1
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyScenarioExample");

Ptr<YansWifiPhy>
GetYansWifiPhyPtr (const NetDeviceContainer &nc)
{
  Ptr<WifiNetDevice> wnd = nc.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<WifiPhy> wp = wnd->GetPhy ();
  return wp->GetObject<YansWifiPhy> ();
}

void
PrintAttributesIfEnabled (bool enabled)
{
  if (enabled)
    {
      ConfigStore outputConfig;
      outputConfig.ConfigureAttributes ();
    }
}

int
main (int argc, char *argv[])
{
  bool printAttributes = true;
  bool verbose = true;
  bool tracing = false;
  bool shortGuardInterval = false;
  uint32_t staNode = 1; /* number of Station nodes */
  uint32_t apNode = 1; /* number of AccessPoint nodes */
  // std::string phyRate = "HtMcs7";       /* Physical layer bitrate. */
  uint16_t stepInterval = 6;   // 6 is default value
  double simulationTime =
      stepInterval * 13; //seconds  13 = the number of steps for moving around the square

  uint32_t numCells = 9;
  uint32_t maxDist =
      400; // maxDist = 7   ->  Velocity = 47.36 km/h    time between 2 steps = 0.25   3.29 = dist between each steps
  // D = 100   -> V = 42.42 km/h     time between 2 steps = 4s    D between 2 steps = 47.14
  uint32_t payloadSizeFEC = 1442; //bytes for UDP packet based on my headers for fec.
  uint32_t payloadSizeNoFEC = 1460; //bytes for UDP packet based on udp header.
  bool withFEC = true; // true = we want to have FEC mechanism.
  string quality_noFEC = "High"; // high== send high quality file , low == send low quality file
  double appThreshold = 1.5;
  string myErrorModel = "Table"; //"Error Models: Yans, Nist, Table"
  string StaticQualityFEC = "No"; // No , High , Low
  uint32_t staticFEC = 0;  // if we want to have a fixed FEC, we need to set this variable
  double AAT_Factor = 0.8; 
  double AAT_Constant = 1.0 ;
  uint32_t changing_value_PLR = 2;

  // enum MyPropModel
  // {
  //   RANDOM_PROPAGATION,
  //   FRIIS_PROPAGATION,
  //   LOG_DISTANCE_PROPAGATION,
  //   TWO_RAY_GROUND_PROPAGATION
  // };

  uint32_t inputPropLoss = 1; ///  Random: 0, Friis:1, LogDistance: 2, TworayGround: 3
  uint32_t m_80211mode = 1; ///< 80211 mode    1=802.11n-2.4GHz; 2=802.11n-5.0GHz
  uint32_t m_LocationEcho = 15;  ///< it should be a number from 1 to 30.
  uint32_t m_seed = 3474632792; ///<  seed

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nStaNode", "Number of staNode", staNode);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("withFEC", "Enable FEC mechanism", withFEC);
  cmd.AddValue ("quality_noFEC", "Enable FEC mechanism", quality_noFEC);
  cmd.AddValue ("myErrorModel", "Error Models: Yans, Nist, Table", myErrorModel);
  cmd.AddValue ("maxDist", "distance between sender and receiver", maxDist);
  cmd.AddValue ("StaticQualityFEC", "select specific quality for FEC mechanism", StaticQualityFEC);
  cmd.AddValue ("inputPropLoss",
                "Propagation Loss Model (Random: 0, Friis:1, LogDistance: 2, TworayGround: 3) ",
                inputPropLoss);
  cmd.AddValue ("80211Mode", "1=802.11n-2.4GHz; 2=802.11n-5.0GHz", m_80211mode);
  cmd.AddValue ("staticFEC", "if we want to have a fixed FEC, we need to set this variable", staticFEC);
  cmd.AddValue ("appThreshold", "max acceptable delay", appThreshold);
  cmd.AddValue ("LocationEcho", "the location of sending eco packet in each GOP", m_LocationEcho);
  cmd.AddValue ("seed", "seed", m_seed);
  cmd.AddValue ("AAT_Factor", "AAT_Factor", AAT_Factor);
  cmd.AddValue ("AAT_Constant", "AAT_Constant", AAT_Constant);
  cmd.AddValue ("changing_value_PLR", "Increasing and decreasing value for PLR", changing_value_PLR);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
    }

  RngSeedManager::SetSeed (m_seed); // by default, ns-3 simulations use a fixed seed = 1
  std::cout << "seed = " << m_seed << std::endl;

  /* --------------------------
   Set up Legacy Channel 
  -----------------------------*/
  YansWifiChannelHelper wifiChannel; //= YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay (
      "ns3::ConstantSpeedPropagationDelayModel"); // we don't use random delay model, because we want to receive packet in order that we send.

  // frequency
  double freq = 0.0;
  double referenceLoss = 0.0;

  if (m_80211mode == 1)
    {
      // 802.11n - 2.4 GHz
      freq = 2.4e9;
      referenceLoss = 40.045997;
    }
  else if (m_80211mode == 2)
    {
      // 802.11n- 5.0 GHz
      freq = 5.15e9;
      referenceLoss = 46.6777;
    }
  else
    {
      //error
      std::cout << "error in 802.11 mode" << std::endl;
      exit (1);
    }

  switch (inputPropLoss)
    {
    case 0:
      std::cout << "SARA ------ PropagationLossModel = RANDOM_PROPAGATION " << std::endl;
      wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel");
      break;

    case 1:
      std::cout << "SARA ------ PropagationLossModel = FRIIS_PROPAGATION " << std::endl;
      wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency",
                                      DoubleValue (freq));
      break;

    case 2:
      std::cout << "SARA ------ PropagationLossModel = LOG_DISTANCE_PROPAGATION " << std::endl;
      wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel", "ReferenceLoss",
                                      DoubleValue (referenceLoss), "Exponent", DoubleValue (2.0));
      // Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue (referenceLoss));
      break;

    case 3:
      std::cout << "SARA ------ PropagationLossModel = TWO_RAY_GROUND_PROPAGATION " << std::endl;
      wifiChannel.AddPropagationLoss (
          "ns3::TwoRayGroundPropagationLossModel", "Frequency", DoubleValue (freq), "HeightAboveZ",
          DoubleValue (50.0)); // set antenna height to 1.5m above z coordinate
      break;

    default:
      std::cout << "Wrong Popagation Loss Model." << std::endl;
      exit (1);
    }

  /* --------------------------
  Setup Physical Layer 
  -----------------------------*/

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel (wifiChannel.Create ());

  if (myErrorModel == "Yans")
    {
      std::cout << "SARA ------ ErrorRateModel =  " << myErrorModel << std::endl;
      wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
    }
  else if (myErrorModel == "Nist")
    {
      std::cout << "SARA ------ ErrorRateModel =  " << myErrorModel << std::endl;
      wifiPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
    }
  else if (myErrorModel == "Table")
    {
      std::cout << "SARA ------ ErrorRateModel =  " << myErrorModel << std::endl;
      wifiPhy.SetErrorRateModel ("ns3::TableBasedErrorRateModel");
    }

  WifiHelper wifi;

  // default values for 802.11n 2.4Ghz:     ChannelNumber = 1 , ChannelWidth = 20, Frequency = 2412
  // default values for 802.11n 5Ghz:       ChannelNumber = 36, ChannelWidth = 20, Frequency = 5180
  // for changing default valuses --->  Go to ../source/ns-3.33/src/wifi/test/wifi-test.cc

  if (m_80211mode == 1)
    {
      // 802.11n - 2.4 GHz
      wifi.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);
      std::cout << "SARA ------ wifi standard =  WIFI_STANDARD_80211n_2_4GHZ " << std::endl;
    }
  else if (m_80211mode == 2)
    {
      // 802.11n- 5.0 GHz
      wifi.SetStandard (WIFI_STANDARD_80211n_5GHZ);
      std::cout << "SARA ------ wifi standard =  WIFI_STANDARD_80211n_5GHZ " << std::endl;
    }
  else
    {
      //error
      std::cout << "error in 802.11 mode" << std::endl;
      exit (1);
    }

  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager", "RtsCtsThreshold",
                                UintegerValue (65535));

  // Set guard interval
  Config::Set (
      "/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HtConfiguration/ShortGuardIntervalSupported",
      BooleanValue (shortGuardInterval));


  /* --------------------------
  Setup Station Nodes 
  -----------------------------*/

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (staNode);
  NodeContainer wifiApNode;
  wifiApNode.Create (apNode);

  WifiMacHelper wifiMac;
  Ssid ssid = Ssid ("ns-3-ssid-80211n-2.4");

  wifiMac.SetType ("ns3::StaWifiMac", /*  AdhocWifiMac, StaWifiMac, ApWifiMac   */
                   "Ssid", SsidValue (ssid));
  // "ActiveProbing", BooleanValue (false));    /* the “ActiveProbing” Attribute is set to false.
  // This means that probe requests will not be sent by MACs created by this helper.  */

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (wifiPhy, wifiMac, wifiStaNodes);

  wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  //"EnableBeaconJitter", BooleanValue (false)); // so as to be sure that first beacon arrives quickly

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNode);

  Ptr<YansWifiPhy> phySta;
  phySta = GetYansWifiPhyPtr (staDevices);
  // DataRate dataRate = DataRate (phySta->GetMode (7).GetDataRate (phySta->GetChannelWidth ()));
  // std::cout << "SARA ------ data rate of station: " << dataRate << std::endl;

  if (m_80211mode == 1)
    {
      // 802.11n - 2.4 GHz
      // We expect channel 1, width 20, frequency 2412
      NS_ASSERT (phySta->GetChannelNumber () == 1);
      NS_ASSERT (phySta->GetChannelWidth () == 20);
      NS_ASSERT (phySta->GetFrequency () == 2412);
      PrintAttributesIfEnabled (printAttributes);
    }
  else if (m_80211mode == 2)
    {
      // 802.11n- 5.0 GHz
      // We expect channel 36, width 20, frequency 5180
      NS_ASSERT (phySta->GetChannelNumber () == 36);
      NS_ASSERT (phySta->GetChannelWidth () == 20);
      NS_ASSERT (phySta->GetFrequency () == 5180);
      PrintAttributesIfEnabled (printAttributes);
    }
  else
    {
      //error
      std::cout << "error in 802.11 mode" << std::endl;
      exit (1);
    }

  /* --------------------------
  Setup Mobility Model 
  -----------------------------*/

  MobilityHelper mobility;

  mobility.SetMobilityModel ("ns3::WaypointMobilityModel");
  mobility.Install (wifiStaNodes);

  double x = 0.0;
  double y = 0.0;
  double z = 50.0;
  double step = sqrt (2 * maxDist * maxDist) / sqrt (numCells);

  Ptr<WaypointMobilityModel> staWaypointMobility =
      wifiStaNodes.Get (0)->GetObject<WaypointMobilityModel> ();
  staWaypointMobility->AddWaypoint (Waypoint (Seconds (0.00), Vector (x, y, z))); //initialPosition

  staWaypointMobility->AddWaypoint (Waypoint (Seconds (1 * stepInterval), Vector (x + step, y, z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (2 * stepInterval), Vector (x + (2 * step), y, z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (3 * stepInterval), Vector (x + (3 * step), y, z)));

  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (4 * stepInterval), Vector (x + (3 * step), y + step, z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (5 * stepInterval), Vector (x + (3 * step), y + (2 * step), z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (6 * stepInterval), Vector (x + (3 * step), y + (3 * step), z)));

  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (7 * stepInterval), Vector (x + (2 * step), y + (3 * step), z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (8 * stepInterval), Vector (x + step, y + (3 * step), z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (9 * stepInterval), Vector (x, y + (3 * step), z)));

  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (10 * stepInterval), Vector (x, y + (2 * step), z)));
  staWaypointMobility->AddWaypoint (
      Waypoint (Seconds (11 * stepInterval), Vector (x, y + step, z)));
  staWaypointMobility->AddWaypoint (Waypoint (Seconds (12 * stepInterval), Vector (x, y, z)));

  
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> positionAllocAp = CreateObject<ListPositionAllocator> ();
  positionAllocAp->Add (
      Vector (sqrt (2 * maxDist * maxDist) / 2, sqrt (2 * maxDist * maxDist) / 2, z));
  mobility.SetPositionAllocator (positionAllocAp);
  mobility.Install (wifiApNode);

  
  /* --------------------------
  Internet stack
  ----------------------------*/
  InternetStackHelper internetStack;
  internetStack.Install (wifiApNode);
  internetStack.Install (wifiStaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign (staDevices);
  apNodeInterface = address.Assign (apDevice);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* --------------------------
  UDP flow
 ----------------------------*/

  std::cout << "SARA ------ Application Threshold: " << appThreshold << std::endl;
  std::cout << "SARA ------ Max Distance: " << maxDist << std::endl;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (withFEC == true)
    {
      std::cout << "SARA ------ Run with FEC: " << withFEC << std::endl;
      std::cout << "SARA ------ Run FEC , StaticQuality: " << StaticQualityFEC << std::endl;
      std::cout << "SARA ------ Run FEC , LocationEcho: " << m_LocationEcho << std::endl;
      std::cout << "SARA ------ Run FEC , AAT_Factor: " << AAT_Factor << std::endl;
      std::cout << "SARA ------ Run FEC , AAT_Constant: " << AAT_Constant << std::endl;
      std::cout << "SARA ------ Run FEC , changing_value_PLR: " << changing_value_PLR << std::endl;

      //
      // Create one EvalvidFecServer applications on drone node
      //
      uint16_t port = 4000;
      EvalvidFecServerHelper server (port);
      server.SetAttribute ("SenderTraceFilename", StringValue ("St_4k30fps_2min.st"));
      server.SetAttribute ("SenderTraceFilenameLowQuality", StringValue ("St_4k30fps_720.st"));    //St_4k30fps_720
      server.SetAttribute ("SenderDumpFilename", StringValue ("1_result/SentVideoPktInfo"));
      server.SetAttribute ("GOPInfoFileName", StringValue ("1_result/GopInfoFile"));
      server.SetAttribute ("SentFecTraceFileName", StringValue ("1_result/SentFECInfo"));
      server.SetAttribute ("PacketPayload", UintegerValue (payloadSizeFEC));
      server.SetAttribute ("AAT_Factor",DoubleValue (AAT_Factor));   // factor for application threshold = 0.8
      server.SetAttribute ("AAT_Constant",DoubleValue (AAT_Constant));   // AAT_Constant for application threshold = 1.0
      server.SetAttribute ("changing_value_PLR", UintegerValue (changing_value_PLR));
      //for static FEC
      server.SetAttribute ("StaticFec", UintegerValue (staticFEC));
      //for using RTT
      server.SetAttribute ("UsingRTT", BooleanValue (true));
      //for using Bandwidth Comparision
      server.SetAttribute ("BandwidthComparison", BooleanValue (true));
      //for Static Quality
      server.SetAttribute ("StaticQuality", StringValue (StaticQualityFEC)); // No, High , Low
      server.SetAttribute ("LocationEcho", UintegerValue (m_LocationEcho)); // it should be a number between 1 to 30
      server.SetAttribute ("AppThreshold",
                           DoubleValue (appThreshold)); //application threshold = 1.5s
      ApplicationContainer clientApp = server.Install (wifiStaNodes.Get (0));
      clientApp.Start (Seconds (0.0));
      clientApp.Stop (Seconds (simulationTime + 3.0));

      //
      // Create one EvalvidFecClient application
      //
      EvalvidFecClientHelper client (staNodeInterface.GetAddress (0), port);
      client.SetAttribute ("ReceiverDumpFilename", StringValue ("1_result/RecvVideoPktInfo"));
      client.SetAttribute ("CorrectedIframeFileName", StringValue ("1_result/CorrectedIframe"));
      client.SetAttribute ("RecvFECPktsFileName", StringValue ("1_result/RecvFECInfo"));
      client.SetAttribute ("RecvIPktsFileName", StringValue ("1_result/RecvIpktInfo"));
      client.SetAttribute ("AppThreshold",
                           DoubleValue (appThreshold)); //application threshold = 1.5s
      // ApplicationContainer clientApp = server.Install (wifiStaNodes.Get (0));
      ApplicationContainer serverApp ;
      serverApp.Add(client.Install (wifiApNode.Get (0)));
      serverApp.Start (Seconds (0.0));
      serverApp.Stop (Seconds (simulationTime + 3.0));
    }
  else
    {
      std::cout << "SARA ------ Run without FEC: " << withFEC << std::endl;
      
      // Create one EvalvidServer applications on drone node
      //
      uint16_t port = 4000;
      EvalvidServerHelper server (port);
      if (quality_noFEC == "High")
        {
          std::cout << "SARA ------ Send quality :  " << quality_noFEC << std::endl;
          server.SetAttribute ("SenderTraceFilename", StringValue ("St_4k30fps_2min.st"));
        }
      else
        {
          std::cout << "SARA ------ Send quality :  " << quality_noFEC << std::endl;
          server.SetAttribute ("SenderTraceFilename", StringValue ("St_4k30fps_720.st"));  //St_4k30fps_720
        }
      server.SetAttribute ("SenderDumpFilename", StringValue ("1_result/SentVideoPktInfo"));
      server.SetAttribute (
          "PacketPayload",
          UintegerValue (payloadSizeNoFEC)); //default = 1460 is equal to with FEC approaches
      ApplicationContainer clientApp = server.Install (wifiStaNodes.Get (0));
      clientApp.Start (Seconds (0.0));
      clientApp.Stop (Seconds (simulationTime + 3.0));

      //
      // Create one EvalvidClient application
      //
      EvalvidClientHelper client (staNodeInterface.GetAddress (0), port);
      client.SetAttribute ("ReceiverDumpFilename", StringValue ("1_result/RecvVideoPktInfo"));
      client.SetAttribute ("AppThreshold",
                           DoubleValue (appThreshold)); //application threshold = 1.5s
      ApplicationContainer serverApp;
      serverApp.Add(client.Install (wifiApNode.Get (0)));
      serverApp.Start (Seconds (0.0));
      serverApp.Stop (Seconds (simulationTime + 3.0));
    }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if (tracing == true)
    {
      wifiPhy.EnablePcapAll ("1_result/myScenarioDrone"); //Packet Capture.
    }

  //Network Animation using NetAnim.
  AnimationInterface anim ("1_result/myScenarioDrone.xml");
  anim.SetMaxPktsPerTraceFile (1000000);

  //Ascii Trace Metrics can be processed using Tracemetrics Software.
  // AsciiTraceHelper ascii;
  // wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("1_result/myScenarioDrone.tr"));


  Simulator::Stop (Seconds (simulationTime + 5.0));
  Simulator::Run ();

  /*
  std::cout << " Total Sent Packets: " << DynamicCast<EvalvidServer> (clientApp.Get (0))->GetSentPacket () << std::endl;
  std::cout << " Total Received Packets: " << DynamicCast<EvalvidClient> (serverApp.Get (0))->GetReceived () << std::endl;
  std::cout << " Total Lost Packets: " << DynamicCast<EvalvidClient> (serverApp.Get (0))->GetLost() << std::endl;

*/  

  std::cout << "SARA ------ Finish --- " << std::endl;

  Simulator::Destroy ();
  return 0;
}
