#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  createVoltageChart();
  createCurrentChart();
  createPowerChart();
  createTempChart();

  ui->tcp_status_label->setText(
      "<font color='red'>Please click 'Connect' button</font>");

  loadCSSStyle();
  cnt = 0;
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_connectBtn_clicked(bool checked) {
  (checked) ? doConnect() : socket->close();
}

void MainWindow::on_resetAxisBtn_clicked() {
  volt_yMax = 0.1;
  volt_yMin = 0;
  volt_axisY->setRange(volt_yMin - volt_yMin * 0.1,
                       volt_yMax + volt_yMax * 0.1);
  cur_yMax = 0.1;
  cur_yMin = 0;
  cur_axisY->setRange(cur_yMin - cur_yMin * 0.1, cur_yMax + cur_yMax * 0.1);
  power_yMax = 0.1;
  power_yMin = 0;
  power_axisY->setRange(power_yMin - power_yMin * 0.1,
                        power_yMax + power_yMax * 0.1);
  temp_yMax = 0.1;
  temp_yMin = -0.1;
  temp_axisY->setRange(temp_yMin - temp_yMin * 0.1,
                       temp_yMax + temp_yMax * 0.1);
}

void MainWindow::on_resetSeriesBtn_clicked() {
  volt_series->clear();
  volt_axisX->setRange(-10, 0);

  cur_series->clear();
  cur_axisX->setRange(-10, 0);

  power_series->clear();
  power_axisX->setRange(-10, 0);

  temp_series->clear();
  temp_axisX->setRange(-10, 0);

  date_list.clear();
  cnt = 0;
}

void MainWindow::on_saveBtn_clicked() {
  QList<QPointF> volt_list = volt_series->points();
  QList<QPointF> cur_list = cur_series->points();
  QList<QPointF> power_list = power_series->points();
  QList<QPointF> temp_list = temp_series->points();
  QStringList list;
  QString bufor;
  int size = volt_list.size();

  if (size == 0) {
    QMessageBox::warning(this, tr("DataLogger"),
                         tr("There is no data to save!"), QMessageBox::Ok);
    return;
  }

  list.append("Lp;VOLTAGE[V];CURRENT[mA];POWER[mW];TEMPERATURE[C];DATE;TIME");
  for (int i = 0; i < size; i++) {
    bufor.clear();
    bufor.append(QString::number(i) + ";");
    bufor.append(QString::number(volt_list.at(i).y()) + ";");
    bufor.append(QString::number(cur_list.at(i).y()) + ";");
    bufor.append(QString::number(power_list.at(i).y()) + ";");
    bufor.append(QString::number(temp_list.at(i).y()) + ";");
    bufor.append(date_list.at(i));
    list.append(bufor);
  }
  saveToFile(list);
}

void MainWindow::connected() {
  ui->tcp_status_label->setText(
      "<font color='green'>Connected with 192.168.1.100:8899</font>");
  ui->connectBtn->setText("Connected");
}

void MainWindow::disconnected() {
  ui->tcp_status_label->setText("<font color='red'>Disconnected</font>");
  ui->connectBtn->setText("Connect");
}

void MainWindow::readyRead() {
  QString bufor = socket->readAll();
  QStringList list1 = bufor.split(',');

  ui->tcp_status_label->setText("<font color='green'>Reading...</font>");

  if (list1.size() == 7) {
    QString voltage = list1.at(0);
    volt_series->append(cnt, voltage.toDouble());
    volt_chart->scroll(volt_chart->plotArea().width() / 20, 0);
    ui->voltageLbl->setText("Voltage: " + voltage + " V");

    QString current = list1.at(1);
    cur_series->append(cnt, current.toDouble());
    cur_chart->scroll(cur_chart->plotArea().width() / 20, 0);
    ui->currentLbl->setText("Current: " + current + " mA");

    QString power = list1.at(2);
    power_series->append(cnt, power.toDouble());
    power_chart->scroll(power_chart->plotArea().width() / 20, 0);
    ui->powerLbl->setText("Power:   " + power + " mW");

    QString date = list1.at(3);
    QString time = list1.at(4);
    date_list.append(date + ";" + time);
    ui->dateTimeLbl->setText(time + " | " + date);

    QString temp = list1.at(5);
    temp_series->append(cnt, temp.toDouble());
    temp_chart->scroll(temp_chart->plotArea().width() / 20, 0);
    ui->tempLbl->setText("Temp:    " + temp + " °C");

    cnt += 0.5;
  }
}

