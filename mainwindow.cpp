#include "mainwindow.h"
#include "reader.h"
#include "ui_mainwindow.h"

#include <QAudioSource>
#include <QBuffer>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <fstream>
#include <lsl_cpp.h>
#include <string>
#include <vector>

lsl::channel_format_t sampleFormatToLSL(QAudioFormat::SampleFormat fmt) {
	switch (fmt) {
		case QAudioFormat::Float: return lsl::cf_float32;
		case QAudioFormat::Int32: return lsl::cf_int32;
		case QAudioFormat::Int16: return lsl::cf_int16;
		case QAudioFormat::UInt8: return lsl::cf_int8;
		default: return lsl::cf_float32;
	}
}

// Helper to convert SampleFormat to string
QString sampleFormatToString(QAudioFormat::SampleFormat fmt) {
    switch (fmt) {
        case QAudioFormat::UInt8: return "UInt8";
        case QAudioFormat::Int16: return "Int16";
        case QAudioFormat::Int32: return "Int32";
        case QAudioFormat::Float: return "Float";
        default: return "Unknown";
    }
}

// Helper to convert string to SampleFormat
QAudioFormat::SampleFormat stringToSampleFormat(const QString &str) {
    if (str == "UInt8") return QAudioFormat::UInt8;
    if (str == "Int16") return QAudioFormat::Int16;
    if (str == "Int32") return QAudioFormat::Int32;
    if (str == "Float") return QAudioFormat::Float;
    return QAudioFormat::Unknown;
}

MainWindow::MainWindow(QWidget *parent, const char *config_file)
	: QMainWindow(parent), ui(new Ui::MainWindow),
	  devices(QMediaDevices::audioInputs()) {
	if(devices.empty()) {
		QMessageBox::warning(this, "Fatal error", "No capture devices found, quitting.");
		exit(1);
	}
	ui->setupUi(this);

	connect(ui->actionLoad_Configuration, &QAction::triggered, [this]() {
		load_config(QFileDialog::getOpenFileName(
			this, "Load Configuration File", "", "Configuration Files (*.cfg)"));
	});

	connect(ui->actionSave_Configuration, &QAction::triggered, [this]() {
		save_config(QFileDialog::getSaveFileName(
			this, "Save Configuration File", "", "Configuration Files (*.cfg)"));
	});

	connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

	connect(ui->actionAbout, &QAction::triggered, [this]() {
		QString infostr = QStringLiteral("LSL library version: ") +
						  QString::number(lsl::library_version()) +
						  "\nLSL library info:" + lsl::library_info();
		QMessageBox::about(this, "About this app", infostr);
	});

	connect(ui->linkButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);

	// audio devices
	for (const auto &dev : devices) ui->input_device->addItem(dev.description());
	auto changeSignal = static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged);

	connect(ui->input_device, changeSignal, this, &MainWindow::deviceChanged);
	deviceChanged();

	connect(ui->btn_checkfmt, &QPushButton::clicked, this, &MainWindow::checkAudioFormat);

	QString cfgfilepath = find_config_file(config_file);
	load_config(cfgfilepath);
	checkAudioFormat();
}

QAudioDevice MainWindow::currentDevice() const {
	return devices.at(ui->input_device->currentIndex());
}

void MainWindow::deviceChanged() {
	auto const dev = currentDevice();

	// Channel count
	ui->input_channels->setMinimum(dev.minimumChannelCount());
	ui->input_channels->setMaximum(dev.maximumChannelCount());
	ui->input_channels->setValue(dev.preferredFormat().channelCount());
	// Sample rate
	ui->input_samplerate->setMinimum(dev.minimumSampleRate());
	ui->input_samplerate->setMaximum(dev.maximumSampleRate());
	ui->input_samplerate->setValue(dev.preferredFormat().sampleRate());
	// Sample format
	ui->input_samplesize->clear();
	for (auto samp_fmt : dev.supportedSampleFormats()) {
		ui->input_samplesize->addItem(sampleFormatToString(samp_fmt), QVariant::fromValue(samp_fmt));
	}
	auto const fmt(dev.preferredFormat());
	if (const auto idx = ui->input_samplesize->findData(QVariant::fromValue(fmt.sampleFormat())); idx >= 0) ui->input_samplesize->setCurrentIndex(idx);
	setFmt(fmt);
}

QAudioFormat MainWindow::selectedAudioFormat() const {
	const auto dev = currentDevice();
	QAudioFormat fmt(dev.preferredFormat());
	qInfo() << "Preferred: " << fmt;
	fmt.setSampleRate(ui->input_samplerate->value());
	fmt.setChannelCount(ui->input_channels->value());
	// fmt.setByteOrder(QAudioFormat::LittleEndian);
	// fmt.setChannelConfig(??);
	fmt.setSampleFormat(stringToSampleFormat(ui->input_samplesize->currentText()));
	return fmt;
}

void MainWindow::setFmt(const QAudioFormat &fmt) {
	qInfo() << "Setting fmt: " << fmt;
	ui->input_samplerate->setValue(fmt.sampleRate());
	ui->input_samplesize->setCurrentText(sampleFormatToString(fmt.sampleFormat()));
	ui->input_channels->setValue(fmt.channelCount());
	auto fmtStr = QStringLiteral("%1 channels, %2 bit @ %3 Hz")
					  .arg(fmt.channelCount())
					  .arg(fmt.sampleFormat())
					  .arg(fmt.sampleRate());
	ui->label_fmtresult->setText(fmtStr);
}

