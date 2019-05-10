#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/tcp-option-winscale.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/random-variable-stream.h"
#include "ns3/tcp-socket-base.h"

using namespace ns3;


int main (int argc, char *argv[]){
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (512));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("500kb/s"));


  
  uint32_t    nLeftLeaf = 5;
  uint32_t    nRightLeaf = 5;
  uint32_t    nLeaf = 0; // If non-zero, number of both left and right
  std::string animFile = "dumbbell-animation.xml" ;  // Name of file for animation output

  CommandLine cmd;
  cmd.AddValue ("nLeftLeaf", "Number of left side leaf nodes", nLeftLeaf);
  cmd.AddValue ("nRightLeaf","Number of right side leaf nodes", nRightLeaf);
  cmd.AddValue ("nLeaf",     "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);

  cmd.Parse (argc,argv);
  if (nLeaf > 0)
    {
      nLeftLeaf = nLeaf;
      nRightLeaf = nLeaf;
    }

    // Crear helpers 
    PointToPointHelper p2pRouter;
    p2pRouter.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
    p2pRouter.SetChannelAttribute ("Delay", StringValue ("1ms"));

    PointToPointHelper p2pEmisor;
    p2pEmisor.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
    p2pEmisor.SetChannelAttribute   ("Delay", StringValue ("1ms"));

    PointToPointHelper p2pReceptor;
    p2pReceptor.SetDeviceAttribute    ("DataRate", StringValue ("5Mbps"));
    p2pReceptor.SetChannelAttribute   ("Delay", StringValue ("1ms"));

    PointToPointDumbbellHelper redDumbbell (3, p2pEmisor,3, p2pReceptor,p2pRouter);

     // Install Stack
    InternetStackHelper stack;
    redDumbbell.InstallStack (stack);

    // Assign IP Addresses
    redDumbbell.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                         Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));
    // and setup ip routing tables to get total ip-level connectivity.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  //crear aplicaciones receptor


  uint16_t puerto=50000;

  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), puerto));

  ApplicationContainer appsReceptor;
  appsReceptor.Add(sink.Install(redDumbbell.GetLeft(0)));
  //appsReceptor.Add(sink.Install(redDumbbell.GetLeft(1))); 
  appsReceptor.Start (Seconds (0.0));
  appsReceptor.Stop (Seconds (1.0));

  //Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (redDumbbell.GetLeft(0), TcpSocketFactory::GetTypeId ());
  //ns3TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));




  PacketSinkHelper sinkUdp ("ns3::UdpSocketFactory",  InetSocketAddress (Ipv4Address::GetAny (), puerto));
  ApplicationContainer appsReceptorUdp = sinkUdp.Install(redDumbbell.GetLeft(2));
  appsReceptorUdp.Start (Seconds (0.0));
  appsReceptorUdp.Stop (Seconds (1.0));


  //crear aplicaciones emisoras
  OnOffHelper emisorHelper ("ns3::TcpSocketFactory", Address ());
  emisorHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  emisorHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  OnOffHelper emisorHelperUdp ("ns3::UdpSocketFactory", Address ());
  emisorHelperUdp.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  emisorHelperUdp.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  
  ApplicationContainer appEmisor;


  // Create an on/off app sending packets to the same leaf right side
      AddressValue remoteAddress0 (InetSocketAddress (redDumbbell.GetLeftIpv4Address (0), puerto));
      emisorHelper.SetAttribute ("Remote", remoteAddress0);
      appEmisor.Add (emisorHelper.Install (redDumbbell.GetRight (0)));

      AddressValue remoteAddress1 (InetSocketAddress (redDumbbell.GetLeftIpv4Address (1), puerto));
      emisorHelper.SetAttribute ("Remote", remoteAddress1);
      //appEmisor.Add (emisorHelper.Install (redDumbbell.GetRight (1)));

      AddressValue remoteAddress2 (InetSocketAddress (redDumbbell.GetLeftIpv4Address (2), puerto));
      emisorHelperUdp.SetAttribute ("Remote", remoteAddress2);
     // appEmisor.Add (emisorHelperUdp.Install (redDumbbell.GetRight (2)));

      appEmisor.Start(Seconds(0.0));
      appEmisor.Stop(Seconds(1.0));
    // Set the bounding box for animation


  AsciiTraceHelper ascii;
  p2pReceptor.EnableAsciiAll (ascii.CreateFileStream ("dumbbellPCAP/dumbellR.tr"));
  p2pReceptor.EnablePcapAll ("dumbbellPCAP/dumbellR");


    redDumbbell.BoundingBox (1, 1, 100, 100);
    // Create the animation object and configure for specified output
    AnimationInterface anim (animFile);
    anim.EnablePacketMetadata (); // Optional
    anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
    // Set up the actual simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  std::cout << "RTT: " << tshark -r dumbbellPCAP/dumbbellR.pcap -Y 'ip.addr == AA.BB.CC.DD' -T fields -e tcp.analysis.ack_rtt
  Simulator::Destroy ();
  return 0;

}