#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	QCommandLineParser parser;
	parser.setApplicationDescription("Capture local audio and stream it over LSL.");
	parser.addHelpOption();
	parser.addPositionalArgument("config", "Optional configuration file (.cfg) to load.");
	parser.addOption({"name", "LSL stream name." , "name"});
	parser.addOption({"device", "Audio input device description text." , "device"});
	parser.addOption({"samplerate", "Audio sample rate in Hz." , "hz"});
	parser.addOption({"sampleformat", "Sample format: UInt8, Int16, Int32, Float." , "format"});
	parser.addOption({"channels", "Number of audio channels." , "count"});
	parser.addOption({"buffer-ms", "Audio source buffer duration in milliseconds." , "ms"});
	parser.addOption({"auto-link", "Automatically start streaming at launch."});
	parser.process(a);

	const QStringList positionalArgs = parser.positionalArguments();
	QByteArray configFileBytes;
	const char *configFile = nullptr;
	if (!positionalArgs.isEmpty()) {
		configFileBytes = positionalArgs.first().toLocal8Bit();
		configFile = configFileBytes.constData();
	}
	MainWindow w(nullptr, configFile, parser);
	w.show();
	return a.exec();
}
