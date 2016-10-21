#include "geonet.hpp"

using namespace std;



NodeProfile::NodeProfile() :
    _id(), _ipv4Address(), _ipv4Port(0), _ipv6Address(), _ipv6Port(0) {}

NodeProfile::NodeProfile(const NodeProfile& other) :
    _id(other._id),
    _ipv4Address(other._ipv4Address), _ipv4Port(other._ipv4Port),
    _ipv6Address(other._ipv6Address), _ipv6Port(other._ipv6Port) {}

NodeProfile::NodeProfile(const NodeId& id,
                         const Ipv4Address& ipv4Address, TcpPort ipv4Port,
                         const Ipv6Address& ipv6Address, TcpPort ipv6Port) :
    _id(id),
    _ipv4Address(ipv4Address), _ipv4Port(ipv4Port),
    _ipv6Address(ipv6Address), _ipv6Port(ipv6Port) {}

const NodeId& NodeProfile::id() const { return _id; }
const Ipv4Address& NodeProfile::ipv4Address() const { return _ipv4Address; }
TcpPort NodeProfile::ipv4Port() const { return _ipv4Port; }
const Ipv6Address& NodeProfile::ipv6Address() const { return _ipv6Address; }
TcpPort NodeProfile::ipv6Port() const { return _ipv6Port; }

bool NodeProfile::operator==(const NodeProfile& other) const
{
    return  _id == other._id &&
            _ipv4Address == other._ipv4Address &&
            _ipv4Port == other._ipv4Port &&
            _ipv6Address == other._ipv6Address &&
            _ipv6Port == other._ipv6Port;
}



GpsLocation::GpsLocation(const GpsLocation &location) :
    _latitude( location.latitude() ), _longitude( location.longitude() ) {}
    
GpsLocation::GpsLocation(double latitude, double longitude) :
    _latitude(latitude), _longitude(longitude) {}

double GpsLocation::latitude()  const { return _latitude; }
double GpsLocation::longitude() const { return _longitude; }



NodeLocation::NodeLocation(const NodeProfile &profile, const GpsLocation &location) :
    _profile(profile), _location(location) {}

NodeLocation::NodeLocation(const NodeProfile &profile, double latitude, double longitude) :
    _profile(profile), _location(latitude, longitude) {}

const NodeProfile& NodeLocation::profile() const { return _profile; }
double NodeLocation::latitude()  const { return _location.latitude(); }
double NodeLocation::longitude() const { return _location.longitude(); }



GeographicNetwork::GeographicNetwork(shared_ptr<SpatialDatabase> spatialDb) :
    _spatialDb(spatialDb) {}

const unordered_map<ServerType,ServerInfo,EnumHasher>& GeographicNetwork::servers() const
    { return _servers; }

    
void GeographicNetwork::RegisterServer(ServerType serverType, const ServerInfo& serverInfo)
{
    auto it = _servers.find(serverType);
    if ( it != _servers.end() ) {
        throw runtime_error("Server type is already registered");
    }
    _servers[serverType] = serverInfo;
}


void GeographicNetwork::AcceptNeighbor(NodeLocation profile)
{
    // TODO
}

void GeographicNetwork::ExchangeNodeProfile(NodeLocation profile)
{
    // TODO
}

vector<NodeLocation> GeographicNetwork::GetClosestNodes(const GpsLocation& location, double radiusKm, uint16_t maxNodeCount) const
{
    // TODO
    return vector<NodeLocation>();
}

vector<NodeLocation> GeographicNetwork::GetNeighbourHood() const
{
    // TODO
    return vector<NodeLocation>();
}

vector<NodeLocation> GeographicNetwork::GetRandomNodes(uint16_t maxNodeCount, bool includeNeighbours) const
{
    // TODO
    return vector<NodeLocation>();
}

void GeographicNetwork::RenewNodeProfile(NodeLocation profile)
{
    // TODO
}

