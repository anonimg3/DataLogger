#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QLineSeries>
#include <QTcpSocket>
#include <QValueAxis>

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

 private slots:
  void on_connectBtn_clicked(bool checked);
  void on_resetAxisBtn_clicked();

  void connected();
  void disconnected();
  void bytesWritten(qint64 bytes);
  void readyRead();
  void doConnect();

  void createVoltageChart();
  void createCurrentChart();
  void createPowerChart();
  void createTempChart();

 private:
  Ui::MainWindow* ui;
  QTcpSocket* socket;

  QLineSeries* volt_series;
  QChartView* volt_chartView;
  QChart* volt_chart;
  QValueAxis* volt_axisY;
  QValueAxis* volt_axisX;
  qreal volt_yMin;
  qreal volt_yMax;

  QLineSeries* cur_series;
  QChartView* cur_chartView;
  QChart* cur_chart;
  QValueAxis* cur_axisY;
  QValueAxis* cur_axisX;
  qreal cur_yMin;
  qreal cur_yMax;

  QLineSeries* power_series;
  QChartView* power_chartView;
  QChart* power_chart;
  QValueAxis* power_axisY;
  QValueAxis* power_axisX;
  qreal power_yMin;
  qreal power_yMax;

  QLineSeries* temp_series;
  QChartView* temp_chartView;
  QChart* temp_chart;
  QValueAxis* temp_axisY;
  QValueAxis* temp_axisX;
  qreal temp_yMin;
  qreal temp_yMax;
};

#endif  // MAINWINDOW_H
