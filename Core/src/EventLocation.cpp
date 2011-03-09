/*
 ============================================================================
 Name		: EventLocation.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventLocation implementation
 ============================================================================
 */

#include "EventLocation.h"

// Forward declaration
LOCAL_C double VincentFormula(double lat1, double lon1, double lat2, double lon2);

const static TInt KIntervalSec = 5;
const static TInt KFixTimeOutMin = 15;

CEventLocation::CEventLocation(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Location, aTriggerId)
	{
	// No implementation required
	}

CEventLocation::~CEventLocation()
	{
	delete iGPS;
	}

CEventLocation* CEventLocation::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventLocation* self = new (ELeave) CEventLocation(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventLocation* CEventLocation::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventLocation* self = CEventLocation::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventLocation::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	iGPS = CGPSPosition::NewL(*this);
	//iLocationParams = (TLocationStruct*) iParams.Ptr();
	Mem::Copy(&iLocationParams,iParams.Ptr(), sizeof(iLocationParams) );

	}

void CEventLocation::StartEventL()
	{
	TBool hasGPSModule = iGPS->ReceiveData(KIntervalSec, KFixTimeOutMin);
	}
    
//void CEventLocation::HandleGPSPositionL(TPosition position)  // original MB
void CEventLocation::HandleGPSPositionL(TPositionSatelliteInfo satPos)
	{
	TPosition position;
	satPos.GetPosition(position);
	TCoordinate coords(iLocationParams.iLatOrigin, iLocationParams.iLonOrigin);
	TReal32 distance = 0;
	position.Distance(coords, distance);

	// Uncomment the lines below to use the HT code to compute the distance
	// Useful URL for comparing results: http://www.movable-type.co.uk/scripts/latlong.html
	/* distance = VincentFormula(position.Latitude(), position.Longitude(), iLocationParams->iLatOrigin, iLocationParams->iLonOrigin);
	 if (distance == 0)
	 return; */

	if (distance <= iLocationParams.iConfDistance)
		{
		// Before trigger the event perform an additional check, just in case.
		if (!iWasInsideRadius)
			{
			iWasInsideRadius = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			}
		}
	else
		{
		if (iWasInsideRadius)
			{
			iWasInsideRadius = EFalse;
			// Triggers the Out-Action
			if (iLocationParams.iExitAction != 0xFFFFFFFF)
				{
				SendActionTriggerToCoreL(iLocationParams.iExitAction);
				}
			}
		}
	}

void CEventLocation::HandleGPSErrorL(TInt error)
	{
	// Can't get a FIX... reschedule a new request...
	TBool hasGPSModule = iGPS->ReceiveData(KIntervalSec, KFixTimeOutMin);
	}

// VincentFormula stuff...

LOCAL_C double atan(double src)
	{
	double res = 0;
	Math::ATan(res, src);
	return res;
	}

LOCAL_C double tan(double src)
	{
	double res = 0;
	Math::Tan(res, src);
	return res;
	}
LOCAL_C double sin(double src)
	{
	double res = 0;
	Math::Sin(res, src);
	return res;
	}
LOCAL_C double cos(double src)
	{
	double res = 0;
	Math::Cos(res, src);
	return res;
	}

LOCAL_C double sqrt(double src)
	{
	double res = 0;
	Math::Sqrt(res, src);
	return res;
	}

LOCAL_C double atan2(double sin, double cos)
	{
	double res = 0;
	Math::ATan(res, sin, cos);
	return res;
	}

LOCAL_C double fabs(double src)
	{
	if (src < 0)
		return -src;
	return src;
	}

LOCAL_C double VincentFormulaL(double lat1, double lon1, double lat2, double lon2)
	{
	double a = 6378137.0f, b = 6356752.3142, f = 1 / 298.257223563;
	double rad = KPi / 180.0f, lon_rad = (lon2 - lon1) * rad;
	double U1 = atan((1 - f) * tan(lat1 * rad));
	double U2 = atan((1 - f) * tan(lat2 * rad));
	double sinU1 = sin(U1), cosU1 = cos(U1);
	double sinU2 = sin(U2), cosU2 = cos(U2);
	double cosSqAlpha, cos2SigmaM, sinSigma, cosSigma, sigma, sinAlpha;
	double cosLambda, sinLambda;
	double uSq, A, B, C, deltaSigma, s;
	double lambda = lon_rad, lambdaP = 2 * KPi;
	int iterations = 20;

	while (fabs(lambda - lambdaP) > 0.000000000001f && --iterations > 0)
		{
		sinLambda = sin(lambda), cosLambda = cos(lambda);

		sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) + (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda)
				* (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));

		if (sinSigma == 0.0f)
			return 0.0f; // I punti sono coincidenti

		cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
		sigma = atan2(sinSigma, cosSigma);

		sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;

		cosSqAlpha = 1 - sinAlpha * sinAlpha;
		cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;

		//if (isnan(cos2SigmaM)) cos2SigmaM = 0;

		C = f / 16 * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));

		lambdaP = lambda;
		lambda = lon_rad + (1 - C) * f * sinAlpha * (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1 + 2
				* cos2SigmaM * cos2SigmaM)));
		}

	if (iterations == 0)
		return 0.0f; // La formula non e' riuscita a convergere

	uSq = cosSqAlpha * (a * a - b * b) / (b * b);

	A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
	B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));

	deltaSigma = B * sinSigma * (cos2SigmaM + B / 4 * (cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM) - B / 6
			* cos2SigmaM * (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));

	s = b * A * (sigma - deltaSigma);
	return s;
	}

