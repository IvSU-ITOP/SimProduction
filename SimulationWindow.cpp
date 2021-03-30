#include "SimulationWindow.h"
#include "SimulationObjects.h"
#include "qheaderview.h"

extern QTextCodec *pCodec;
extern QSqlDatabase s_DB;

MainWindow::MainWindow ()
{
  setWindowState (Qt::WindowMaximized);
  QMenu *pMenuBase = new QMenu (Rus ("&Моделирование"));
  pMenuBase->addAction (Rus ("&Ввести основные параметры..."), this, SLOT (slotOpenDialog ()), QKeySequence ("CTRL+D"));
  m_pModeling = pMenuBase->addAction (Rus ("&Начать моделирование "), this, SLOT (slotOpenSumulation ()), QKeySequence ("CTRL+S"));
  menuBar ()->addMenu (pMenuBase);
  pMenuBase->addAction (Rus ("&Конец работы "), qApp, SLOT (quit ()), QKeySequence ("CTRL+Q"));
  menuBar ()->addMenu (pMenuBase);
  QMenu *pMenuTable = new QMenu (Rus ("&Таблицы"));
  m_pTableNine = pMenuTable->addAction (Rus ("Отобразить Таблицу 9..."), this, SLOT (slotShowTableNine ()), QKeySequence ("CTRL+N"));
  m_pTableTen = pMenuTable->addAction (Rus ("Отобразить Таблицу 10..."), this, SLOT (slotShowTableTen ()), QKeySequence ("CTRL+T"));
  m_pMachineChartOne = pMenuTable->addAction (Rus ("Отобразить Машинограмму №1..."), this, SLOT (slotShowMachineChartOne ()), QKeySequence ("CTRL+O"));
  m_pMachineChartTwo = pMenuTable->addAction (Rus ("Отобразить Машинограмму №2..."), this, SLOT (slotShowMachineChartTwo ()), QKeySequence ("CTRL+M"));
  m_pMachineChartFour = pMenuTable->addAction (Rus ("Отобразить Машинограмму №4..."), this, SLOT (slotShowMachineChartFour ()), QKeySequence ("CTRL+F"));
  m_pModeling->setEnabled (false);
  m_pTableNine->setEnabled (false);
  m_pTableTen->setEnabled (false);
  m_pMachineChartOne->setEnabled (false);
  m_pMachineChartTwo->setEnabled (false);
  m_pMachineChartFour->setEnabled (false);
  menuBar ()->addMenu (pMenuTable);
  show ();
}

void MainWindow::slotOpenDialog ()
{
  DlgInputParms D;
  if (D.exec () == QDialog::Rejected) return;
  m_pModeling->setEnabled (true);
  m_pTableNine->setEnabled (true);
}

void MainWindow::slotOpenSumulation()
  {
  ProductionState::sm_pProductionState->Evaluate();
  m_pModeling->setEnabled( false );
  m_pTableTen->setEnabled (true);
  m_pMachineChartOne->setEnabled (true);
  m_pMachineChartTwo->setEnabled (true);
  m_pMachineChartFour->setEnabled (true);
  }

void MainWindow::slotShowTableNine ()
{
    QSqlQuery Query;
    QTableView *pView = new QTableView;
    QSqlQueryModel *pModel = new QSqlQueryModel;
    QString m_SQLQuery = Rus (" SELECT [Номер_изделия], [Номер_детали], [Применяемость], ") +
      Rus (" (SELECT[Количество, шт.]  from [Варианты] where [Номер варианта] = ") + QString::number (Route::sm_Variant) +
      Rus (" and [Варианты].[Номер изделия] = [Состав_изделия].[Номер_изделия]) as [План производства изделий],") +
      Rus (" (SELECT[Количество, шт.]  from [Варианты] where [Номер варианта] = ") + QString::number (Route::sm_Variant) + 
      Rus (" and [Варианты].[Номер изделия] = [Состав_изделия].[Номер_изделия])  * [Применяемость] as[План производства деталей] ") +
      Rus ("from[Состав_изделия] ");
    Query.exec (m_SQLQuery);
    pModel->setQuery (m_SQLQuery);
    pView->setModel (pModel);
    pView->showMaximized ();
}

