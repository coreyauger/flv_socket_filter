//-----------------------------------------------------------------------------
//
//	Author : Corey Auger
//
//-----------------------------------------------------------------------------
#pragma once

namespace FLV{

#pragma pack()
class FLVHeader
{
public:		
	BYTE Signature[3];
	BYTE Version;
	BYTE Flags;
	BYTE Offset[4];			// if this is (int) type it forces an alignment..
};

class FLVStream
{
public:
	UINT PreviousTagSize;
};


enum FLVTagType{ AUDIO = 0x08, VIDEO = 0x09, META = 0x18 };

#pragma pack()
class FLVTag
{
public:		
	BYTE Type;
	BYTE BodyLength[3];
	BYTE TimeStramp[3];
	BYTE TimestampExtended;		
	BYTE StreamId[3];	
};

}