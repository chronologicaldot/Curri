// (c) 2018 Nicolaus Anderson

#include "AppLogger.h"

// Copper, CuBridge
#include <EngMsgToStr.h>
#include <cubr_messagecodes.h>
	// For type casting
#include <cubr_base.h>
#include <cubr_guiwatcher.h>
#include <cubr_mfrunner.h>

// Irrlicht
#include <IFileSystem.h>

// For printf and console writing
#include <cstdio>
#include <cstring> // For strlen


using irr::core::stringc;

AppLogger::AppLogger()
	: loggingFile(0)
	, messageProvider(nullptr)
	, typeNameProvider(nullptr)
	, writeInterface(nullptr)
	, Enabled(false)
	, AlwaysFlush(true)
	, WriteToConsole(false)
	, WriteToFile(true)
	, WriteToInterface(false)
	, ShowInfoMessages(true)
	, ShowWarningMessages(true)
	, ShowErrorMessages(true)
{}

AppLogger::~AppLogger() {
}

bool
AppLogger::isReady() {
	if ( !Enabled ) return false;

	if ( WriteToConsole )
		return true;

	if ( WriteToFile )
		return loggingFile != nullptr;

	if ( WriteToInterface )
		return writeInterface != nullptr;

	return false;
}

bool
AppLogger::willShow( Cu::LogLevel::Value  logLevel ) {
	switch( logLevel )
	{
	case Cu::LogLevel::info:
		return ShowInfoMessages;

	case Cu::LogLevel::warning:
		return ShowWarningMessages;

	case Cu::LogLevel::error:
		return ShowErrorMessages;

	default:
		return true;
	}
}

bool
AppLogger::createLoggingFile( irr::io::IFileSystem*  fileSystem ) {
	if ( !fileSystem )
		return false;

	loggingFile = fileSystem->createAndWriteFile(irr::io::path("app.log"), false);
	return loggingFile != 0;
}

void
AppLogger::setCustomMessageProvider( CustomLogMessageProvider  provider ) {
	messageProvider = provider;
}

void
AppLogger::setCustomTypeNameProvider( CustomTypeNameProvider  provider ) {
	typeNameProvider = provider;
}

void
AppLogger::setWriteInterface( WriteInterface&  w )
{
	writeInterface = w;
}

void
AppLogger::write( const stringc&  message, bool  isError ) {
	if ( !Enabled ) return;

	if ( WriteToConsole ) {
		if ( isError )
			std::fputs(message.c_str(), stderr);
		else
			std::puts(message.c_str());
	}

	if ( WriteToInterface && writeInterface ) {
		if ( writeInterface->write(message) && AlwaysFlush )
			writeInterface->flush();
	}

	if ( !loggingFile ) return;

	const stringc  m = message + stringc("\n");

	size_t amountWritten = loggingFile->write(m.c_str(), (size_t)m.size());
	if ( amountWritten > 0 && AlwaysFlush )
		loggingFile->flush();
}

void
AppLogger::flush() {
	if ( WriteToConsole )
		fflush(stdout);

	if ( WriteToFile && loggingFile )
		loggingFile->flush();

	if ( WriteToInterface && writeInterface )
		writeInterface->flush();
}

void AppLogger::print(const Cu::LogLevel::Value  logLevel, const irr::core::stringc&  message ) {
	if ( !willShow(logLevel) )
		return;

	stringc  finalMessage;

	switch( logLevel ) {
	case Cu::LogLevel::warning:
		finalMessage = "WARNING: ";
		finalMessage += message;
		write(finalMessage, true);
		break;

	case Cu::LogLevel::error:
		finalMessage = "ERROR: ";
		finalMessage += message;
		write(finalMessage, true);
		break;

	default:
		finalMessage += message;
		write(finalMessage);
		break;
	}
}

void
AppLogger::print(const Cu::LogLevel::Value  logLevel, const char*  message) {
	print( logLevel,  irr::core::stringc(message) );
}

void
AppLogger::print( const Cu::LogLevel::Value  logLevel, const Cu::EngineMessage::Value  engineMessage) {
	Cu::EngineErrorLevel::Value  errorLevel;
	const util::String  msg = getStringFromEngineMessage(engineMessage, errorLevel);
	print(logLevel, msg.c_str());
}