void MainWindow::checkAudioFormat() {
	auto fmt = selectedAudioFormat();
	if (const auto dev = currentDevice(); dev.isFormatSupported(fmt))
		qInfo() << "Format is supported";
	else {
		QMessageBox::warning(this, "Format not supported",
			"The requested format isn't supported; the preferred format was automatically selected.");
		fmt = dev.preferredFormat();
	}
	setFmt(fmt);
}

void MainWindow::load_config(const QString &filename) const {
	const QSettings settings(filename, QSettings::Format::IniFormat);
	ui->input_name->setText(settings.value("AudioCapture/name", "MyAudioStream").toString());
	ui->input_device->setCurrentIndex(settings.value("AudioCapture/device", 0).toInt());
	ui->input_samplerate->setValue(settings.value("AudioCapture/samplerate", 1).toInt());
	const QString sampleSize = settings.value("AudioCapture/samplesize", "Int16").toString();
	ui->input_samplesize->setCurrentIndex(ui->input_samplesize->findData(QVariant::fromValue(sampleSize)));
	ui->input_channels->setValue(settings.value("AudioCapture/channels", 0).toInt());
}

void MainWindow::save_config(const QString &filename) const {
	QSettings settings(filename, QSettings::Format::IniFormat);
	settings.beginGroup("AudioCapture");
	settings.setValue("name", ui->input_name->text());
	settings.setValue("device", ui->input_device->currentIndex());
	settings.setValue("samplerate", ui->input_samplerate->value());
	settings.setValue("samplesize", ui->input_samplesize->currentText());
	settings.setValue("channels", ui->input_channels->value());
	settings.sync();
}

void MainWindow::closeEvent(QCloseEvent *ev) {
	if (reader) {
		QMessageBox::warning(this, "Recording still running", "Can't quit while recording");
		ev->ignore();
	}
}

void MainWindow::toggleRecording() {
	if (!reader) {
		// read the configuration from the UI fields
		auto const name = ui->input_name->text().toStdString();
		auto fmt = selectedAudioFormat();
		auto const channel_count = fmt.channelCount();
		auto const samplerate = fmt.sampleRate();
		auto const channel_format = sampleFormatToLSL(fmt.sampleFormat());
		auto const stream_id = currentDevice().description().toStdString();

		// Create the LSL stream info
		lsl::stream_info info(name, "Audio", channel_count, samplerate, channel_format, stream_id);
		info.desc().append_child("provider").append_child_value("api", "QtMultimedia");
		info.desc().append_child_value("device", ui->input_device->currentText().toStdString());

		// Create and open the QIODevice that will receive the audio data
		reader = std::make_unique<LslPusher>(lsl::stream_outlet(info));
		reader->open(QIODevice::OpenModeFlag::WriteOnly);

		// Create the AudioSource
		audiosrc = std::make_unique<QAudioSource>(currentDevice(), fmt, this);
		// auto const buffer_ms = ui->input_buffersize->value();
		// audiosrc->setBufferSize(fmt.bytesForDuration(2 * buffer_ms * 1000));

		// Start sinking audio data to the LSL IO device
		audiosrc->start(&*reader);
		qInfo() << audiosrc->state() << ' ' << audiosrc->error();
		ui->linkButton->setText("Unlink");
	} else {
		qInfo() << "Read " << reader->pos() << " bytes, " <<
				   reader->samples_written() << " samples, " <<
				   (static_cast<double>(reader->samples_written())/audiosrc->format().sampleRate()) << 's';
		audiosrc->stop();
		qInfo() << audiosrc->state() << ' ' << audiosrc->error();
		reader->close();
		audiosrc = nullptr;
		reader = nullptr;
		ui->linkButton->setText("Link");
	}
}


/**
 * Find a config file to load. This is (in descending order or preference):
 * - a file supplied on the command line
 * - [executablename].cfg in one the following folders:
 *	- the current working directory
 *	- the default config folder, e.g. '~/Library/Preferences' on OS X
 *	- the executable folder
 * @param filename	Optional file name supplied e.g. as command line parameter
 * @return Path to a found config file
 */
QString MainWindow::find_config_file(const char *filename) {
	if (filename) {
		QString qfilename(filename);
		if (!QFileInfo::exists(qfilename))
			QMessageBox(QMessageBox::Warning, "Config file not found",
				QStringLiteral("The file '%1' doesn't exist").arg(qfilename), QMessageBox::Ok,
				this);
		else
			return qfilename;
	}
	const QFileInfo exeInfo(QCoreApplication::applicationFilePath());
	const QString defaultCfgFilename(exeInfo.completeBaseName() + ".cfg");
	QStringList cfgpaths;
	cfgpaths << QDir::currentPath()
			 << QStandardPaths::standardLocations(QStandardPaths::ConfigLocation) << exeInfo.path();
	for (const auto& path : cfgpaths) {
		QString cfgfilepath = path + QDir::separator() + defaultCfgFilename;
		if (QFileInfo::exists(cfgfilepath)) return cfgfilepath;
	}
	QMessageBox msg_box(QMessageBox::Warning, "No config file not found",
		QStringLiteral("No default config file could be found"), QMessageBox::Ok, this);
	return "";
}


MainWindow::~MainWindow() noexcept = default;
