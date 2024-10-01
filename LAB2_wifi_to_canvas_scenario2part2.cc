/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mobility-model.h"
#include "ns3/csma-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/internet-module.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/wifi-mac-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/olsr-module.h"

#include <iostream>


// Default Network Topology
//
//  Wifi 10.1.1.0
//           
//  *            *   
//  |            |     
// n0(transm)   n1(receiver)    


           
        
using namespace ns3;



NS_LOG_COMPONENT_DEFINE ("LAB1");

std::vector<std::string> bitrates = {
    "DsssRate1Mbps"
};

std::vector<int> seeds {
    100
};


int
main (int argc, char *argv[])
{

  for (auto &seed : seeds) {
  for (auto &bitrate : bitrates) {
    for (int i = 0; i <= 1; i++)
    {
   
    if(i==1){
      Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
    }
  
 
  RngSeedManager::SetSeed (seed);
  bool verbose = true;
  uint32_t nWifi = 6;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.Parse (argc,argv);

  if (nWifi > 18)
    {
      std::cout << "Number of wifi nodes " << nWifi << 
                   " specified exceeds the mobility bounding box" << std::endl;
      exit (1);
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

/////////////////////////////Nodes/////////////////////////////  
  
  NodeContainer ap;
  NodeContainer stas; 
  ap.Create (1);
  stas.Create (2);
  
 /////////////////////////////Wi-Fi part///////////////////////////// 

  Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();  //create a pointer for channel object
  Ptr<TwoRayGroundPropagationLossModel> lossModel = CreateObject<TwoRayGroundPropagationLossModel> (); //create a pointer for propagation loss model
  wifiChannel->SetPropagationLossModel (lossModel); // install propagation loss model
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();       
  wifiChannel->SetPropagationDelayModel (delayModel); // install propagation delay model
  

 
 
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));  // RTS/CTS disabled
  
 // disable fragmentation
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (wifiChannel);
  phy.Set("TxPowerEnd", DoubleValue(16));
  phy.Set("TxPowerStart", DoubleValue(16));
  phy.Set("RxSensitivity", DoubleValue(-80));
//  phy.Set("CcaMode1Threshold", DoubleValue(-99));
  phy.Set("ChannelNumber", UintegerValue(7));

  WifiHelper wifi = WifiHelper();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  
  Ssid ssid = Ssid ("wifi-default");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue (bitrate), "ControlMode",StringValue (bitrate));
  
  WifiMacHelper mac = WifiMacHelper();

  // setup ap.
  NetDeviceContainer apDevices;
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  apDevices=wifi.Install (phy, mac, ap);
  // setup stas.
 mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, stas);


/////////////////////////////Deployment///////////////////////////// 
      double d_1 = 251.18864315;
      //double d_1 = 10;
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
      //positionAlloc->Add (Vector (0.0, 0.0, 1.0));
      //positionAlloc->Add (Vector (502.377286302, 0.0, 1.0));
      positionAlloc->Add (Vector (-d_1, 0.0, 1.0));
      positionAlloc->Add (Vector (d_1, 0.0, 1.0));
      

      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (stas);
      Ptr<ListPositionAllocator> positionAllocAP = CreateObject<ListPositionAllocator> ();
      positionAllocAP->Add (Vector (0, 0.0, 1.0));
      //positionAllocAP->Add (Vector (251.188643151, 0.0, 1.0));
      mobility.SetPositionAllocator (positionAllocAP);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (ap);
  
/////////////////////////////Stack of protocols///////////////////////////// 

  // Set up internet stack
  InternetStackHelper stack;
  stack.Install (ap);
  stack.Install (stas);
  Ipv4AddressHelper address;

 /////////////////////////////Ip addresation/////////////////////////////  
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer wifiInterfaces;
  Ipv4InterfaceContainer wifiAPInterface;
   
  wifiAPInterface  = address.Assign (apDevices);  
  wifiInterfaces = address.Assign (staDevices);
  
/////////////////////////////Application part///////////////////////////// 
   
  uint16_t dlPort = 1000;
  uint16_t dlPort2 = 2000;
   
   ApplicationContainer onOffApp;
   ApplicationContainer onOffApp2;

    OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(wifiAPInterface.GetAddress (0), dlPort)); //OnOffApplication, UDP traffic, Please refer the ns-3 API
    onOffHelper.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5000]"));
    onOffHelper.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate("20.0Mbps"))); //Traffic Bit Rate
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1000));

    
    OnOffHelper onOffHelper2("ns3::UdpSocketFactory", InetSocketAddress(wifiAPInterface.GetAddress (0), dlPort2)); //OnOffApplication, UDP traffic, Please refer the ns-3 API
    onOffHelper2.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5000]"));
    onOffHelper2.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper2.SetAttribute("DataRate", DataRateValue(DataRate("20.0Mbps"))); //Traffic Bit Rate
    onOffHelper2.SetAttribute("PacketSize", UintegerValue(1000));
    
    onOffApp.Add(onOffHelper.Install(stas.Get(0)));  
    onOffApp2.Add(onOffHelper2.Install(stas.Get(1)));  
    
 
    
    //Receiver socket on ap 1
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (ap.Get (0), tid);
  Ptr<Socket> recvSink2 = Socket::CreateSocket (ap.Get (0), tid);
  InetSocketAddress local = InetSocketAddress (wifiAPInterface.GetAddress (0), dlPort);
  InetSocketAddress local2 = InetSocketAddress (wifiAPInterface.GetAddress (0), dlPort2);
  bool ipRecvTos = true;
  recvSink->SetIpRecvTos (ipRecvTos);
  recvSink2->SetIpRecvTos (ipRecvTos);
  bool ipRecvTtl = true;
  recvSink->SetIpRecvTtl (ipRecvTtl);
  recvSink2->SetIpRecvTtl (ipRecvTtl);
  recvSink->Bind (local);
  recvSink2->Bind (local2);

////////////////////////////////////////////////////////////


  Simulator::Stop (Seconds (100.0));
/////////////////////////////PCAP tracing/////////////////////////////   
  std::string seed_string = std::to_string(seed);
  if(i==1){
    phy.EnablePcap ("LAB2_SCENARIO2PART2_seed_"+seed_string+"_Bitrate_"+bitrate+"_RTS_WIFI_STA", stas, true); 
    phy.EnablePcap ("LAB2_SCENARIO2PART2_seed_"+seed_string+"_Bitrate_"+bitrate+"_RTS_WIFI_AP", ap, true); 
    NS_LOG_UNCOND ("LAB2_SCENARIO2PART2_seed_"+seed_string+"_Bitrate_"+bitrate+"_RTS");

  }
  else{
    phy.EnablePcap ("LAB2_SCENARIO2PART2_seed_"+seed_string+"_Bitrate_"+bitrate+"_WIFI_STA", stas, true); 
    phy.EnablePcap ("LAB2_SCENARIO2PART2_seed_"+seed_string+"_Bitrate_"+bitrate+"_WIFI_AP", ap, true); 
    NS_LOG_UNCOND ("LAB2_SCENARIO2PART2_seed_"+seed_string+"_Bitrate_"+bitrate);

  }
  
  
  Simulator::Run ();
  Simulator::Destroy ();

  }} }
return 0;
};