void
AppLogger::print( Cu::LogMessage  logMessage ) {
	if ( !Enabled ) return;

	switch ( logMessage.messageId )
	{
	case Cu::EngineMessage::WrongArgCount:
		printVerboseLogMessage(logMessage, "Wrong argument count");
		break;

	case Cu::EngineMessage::WrongArgType:
		printVerboseLogMessage(logMessage, "Wrong argument type");
		break;

	case Cu::EngineMessage::DestroyedFuncAsArg:
		printVerboseLogMessage(logMessage, "Destroyed function given as argument");
		break;

	case Cu::EngineMessage::MissingArg:
		printVerboseLogMessage(logMessage, "Missing argument");
		break;

	case Cu::EngineMessage::IndexOutOfBounds:
		printVerboseLogMessage(logMessage, "Index out of bounds");
		break;

	case Cu::EngineMessage::BadArgIndexInForeignFunc:
		printVerboseLogMessage(logMessage, "Invalid argument-index access in foreign function");
		break;

	case Cu::EngineMessage::SystemFunctionBadArg:
		printVerboseLogMessage(logMessage, "Invalid argument given to system function");
		break;

	case Cu::EngineMessage::CustomMessage:
		printCustomMessage(logMessage);
		break;

	default:
		printStandardMessage(logMessage);
		break;
	}
}

void
AppLogger::printTaskTrace(
	Cu::TaskType::Value  taskType,
	const util::String&  taskName,
	Cu::UInteger  taskNumber
) {
	if ( !ShowErrorMessages )
		return;

	stringc  finalMessage = "TASK TRACE: ";
	finalMessage += stringc(taskNumber)
		+ stringc(": ") + stringc(getTaskType(taskType))
		+ stringc(" ") + stringc(taskName.c_str());

	write(finalMessage);
}

void
AppLogger::printStackTrace(
	const util::String&  frameName,
	Cu::UInteger  frameNumber
) {
	if ( !ShowErrorMessages )
		return;

	stringc  finalMessage = "STACK TRACE: Frame: ";
	finalMessage += stringc(frameName.c_str())
					+ stringc(", # ")
					+ stringc(frameNumber);

	write(finalMessage);
}

const char*
AppLogger::getCuBridgeErrorMessage( Cu::UInteger  id ) {

	switch ( static_cast<cubr::CuBridgeMessageCode::Value>(id) )
	{
	case cubr::CuBridgeMessageCode::START:
		return "CuBridge START";

		//! Info - Child GUI element not found
	case cubr::CuBridgeMessageCode::GUIElementChildNotFound:
		return "CuBridge GUIElement child not found";

		//! C++ Error - A GUI element was empty (not set)
	case cubr::CuBridgeMessageCode::GUIElementIsEmpty:
		return "CuBridge GUIElement is empty";

		//! Warning - GUI element factory cannot produce the requested element
	case cubr::CuBridgeMessageCode::GUIElementCannotBeCreated:
		return "CuBridge GUIElement cannot be created";

		//! Warning - GUI Watcher construction did not receive the correct arguments.
		// Are the call arguments are in the wrong order?
	case cubr::CuBridgeMessageCode::GUIWatcherWrongConstructionArgs:
		return "CuBridge GUIWatcher wrong construction arguments";

		//! Warning - GUI element did not have a parent
	case cubr::CuBridgeMessageCode::GUIElementLacksParent:
		return "CuBridge GUIElement lacks parent";

	default: break;
	}

	return "";
}

const char*
AppLogger::getCuMultiFileRunnerErrorMessage( Cu::UInteger  id ) {
	switch ( static_cast<cubr::MultifileRunner::MessageCode::Value>(id) )
	{
	case cubr::MultifileRunner::MessageCode::FileNonExistent:
		return "Multifile Runner: File does not exist";

	case cubr::MultifileRunner::MessageCode::ImportUsageInWrongContext:
		return "Multifile Runner: import() used on the wrong context";

	case cubr::MultifileRunner::MessageCode::RequireUsageInWrongContext:
		return "Multifile Runner: require() used in the wrong context";

	case cubr::MultifileRunner::MessageCode::InvalidPathArg:
		return "Multifile Runner: Invalid path argument";

	default: break;
	}
	return "";
}