void MainWindow::doConnect() {
  socket = new QTcpSocket(this);

  connect(socket, SIGNAL(connected()), this, SLOT(connected()));
  connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

  ui->tcp_status_label->setText("<font color='green'>Connecting...</font>");

  socket->connectToHost("192.168.1.100", 8899);
  /* we need to wait... */
  if (!socket->waitForConnected(1000)) {
    ui->tcp_status_label->setText("<font color='red'>" + socket->errorString() +
                                  "</font>");
  }
}

void MainWindow::loadCSSStyle() {
  QFile styleSheet("style.qss");
  if (!styleSheet.open(QIODevice::ReadOnly)) {
    return;
  }
  qApp->setStyleSheet(styleSheet.readAll());
}

void MainWindow::saveToFile(QStringList data) {
  QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save Logs"), "Logs.txt",
                                   tr("Text files (*.txt);;All Files (*)"));
  if (fileName.isEmpty()) {
    return;
  } else {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
      QMessageBox::information(this, tr("Unable to open file"),
                               file.errorString());
      return;
    }

    QTextStream out(&file);

    for (int i = 0; i < data.size(); i++) {
      out << data.at(i) << '\n';
    }
    file.flush();
    file.close();
  }
}

void MainWindow::createVoltageChart() {
  volt_series = new QLineSeries(this);
  QPen blue(Qt::blue);
  blue.setWidth(3);
  volt_series->setPen(blue);
  volt_chart = new QChart();
  volt_chart->addSeries(volt_series);
  volt_chart->legend()->hide();
  volt_chart->setAnimationOptions(QChart::AllAnimations);
  volt_axisX = new QValueAxis;
  volt_axisX->setTickCount(5);
  volt_axisX->setRange(-10, 0);
  volt_chart->addAxis(volt_axisX, Qt::AlignBottom);
  volt_series->attachAxis(volt_axisX);
  volt_axisY = new QValueAxis;
  volt_axisY->setTitleText("U [V]");
  volt_chart->addAxis(volt_axisY, Qt::AlignLeft);
  volt_series->attachAxis(volt_axisY);
  volt_chartView = new QChartView(volt_chart);
  volt_chartView->setRenderHint(QPainter::Antialiasing);
  ui->layout->addWidget(volt_chartView);
  volt_yMax = 0.1;
  volt_yMin = 0;

  QObject::connect(volt_series, &QLineSeries::pointAdded, [=](int index) {
    qreal y = volt_series->at(index).y();

    if (y < volt_yMin || y > volt_yMax) {
      if (y < volt_yMin)
        volt_yMin = y;
      if (y > volt_yMax)
        volt_yMax = y;
      volt_axisY->setRange(volt_yMin - volt_yMin * 0.1,
                           volt_yMax + volt_yMax * 0.1);
    }
  });
}

