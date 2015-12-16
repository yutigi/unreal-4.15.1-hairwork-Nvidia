// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "IPluginWardenModule.h"

/**
 * The Plugin Warden is a simple module used to verify a user has purchased a plug-in.  This
 * module won't prevent a determined user from avoiding paying for a plug-in, it is merely to
 * prevent accidental violation of a per-seat license on a plug-in, and to direct those users
 * to the marketplace page where they may purchase the plug-in.
 */
class FPluginWardenModule : public IPluginWardenModule
{
public:

	/**
	 * Called right after the module DLL has been loaded and the module object has been created
	 */
	virtual void StartupModule();

	/**
	 * Called before the module is unloaded, right before the module object is destroyed.
	 */
	virtual void ShutdownModule();

	/**
	 * Ask the launcher if the user has authorization to use the given plug-in. The authorized 
	 * callback will only be called if the user is authorized to use the plug-in.
	 * 
	 * FPluginWardenModule& PluginWarden = FModuleManager::LoadModuleChecked<FPluginWardenModule>("PluginWarden");
	 * PluginWarden.CheckEntitlementForPlugin(LOCTEXT("AwesomePluginName", "My Awesome Plugin"), TEXT("1a4ad2a06f174a26af8a29728241ce42"), [&] () {
	 *		// Authorized Code Here
	 * });
	 * 
	 * @param PluginFriendlyName The localized friendly name of the plug-in.
	 * @param PluginGuid The unique identifier of the plug-in on the marketplace.
	 * @param AuthorizedCallback This function will be called after the user has been given entitlement.
	 */
	virtual void CheckEntitlementForPlugin(const FText& PluginFriendlyName, const FString& PluginGuid, TFunction<void()> AuthorizedCallback) override;
};