util::String
AppLogger::getObjectTypeNameFromType( Cu::ObjectType::Value  type ) {
	switch( type ) {
	// rely on implicit conversion to util::String
	case Cu::ObjectType::Function: return Cu::FunctionObject::StaticTypeName();
	case Cu::ObjectType::Bool: return Cu::BoolObject::StaticTypeName();
	case Cu::ObjectType::String: return Cu::StringObject::StaticTypeName();
	case Cu::ObjectType::Numeric: return Cu::NumericObject::StaticTypeName();
	case Cu::ObjectType::List: return Cu::ListObject::StaticTypeName();
	default: break;
	}

	if ( type == cubr::GUIElement::getTypeAsCuType() ) {
		return util::String("GUI element");
	}
	else if ( type == cubr::GUIWatcherObject::getTypeAsCuType() ) {
		return util::String("GUI watcher");
	}
	else if ( type == cubr::Image::getTypeAsCuType() ) {
		return util::String("image");
	}
	else if ( type == cubr::Texture::getTypeAsCuType() ) {
		return util::String("texture");
	}

	if ( typeNameProvider ) {
		return typeNameProvider( type );
	}

	return util::String("unknown");
}

const char*
AppLogger::getTaskType( Cu::TaskType::Value  typeValue ) {
	switch( typeValue )
	{
	case Cu::TaskType::FuncBuild: return "Function Build";
	case Cu::TaskType::FuncFound: return "Function Found";
	default: return "";
	}
}

void
AppLogger::printStandardMessage( Cu::LogMessage  logMessage ) {
	Cu::EngineErrorLevel::Value  errorLevel;
	const util::String  msg = getStringFromEngineMessage(logMessage.messageId, errorLevel);
	print(logMessage.level, msg.c_str());

	//print(logMessage.level, logMessage.messageId);
}

void
AppLogger::printVerboseLogMessage( Cu::LogMessage  logMessage, const char*  header ) {

	Cu::String  format = "Function %s, arg %u of %u: %s";
	size_t  msgSize = 0;

	if ( logMessage.functionName.size() == 0 ) {
		if ( logMessage.systemFunctionId != Cu::SystemFunction::_unset ) {
			logMessage.functionName = Cu::getSystemFunctionDefaultName(logMessage.systemFunctionId);
		} else {
			logMessage.functionName = "global";
		}
	}

	const util::String givenArgTypeName = getObjectTypeNameFromType( logMessage.givenArgType );
	const util::String expectedArgTypeName = getObjectTypeNameFromType( logMessage.expectedArgType );

	bool useExpectationMsg =
		logMessage.expectedArgType != logMessage.givenArgType // If these are the same, there must be a different issue
		&& expectedArgTypeName.size() > 0;

	if ( useExpectationMsg )
	{
		if ( givenArgTypeName.size() > 0 ) {
			format = "Function %s, arg %u of %u: %s\nExpected: %s, Given: %s";
			msgSize = (size_t)format.size()
					+ (size_t)logMessage.functionName.size()
					+ strlen(header)
					+ (size_t)givenArgTypeName.size()
					+ (size_t)expectedArgTypeName.size()
					+ sizeof(Cu::UInteger)*2;
		} else {
			format = "Function %s, arg %u of %u: %s\nExpected: %s";
			msgSize = (size_t)format.size()
					+ (size_t)logMessage.functionName.size()
					+ strlen(header)
					+ (size_t)expectedArgTypeName.size()
					+ sizeof(Cu::UInteger)*2;
		}
	}
	else {
		msgSize = (size_t)format.size()
				+ (size_t)logMessage.functionName.size()
				+ strlen(header)
				+ sizeof(Cu::UInteger)*2;
	}

	char msgFull[ msgSize ];

	if ( useExpectationMsg ) {
		if ( givenArgTypeName.size() > 0 ) {
			snprintf(msgFull, msgSize, format.c_str(),
					logMessage.functionName.c_str(), logMessage.argIndex, logMessage.argCount, header,
					expectedArgTypeName.c_str(), givenArgTypeName.c_str());
		} else {
			snprintf(msgFull, msgSize, format.c_str(),
					logMessage.functionName.c_str(), logMessage.argIndex, logMessage.argCount, header,
					expectedArgTypeName.c_str());
		}

	} else {
		snprintf(msgFull, msgSize, format.c_str(),
				logMessage.functionName.c_str(), logMessage.argIndex, logMessage.argCount, header);
	}

	print( logMessage.level, msgFull );
}

void
AppLogger::printCustomMessage( Cu::LogMessage  logMessage ) {

	stringc  message = getCuBridgeErrorMessage( logMessage.customCode );

	if ( message.size() != 0 ) {
		print( logMessage.level, message );
		return;
	}

	message = getCuMultiFileRunnerErrorMessage( logMessage.customCode );

	if ( message.size() != 0 ) {
		print( logMessage.level, message );
		return;
	}

	if ( messageProvider ) {
		message = messageProvider(logMessage.customCode);

		if ( message.size() != 0 ) {
			print( logMessage.level, message );
			return;
		}
	}
}
