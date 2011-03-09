
// LIBRARY flogger.lib

#ifndef _DEBUG
#undef _LOGGING  // disable the file logging feature for the Release Builds
#endif

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <e32std.h>
#include <f32file.h>
#include <flogger.h>


#define ASSERT_PANIC(x, y) __ASSERT_DEBUG(x, User::Panic(_L("MyPanic"), y));

#ifdef _LOGGING

_LIT(KLogFileStart, "-----log start-----\n");
_LIT(KLogFileEnd, "----- log end -----\n");

#define __FLOG_DECLARATION_MEMBER		RFileLogger __logger__;

#define __FLOG_OPEN_LIT(KLogDir, KLogFile)  \
			if( __logger__.Connect() == KErrNone ) \
			{ \
				__logger__.CreateLog(KLogDir, KLogFile, EFileLoggingModeAppend); \
				__logger__.SetDateAndTime(ETrue, ETrue);\
			} 

#define __FLOG_OPEN(subsys,compnt) 	\
			_LIT(KLogFile, compnt); \
			_LIT(KLogDir, subsys); \
			__FLOG_OPEN_LIT(KLogDir, KLogFile);

#define __FLOG_OPEN_ID(subsys, compnt) \
			TUint32 _addr_ = (TUint32) (this); \
			TBuf<50> KLogFile; \
			KLogFile.AppendNum(_addr_, EHex); \
			KLogFile.Append(_L("_")); \
			KLogFile.Append(_L(compnt)); \
			_LIT(KLogDir, subsys); \
			__FLOG_OPEN_LIT(KLogDir, KLogFile);	

#define __FLOG_CLOSE					__logger__.Close()

#define __FLOG_0(text)						__logger__.Write(text)
#define __FLOG_1(text,a)					__logger__.WriteFormat(text,a)
#define __FLOG_2(text,a,b)					__logger__.WriteFormat(text,a,b)
#define __FLOG_3(text,a,b,c)				__logger__.WriteFormat(text,a,b,c)
#define __FLOG_4(text,a,b,c,d)				__logger__.WriteFormat(text,a,b,c,d)
#define __FLOG_5(text,a,b,c,d,e)			__logger__.WriteFormat(text,a,b,c,d,e)
#define __FLOG_6(text,a,b,c,d,e,f)			__logger__.WriteFormat(text,a,b,c,d,e,f)
#define __FLOG_7(text,a,b,c,d,e,f,g)		__logger__.WriteFormat(text,a,b,c,d,e,f,g)
#define __FLOG_8(text,a,b,c,d,e,f,g,h)		__logger__.WriteFormat(text,a,b,c,d,e,f,g,h)
#define __FLOG_9(text,a,b,c,d,e,f,g,h,i)	__logger__.WriteFormat(text,a,b,c,d,e,f,g,h,i)
#define __FLOG(text)						__logger__.Write(text)
#define __FLOG_VA(fmt, va_list)				__logger__.Write(fmt, va_list)
#define __FLOG_HEXDUMP(data,length)			__logger__.HexDump(0,0,data,length)
#define __QINFO(aText)				{User::InfoPrint(aText); User::After(1000000);} 

#define	_T8(text)							TPtrC8((const TText8 *)(text))


#else

// NULL macros
#define __FLOG_DECLARATION_MEMBER	
#define __FLOG_OPEN_LIT(KLogDir, KLogFile)	
#define __FLOG_OPEN(subsys,compnt)
#define __FLOG_OPEN_ID(subsys, compnt)
#define __FLOG_CLOSE				

#define __FLOG_0(text)						
#define __FLOG_1(text,a)					
#define __FLOG_2(text,a,b)					
#define __FLOG_3(text,a,b,c)				
#define __FLOG_4(text,a,b,c,d)				
#define __FLOG_5(text,a,b,c,d,e)			
#define __FLOG_6(text,a,b,c,d,e,f)			
#define __FLOG_7(text,a,b,c,d,e,f,g)		
#define __FLOG_8(text,a,b,c,d,e,f,g,h)		
#define __FLOG_9(text,a,b,c,d,e,f,g,h,i)	
#define __FLOG(text)						
#define __FLOG_VA(fmt, va_list)				
#define __FLOG_HEXDUMP(data,length)
#define __QINFO(aText) 

#define	_T8(text)

#endif


#ifdef _LOGGING

class TTrace
	{
public:
	inline TTrace(const TDesC& aFnName)
		: iFnName(aFnName)
		{
		_LIT(KMsg, "<Trace fn=\"%S\">\r\n");
		RDebug::Print(KMsg, &iFnName);
		User::Heap().Check();
		};
	inline ~TTrace()
		{
		_LIT(KMsg, "</Trace> <!-- %S -->\r\n");
		RDebug::Print(KMsg, &iFnName);
		User::Heap().Check();
		};
private:
	const TDesC& iFnName;
	};

	#define __FLOG_TRACE(X) TTrace _tracer(_L(X));
#else
	#define __FLOG_TRACE(X)
#endif

#endif // __LOGGING_H__