void MainWindow::createCurrentChart() {
  cur_series = new QLineSeries(this);
  QPen red(Qt::red);
  red.setWidth(3);
  cur_series->setPen(red);
  cur_chart = new QChart();
  cur_chart->addSeries(cur_series);
  cur_chart->legend()->hide();
  cur_chart->setAnimationOptions(QChart::AllAnimations);
  cur_axisX = new QValueAxis;
  cur_axisX->setTickCount(5);
  cur_axisX->setRange(-10, 0);
  cur_chart->addAxis(cur_axisX, Qt::AlignBottom);
  cur_series->attachAxis(cur_axisX);
  cur_axisY = new QValueAxis;
  cur_axisY->setTitleText("I [mA]");
  cur_chart->addAxis(cur_axisY, Qt::AlignLeft);
  cur_series->attachAxis(cur_axisY);
  cur_chartView = new QChartView(cur_chart);
  cur_chartView->setRenderHint(QPainter::Antialiasing);
  ui->layout->addWidget(cur_chartView);
  cur_yMax = 0.1;
  cur_yMin = 0;

  QObject::connect(cur_series, &QLineSeries::pointAdded, [=](int index) {
    qreal y = cur_series->at(index).y();

    if (y < cur_yMin || y > cur_yMax) {
      if (y < cur_yMin)
        cur_yMin = y;
      if (y > cur_yMax)
        cur_yMax = y;
      cur_axisY->setRange(cur_yMin - cur_yMin * 0.1, cur_yMax + cur_yMax * 0.1);
    }
  });
}

void MainWindow::createPowerChart() {
  power_series = new QLineSeries(this);
  QPen darkGreen(Qt::darkGreen);
  darkGreen.setWidth(3);
  power_series->setPen(darkGreen);
  power_chart = new QChart();
  power_chart->addSeries(power_series);
  power_chart->legend()->hide();
  power_chart->setAnimationOptions(QChart::AllAnimations);
  power_axisX = new QValueAxis;
  power_axisX->setTickCount(5);
  power_axisX->setRange(-10, 0);
  power_chart->addAxis(power_axisX, Qt::AlignBottom);
  power_series->attachAxis(power_axisX);
  power_axisY = new QValueAxis;
  power_axisY->setTitleText("P [mW]");
  power_chart->addAxis(power_axisY, Qt::AlignLeft);
  power_series->attachAxis(power_axisY);
  power_chartView = new QChartView(power_chart);
  power_chartView->setRenderHint(QPainter::Antialiasing);
  ui->layout->addWidget(power_chartView);
  power_yMax = 0.1;
  power_yMin = 0;

  QObject::connect(power_series, &QLineSeries::pointAdded, [=](int index) {
    qreal y = power_series->at(index).y();

    if (y < power_yMin || y > power_yMax) {
      if (y < power_yMin)
        power_yMin = y;
      if (y > power_yMax)
        power_yMax = y;
      power_axisY->setRange(power_yMin - power_yMin * 0.1,
                            power_yMax + power_yMax * 0.1);
    }
  });
}

void MainWindow::createTempChart() {
  temp_series = new QLineSeries(this);
  QPen magenta(Qt::magenta);
  magenta.setWidth(3);
  temp_series->setPen(magenta);
  temp_chart = new QChart();
  temp_chart->addSeries(temp_series);
  temp_chart->legend()->hide();
  temp_chart->setAnimationOptions(QChart::AllAnimations);
  temp_axisX = new QValueAxis;
  temp_axisX->setTickCount(5);
  temp_axisX->setRange(-10, 0);
  temp_chart->addAxis(temp_axisX, Qt::AlignBottom);
  temp_series->attachAxis(temp_axisX);
  temp_axisY = new QValueAxis;
  temp_axisY->setTitleText("Temp [°C]");
  temp_chart->addAxis(temp_axisY, Qt::AlignLeft);
  temp_series->attachAxis(temp_axisY);
  temp_chartView = new QChartView(temp_chart);
  temp_chartView->setRenderHint(QPainter::Antialiasing);
  ui->layout->addWidget(temp_chartView);
  temp_yMax = 0.1;
  temp_yMin = 0;

  QObject::connect(temp_series, &QLineSeries::pointAdded, [=](int index) {
    qreal y = temp_series->at(index).y();

    if (y < temp_yMin || y > temp_yMax) {
      if (y < temp_yMin)
        temp_yMin = y;
      if (y > temp_yMax)
        temp_yMax = y;
      temp_axisY->setRange(temp_yMin - temp_yMin * 0.1,
                           temp_yMax + temp_yMax * 0.1);
    }
  });
}
