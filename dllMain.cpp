//Each epic store game should be launched with a full command line parameters, like this example
//SOTTR.exe -AUTH_LOGIN=unused -AUTH_PASSWORD=13371337133713371337133713371337 -AUTH_TYPE=exchangecode -epicapp=890d9cf396d04922a1559333df419fed -epicenv=Prod -EpicPortal -epicusername="Paynoh12345" -epicuserid=baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -epiclocale=it -epicsandbox 
//Rename EOSSDK-Win64-Shipping.dll to EOSSDK-Win64-Shipping.original
//build using clang++ dllMain.cpp EOSSDK-Win64-Shipping.def -shared -O2 -o EOSSDK-Win64-Shipping.dll
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define EOS_BUILD_DLL 1
#include "EOSSDK/eos_sdk.h"
#include "EOSSDK/eos_auth.h"
#include "EOSSDK/eos_connect_types.h"
#include "EOSSDK/eos_userinfo_types.h"
#include "EOSSDK/eos_ecom_types.h"

static constexpr const char* FakeEpicIDString  = "baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
static constexpr const char* FakeProductIDString = "baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
static constexpr const char* FakeDisplayName = "Paynoh12345";
static constexpr const char* FakeLocale = "it";

static HMODULE g_RealDllHandle{nullptr};
static EOS_EpicAccountId g_EpicID{nullptr};
static EOS_ProductUserId g_PUID{nullptr};
static EOS_Auth_Token g_FakeToken{};
static EOS_UserInfo g_FakeUserInfo{};
 
//macro to forward call to the real dll

template<typename T>
static T GetFunc(const char* Name)
{
	return reinterpret_cast<T>(GetProcAddress(g_RealDllHandle, Name));
}
#define REAL(fn) GetFunc<decltype(&fn)>(#fn)


///////////////////////////////


static void InitStuff()
{
	if(auto fn = REAL(EOS_EpicAccountId_FromString))
		g_EpicID = fn(FakeEpicIDString);	
	if(auto fn = REAL(EOS_ProductUserId_FromString))
		g_PUID = fn(FakeProductIDString);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		g_RealDllHandle = LoadLibraryA("EOSSDK-Win64-Shipping.original");
		if(g_RealDllHandle == nullptr)
		{
			MessageBoxA(NULL, "Failed to get a handle to the real EOS dll...", "Oh no!", MB_OK);
			return FALSE;
		}
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		if(g_RealDllHandle != nullptr)
			FreeLibrary(g_RealDllHandle);
	}
	return TRUE;
}