void MainWindow::slotShowTableTen ()
{
  QTableWidget *tbl = new QTableWidget (8, 12);
  QStringList lst;
  lst << Rus ("Название детали") << Rus ("Суммарное время \n (штучное)") << Rus ("Суммарное время \n подготовительно-\nзаключительное")
    << Rus ("Размер партии \n предварительный") << Rus ("Размер партии \nпринятый ") << Rus ("Время выполнения\n технологических\n операций ")
    << Rus ("Суммарное время\n межоперационного \n пролеживания") << Rus ("Длительность \nпроизводственного\n цикла\n на проектируемом\n участке (час.)")
    << Rus ("Длительность \n  производственного  \n цикла \nна проектируемом\n участке (сут.)") << Rus ("Время обработки \nв других \nцехах (час)")
    << Rus ("Время обработки\n в других \nцехах (сут.)") << Rus ("Обшая длительность\n производственного\n цикла (сут.)");
  tbl->setHorizontalHeaderLabels (lst);
  tbl->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  QTableWidgetItem *pi = 0;
  int i = 0;
  for (QVector<Route>::iterator route = ProductionState::sm_pProductionState->begin (); route != ProductionState::sm_pProductionState->end (); route++)
  {
    QString Name = route->DetailName ();//1
    double SumDetailTime = route->SumDetailTime();//2
    double SumReadjustmentTime = route->SumReadjustmentTime();//3
    double PrelVolumeParty = route->PrelVolumeParty();//4
    double VolumeParty = route->VolumeParty();//5
    double ExTechOper = SumDetailTime*VolumeParty;//6
    double SumLieTime = route->SumLieTime();//7
    double SumGalTime = SumReadjustmentTime + ExTechOper + SumLieTime;//8
    double SumGalDay = SumGalTime / Time::DobeLength ();//9
    double SumGalvanic = route->SumGalvanic();//10
    double SumGalvanicDay = SumGalvanic / Time::DobeLength ();//11
    double AllTime = SumGalDay + SumGalvanicDay;//12
    pi = new QTableWidgetItem (Name);
    tbl->setItem (i, 0, pi);
    pi = new QTableWidgetItem (QString::number (SumDetailTime));
    tbl->setItem (i, 1, pi);
    pi = new QTableWidgetItem (QString::number (SumReadjustmentTime));
    tbl->setItem (i, 2, pi);
    pi = new QTableWidgetItem (QString::number (PrelVolumeParty));
    tbl->setItem (i, 3, pi);
    pi = new QTableWidgetItem (QString::number (VolumeParty));
    tbl->setItem (i, 4, pi);
    pi = new QTableWidgetItem (QString::number (ExTechOper));
    tbl->setItem (i, 5, pi);
    pi = new QTableWidgetItem (QString::number (SumLieTime));
    tbl->setItem (i, 6, pi);
    pi = new QTableWidgetItem (QString::number (SumGalTime));
    tbl->setItem (i, 7, pi);
    pi = new QTableWidgetItem (QString::number (SumGalDay));
    tbl->setItem (i, 8, pi);
    pi = new QTableWidgetItem (QString::number (SumGalvanic));
    tbl->setItem (i, 9, pi);
    pi = new QTableWidgetItem (QString::number (SumGalvanicDay));
    tbl->setItem (i, 10, pi);
    pi = new QTableWidgetItem (QString::number (AllTime));
    tbl->setItem (i, 11, pi);
    i++;
  }
  tbl->showMaximized();
}

