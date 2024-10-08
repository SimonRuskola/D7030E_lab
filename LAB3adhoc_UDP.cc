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


           
        
using namespace ns3;



NS_LOG_COMPONENT_DEFINE ("LAB3");


int payloadSizes[] = {300, 700, 1200};

int
main (int argc, char *argv[])
{
  uint32_t nWifi;

  for (nWifi = 3; nWifi <= 6; nWifi += 1){
    for (long unsigned int i = 0; i < sizeof(payloadSizes)/sizeof(payloadSizes[0]); i++){
      int payloadSize = payloadSizes[i];
    
    

  bool verbose = true;

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
  //TODO
  // Create nodes
  NodeContainer stas; 
  stas.Create (nWifi);
 /////////////////////////////Wi-Fi part///////////////////////////// 

  //TODO
  // Create WifiChannel with PropagationLossModel and SpeedPropagationDelayModel
  Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();  //create a pointer for channel object
  Ptr<TwoRayGroundPropagationLossModel> lossModel = CreateObject<TwoRayGroundPropagationLossModel> (); //create a pointer for propagation loss model
  wifiChannel->SetPropagationLossModel (lossModel); // install propagation loss model
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();       
  wifiChannel->SetPropagationDelayModel (delayModel); // install propagation delay model
  


  //maybe need these
  //Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  //Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  
 
  
  
  //Physical layer of WiFi
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  
      
  //TODO
  //Attach WiFi channel to physical layer
  phy.SetChannel(wifiChannel);
  phy.Set("TxPowerEnd", DoubleValue(16));
  phy.Set("TxPowerStart", DoubleValue(16));
  phy.Set("RxSensitivity", DoubleValue(-80));
  //  phy.Set("CcaMode1Threshold", DoubleValue(-99));
  phy.Set("ChannelNumber", UintegerValue(7));


  //TODO
  //Create WiFi helper and set standart and RemoteStationManager
  WifiHelper wifi = WifiHelper();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  Ssid ssid = Ssid ("wifi-default");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue ("DsssRate1Mbps"), "ControlMode",StringValue ("DsssRate1Mbps"));
  
  //Mac layer
  WifiMacHelper mac = WifiMacHelper();



/////////////////////////////Devices///////////////////////////// 


  //TODO
  // Create network devices
  // Attach devices and all parts of WiFi system and Nodes
  
  //mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, stas);



 /////////////////////////////Deployment///////////////////////////// 
//TODO
// Create mobility model with constant positions and deploy all devices in a line with 200m distance between them; Z coordinate for all of them should be 1m!! 
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  
  for (uint32_t i = 0; i < nWifi; ++i)
    {
      positionAlloc->Add (Vector (i*200.0, 0.0, 1.0));
      mobility.SetPositionAllocator (positionAlloc);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (stas.Get(i));
    }


/////////////////////////////Stack of protocols///////////////////////////// 

  
//  Enable OLSR routing
   OlsrHelper olsr;

 //  Install the routing protocol
   Ipv4ListRoutingHelper list;
   list.Add (olsr, 10);

  // Set up internet stack
  InternetStackHelper stack;
  stack.SetRoutingHelper (list);
  stack.Install (stas);
  Ipv4AddressHelper address;

 /////////////////////////////Ip addresation/////////////////////////////  
  address.SetBase ("10.1.1.0", "255.255.255.0");
  //TODO
  //Create Ipv4InterfaceContainer 
  // assign IP addresses to WifiDevices into Ipv4InterfaceContainer
  Ipv4InterfaceContainer wifiInterfaces;
  wifiInterfaces = address.Assign (staDevices);


/////////////////////////////Application part///////////////////////////// 
 
   uint16_t dlPort = 1000; //Port number
    
    //Sending application on the first station
    
    ApplicationContainer onOffApp;
    OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(wifiInterfaces.GetAddress (nWifi-1), dlPort)); //OnOffApplication, UDP traffic,
    onOffHelper.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5000]"));
    onOffHelper.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate("10.0Mbps"))); //Traffic Bit Rate
    onOffHelper.SetAttribute("PacketSize", UintegerValue(payloadSize)); // Packet size
    onOffApp.Add(onOffHelper.Install(stas.Get(0)));  
 
    
  //Opening receiver socket on the last station
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (stas.Get (nWifi-1), tid);
  InetSocketAddress local = InetSocketAddress (wifiInterfaces.GetAddress (nWifi-1), dlPort);
  bool ipRecvTos = true;
  recvSink->SetIpRecvTos (ipRecvTos);
  bool ipRecvTtl = true;
  recvSink->SetIpRecvTtl (ipRecvTtl);
  recvSink->Bind (local);

/////////////////////////////Application part///////////////////////////// 

  Simulator::Stop (Seconds (100.0));
/////////////////////////////PCAP tracing/////////////////////////////   
  //TODO 
  //Enable PCAP tracing for all devices
  std::string payloadSizeString = std::to_string(payloadSize);
  std::string nWifiString = std::to_string(nWifi);
  phy.EnablePcap ("LAB3_WIFI_STA_UDP_nwifi_"+nWifiString+"_payloadSize_"+payloadSizeString, stas, true); 

  Simulator::Run ();
  Simulator::Destroy ();

  }}
return 0;
};
