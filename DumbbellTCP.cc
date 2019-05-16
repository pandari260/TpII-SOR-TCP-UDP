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
#include "ns3/onoff-application.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/data-rate.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"


using namespace ns3;



int main (int argc, char *argv[]){

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (512));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("100KB/s"));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (100));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));

  std::string animFile = "dumbbell-animation.xml" ;  // Name of file for animation output
 

  // Crear helpers 
  PointToPointHelper p2pRouter;
  p2pRouter.SetDeviceAttribute  ("DataRate", StringValue ("100KB/s"));
  p2pRouter.SetChannelAttribute("Delay", StringValue("1ms"));

  PointToPointHelper p2pEmisor;
  p2pEmisor.SetDeviceAttribute    ("DataRate", StringValue ("100KB/s"));
  p2pEmisor.SetChannelAttribute ("Delay", StringValue ("1ms"));

  PointToPointHelper p2pReceptor;
  p2pReceptor.SetDeviceAttribute    ("DataRate", StringValue ("100KB/s"));
  p2pReceptor.SetChannelAttribute ("Delay", StringValue ("1ms"));

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
  appsReceptor.Add(sink.Install(redDumbbell.GetLeft(1))); 
  appsReceptor.Start (Seconds (0.0));
  appsReceptor.Stop (Seconds (55.0));


  PacketSinkHelper sinkUdp ("ns3::UdpSocketFactory",  InetSocketAddress (Ipv4Address::GetAny (), puerto));
  ApplicationContainer appsReceptorUdp = sinkUdp.Install(redDumbbell.GetLeft(2));
  appsReceptorUdp.Start (Seconds (0.0));
  appsReceptorUdp.Stop (Seconds (55.0));

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
  appEmisor.Add (emisorHelper.Install (redDumbbell.GetRight (1)));

  AddressValue remoteAddress2 (InetSocketAddress (redDumbbell.GetLeftIpv4Address (2), puerto));
  emisorHelperUdp.SetAttribute ("Remote", remoteAddress2);
  //appEmisor.Add (emisorHelperUdp.Install (redDumbbell.GetRight (2)));

  appEmisor.Start(Seconds(0.0));
  appEmisor.Stop(Seconds(50.0));

  AsciiTraceHelper ascii;
  p2pRouter.EnableAsciiAll (ascii.CreateFileStream ("dumbbellPCAP/dumbellR.tr"));
  p2pRouter.EnablePcapAll ("dumbbellPCAP/dumbellR");

  redDumbbell.BoundingBox (1, 1, 100, 100);
  // Create the animation object and configure for specified output
  AnimationInterface anim (animFile);
  anim.EnablePacketMetadata (); // Optional
  anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (55 )); // Optional
  // Set up the actual simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Stop(Seconds(55.0));
  Simulator::Destroy ();
  return 0;

}
