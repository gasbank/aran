#include "AranDx9PCH.h"
#include "UndefinedCallback.h"

UndefinedCallback::UndefinedCallback(void)
{
}

UndefinedCallback::~UndefinedCallback(void)
{
}

void UndefinedCallback::DoCallbackFirstTimeOnly( void* pData /* in */, void* pResultData /* out */ )
{
	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(pResultData);
}

void UndefinedCallback::DoCallback( void* pData /* in */, void* pResultData /* out */ )
{
	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(pResultData);
}

void UndefinedCallback::DoUnregisterCallback()
{

}