// Emulated functions
extern "C" 
{
	EOS_EResult EOS_CALL EOS_Initialize(const EOS_InitializeOptions* Options)
	{
		auto fn = REAL(EOS_Initialize);
		EOS_EResult Result = fn ? fn(Options) : EOS_EResult::EOS_Success;
		InitStuff();
		return Result;
	}

	void EOS_CALL EOS_Auth_Login(EOS_HAuth Handle,const EOS_Auth_LoginOptions* Options,void* ClientData,EOS_Auth_OnLoginCallback CompletionDelegate)
	{
		EOS_Auth_LoginCallbackInfo CB{};
		CB.ResultCode = EOS_EResult::EOS_Success;
    	CB.ClientData = ClientData;
    	CB.LocalUserId = g_EpicID;
    	CB.SelectedAccountId = g_EpicID;
    	CompletionDelegate(&CB);
	}

	EOS_ELoginStatus EOS_CALL EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
	{
		return EOS_ELoginStatus::EOS_LS_LoggedIn;
	}
	
	EOS_EResult EOS_CALL EOS_Auth_CopyUserAuthToken(EOS_HAuth Handle,const EOS_Auth_CopyUserAuthTokenOptions* Options,EOS_EpicAccountId LocalUserId,EOS_Auth_Token** OutUserAuthToken)
	{
		g_FakeToken.ApiVersion = EOS_AUTH_TOKEN_API_LATEST;
		g_FakeToken.App = "Cinereous";
		g_FakeToken.ClientId = "00000000XYfgHfYo1bSagtshh0000000";
		g_FakeToken.AccountId = g_EpicID;
		g_FakeToken.AccessToken = "Fake_access_token_000000000000000";
		g_FakeToken.ExpiresIn = 888800.0;
		g_FakeToken.ExpiresAt = "9999-12-31T00:00:00.000Z";
		g_FakeToken.AuthType = EOS_EAuthTokenType::EOS_ATT_User;
		g_FakeToken.RefreshToken = "Fake_refresh_token_00000000000000";
		g_FakeToken.RefreshExpiresIn = 888800.0;
		g_FakeToken.RefreshExpiresAt = "9999-12-31T00:00:00.000Z";
		*OutUserAuthToken = &g_FakeToken;
		return EOS_EResult::EOS_Success;
	}

	void EOS_CALL EOS_Auth_Token_Release(EOS_Auth_Token*)
	{
		return;
	}

	void EOS_CALL EOS_Connect_Login(EOS_HConnect Handle,const EOS_Connect_LoginOptions* Options,void* ClientData,EOS_Connect_OnLoginCallback CompletionDelegate)
	{
		EOS_Connect_LoginCallbackInfo CB{};
		CB.ResultCode  = EOS_EResult::EOS_Success;
		CB.ClientData  = ClientData;
		CB.LocalUserId = g_PUID;
		CompletionDelegate(&CB);
	}

	void EOS_CALL EOS_Connect_CreateUser(EOS_HConnect Handle,const EOS_Connect_CreateUserOptions* Options,void* ClientData,EOS_Connect_OnCreateUserCallback CompletionDelegate)
	{
		EOS_Connect_CreateUserCallbackInfo CB{};
		CB.ResultCode  = EOS_EResult::EOS_Success;
		CB.ClientData  = ClientData;
		CB.LocalUserId = g_PUID;
		CompletionDelegate(&CB);
	}

	void EOS_CALL EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle,const EOS_UserInfo_QueryUserInfoOptions* Options,void* ClientData,EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate)
	{
		EOS_UserInfo_QueryUserInfoCallbackInfo CB{};
		CB.ResultCode = EOS_EResult::EOS_Success;
		CB.ClientData = ClientData;
		CB.LocalUserId  = g_EpicID;
		CB.TargetUserId = g_EpicID;
		CompletionDelegate(&CB);
	}

	EOS_EResult EOS_CALL EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle,const EOS_UserInfo_CopyUserInfoOptions* Options,EOS_UserInfo** OutUserInfo)
	{
		g_FakeUserInfo.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
		g_FakeUserInfo.UserId = g_EpicID;
		g_FakeUserInfo.DisplayName = FakeDisplayName;
		g_FakeUserInfo.Nickname = FakeDisplayName;
		g_FakeUserInfo.PreferredLanguage = FakeLocale;
		g_FakeUserInfo.Country = "IT";
		*OutUserInfo = &g_FakeUserInfo;
		return EOS_EResult::EOS_Success;
	}

	void EOS_CALL EOS_UserInfo_Release(EOS_UserInfo* UserInfo)
	{
		return;
	}

	void EOS_CALL EOS_Ecom_QueryOwnership(EOS_HEcom Handle,const EOS_Ecom_QueryOwnershipOptions* Options,void* ClientData,const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate)
	{
		uint32_t Count = Options->CatalogItemIdCount; 
		static EOS_Ecom_ItemOwnership Owned[128]{};
		uint32_t Filled = Count < 128 ? Count : 128;
		for(uint32_t i = 0; i < Filled; ++i)
		{
			Owned[i].ApiVersion = EOS_ECOM_ITEMOWNERSHIP_API_LATEST;
			Owned[i].Id = Options->CatalogItemIds[i];
			Owned[i].OwnershipStatus = EOS_EOwnershipStatus::EOS_OS_Owned;
		}
		
		EOS_Ecom_QueryOwnershipCallbackInfo CB{};
    	CB.ResultCode = EOS_EResult::EOS_Success;
		CB.ClientData = ClientData;
		CB.LocalUserId = g_EpicID;
		CB.ItemOwnership = Owned;
		CB.ItemOwnershipCount = Filled;
		CompletionDelegate(&CB);
	}

	void EOS_CALL EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle,const EOS_Ecom_QueryOwnershipTokenOptions* Options,void* ClientData,const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate)
	{
		EOS_Ecom_QueryOwnershipTokenCallbackInfo CB{};
		CB.ResultCode = EOS_EResult::EOS_Success;
		CB.ClientData = ClientData;
		CB.LocalUserId = g_EpicID;
		CB.OwnershipToken = "0000bGciOiJub25lIn0.eyJvd25lZCI6dHJ1ZX0.";
		CompletionDelegate(&CB);
	}
	EOS_EResult EOS_CALL EOS_Platform_CheckForLauncherAndRestart(EOS_HPlatform Handle)
	{
		return EOS_EResult::EOS_NoChange;
	}
}