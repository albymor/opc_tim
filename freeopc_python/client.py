import sys
sys.path.insert(0, "..")

from opcua import Client
import datetime
import time


if __name__ == "__main__":

    measures = 100000
    
    filename = sys.argv[1]+".txt"

    client = Client("opc.tcp://%s:4840/freeopcua/server/" % sys.argv[2])
    # client = Client("opc.tcp://admin@localhost:4840/freeopcua/server/") #connect using a user
    try:
        client.connect()

        # Client has a few methods to get proxy to UA nodes that should always be in address space such as Root or Objects
        root = client.get_root_node()
        print("Objects node is: ", root)

        # Node objects have methods to read and write node attributes as well as browse or populate address space
        print("Children of root are: ", root.get_children())

        # Now getting a variable node using its browse path
        myvar = root.get_child(["0:Objects", "2:MyObject", "2:MyVariable"])
        obj = root.get_child(["0:Objects", "2:MyObject"])
        print("myvar is: ", myvar)
        print("myobj is: ", obj)
        
        arr = [None]*measures

        for i in range(measures):
            try:
                if i % 1000 == 0:
                    print(datetime.datetime.now(), i)
                    fo = open(filename, "w")
                    fo.write(",".join(map(str,arr)))
                    fo.close()
            except:
                pass
            now = time.perf_counter()
            val = myvar.get_value()
            time_elapsed = int((time.perf_counter()) - now)
            arr[i] = time_elapsed
        print(val)

        print("totale misure: ", len(arr))

        fo = open(filename, "w")
        fo.write(",".join(map(str,arr)))
        fo.close()
        
    finally:
        client.disconnect()
