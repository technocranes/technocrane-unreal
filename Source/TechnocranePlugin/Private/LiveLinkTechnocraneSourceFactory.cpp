// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// LiveLinkTechnocraneSourceFactory.cpp
// Sergei <Neill3d> Solokhin 

#include "LiveLinkTechnocraneSourceFactory.h"

#include "LiveLinkTechnocraneSource.h"
#include "SLiveLinkTechnocraneSourceFactory.h"

#define LOCTEXT_NAMESPACE "LiveLinkTechnocraneSourceFactory"

FText ULiveLinkTechnocraneSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("SourceDisplayName", "Technocrane LiveLink");
}

FText ULiveLinkTechnocraneSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("SourceTooltip", "Creates a connection to a Technocrane UDP Stream");
}

TSharedPtr<SWidget> ULiveLinkTechnocraneSourceFactory::BuildCreationPanel(FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	return SNew(SLiveLinkTechnocraneSourceFactory)
		.OnOkClicked(SLiveLinkTechnocraneSourceFactory::FOnOkClicked::CreateUObject(this, &ULiveLinkTechnocraneSourceFactory::OnOkClicked, InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> ULiveLinkTechnocraneSourceFactory::CreateSource(const FString& InConnectionString) const
{
	FIPv4Endpoint DeviceEndPoint;
	if (!FIPv4Endpoint::Parse(InConnectionString, DeviceEndPoint))
	{
		return TSharedPtr<ILiveLinkSource>();
	}

	return MakeShared<FLiveLinkTechnocraneSource>(true, 1, DeviceEndPoint, true, false);
}

void ULiveLinkTechnocraneSourceFactory::OnOkClicked(SCreationInfo info, FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	InOnLiveLinkSourceCreated.ExecuteIfBound(MakeShared<FLiveLinkTechnocraneSource>(
		info.m_UseNetwork,
		info.m_SerialPort,
		info.m_Address,
		info.m_NetworkBindAny,
		info.m_NetworkBroadcast), 
		info.m_Address.ToString());
}

#undef LOCTEXT_NAMESPACE