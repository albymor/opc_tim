/// @author Alexander Rykovanov 2013
/// @email rykovanov.as@gmail.com
/// @brief Remote Computer implementaion.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

#include <opc/ua/client/client.h>
#include <opc/ua/node.h>
#include <opc/ua/subscription.h>

#include <opc/common/logger.h>
#include <opc/ua/model.h>

#include <iostream>
#include <stdexcept>
#include <thread>

#include <../src/core/model_impl.h>
#include <opc/ua/model.h>
#include <opc/ua/services/attributes.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdint.h>

#define FILE_PATH "misure.txt"

using namespace OpcUa;

#ifdef _RPI_CCR
#pragma message "REMEMBER TO ENABLE CCR KERNEL MODULE"
static inline uint32_t GetTimeStamp(void)
{
  uint32_t cc = 0;
  __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0":"=r" (cc));
  return cc;
}
#else
long GetTimeStamp() { //timestamp in us
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(long)1000000+tv.tv_usec;
}
#endif

void produce_measures_file(long *measures_vector, int n_measures, std::string filename)
{ //file delle misure in us
    FILE *file_pointer = fopen(filename.c_str(), "w");
    int i;
    //fprintf(file_pointer, "%s", "measures_vector = [ ");
    fprintf(file_pointer, "%d", *(measures_vector++));
    for (i = 0; i < n_measures - 1; i++)
    {
        fprintf(file_pointer, "%s %d", ", ", *(measures_vector++));
    }
    //fprintf(file_pointer, "%s", " ];");
    fclose(file_pointer);
}

class SubClient : public SubscriptionHandler
{
    void DataChange(uint32_t handle, const Node &node, const Variant &val, AttributeId attr) override
    {
        std::cout << "Received DataChange event, value of Node " << node << " is now: " << val.ToString() << std::endl;
    }
};

int main(int argc, char **argv)
{
    auto logger = spdlog::stderr_color_mt("client");
    //logger->set_level(spdlog::level::debug);
    try
    {
        //std::string endpoint = "opc.tcp://localhost:4840";
        //std::string endpoint = "opc.tcp://user:password@192.168.56.101:48030";
        std::string endpoint = "opc.tcp://127.0.0.1:4840/freeopcua/server/";
        //std::string endpoint = "opc.tcp://localhost:53530/OPCUA/SimulationServer/";
        //std::string endpoint = "opc.tcp://localhost:48010";

        if (argc > 1)
        {
            endpoint = argv[1];
        }

        logger->info("Connecting to: {}", endpoint);

        OpcUa::UaClient client(logger);
        client.Connect(endpoint);

        //get Root node on server
        OpcUa::Node root = client.GetRootNode();
        logger->info("Root node is: {}", root);

        //get and browse Objects node
        logger->info("Child of objects node are:");
        Node objects = client.GetObjectsNode();

        for (OpcUa::Node node : objects.GetChildren())
        {
            logger->info("    {}", node);
        }

        //get a node from standard namespace using objectId
        logger->info("NamespaceArray is:");
        OpcUa::Node nsnode = client.GetNode(ObjectId::Server_NamespaceArray);
        OpcUa::Variant ns = nsnode.GetValue();

        for (std::string d : ns.As<std::vector<std::string>>())
        {
            logger->info("    {}", d);
        }

        OpcUa::Node myvar;
        OpcUa::Node myobject;
        OpcUa::Node mymethod;

        std::vector<std::string> varpath{"Objects", "2:NewObject", "2:MyVariable"};


        //CORE------------------------------------------------------------------------------------------------------
        int n_measures = 100000;
        int i;
        long measures_vector[n_measures];
        myvar = root.GetChild(varpath);
        for (i = 0; i < n_measures; i++)
        {
            if (!(i % 1000))
                logger->info(i);
            long init = GetTimeStamp();
            OpcUa::Variant val = myvar.GetValue();
            //std::cout << "Var value: " << val.ToString() << " iteration " << i << std::endl;
            long duration = GetTimeStamp() - init;
            measures_vector[i] = duration;
            //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::string filename;
        if (argc > 2)
        {
            filename = argv[2];
            filename += ".txt";
        }
        else
        {
            filename = FILE_PATH;
        }
        produce_measures_file(measures_vector, n_measures, filename);
        logger->info("Prodotto file delle misureeee");
        //FINE----------------------------------------------------------------------------------------------------------

        logger->info("Disconnecting");
        client.Disconnect();
        logger->flush();
        return 0;
    }

    catch (const std::exception &exc)
    {
        logger->error("Error: {}", exc.what());
    }

    catch (...)
    {
        logger->error("Unknown error.");
    }

    return -1;
}
