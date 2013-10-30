flv socket filter
=================

Windows Direct Show filter.  Stream FLV and header information to any connecting application.  Takes care of late joiners and resending h264/aac headers.

# Overview
FLV files consist of a single header, and metadata, chunk followed by N interleaved audio and video chunks.  
Most streaming filter implement RTMP and provide a single client with the ability to connect and start injesting a stream.  
This filter provides a much more flexible and free form way of Interprocess Communication with you data.  

* Supports M number of simultaneous connection steams to FLV
* Supports "late joiners" headers(Meta,Head) + AAC/H264 packets are retransmitted to new connections
* Actionscript ByteStream clients will receive data on chunk boundaries for easy parsing.
* Simple to right stream out to file on disk at the same time as other operations.


# Requirements
The only requirement are the direct show base classes.  Sockets are winsock implementation.

# Usage
Currently the filter has the listening port hardcoded.  You could add this as an option to the filter option page 
if you so desire.  Once this filter is up and running you can connect to it via the port number and start to injesting your FLV.

# Example
Here is an example client written in C# that will connect and write the FLV to disk.

This example uses the FlvStream2FileWriter that is also an open source project I have on GIT here:
(https://github.com/coreyauger/flv-streamer-2-file)[https://github.com/coreyauger/flv-streamer-2-file]

This takes care of inseting the new timing information into the FLV so that your file on disk will allow seek operations.

```
internal void SaveStreamToFile(string path)
        {
            const int count = 4096;
            byte[] buffer = new byte[count];

            // Declare the callback.  Need to do that so that
            // the closure captures it.

            AsyncCallback callback = null;

            System.IO.File.Delete(path);
            FlvStream2FileWriter stream2File = new FlvStream2FileWriter(path);

            // Assign the callback.
            callback = ar =>
            {
                try
                {
                    // Call EndRead.
                    int bytesRead = _clientStream.GetStream().EndRead(ar);

                    // Process the bytes here.
                    if (bytesRead != buffer.Length)
                    {
                        stream2File.Write(buffer.Take(bytesRead).ToArray());
                    }
                    else
                    {
                        stream2File.Write(buffer);
                    }

                    // Determine if you want to read again.  If not, return.                
                    // Read again.  This callback will be called again.
                    if (!_clientStream.Connected)
                    {
                        throw new Exception("Just catch and exec finallize");
                    }
                    _clientStream.GetStream().BeginRead(buffer, 0, count, callback, null);
                }
                catch
                {   // most likly discontinuity ...just fall through                   
                    stream2File.FinallizeFile();
                    return;
                }               
            };
            _clientStream = new TcpClient("127.0.0.1", 22822);  // (CA) TODO: make port configurable on the filter and here
            // Trigger the initial read.
            _clientStream.GetStream().BeginRead(buffer, 0, count, callback, null);
        }
```        
