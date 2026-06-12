#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "ui_mainwindow.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <QCommandLineParser>
#include <QMainWindow>
#include <memory> //for std::unique_ptr

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent, const char *config_file,
		const QCommandLineParser &parser);
	~MainWindow() noexcept override;

private slots:
	void closeEvent(QCloseEvent *ev) override;
	void toggleRecording();
	void deviceChanged() const;
	void checkAudioFormat(bool showModal);

private:
	// Audio device handling
	[[nodiscard]] QAudioDevice currentDevice() const;
	void setFmt(const QAudioFormat &fmt) const;
	[[nodiscard]] QAudioFormat selectedAudioFormat() const;
	// void updateSampleRates();

	// function for loading / saving the config file
	QString find_config_file(const char *filename);
	void load_config(const QString &filename) const;
	void save_config(const QString &filename) const;
	void applyRuntimeConfiguration(const QCommandLineParser &parser) const;
	std::unique_ptr<class LslPusher> reader;
	std::unique_ptr<class QAudioSource> audiosrc;
	std::unique_ptr<Ui::MainWindow> ui; // window pointer
	QList<QAudioDevice> devices;
};

#endif // MAINWINDOW_H
