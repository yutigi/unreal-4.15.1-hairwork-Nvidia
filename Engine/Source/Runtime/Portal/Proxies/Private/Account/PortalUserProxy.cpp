// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "PortalProxiesPrivatePCH.h"
#include "PortalUserProxy.h"
#include "IMessageRpcClient.h"
#include "IPortalUser.h"
#include "PortalUserMessages.h"

class FPortalUserProxy
	: public IPortalUser
{
public:

	virtual ~FPortalUserProxy() { }

	virtual bool IsAvailable() const override
	{
		return RpcClient->IsConnected();
	}

public:

	virtual TAsyncResult<FPortalUserDetails> GetUserDetails() override
	{
		return RpcClient->Call<FPortalUserGetUserDetails>();
	}
	
	virtual TAsyncResult<FPortalUserIsEntitledToItemResult> IsEntitledToItem(const FString& ItemId, EEntitlementCacheLevelRequest CacheLevel) override
	{
		return RpcClient->Call<FPortalUserIsEntitledToItem>(ItemId, CacheLevel);
	}

private:

	FPortalUserProxy(const TSharedRef<IMessageRpcClient>& InRpcClient)
		: RpcClient(InRpcClient)
	{ }

private:

	TSharedRef<IMessageRpcClient> RpcClient;

	friend FPortalUserProxyFactory;
};

TSharedRef<IPortalService> FPortalUserProxyFactory::Create(const TSharedRef<IMessageRpcClient>& RpcClient)
{
	return MakeShareable(new FPortalUserProxy(RpcClient));
}