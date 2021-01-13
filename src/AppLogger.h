// (C) 2018 Nicolaus Anderson

#ifndef APP_LOGGER_H
#define APP_LOGGER_H

#include <Copper.h>
#include <IWriteFile.h>
#include <IFileSystem.h>

// Pointer type for a function for returning the associated text with a log message.
// The code is provided from foreign functions to Copper passed via one of the following:
// 	void FFIServices::printCustomInfoCode( UInteger )
//	void FFIServices::printCustomWarningCode( UInteger )
//	void FFIServices::printCustomErrorCode( UInteger )
typedef const char*  (*CustomLogMessageProvider) ( unsigned );

// Pointer type for a function that returns a type name when given a Copper type value.
// This is useful for printing the name of a custom user object type.
typedef const char*  (*CustomTypeNameProvider) ( Cu::ObjectType::Value );

// ********* Writing Interface ********
// A class that receives irrlicht strings.
// It can be buffered.
// It is expected to perform its own formatting (such as adding a newline to the string end).
struct WriteInterface {
	// Writes the text to the buffer and returns success on writing.
	virtual bool write( const irr::core::stringc )=0;

	// Finishes writing the text by flushing any buffer (if there is one)
	virtual void flush()=0;
};


// ********* Logger Class *********
class AppLogger : public Cu::Logger {

	irr::io::IWriteFile*  loggingFile;
	CustomLogMessageProvider  messageProvider;
	CustomTypeNameProvider  typeNameProvider;
	WriteInterface*  writeInterface;

public:

	bool Enabled;
	bool AlwaysFlush;
	bool WriteToConsole;
	bool WriteToFile;
	bool WriteToInterface;
	bool ShowInfoMessages;
	bool ShowWarningMessages;
	bool ShowErrorMessages;

	AppLogger();
	~AppLogger();

	bool isReady(); // Checks both enabled status and device
	bool willShow( Cu::LogLevel::Value );

	bool createLoggingFile( irr::io::IFileSystem* );
	void setCustomMessageProvider( CustomLogMessageProvider  provider );
	void setCustomTypeNameProvider( CustomTypeNameProvider  provider );
	void setWriteInterface( WriteInterface* );

	void write( const irr::core::stringc&  message, bool  isError = false ); // Write a string to the output channels.
	void flush(); // Unnecessary to call if AlwaysFlush is set to true.

	virtual void print(const Cu::LogLevel::Value, const irr::core::stringc& );

	// Cu::Logger interface
	virtual void print(const Cu::LogLevel::Value, const char*);
	virtual void print(const Cu::LogLevel::Value, const Cu::EngineMessage::Value);
	virtual void print( Cu::LogMessage );
	virtual void printTaskTrace( Cu::TaskType::Value  taskType, const util::String&  taskName, Cu::UInteger  taskNumber );
	virtual void printStackTrace( const util::String&  frameName, Cu::UInteger  frameNumber );

protected:
	const char*  getCuBridgeErrorMessage( Cu::UInteger );
	const char*  getCuMultiFileRunnerErrorMessage( Cu::UInteger );
	util::String  getObjectTypeNameFromType( Cu::ObjectType::Value );
	const char*  getTaskType( Cu::TaskType::Value );

	void printStandardMessage( Cu::LogMessage );
	void printVerboseLogMessage( Cu::LogMessage, const char* );
	void printCustomMessage( Cu::LogMessage );
};

#endif
