#include <iostream>
#include <csignal>

#include <easylogging++.h>

#include "config.hpp"
#include "network.hpp"

INITIALIZE_EASYLOGGINGPP

using namespace std;
using namespace LocNet;



function<void(int)> mySignalHandlerFunc;

void signalHandler(int signal)
    { mySignalHandlerFunc(signal); }


int main(int argc, const char *argv[])
{
    try
    {
        bool configCreated = Config::Init(argc, argv);
        if (! configCreated)
            { return 1; }

        const Config &config( Config::Instance() ); 
        if ( config.versionRequested() )
        {
            cout << "Internet of People - Location based network " << config.version() << endl;
            return 0;
        }
        
        
        el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "%datetime %level %msg (%fbase:%line)");
        el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename, config.logPath());
        el::Loggers::reconfigureAllLoggers(el::Level::Trace, el::ConfigurationType::ToStandardOutput, "false");
        
        // Initialize server components
        NodeInfo myNodeInfo( config.myNodeInfo() );
        LOG(INFO) << "Initializing server with node info: " << myNodeInfo;
        
        shared_ptr<ISpatialDatabase> geodb( new SpatiaLiteDatabase(
            myNodeInfo, config.dbPath(), config.dbExpirationPeriod() ) );

        TcpStreamConnectionFactory *connFactPtr = new TcpStreamConnectionFactory();
        shared_ptr<INodeConnectionFactory> connectionFactory(connFactPtr);
        shared_ptr<Node> node( new Node(geodb, connectionFactory) );

        LOG(INFO) << "Connecting node to the network";
        shared_ptr<IProtoBufRequestDispatcherFactory> nodeDispatcherFactory(
            new StaticDispatcherFactory( shared_ptr<IProtoBufRequestDispatcher>(
                new IncomingNodeRequestDispatcher(node) ) ) );
        ProtoBufDispatchingTcpServer nodeTcpServer(
            myNodeInfo.contact().nodePort(), nodeDispatcherFactory );
        
        connFactPtr->detectedIpCallback( [node](const Address &addr)
            { node->DetectedExternalAddress(addr); } );
        node->EnsureMapFilled();

        LOG(INFO) << "Serving local and client interfaces";
        shared_ptr<IProtoBufRequestDispatcherFactory> localDispatcherFactory(
            new LocalServiceRequestDispatcherFactory(node) );
        shared_ptr<IProtoBufRequestDispatcherFactory> clientDispatcherFactory(
            new StaticDispatcherFactory( shared_ptr<IProtoBufRequestDispatcher>(
                new IncomingClientRequestDispatcher(node) ) ) );
        
        ProtoBufDispatchingTcpServer localTcpServer(
            config.localServicePort(), localDispatcherFactory );
        ProtoBufDispatchingTcpServer clientTcpServer(
            myNodeInfo.contact().clientPort(), clientDispatcherFactory );

        // Set up signal handlers to stop on Ctrl-C and further events
        bool ShutdownRequested = false;

        mySignalHandlerFunc = [&ShutdownRequested, &localTcpServer, &nodeTcpServer, &clientTcpServer] (int)
        {
            ShutdownRequested = true;
            localTcpServer.Shutdown();
            nodeTcpServer.Shutdown();
            clientTcpServer.Shutdown();
        };
        
        signal(SIGINT,  signalHandler);
        signal(SIGTERM, signalHandler);
        
        // start threads for periodic db maintenance (relation renewal and expiration) and discovery
        thread dbMaintenanceThread( [&ShutdownRequested, &config, node]
        {
            while (! ShutdownRequested)
            {
                try
                {
                    this_thread::sleep_for( config.dbMaintenancePeriod() );
                    node->RenewNodeRelations();
                    node->ExpireOldNodes();
                }
                catch (exception &ex)
                    { LOG(ERROR) << "Maintenance thread failed: " << ex.what(); }
            }
        } );
        dbMaintenanceThread.detach();
        
        thread discoveryThread( [&ShutdownRequested, &config, node]
        {
            while (! ShutdownRequested)
            {
                try
                {
                    this_thread::sleep_for( config.discoveryPeriod() );
                    node->DiscoverUnknownAreas();
                }
                catch (exception &ex)
                    { LOG(ERROR) << "Periodic discovery thread failed: " << ex.what(); }
            }
        } );
        discoveryThread.detach();
        
        // TODO we should do something more useful here instead of polling,
        //      maybe run io_service::run, does it block CTRL-C and other signals?
        // Avoid exiting from this thread
        while (! ShutdownRequested)
            { this_thread::sleep_for( chrono::milliseconds(50) ); }
        
        LOG(INFO) << "Shutting down location-based network";
        return 0;
    }
    catch (exception &e)
    {
        LOG(ERROR) << "Failed with exception: " << e.what();
    }
}
