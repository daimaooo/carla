// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "MapPreviewUserWidget.h"

#include "Sockets.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Common/TcpSocketBuilder.h"
#include "SocketSubsystem.h"
#include "SocketTypes.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Engine/Texture2D.h"
#include "Containers/ResourceArray.h"
#include "Rendering/Texture2DResource.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "Misc/Timespan.h"

void UMapPreviewUserWidget::ConnectToSocket()
{
  //FSocket* Socket = FSocket::CreateTCPConnection(nullptr, TEXT("OSMRendererSocket"));
  Socket = FTcpSocketBuilder(TEXT("OSMRendererSocket")).AsReusable();
  FIPv4Address RemoteAddress;
  FIPv4Address::Parse(FIPv4Address::InternalLoopback.ToString(), RemoteAddress);
  TSharedRef<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
  RemoteAddr->SetIp(RemoteAddress.Value);
  RemoteAddr->SetPort(5000);

  // Connect to the remote server
  bool Connected = Socket->Connect(*RemoteAddr);
  if (!Connected)
  {
    UE_LOG(LogTemp, Error, TEXT("Error connecting to remote server"));
    return;
  }

  MapTexture = UTexture2D::CreateTransient(512,512,EPixelFormat::PF_R8G8B8A8,"MapTextureRendered");
  MapTexture->UpdateResource();

  // Send a message
  FString Message = "-C /home/aollero/Downloads/libosmcout/libosmscout-master/maps/madrid_downtown/madrid_big /home/aollero/Downloads/libosmcout/libosmscout-master/stylesheets/standard.oss 512";
  SendStr(Message);
  
}

void UMapPreviewUserWidget::RenderMap()
{
  FString Message = "-R 40.415 -3.702 100000";
  SendStr(Message);

  TArray<uint8_t> ReceivedData;
  uint32 ReceivedDataSize;
  
  if(Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(5)))
  {
    while(Socket->HasPendingData(ReceivedDataSize))
    {
      int32 BytesReceived = 0;
      TArray<uint8_t> ThisReceivedData;
      ThisReceivedData.Init(0, FMath::Min(ReceivedDataSize, uint32(512*512*4)));
      bool bRecv = Socket->Recv(ThisReceivedData.GetData(), ThisReceivedData.Num(), BytesReceived);
      if (!bRecv)
      {
        UE_LOG(LogTemp, Error, TEXT("Error receiving message"));
      }
      else
      {
        UE_LOG(LogTemp, Log, TEXT("Received %d bytes. %d"), BytesReceived, ReceivedDataSize);
        ReceivedData.Append(ThisReceivedData);
        UE_LOG(LogTemp, Log, TEXT("Size of Data: %d"), ReceivedData.Num());
      }
    }
    
    // TODO: Move to function
    if(ReceivedData.Num() > 0)
    {
      ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)
      (
        [NewData=ReceivedData, Texture=MapTexture](auto &InRHICmdList) mutable
        {
          UE_LOG(LogTemp, Log, TEXT("RHI: Updating texture"));
          FUpdateTextureRegion2D Region;
          Region.SrcX = 0;
          Region.SrcY = 0;
          Region.DestX = 0;
          Region.DestY = 0;
          Region.Width = Texture->GetSizeX();
          Region.Height = Texture->GetSizeY();
 
          FTexture2DResource* Resource = (FTexture2DResource*)Texture->Resource;
          RHIUpdateTexture2D(Resource->GetTexture2DRHI(), 0, Region, Region.Width * sizeof(uint8_t) * 4, &NewData[0]);
        }
      );
    }
  }
}

void UMapPreviewUserWidget::Shutdown()
{
  // Close the socket
  Socket->Close();
}

bool UMapPreviewUserWidget::SendStr(FString Msg)
{
  std::string MessageStr = std::string(TCHAR_TO_UTF8(*Msg));
  int32 BytesSent = 0;
  bool bSent = Socket->Send((uint8*)MessageStr.c_str(), MessageStr.size(), BytesSent);
  if (!bSent)
  {
    UE_LOG(LogTemp, Error, TEXT("Error sending message"));
  }
  else
  {
    UE_LOG(LogTemp, Log, TEXT("Sent %d bytes"), BytesSent);
  }
  return bSent;
}