void MainWindow::slotShowMachineChartOne()
  {
  QTableWidget *tbl = new QTableWidget( 4, 10 );
  QTableWidgetItem *pi = 0;
  QStringList lst;
  lst << Rus( "Номер изделия" ) << Rus( "План" ) << Rus( "Факт" )
    << Rus( "Отклонение" ) << Rus( "Некомплектованные\n детали" ) << Rus( "Некомплектованные\n детали" )
    << Rus( "Некомплектованные\n детали " ) << Rus( "Потребность до  \n планового объема" )
    << Rus( "Потребность до  \n планового объема" ) << Rus( "Потребность до  \n планового объема" );
  tbl->setHorizontalHeaderLabels( lst );
  tbl->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  QMap<int, Incomplete> &IP = ProductionState::sm_pProductionState->m_IncompleteProduction;
  int Detail1 = IP[1].m_Detail;
  int Detail2 = IP[2].m_Detail;
  int Detail3 = IP[3].m_Detail;
  int Detail4 = IP[4].m_Detail;
  int Detail5 = IP[5].m_Detail;
  int Detail6 = IP[6].m_Detail;
  int Detail7 = IP[7].m_Detail;
  int Detail8 = IP[8].m_Detail;
  int i = 0;
  for( auto pProduct = ProductionState::sm_pProductionState->m_Ware.begin(); pProduct != ProductionState::sm_pProductionState->m_Ware.end(); 
    pProduct++, i++ )
    {
    QString NameProduct = pProduct->m_Name;
    int PlanProduct = pProduct->m_Plan;
    int FinishPlan = pProduct->m_Finished;
    int Deviation = FinishPlan - PlanProduct;
    pi = new QTableWidgetItem( NameProduct );
    tbl->setItem( i, 0, pi );
    pi = new QTableWidgetItem( QString::number( PlanProduct ) );
    tbl->setItem( i, 1, pi );
    pi = new QTableWidgetItem( QString::number( FinishPlan ) );
    tbl->setItem( i, 2, pi );
    pi = new QTableWidgetItem( QString::number( Deviation ) );
    tbl->setItem( i, 3, pi );
    if( i == 0 )
      {
      pi = new QTableWidgetItem( "1 = " + QString::number( Detail1 ) );
      tbl->setItem( i, 4, pi );
      pi = new QTableWidgetItem( "2 = " + QString::number( Detail2 ) );
      tbl->setItem( i, 5, pi );
      pi = new QTableWidgetItem( "3 = " + QString::number( Detail3 ) );
      tbl->setItem( i, 6, pi );
      }
    if( i == 1 )
      {
      pi = new QTableWidgetItem( "4 = " + QString::number( Detail4 ) );
      tbl->setItem( i, 4, pi );
      pi = new QTableWidgetItem( "5 = " + QString::number( Detail5 ) );
      tbl->setItem( i, 5, pi );
      }
    if( i == 2 )
      {
      pi = new QTableWidgetItem( "6 = " + QString::number( Detail6 ) );
      tbl->setItem( i, 4, pi );
      }
    if( i == 3 )
      {
      pi = new QTableWidgetItem( "7 = " + QString::number( Detail7 ) );
      tbl->setItem( i, 4, pi );
      pi = new QTableWidgetItem( "8 = " + QString::number( Detail8 ) );
      tbl->setItem( i, 5, pi );
      }
    }
  ProductionState &PS = *ProductionState::sm_pProductionState;
  for( int j = 0; j < PS.count(); j++ )
    {
    double Detail = PS[j].PlanCount() - PS[j].ReadyCount();
    if( j == 0 )
      tbl->setItem( 0, 7, new QTableWidgetItem( "1 = " + QString::number( Detail ) ) );
    if( j == 1 )
      tbl->setItem( 0, 8, new QTableWidgetItem( "2 = " + QString::number( Detail ) ) );
    if( j == 2 )
      tbl->setItem( 0, 9, pi = new QTableWidgetItem( "3 = " + QString::number( Detail ) ) );
    if( j == 3 )
      tbl->setItem( 1, 7, new QTableWidgetItem( "4 = " + QString::number( Detail ) ) );
    if( j == 4 )
      tbl->setItem( 1, 8, new QTableWidgetItem( "5 = " + QString::number( Detail ) ) );
    if( j == 5 )
      tbl->setItem( 2, 7, pi = new QTableWidgetItem( "6 = " + QString::number( Detail ) ) );
    if( j == 6 )
      tbl->setItem( 3, 7, new QTableWidgetItem( "7 = " + QString::number( Detail ) ) );
    if( j == 7 )
      tbl->setItem( 3, 8, new QTableWidgetItem( "8 = " + QString::number( Detail ) ) );
    }
  tbl->showMaximized();
  }

