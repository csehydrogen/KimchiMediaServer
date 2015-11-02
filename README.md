# Kimchi Media Server

A media streaming server supporting H.265.

## Who made this??!

Team F([csehydrogen](https://github.com/csehydrogen), [cseteram](https://github.com/cseteram), [javelinsman](https://github.com/javelinsman)) in 창의적통합설계(M1522.000200-001, 2015 fall) of [SNU](http://www.snu.ac.kr/).

## Logo

![I.HeeHoon.You](https://github.com/csehydrogen/KimchiMediaServer/blob/master/IHeeHoonU.png) 

## Dependency

[live555](http://www.live555.com/) should be installed first via ```make install```.

## Features

* Streaming a MPEG-2 TS file
* Trick play (an index file should be created in advance)

## Test set

* Download an example H.265 Video ES file from [here](http://www.live555.com/liveMedia/public/265/surfing.265).
* Make a MPEG-2 TS file using live555/testProgs/testH265VideoToTransportStream.
* Make an index file using live555/testProgs/MPEG2TransportStreamIndexer.
