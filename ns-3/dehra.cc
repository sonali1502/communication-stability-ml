#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ns2-mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DehradunVANET");

int main(int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    int nVehicles = 1199;
    NodeContainer nodes;
    nodes.Create(nVehicles);

    // SUMO mobility
    Ns2MobilityHelper ns2mobility("/home/hdoop/sumo-1.12.0/tools/2025-08-22-16-01-36/mobility.tcl");
    ns2mobility.Install();

    // Wi-Fi 802.11p/WAVE
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());

    Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default();
    NqosWaveMacHelper mac = NqosWaveMacHelper::Default();
    mac.SetType("ns3::OcbWifiMac"); // ✅ Required for 802.11p/WAVE

    NetDeviceContainer devices = wifi80211p.Install(phy, mac, nodes);

    // Internet stack
    InternetStackHelper internet;
    OlsrHelper olsr;
    internet.SetRoutingHelper(olsr);
    internet.Install(nodes);

    Ipv4AddressHelper address;
   address.SetBase("10.1.0.0", "255.255.0.0");


    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // UDP Echo Server/Client
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(600.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(nVehicles - 1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(600.0));

    // Flow monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(600.0));
    Simulator::Run();
    monitor->SerializeToXmlFile("dehra_flowmon.xml", true, true);
    Simulator::Destroy();
    return 0;
}