void MainWindow::slotShowMachineChartTwo()
  {
  QTableWidget *pTbl = new QTableWidget( 8, 8 );
  QTableWidgetItem *pItem = 0;
  QStringList Lst;
  Lst << Rus( "Название детали" ) << Rus( "НЗП на начало" ) << Rus( "Готовые детали \n на начало" )
    << Rus( "Запуск" ) << Rus( "НЗП на конец" ) << Rus( "Готовые детали \n на конец" )
    << Rus( "Деталей в изделиях" ) << Rus( "Баланс \n Н - К" );
  pTbl->setHorizontalHeaderLabels( Lst );
  pTbl->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  int i = 0;
  QMap<int, Incomplete> &IPOld = ProductionState::sm_pProductionState->m_IncompleteProductionOld;
  QMap<int, Incomplete> &IP = ProductionState::sm_pProductionState->m_IncompleteProduction;
  for( QVector<Route>::iterator pRoute = ProductionState::sm_pProductionState->begin(); pRoute != ProductionState::sm_pProductionState->end(); 
    pRoute++, i++ )
    {
    pItem = new QTableWidgetItem( pRoute->DetailName() );
    pTbl->setItem( i, 0, pItem );
    int StartPartyIncomplete = IPOld[i + 1].m_Party;
    int StartDetailIncomplete = StartPartyIncomplete * pRoute->OldVolumeParty();
    pItem = new QTableWidgetItem( QString::number( StartDetailIncomplete ) + '(' + QString::number( StartPartyIncomplete ) + ')' );
    pTbl->setItem( i, 1, pItem );
    int StartReady = IPOld[i + 1].m_Detail;
    pItem = new QTableWidgetItem( QString::number( StartReady ) );
    pTbl->setItem( i, 2, pItem );
    int PartyCount = pRoute->PartyCount();
    int Start = PartyCount * pRoute->VolumeParty();
    pItem = new QTableWidgetItem( QString::number( Start ) + '(' + QString::number( PartyCount ) + ')' );
    pTbl->setItem( i, 3, pItem );
    int EndPartyIncomplete = IP[i + 1].m_Party;
    int EndDetailIncomplete = EndPartyIncomplete * pRoute->VolumeParty();
    pItem = new QTableWidgetItem( QString::number( EndDetailIncomplete ) + '(' + QString::number( EndPartyIncomplete ) + ')' );
    pTbl->setItem( i, 4, pItem );
    int EndReady = IP[i + 1].m_Detail;
    pItem = new QTableWidgetItem( QString::number( EndReady ) );
    pTbl->setItem( i, 5, pItem );
    QVector<Product> &Ware = ProductionState::sm_pProductionState->m_Ware;
    int iProduct = 0;
    for( ; iProduct < Ware.count() && Ware[iProduct].m_Id != pRoute->ProductId(); iProduct++ );
    int InProduct = Ware[iProduct].m_Finished * pRoute->Applicability();
    pItem = new QTableWidgetItem( QString::number( InProduct ) );
    pTbl->setItem( i, 6, pItem );
    int Balance = InProduct + EndReady + EndDetailIncomplete - StartReady - StartDetailIncomplete - Start;
    pItem = new QTableWidgetItem( QString::number( Balance ) );
    pTbl->setItem( i, 7, pItem );
    }
  pTbl->showMaximized();
  }

void MainWindow::slotShowMachineChartFour ()
{
  QTableWidget *tbl = new QTableWidget (11, 9);
  QTableWidgetItem *pItem = 0;
  QStringList lst;
  lst << Rus ("Наименование группы\n оборудования") << Rus ("Режимный фонд\n времени") << Rus ("Общий фонд\n времени")
    << Rus ("Время ремонта\n оборудования") << Rus ("Простои по причине\n отстутствия рабочих") << Rus ("Время простоя\n оборудования")
    << Rus ("Время на переналадку\n оборудования") << Rus ("Время работы") << Rus ("Общее время\n работы");
  tbl->setHorizontalHeaderLabels (lst);
  tbl->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
  int i = 0;
  for (auto pActing = GroupEquipment::sm_pSimulator->m_ActingBlocks.begin (); pActing != GroupEquipment::sm_pSimulator->m_ActingBlocks.end (); pActing++)
  {
    GroupEquipment* pGE = dynamic_cast<GroupEquipment *>(*pActing);
    if (pGE == nullptr) continue;
    QString Name =  pGE->Name();//1
    double TimeFundMode = 377.2;//2
    double TotalWorkFund = pGE->TotalWorkFund ();//3
    double RepairTime = pGE->TotalRepairTime();//4
    double TimeNoWorker = pGE->TimeNoWorker ();//5
    double TotalFreeTime = pGE->TotalFreeTime ();//6
    double TotalReadjustmentTime = pGE->TotalReadjustmentTime ();//7
    double WorkTime = pGE->TotalWorkTime ();//8
    double TotlaTime = RepairTime + TimeNoWorker + TotalReadjustmentTime + WorkTime + TotalFreeTime;//9
    pItem = new QTableWidgetItem (Name);
    tbl->setItem (i, 0, pItem);
    pItem = new QTableWidgetItem (QString::number (TimeFundMode));
    tbl->setItem (i, 1, pItem);
    pItem = new QTableWidgetItem (QString::number (TotalWorkFund));
    tbl->setItem (i, 2, pItem);
    pItem = new QTableWidgetItem (QString::number (RepairTime));
    tbl->setItem (i, 3, pItem);
    pItem = new QTableWidgetItem (QString::number (TimeNoWorker));
    tbl->setItem (i, 4, pItem);
    pItem = new QTableWidgetItem (QString::number (TotalFreeTime));
    tbl->setItem (i, 5, pItem);
    pItem = new QTableWidgetItem (QString::number (TotalReadjustmentTime));
    tbl->setItem (i, 6, pItem);
    pItem = new QTableWidgetItem (QString::number (WorkTime));
    tbl->setItem (i, 7, pItem);
    pItem = new QTableWidgetItem (QString::number (TotlaTime));
    tbl->setItem (i, 8, pItem);
    i++;
  }  
  tbl->showMaximized ();
}

