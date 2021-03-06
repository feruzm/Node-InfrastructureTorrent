#ifndef SERVER_H_
#define SERVER_H_

#include <mh/mhd/MHD.h>

#include <string>
#include <atomic>

namespace torrent_node_lib {
class Sync;
}

class Server: public mh::mhd::MHD {
public:
    Server(const torrent_node_lib::Sync &sync, int port, std::atomic<int> &countRunningThreads) 
        : sync(sync)
        , port(port)
        , countRunningThreads(countRunningThreads)
        , isStoped(false)
    {}
    
    ~Server() override {}
    
    bool run(int thread_number, Request& mhd_req, Response& mhd_resp) override;
    
    bool init() override;
    
private:
    
    const torrent_node_lib::Sync &sync;
    
    const int port;

    std::atomic<int> &countRunningThreads;
    
    std::atomic<bool> isStoped;
        
};

#endif // SERVER_H_
