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

#include "evalvid-fec-client-server-helper.h"
// #include "ns3/evalvid-fec-client.h"
// #include "ns3/evalvid-fec-server.h"
//#include "ns3/evalvid-trace-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

EvalvidFecServerHelper::EvalvidFecServerHelper ()
{}

EvalvidFecServerHelper::EvalvidFecServerHelper (uint16_t port)
{
  m_factory.SetTypeId (EvalvidFecServer::GetTypeId ());
  //SetAttribute ("RemoteAddress", Ipv4AddressValue (ip));
  SetAttribute ("Port", UintegerValue (port));
}

void
EvalvidFecServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
EvalvidFecServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      m_server = m_factory.Create<EvalvidFecServer> ();
      node->AddApplication (m_server);
      apps.Add (m_server);

    }
  return apps;
}

Ptr<EvalvidFecServer>
EvalvidFecServerHelper::GetServer (void)
{
  return m_server;
}

EvalvidFecClientHelper::EvalvidFecClientHelper ()
{}

EvalvidFecClientHelper::EvalvidFecClientHelper (Ipv4Address ip,uint16_t port)
{
  m_factory.SetTypeId (EvalvidFecClient::GetTypeId ());
  SetAttribute ("RemoteAddress", Ipv4AddressValue (ip));
  SetAttribute ("RemotePort", UintegerValue (port));
}

void
EvalvidFecClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
EvalvidFecClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<EvalvidFecClient> client = m_factory.Create<EvalvidFecClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

/*
EvalvidTraceServerHelper::EvalvidTraceServerHelper ()
{}

EvalvidTraceServerHelper::EvalvidTraceServerHelper (Ipv4Address address, uint16_t port, std::string filename)
{
  m_factory.SetTypeId (EvalvidTraceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", Ipv4AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("TraceFilename", StringValue (filename));
}

void
EvalvidTraceServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
EvalvidTraceServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<EvalvidTraceClient> client = m_factory.Create<EvalvidTraceClient> ();
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}
*/
} // namespace ns3
