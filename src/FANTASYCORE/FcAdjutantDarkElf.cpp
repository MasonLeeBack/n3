#include "StdAfx.h"
#include "FcAdjutantObject.h"

#include "FcFXCommon.h"
#include "FcFxManager.h"

#include "FcWorld.h"
#include "./Data/SignalType.h"
#include "FcAIObject.h"
#include "FcAISearchSlot.h"
#include "FcAdjutantDarkElf.h"
#include "FcWorld.h"
#include "FcTroopObject.h"
#include "FcTroopManager.h"


CFcAdjutantDarkElf::CFcAdjutantDarkElf(CCrossVector *pCross)
: CFcAdjutantObject( pCross )
{
	SetAdjutantClassID( Class_ID_Adjutant_DarkElf );
}

CFcAdjutantDarkElf::~CFcAdjutantDarkElf()
{
}

void CFcAdjutantDarkElf::SignalCheck( ASSignalData *pSignal )
{
	CFcAdjutantObject::SignalCheck( pSignal );
}