DlgInputParms::DlgInputParms() : m_pShiftLength( new QDoubleSpinBox ),
m_pShiftCount( new QSpinBox ), m_pStartDay( new QDoubleSpinBox ),
m_pTimeSimulation( new QSpinBox ), m_pVariant( new QSpinBox )
  {
  QSqlQuery Query;
  Query.exec( Rus( "SELECT  min([Номер варианта]), max([Номер варианта]) FROM Варианты" ) );
  Query.next();
  QVBoxLayout *pVLayout = new QVBoxLayout;
  QBoxLayout *pHLayout = new QHBoxLayout;
  pHLayout->addWidget( new QLabel( Rus( "Введите номер Вашего варианта" ) ) );
  pHLayout->addWidget( m_pVariant );
  m_pVariant->setMaximum( Query.value( 1 ).toInt() );
  m_pVariant->setMinimum( Query.value( 0 ).toInt() );
  pVLayout->addLayout( pHLayout );
  pHLayout = new QHBoxLayout;
  pHLayout->addWidget( new QLabel( Rus( "Введите длину смены " ) ) );
  pHLayout->addWidget( m_pShiftLength );
  m_pShiftLength->setMaximum( 14 );
  m_pShiftLength->setMinimum( 3 );
  m_pShiftLength->setValue( Time::sm_ShiftLength );
  pVLayout->addLayout( pHLayout );
  pHLayout = new QHBoxLayout;
  pHLayout->addWidget( new QLabel( Rus( "Введите количество смен " ) ) );
  pHLayout->addWidget( m_pShiftCount );
  m_pShiftCount->setMaximum( 3 );
  m_pShiftCount->setMinimum( 1 );
  m_pShiftCount->setValue( Time::sm_ShiftCount );
  pVLayout->addLayout( pHLayout );
  pHLayout = new QHBoxLayout;
  pHLayout->addWidget( new QLabel( Rus( "Введите время начала рабочего дня " ) ) );
  pHLayout->addWidget( m_pStartDay );
  m_pStartDay->setMaximum( 12 );
  m_pStartDay->setMinimum( 5 );
  m_pStartDay->setValue( Time::sm_StartWork );
  pVLayout->addLayout( pHLayout );
  pHLayout = new QHBoxLayout;
  pHLayout->addWidget( new QLabel( Rus( "Введите продолжительность симуляции" ) ) );
  pHLayout->addWidget( m_pTimeSimulation );
  pVLayout->addLayout( pHLayout );
  pHLayout = new QHBoxLayout;
  m_pTimeSimulation->setMaximum( 120 );
  m_pTimeSimulation->setMinimum( 24 );
  m_pTimeSimulation->setValue( ProductionState::sm_WorkDayCount );
  QPushButton *pCancel = new QPushButton( Rus( "Отмена" ) );
  pHLayout->addWidget( pCancel );
  QPushButton *pSelect = new QPushButton( Rus( "Сохранить" ) );
  pHLayout->addWidget( pSelect );
  pVLayout->addLayout( pHLayout );
  setLayout( pVLayout );
  connect( pSelect, SIGNAL( clicked() ), SLOT( accept() ) );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) ); //7
  }

void DlgInputParms::accept ()
{
  Time::SetDayParameters (m_pShiftLength->value (), m_pShiftCount->value (), m_pStartDay->value ());
  double TimeAfterStartDay = 24.0 - Time::sm_StartWork; //Время, прошедшее от начала последнего рабочего дня. 
  double TimeAfterEndLastShift = TimeAfterStartDay - Time::DobeLength ();
  ProductionState::sm_WorkDayCount = m_pTimeSimulation->value();
  Simulator::sm_End = ceil( ProductionState::sm_WorkDayCount * 24.0 - TimeAfterEndLastShift - Time::sm_ReShift );
  Route::sm_Variant = m_pVariant->value ();
  QDialog::accept ();
}
