// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// LiveLinkTechnocraneSourceFactory.h
// Sergei <Neill3d> Solokhin

#pragma once

#include "LiveLinkSourceFactory.h"
#include "SLiveLinkTechnocraneSourceFactory.h"
#include "LiveLinkTechnocraneSourceFactory.generated.h"


class SLiveLinkTechnocraneSourceEditor;

UCLASS()
class ULiveLinkTechnocraneSourceFactory : public ULiveLinkSourceFactory
{
public:
	GENERATED_BODY()

	virtual FText GetSourceDisplayName() const;
	virtual FText GetSourceTooltip() const;

	virtual EMenuType GetMenuType() const override { return EMenuType::SubPanel; }
	virtual TSharedPtr<SWidget> BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const override;
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

private:
	void OnOkClicked(SCreationInfo info, FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const;
};
