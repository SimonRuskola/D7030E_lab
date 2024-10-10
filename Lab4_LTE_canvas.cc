
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/propagation-loss-model.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Simple Exmaple script for LTE+EPC System. It instantiates one eNodeB 
 * which attaches one UE, and a remote host starts a downlink flow for UE.
 */
NS_LOG_COMPONENT_DEFINE ("LteExampleSim");

int
main (int argc, char *argv[])
{


  //uint16_t numberOfNodes = 2;
  double simTime = 20;
  uint32_t nofue = 1;
  std::string outputpath = "";

  // Command line arguments
  CommandLine cmd;
  


  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("nofue", "the number of ues", nofue);
  cmd.AddValue("outputpath", "The path of the stats outputs", outputpath);
  
  cmd.Parse(argc, argv);

  int degree;

  std::string antennaType[3] = {"IsotropicAntennaModel", "CosineAntennaModel", "ParabolicAntennaModel"};

  for(int i = 0; i < 3; i++){
    for(degree = 0; degree < 360; degree += 30){

      std::string degreeString = std::to_string(degree);

  //Define the path for the generated trace files
  if (outputpath!="")
  {
    Config::SetDefault ("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename", StringValue (outputpath+"/DlRlcStats"+antennaType[i]+"_degree_"+degreeString +".txt"));
    Config::SetDefault ("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename", StringValue (outputpath+"/UlRlcStats"+antennaType[i]+"_degree_"+degreeString +".txt"));
    Config::SetDefault ("ns3::RadioBearerStatsCalculator::DlPdcpOutputFilename", StringValue (outputpath+"/DlPdcpStats"+antennaType[i]+"_degree_"+degreeString +".txt"));
    Config::SetDefault ("ns3::RadioBearerStatsCalculator::UlPdcpOutputFilename", StringValue (outputpath+"/UlPdcpStats"+antennaType[i]+"_degree_"+degreeString +".txt"));
    Config::SetDefault ("ns3::MacStatsCalculator::DlOutputFilename", StringValue (outputpath+"/DlMacStats"+antennaType[i]+"_degree_"+degreeString +".txt"));
    Config::SetDefault ("ns3::MacStatsCalculator::UlOutputFilename", StringValue (outputpath+"/UlMacStats"+antennaType[i]+"_degree_"+degreeString +".txt")); 
  }
 

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();
     
  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);  

 
    
    

    

  

  //Configure the LTE+EPC system. Don't touch these before you already understand the whole LTE system and ns-3 source codes
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));

  switch(i){
    case 0:
      lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
      break;
    case 1:
      lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
      break;
    case 2:
      lteHelper->SetEnbAntennaModelType ("ns3::ParabolicAntennaModel");
      break;
  }
  if(i == 1 || i == 2){
    lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (degree));
  }
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (100 + 18000));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (50));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (50));
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::TwoRayGroundPropagationLossModel"));
                        
  

  //Define P-Gateway in EPC
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create a single RemoteHost
  
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(1);
  ueNodes.Create(1); //Default: there is only 1 UE in this example

  // Install Mobility Model
  
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);
  
  Ptr<MobilityModel> MM;
  
  //Define the location of eNodeB (0,0,0)
  MM = enbNodes.Get(0) -> GetObject<MobilityModel>();
  MM -> SetPosition(Vector3D(0,0,1));
  
  //Define the location of the first UE
  MM = ueNodes.Get(0) -> GetObject<MobilityModel>();
  MM -> SetPosition(Vector3D(2000,0,1)); // distance between UE and eNodeB is 5 metres
  
  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);


  
 

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  //Ipv4InterfaceContainer ueIpIface;
  //ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));// Assign IP address to UEs, and install applications
 // ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs.Get (0)));// Assign IP address to UEs, and install applications
    Ptr<NetDevice> ueLteDevice = ueLteDevs.Get (0);
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice));
    // set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting;
        ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(0)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
 
  // Attach one UE per eNodeB
lteHelper->Attach (ueLteDevs, enbLteDevs.Get(0));



// Install and start applications on UE and remote host

 uint16_t dlPort = 1000;
   Ptr<Ipv4> ueIpv4;
  int32_t interface;
  Ipv4Address ueAddr;
  ApplicationContainer onOffApp;
  
    
    
    ueIpv4 = ueNodes.Get(0)->GetObject<Ipv4> ();
    interface =  ueIpv4->GetInterfaceForDevice (ueNodes.Get(0)->GetDevice (0)->GetObject <LteUeNetDevice> ());
    NS_ASSERT (interface >= 0);
    NS_ASSERT (ueIpv4->GetNAddresses (interface) == 1);
    ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();
    
    
    OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(ueAddr, dlPort)); //OnOffApplication, UDP traffic, Please refer the ns-3 API
    onOffHelper.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5000]"));
    onOffHelper.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate("50.0Mbps"))); //Traffic Bit Rate
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1024));
    onOffApp.Add(onOffHelper.Install(remoteHost));  
      
    //LTE QoS bearer    
    EpsBearer bearer (EpsBearer::NGBR_VOICE_VIDEO_GAMING);
  //  lteHelper->ActivateEpsBearer (ueLteDevs.Get(0), bearer, EpcTft::Default ()); 
      lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(0), bearer, EpcTft::Default ());     
     // epcHelper->ActivateEpsBearer (ueLteDevs.Get(0), bearer, EpcTft::Default ());  

  lteHelper->EnableTraces ();
  //phy.EnablePcap ("LAB1", staDevices .Get (nWifi-1), true);


  p2ph.EnablePcap ("LTE_"+antennaType[i]+"degree"+degreeString, internetDevices.Get (1), true);
  

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();


  }}
  return 0;

}