LOCAL_C double VincentFormula(double lat1, double lon1, double lat2, double lon2)
	{
	double res = 0;
	TRAP_IGNORE(res = VincentFormulaL(lat1, lon1, lat2, lon2));
	return res;
	}

/*
 L'EventLocation, definito nel file di configurazione dal relativo EventId, triggera l'azione ad esso associata quando il device si trova all'interno di un raggio che ha come centro la posizione GPS definita nella configurazione.

 L'evento e' in grado di triggerare due azioni:

 1. In-Action: quando il device raggiunge la posizione stabilita.
 2. Out-Action: quando il device esce dalla posizione stabilita. 

 Parametri

 L'evento riceve quattro parametri di configurazione all'interno della propria EventStruct:

 uExitAction
 E' un UINT ed assume il numero dell'azione da eseguire quando il device esce dalla posizione (e' inizializzato a 0xffffffff nel caso in cui non ci sia alcuna out-action definita). 
 confDistance
 E' un INT ed indica la dimensione, in metri, del raggio che ha come centro le coordinate GPS definite nel file di configurazione. 
 latOrigin
 E' un DOUBLE ed indica la latitudine del punto di origine. 
 lonOrigin
 E' un DOUBLE ed indica la longitudine del punto di origine.

 Calcolo della distanza

 La distanza viene calcolata secondo la seguente formula, tra le ultime righe si trova la gestione di un'eccezione che va tenuta in conto poiche' la formula non converge necessariamente:

 // 6378137.0f -> semiasse equatoriale dell'ellissoide internazionale WGS-84
 // 1/298.257223563 -> schiacciamento dell'ellissoide internazionale WGS-84
 // 6356752.3142 -> raggio polare dell'ellissoide internazionale WGS-84
 DOUBLE GPS::VincentFormula(DOUBLE lat1, DOUBLE lon1, DOUBLE lat2, DOUBLE lon2) {
 double a = 6378137.0f, b = 6356752.3142,  f = 1/298.257223563;
 double rad = PI / 180.0f, lon_rad = (lon2 - lon1) * rad;
 double U1 = atan((1 - f) * tan(lat1 * rad));
 double U2 = atan((1 - f) * tan(lat2 * rad));
 double sinU1 = sin(U1), cosU1 = cos(U1);
 double sinU2 = sin(U2), cosU2 = cos(U2);
 double cosSqAlpha, cos2SigmaM, sinSigma, cosSigma, sigma, sinAlpha;
 double cosLambda, sinLambda;
 double uSq, A, B, C, deltaSigma, s; 
 double lambda = lon_rad, lambdaP = 2 * PI;
 int iterations = 20;

 __try {
 while (fabs(lambda-lambdaP) > 0.000000000001f && --iterations > 0) {
 sinLambda = sin(lambda), cosLambda = cos(lambda);

 sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) + 
 (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) * (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));

 if (sinSigma == 0.0f) 
 return 0.0f;  // I punti sono coincidenti

 cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
 sigma = atan2(sinSigma, cosSigma);

 sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;

 cosSqAlpha = 1 - sinAlpha * sinAlpha;
 cos2SigmaM = cosSigma - 2 * sinU1 * sinU2/cosSqAlpha;

 //if (isnan(cos2SigmaM)) cos2SigmaM = 0;

 C = f / 16 * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));

 lambdaP = lambda;
 lambda = lon_rad + (1-C) * f * sinAlpha *
 (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
 }

 if (iterations == 0) 
 return 0.0f;  // La formula non e' riuscita a convergere

 uSq = cosSqAlpha * (a * a - b * b) / (b * b);

 A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
 B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));

 deltaSigma = B * sinSigma * (cos2SigmaM + B / 4 * (cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)-
 B / 6* cos2SigmaM * (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));

 s = b * A * (sigma - deltaSigma);
 return s;
 } __except(EXCEPTION_EXECUTE_HANDLER) {
 return 0.0f;
 }
 }

 Il valore di ritorno e' la distanza dall'origine in metri. Se il valore di ritorno e' 0.0 la posizione non viene considerata attendibile poiche' si assume che la formula non ha potuto convergere. 
 */
