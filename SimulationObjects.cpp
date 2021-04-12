#include "SimulationObjects.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <qsqlerror.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qlabel.h>
#include <QPushButton>
#include <cmath>
#include <qinputdialog.h>
#include <QTableView>
#include <qdebug.h>
#include <qtimer.h>
#include <qthread.h>

extern QFile s_LogFile;

const double Time::sc_BadTime = 1e300;
Simulator* BaseActingBlock::sm_pSimulator = NULL;
const double BaseActingBlock::sc_NoTime = Time::sc_BadTime;
double Time::sm_ShiftLength = 8.0;
int Time::sm_ShiftCount = 1.0;
double Time::sm_StartWork = 5.0;
double Time::sm_ReShift = 0.0; // время пересменки 
double Time::sm_Precision = 0.0;
Time Simulator::sm_End;
Time Time::sm_CurrentTime(0);
QSqlDatabase s_DB(QSqlDatabase::addDatabase("QMYSQL"));
ProductionState* ProductionState::sm_pProductionState;
int ProductionState::sm_WorkDayCount = 24;
int Route::sm_Variant = 0;
Random Worker::sm_TestAppear;

double DRest(double Nom, int Denom)
  {
  int iNom = Nom;
  return iNom % Denom + (Nom - iNom);
  }

int Round( double Value )
  {
  double Sign = 1.0;
  if( Value < 0.0 )
    {
    Sign = -1.0;
    Value = -Value;
    }
  return floor(Value + 0.5) * Sign;
  }

double Round( double Value, int Digits )
  {
  double K = pow(10.0, Digits);
  return Round( Value * K ) / K;
  }

Actor::Actor(Time TimeStart, int DetailCount, Route *pRoute) : m_TimeStart(TimeStart),
  m_DetailCount(DetailCount), m_pRoute(pRoute), m_CurrentStep(-1), m_RawCount(DetailCount), 
  m_LieTime(0.0)
  {
  NextStep();
  }

ServiceResult Actor::SelectQueue( VectorQueues *pVQ )
  {
  if( m_CurrentStep == m_pRoute->count() ) 
    return BadActor;
  for (VectorQueues::iterator pQ = pVQ->begin(); pQ != pVQ->end(); pQ++)
    if ((*pQ)->Evaluate(this) == Success) return Success;
  return BadActor;
  }  //Выбираем очередь к той группе оборудования, которая может обслужить эту партию деталей.

bool Actor::GetRawDetail()
  {
  if (m_RawCount == 0 ) return false;
  m_RawCount--;
  return true;
  }
  
double Actor::GetGalvanicTime()
  {
  return (*m_pRoute)[m_CurrentStep].m_Galvanic;
  }

double Actor::GetLieTime()
  {
  if( m_CurrentStep == m_pRoute->count() ) return 0.0;
  return m_LieTime = ( *m_pRoute )[m_CurrentStep].m_LieTime;
  }

bool Actor::NextStep()
  {
  if( m_CurrentStep != -1  )
    qDebug() << "Next Step" << m_pRoute->DetailName() << "Step" << m_CurrentStep << ( *m_pRoute )[m_CurrentStep].m_GroupId;
  if( ++m_CurrentStep == m_pRoute->count() ) return false;
  m_RawCount = m_DetailCount;
  m_DetailWorkTime = RNormal( m_pRoute->at( m_CurrentStep ).m_DetailTime, 0.04 * m_pRoute->at( m_CurrentStep ).m_DetailTime ); //логарифм времени изготовления будет распределен по нормальному закону. Время обработки будет изменяться примерно на 10%
  double ReadjustmentTime = m_pRoute->at(m_CurrentStep).m_PartyTime;
  if( ReadjustmentTime == 0.0 )
    m_PartyWorkTime = RNormal();
  else
    m_PartyWorkTime = RNormal( ReadjustmentTime, 0.04 * ReadjustmentTime ); //логарифм подготовительно-заключительного времени будет распределен по нормальному закону. Время обработки будет изменяться примерно на 10%
  return true;
  }

double Actor::DetailWorkTime()
  {
  return m_DetailWorkTime.GetNumber();
  }

int Actor::GetCurrentGroupEquipment()
  { 
  if( m_CurrentStep >= m_pRoute->count() ) return -1;
//  qDebug() << "Actor CurrentGroupEquipment" << ( *m_pRoute )[m_CurrentStep].m_GroupId;
  return (*m_pRoute)[m_CurrentStep].m_GroupId;
  }

Simulator::Simulator() : m_CurrentActor(0)
  {
  ActingBlock::sm_pSimulator = this;
  }

void Simulator::Evaluate()
  {
  do
    {
    int iMinTime = 0;
    m_ActingBlocks[0]->StartWork();
    for( size_t iBlock = 1; iBlock < m_ActingBlocks.size(); iBlock++ )
      if( m_ActingBlocks[iBlock]->NextEventTime() < m_ActingBlocks[iMinTime]->NextEventTime() )
        iMinTime = iBlock;
    Time NextTime = m_ActingBlocks[iMinTime]->NextEventTime();
    if( NextTime == sm_End || NextTime > sm_End ) break;
    Time::sm_CurrentTime = NextTime;
    qDebug() << "Simulator" << Time::sm_CurrentTime.Hour() << m_ActingBlocks[iMinTime]->GetName();
    m_ActingBlocks[iMinTime]->Evaluate();
    } while( true );
  }

void Generator::StartWork()
  {
  ProductionState& PS = *ProductionState::sm_pProductionState;
  auto pFirstRoute = PS.begin();
  for( auto pRoute = pFirstRoute; pRoute != PS.end(); pRoute++ )
    if( pRoute->m_CorrectedTimeStartParty < pFirstRoute->m_CorrectedTimeStartParty ) pFirstRoute = pRoute;
  m_NextEventTime = pFirstRoute->m_CorrectedTimeStartParty;
  m_pNextRoute = &( *pFirstRoute );
  }

void Generator::Evaluate()
  {
  Actor *pParty = m_pNextRoute->GetParty();
  qDebug() << "Generator" << m_pNextRoute->m_DetailName << m_NextEventTime.Hour();
  m_pNextBlock->Evaluate( pParty );
  }

 void NamedBlock::Report()
   {
//   sm_Sout << "занятость объекта " << m_Name << ' ' << Round( m_WorkTime / Time::GetCurrentTime(), 4 ) << endl;
   }

 void NamedBlock::StartWork()
   {
   qDebug() << "Начало работы следущего времени события" << NextEventTime().Hour() << m_Name;
   }

 Adder::Adder(ActingBlock *pPrevBlock, ...)
   {
   va_list args;
   va_start(args, pPrevBlock);
   while (pPrevBlock != NULL)
     {
     pPrevBlock->SetNextBlock(this);
     pPrevBlock = va_arg(args, ActingBlock *);
     }
   va_end(args);
   }

ServiceResult Queue::Evaluate(Actor *pA)
  {
  ServiceResult R = m_pNextBlock->Evaluate( pA );
  if( R != IsBusy ) return R;
  if( m_Actors.empty() )
    m_TimeStart = Time::GetCurrentTime();
  qDebug() << "Queue? Type " << pA->DetailType() << "Addr " << pA << "Block" <<  m_pNextBlock->GetName();
  m_Actors.push_back( pA );
  m_MaxLen = max( m_MaxLen, m_Actors.size() );
  return Success;
  }
 
Actor* Queue::GetActor()
  {
  if( m_Actors.empty() ) return nullptr;
  auto pFirst = m_Actors.begin();
  for( auto pNext = ( pFirst + 1 ); pNext != m_Actors.end(); pNext++ )
    if( ( *pNext )->PlanPercent() < ( *pFirst )->PlanPercent() ) pFirst = pNext;
  Actor* Result = *pFirst;
  qDebug() << "GetActor" << Result->DetailType() << "ADDR" << Result;
  m_Actors.erase( pFirst );
  if( m_Actors.empty() ) m_WorkTime += Time::GetCurrentTime() - m_TimeStart;
  return Result;
  }

void Queue::Report()
  {
  NamedBlock::Report();
//  sm_Sout << "максимальная длина " << m_MaxLen << endl;
  }

ServiceResult Service::Evaluate(Actor *pA)
  {
  if( m_NextEventTime != sc_NoTime ) return IsBusy;
  m_pCurrentActor = pA;
  m_TimeStart = Time::GetCurrentTime();
//  m_NextEventTime = m_TimeStart + GetRNumber();
  return Success;
  }

ServiceResult Terminator::Evaluate(Actor *pA) 
  {
  if( pA->GetCurrentGroupEquipment() != -1 ) return BadActor;
  qDebug() << "Terminator, detail:" << pA->DetailType();
  ProductionState::sm_pProductionState->Finish( pA );
  return Success;
  }

void Terminator::Report()
  {
//  sm_Sout << "Прошло " << m_ActorCount << " акторов." << endl;
//  sm_Sout << "Минимальное время обслуживания " << m_MinTime << endl;
//  sm_Sout << "Максимальное время обслуживания " << m_MaxTime << endl;
//  sm_Sout << "Среднее время обслуживания " << m_SummTime / m_ActorCount << endl;
//  sm_Sout << "Средеквадратичное отклонение времени обслуживания " 
//    << sqrt( (m_SummTimeSquares * m_ActorCount - m_SummTime * m_SummTime) /( m_ActorCount * ( m_ActorCount - 1 ) ) ) << endl;
  }

void VectorQueues::SetNextBlock( ActingBlock* pAB )
  {
  Service *pService = (Service*) pAB;
  Queue *pQueue = new Queue( m_Prefix + pService->GetName(), this );
  pService->SetQueue( pQueue );
  push_back( pQueue );
  }

VectorQueues::~VectorQueues()
  {
  for( int iQueue = 0; iQueue < size(); delete at( iQueue++ ) );
  }

ServiceResult Switch::Evaluate(Actor *pActor)
  {
  size_t iNextBlock = 0;
  for( ; iNextBlock < m_NextBlocks.size() && m_NextBlocks[iNextBlock]->Evaluate( pActor ) != Success; iNextBlock++ );
  if( iNextBlock == m_NextBlocks.size() ) return IsBusy;
  return Success;
  }

Time::Time(double Hour) : Time()
  {
  if (Hour == sc_BadTime) return;
  if (Hour < 0.0) throw "Заданное время: " + QString::number(Hour) + " меньше нуля";
  m_Hour = Hour;
  } 

bool Time::SetDayParameters(double ShiftLength, int ShiftCount, double StartDay)
  {
  if ((ShiftLength*ShiftCount) > 24)
    {
    QMessageBox::critical(nullptr, "Ошибка! ", "Заданные параметры рабочего дня неверны");
    return false;
    }
  sm_ShiftLength = ShiftLength;
  sm_ShiftCount = ShiftCount;
  sm_StartWork = StartDay;
  sm_ReShift = 0.000001 * ShiftLength;
  sm_Precision = 0.00000001 * sm_ReShift;
  }

Time Time::EndCurrentShift() const
  {
  return floor( ( m_Hour - sm_StartWork ) / 24 ) * 24 + GetCurrentShift() * sm_ShiftLength + sm_StartWork - sm_ReShift;
  }

int Time::GetCurrentShift() const
  {
  int Result = DRest( m_Hour - sm_StartWork, 24 ) / sm_ShiftLength + 1;
  return Result;
  }

Time Time::NextShift() const
  {
  int iShift = GetCurrentShift();
  if( iShift >= sm_ShiftCount )
    return sm_CurrentTime + ( 24 - iShift * sm_ShiftLength + sm_ReShift );
  else
    return EndCurrentShift().m_Hour + sm_ReShift * 2;
  }

Time &Time::operator=(const Time &M) //1
  {
  m_Hour = M.m_Hour;
  return *this;
  }

Time &Time::operator += (double Par)
  {
  m_Hour += Par;
  return *this;
  }

Time Time::operator + (double Shift) const
  {
  return Time(*this) += Shift;
  }

bool Time::operator == (const Time &B) const
  {
  return B.m_Hour == m_Hour || B.m_Hour < m_Hour + sm_Precision && B.m_Hour > m_Hour - sm_Precision;
  }

bool Time::operator < (const Time&R) const
  {
  return m_Hour < R.m_Hour; 
  }

bool Time::operator > (const Time& H) const
  {
  return m_Hour > H.m_Hour; 
  }

double Time::operator - (const Time&M) const
  {
  return m_Hour - M.m_Hour;
  }

bool Time::operator != (const Time& N) const
  {
  return !((*this) == N);
  }

void IncompleteMap::Add( int Id, int Party, int Detail )
  {
  auto pPair = find( Id );
  if( pPair == end() )
    insert( Id, Incomplete( Party, Detail ) );
  else
    {
    pPair->m_Party += Party;
    pPair->m_Detail += Detail;
    }
  }

ProductionState::ProductionState() : m_ShiftStarted( true ), m_pWait( new WaitWindow )
  {
  sm_pProductionState = this;
  QSqlQuery Query;
  Query.exec("SELECT Номер_детали, Название_детали FROM Деталь");
  if (Query.lastError().isValid())
    throw "Ошибка при выполнении запроса: " + Query.lastError().text();
  while (Query.next())
    {
    int Id = Query.value(0).toInt();
    QString Name = Query.value(1).toString();
    push_back(Route(Id, Name));
    }
  }

void ProductionState::Evaluate()
  {
  s_LogFile.resize( 0 );
  Time::sm_CurrentTime = Time::sm_StartWork;
  for( auto pRoute = begin(); pRoute != end(); pRoute++ )
    pRoute->StartWork();
  vector<BaseActingBlock*> ActingBlocks( m_ActingBlocks );
  for( auto pActing = ActingBlocks.begin(); pActing != ActingBlocks.end(); pActing++ )
    ( *pActing )->StartWork();
  m_pWait->exec();
  m_pWait->show();
  Simulator::Evaluate();
  m_pWait->hide();
  m_EndWorkTime = Time::GetCurrentTime();
  m_IncompleteProductionOld = m_IncompleteProduction;
  m_IncompleteProduction.clear();
  for( auto pActing = m_ActingBlocks.rbegin(); pActing != m_ActingBlocks.rend();  pActing++)
    ( *pActing )->Report();
  for( auto pActing = m_ActingBlocks.begin(); pActing != m_ActingBlocks.end();  ) 
    if( dynamic_cast< Bench* >( *pActing ) == nullptr )
      pActing++;
    else
      pActing = m_ActingBlocks.erase( pActing );
  m_Ware.clear();
  QSqlQuery Query;
  QString RusQuery = QString("SELECT Номер_изделия, Наименование_изделия, Цена_изделия, Количество,_шт. from Варианты") +
     "Inner Join Изделия ON Варианты. Номер изделия = " +
    "Изделия.Номер_изделия where [Номер варианта] = " + QString::number( Route::sm_Variant );
  Query.exec( RusQuery );
  if( Query.lastError().isValid() )
    throw "Ошибка при выполнении запроса: " + Query.lastError().text();
  while( Query.next() )
    {
    Product P( Query.value( 0 ).toInt(), Query.value( 1 ).toString(), Query.value( 2 ).toDouble(), Query.value( 3 ).toInt() );
    for( auto pRoute = begin(); pRoute != end(); pRoute++ )
      if( pRoute->m_ProductId == P.m_Id )
        P.m_Finished = min( P.m_Finished, pRoute->ProductFinished() );
    P.Finish();
    qDebug() << P.m_Name << P.m_Plan << P.m_Finished;
    for( auto pRoute = begin(); pRoute != end(); pRoute++ )
      if( pRoute->m_ProductId == P.m_Id )
        {
        qDebug() << "Detail Id" << pRoute->m_DetailType << "Plan" << pRoute->m_PlanCount << "Made" << pRoute->m_ReadyCount;
        m_IncompleteProduction.Add( pRoute->m_DetailType, 0, pRoute->CalcIncomplete( P.m_Finished ) );
        }
    m_Ware.append( P );
    }
  for( auto pPair = m_IncompleteProduction.begin(); pPair != m_IncompleteProduction.end(); pPair++ )
    qDebug() << "Incomplete detail, Id" << pPair.key() << "Party number" << pPair.value().m_Party << "Detail number" << pPair.value().m_Detail;
  }

void ProductionState::EndShift()
  {
  if( !m_ShiftStarted ) return;
  m_ShiftStarted = false;
  Time T( Time::GetCurrentTime() );
  for( auto pRoute = begin(); pRoute != end(); pRoute++ )
    if( pRoute->m_CorrectedTimeStartParty > T.EndCurrentShift() && pRoute->m_CorrectedTimeStartParty < T.NextShift() )
      pRoute->m_CorrectedTimeStartParty = T.NextShift();
  }

void ProductionState::Finish( DetailParty* pParty ) 
  {
  pParty->Finish();
  delete pParty;
  }

Route::Route(int Id, const QString& DetailName) : m_DetailType(Id), m_DetailName(DetailName), m_ReadyCount(0),
  m_SumDetailTime( 0 ), m_SumReadjustmentTime( 0 ), m_SumGalvanic( 0 ), m_SumLieTime( 0 )
  {
  QSqlQuery Query; 
  Query.exec("SELECT  Операции.Номер_группы, Штучное_время, Подготовительно_заключительное_время, \
    Время_предварительного_пролеживания, Время_обработки_в_другом_цехе FROM Операции Inner Join Оборудование_цеха on \
    Операции.Номер_группы=Оборудование_цеха.Номер_группы where Номер_детали = " +
    QString::number(Id) + " order by Номер_операции");
  if (Query.lastError().isValid())
    throw "Ошибка при выполнении запроса:" + Query.lastError().text();
  bool FirstStep = true;
  while (Query.next())  //3
    {
    double DetailTime = Query.value(1).toDouble();
    m_SumDetailTime += DetailTime;
    double ReadjustmentTime = Query.value( 2 ).toDouble();
    m_SumReadjustmentTime += ReadjustmentTime;
    double LieTime = FirstStep ? 0.0 : Query.value( 3 ).toDouble();
    m_SumLieTime += LieTime;
    double Galvanic = Query.value( 4 ).toDouble();
    m_SumGalvanic += Galvanic;
    push_back( Step( Query.value( 0 ).toInt(), DetailTime, ReadjustmentTime, LieTime, Galvanic ) );
    FirstStep = false;
    }
  m_PrelVolumeParty = m_SumReadjustmentTime / ( m_SumDetailTime * 0.02 );
  }

void Route::StartWork()
  {
  int iPrelPartVol = ceil( m_PrelVolumeParty );
  m_VolumeParty = QInputDialog::getInt( nullptr,  "Ввод объема партии" ,  "Деталь: "  + m_DetailName +
    ", предв. объем партии: "  + QString::number( m_PrelVolumeParty ), iPrelPartVol, iPrelPartVol );
  m_TotalTime = ( m_SumDetailTime * m_VolumeParty + m_SumReadjustmentTime + m_SumLieTime + m_SumGalvanic ) / Time::DobeLength();
  QString RusQuery = "SELECT [Количество, шт.], [Применяемость], Варианты.[Номер изделия] from Варианты \
    Inner Join Состав_изделия ON Варианты.[Номер изделия] =  \
    Состав_изделия.Номер_изделия where [Номер варианта] = "  +
    QString::number( sm_Variant ) + " and Номер_Детали = " + QString::number( m_DetailType );
  QSqlQuery Query;
  Query.exec( RusQuery );
  if( Query.lastError().isValid() )
    throw  "Ошибка при выполнении запроса:" + Query.lastError().text();
  Query.next();
  m_Applicability = Query.value( 1 ).toInt();
  m_PlanCount = Query.value( 0 ).toInt() * m_Applicability;
  m_ProductId = Query.value( 2 ).toInt();
  double DobeNeed = m_PlanCount / ProductionState::sm_WorkDayCount;
  int IncompleteNorm = DobeNeed * m_TotalTime;
  int IncompleteProduction = 0;
  m_ReadyCount = 0;
  QMap<int, Incomplete> &IP = ProductionState::sm_pProductionState->m_IncompleteProduction;
  if( !IP.isEmpty() )
    {
    m_ReadyCount = IP[m_DetailType].m_Detail;
    IncompleteProduction = IP[m_DetailType].m_Party * m_OldVolumeParty + m_ReadyCount;
    }
  m_OldVolumeParty = m_VolumeParty;
  m_PartyCount = max( 0.0, ceil( ( double ) ( m_PlanCount + IncompleteNorm - IncompleteProduction ) / m_VolumeParty ) );
  m_StartInterval = m_PartyCount == 0 ? 0 : floor( ( ProductionState::sm_WorkDayCount * 24.0 ) / ( m_PartyCount + 1 ) );
  m_PartyStarted = 0;
  if( IncompleteProduction == 0 )
    m_TimeStartParty = Time::sm_StartWork + Time::sm_ReShift;
  else
    if( m_StartInterval == 0 )
      m_TimeStartParty = Time::sc_BadTime;
    else
      m_TimeStartParty = Time::GetCurrentTime() + m_StartInterval;
  m_CorrectedTimeStartParty = m_TimeStartParty;
  qDebug() << "Route " << m_DetailName << "Volume Party" << m_VolumeParty << "Party count" << m_PartyCount << "Start Interval" << m_StartInterval;
  }

double Route::BusyTime( int GroupId )
  {
  double Time = 0.0;
  for( auto pStep = begin(); pStep != end(); pStep++ )
    if( pStep->m_GroupId == GroupId )
      Time += pStep->m_DetailTime * m_PlanCount + pStep->m_PartyTime;
  return Time;
  }

Actor* Route::GetParty()
  {
  Actor *pParty = new Actor(m_TimeStartParty, m_VolumeParty, this);
  if( ++m_PartyStarted == m_PartyCount )
    m_CorrectedTimeStartParty = Time::sc_BadTime;
  else
    m_CorrectedTimeStartParty = m_TimeStartParty += m_StartInterval;
  return pParty;
  }

double Route::CalcIncomplete( int ProdCount )
  {
  return m_ReadyCount - m_Applicability * ProdCount;
  }

int Worker::AppearCount()
  {
  int iCount = m_ListCount / Time::ShiftCount();
  int iResult = 0;
  for( int i = 0; i < iCount; i++ )
    if( sm_TestAppear.GetStandard() > m_PropUnappear ) iResult++;
  return iResult;
  }

GroupEquipment::GroupEquipment( int GroupId, ActingBlock *pPrevQueue, Delay* pLieTime) :
  m_GroupId(GroupId)
  {
  m_pNextBlock = pLieTime;
  m_pRNumber = nullptr;
  pPrevQueue->SetNextBlock(this); //подключаемся к очереди
  QSqlQuery Query;
  Query.exec("  Наименования_оборудования, \
    Время_обработки_в_другом_цехе, Длительность_профилактики, \
    Интервал_между_профилактиками, Средний_коэффициент FROM Оборудование_цеха where Номер_группы = " +
    QString::number(GroupId));
  if( Query.lastError().isValid() )
    throw "Ошибка при выполнении запроса:" + Query.lastError().text();
  Query.next();
  m_Name = Query.value(0).toString();
  m_AnotherTime = Query.value(1).toDouble();
  m_MeanRepairTime = Query.value( 2 ).toDouble();
  if( m_MeanRepairTime == 0 )
    m_RepairTimeNeed = RNormal( 0, 0 );
  else
    m_RepairTimeNeed = RNormal( m_MeanRepairTime, m_MeanRepairTime * 0.3 ); //логарифм времени профилактики будет распределен по нормальному закону. Время профилактики будет изменяться примерно в два раза
  m_RepairInterval = Query.value(3).toDouble();
  m_MeanCoeff = Query.value( 4 ).toDouble();
  }

void GroupEquipment::StartWork()
  {
  m_WorkTime = Time::sm_StartWork;
  m_TimeStart = Time::sm_StartWork;
  ProductionState &PS = *ProductionState::sm_pProductionState;
  double TotalTime = 0.0;
  for( auto pRoute = PS.begin(); pRoute != PS.end(); pRoute++ )
    TotalTime += pRoute->BusyTime( m_GroupId );
  TotalTime /= m_MeanCoeff;
  double RegFund = Time::DobeLength() * ProductionState::sm_WorkDayCount;
  double RepairTime = m_MeanRepairTime == 0.0 ? 0.0 : m_MeanRepairTime * RegFund / m_RepairInterval;
  RegFund -= RepairTime;
  double dBenchCount = TotalTime / RegFund;
  double WorkFund = Time::sm_ShiftLength * ProductionState::sm_WorkDayCount;
  double K = WorkFund / ( Time::sm_ShiftLength * ( ProductionState::sm_WorkDayCount - 26.0 / 12 ) );
  int BenchCount = ceil( K * dBenchCount );
  m_TotalWorkFund = WorkFund * BenchCount * Time::ShiftCount();
  m_BenchArray.clear();
  m_BenchArray.resize( BenchCount );
  double FirstRepairInterval = m_RepairInterval / BenchCount;
  double RepairStart = FirstRepairInterval;
  for( int iBench = 0; iBench < BenchCount; iBench++, RepairStart += FirstRepairInterval )
    {
    m_BenchArray[iBench].m_NextRepairTime = RepairStart;
    m_BenchArray[iBench].m_pGroup = this;
    m_BenchArray[iBench].m_Id = iBench;
    }
  m_EventQueue.clear();
  m_EventQueue.insert( Time::sm_StartWork, evSchiftStart );
  QSqlQuery Query;
  Query.exec( " Название FROM Профессии where Группа_оборудования = " + QString::number( m_GroupId ) );
  if( Query.lastError().isValid() )
    throw "Ошибка при выполнении запроса:" + Query.lastError().text();
  Query.next();
  double ProbUnappear = 1.0 - 1.0 / K;
  int ListCount = BenchCount * Time::sm_ShiftCount;
  qDebug() << "GroupEquipment, Constructor " << m_Name << " ProbUnappear " << ProbUnappear << " ListCount " << ListCount <<
    "BenchCount" << BenchCount << "Plan Repair Time" << RepairTime << "Mean repair time" << m_MeanRepairTime <<
    "Repair interval" << m_RepairInterval;
  m_Worker = Worker( Query.value( 0 ).toString(), ProbUnappear, ListCount );
  m_TotalRepairTime = m_TimeNoWorker = m_TotalWorkTime = m_TotalFreeTime = m_TotalReadjustmentTime = 0.0;
  NamedBlock::StartWork();
  }

void GroupEquipment::StartFree()
  {
  for( auto pBench = m_BenchArray.begin(); pBench != m_BenchArray.end(); pBench++ )
    pBench->StartFreeTime();
  }

void GroupEquipment::Evaluate()
  {
  auto First = m_EventQueue.begin();  //получаем итератор ближайшего события.
  EventType Type = First.value();  //получаем тип события
  m_EventQueue.erase(First); //удаляем событие из очереди.
  
  switch( Type )
    {
    case evSchiftStart:
      {
      StartFree();
      if( m_pCurrentActor == nullptr )
        m_pCurrentActor = m_pPrevQueue->GetActor();
      int WorkersCount = m_Worker.AppearCount();
      int WorkersInShift = m_Worker.ListCount() / Time::ShiftCount();
      qDebug() << "GroupEquipment, Evaluate, evSchiftStart " << m_Name << " WorkersCount " << WorkersCount << " From " << WorkersInShift;
      ProductionState::sm_pProductionState->StartShift();
      m_EventQueue.insert( Time::GetCurrentTime().EndCurrentShift(), evSchiftEnd ); //обеспечиваем событие "Конец смены"
      if( WorkersCount == 0 )
        {
        for( auto pBench = m_BenchArray.begin(); pBench != m_BenchArray.end(); pBench++ )
          pBench->SetWorker( false );
        return;
        }
      if( WorkersCount == m_BenchArray.count() )
        for( int iBench = 0; iBench < m_BenchArray.count(); iBench++ )
          m_BenchArray[iBench].SetWorker( true );
      else
        {
        int iBenchComplected = 0; //число станков, которые получили рабочего
        int iBench = 0;
        for( ; iBench < m_BenchArray.count() && iBenchComplected < WorkersCount; iBench++ )
          if( m_BenchArray[iBench].m_RepairTime == 0 )
            {
            m_BenchArray[iBench].SetWorker( true );
            iBenchComplected++;
            }
        if( iBench < m_BenchArray.count() ) //рабочих не хватило даже станкам, не стоящим в профилактике
          {
          int iBnch = 0;
          for( ; iBnch < iBench; iBnch++ )   //устанавливаем признак отсутствия для ранее проверенных станков, стоящих на профилактике
            if( m_BenchArray[iBnch].m_RepairTime != 0 )
              m_BenchArray[iBnch].SetWorker( false );
          for( ; iBnch < m_BenchArray.count(); iBnch++ ) //устанавливаем признак отсутствия для остальных станков
            m_BenchArray[iBnch].SetWorker( false );
          }
        else   //рабочие назначены для всех станков, не стоящих на профилактике. Обеспечим рабочимии часть станков, требующих профилактики
          for( iBench = 0; iBench < m_BenchArray.count() && iBenchComplected < WorkersCount; iBench++ )
            if( m_BenchArray[iBench].m_RepairTime != 0 )
              {
              m_BenchArray[iBench].SetWorker( true );
              iBenchComplected++;
              }
        }
      }
      return;
    case evSchiftEnd:
      {
      ProductionState::sm_pProductionState->EndShift();
      for( int iBench = 0; iBench < m_BenchArray.count(); iBench++ )
        m_BenchArray[iBench].EndFreeTime();
      ShiftsReport();
      Time StartNextShift = Time::GetCurrentTime().NextShift();
      qDebug() << "GroupEquipment, Evaluate, evSchiftEnd " << m_Name << "Start next Shift" << StartNextShift.Hour();
      m_EventQueue.insert( StartNextShift, evSchiftStart );
      for( int iBench = 0; iBench < m_BenchArray.count(); iBench++ )
        m_BenchArray[iBench].CorrectRepairTime();
      return;
      }
    }
  }

void GroupEquipment::NewParty()
  {
  double dReadjustmentTime = ReadjustmentTime();
  if( dReadjustmentTime != 0 )
    {
    qDebug() << "ReadjustmentTime" << dReadjustmentTime << m_Name;
    for( auto pBench = m_BenchArray.begin(); pBench != m_BenchArray.end(); pBench++ )
      pBench->SetReadjustment( dReadjustmentTime );
    return;
    }
  for( int iBnch = 0; iBnch < m_BenchArray.count(); iBnch++ )
    m_BenchArray[iBnch].Evaluate();
  }

ServiceResult GroupEquipment::Evaluate( DetailParty* pParty )
  {
  if( pParty->GetCurrentGroupEquipment() != m_GroupId ) return BadActor;
  if( m_pCurrentActor != nullptr ) return IsBusy;
  m_pCurrentActor = pParty;
  if( IsTimeOff() )
    {
    m_pCurrentActor = nullptr;
    return IsBusy;
    }
  NewParty();
  return Success;
  }

Bench::StateType GroupEquipment::GetDetail(Bench &Bench)
  {
  if( m_pCurrentActor == nullptr )
    {
    qDebug() << "No Current actor" << m_Name;
    StartFree();
    return Bench::bnchFree;
    }
  if( NextEventTime() == Time::sc_BadTime )
    {
    qDebug() << "Start free Time" << Time::GetCurrentTime().Hour();
    StartFree();
    return Bench::bnchFree;
    }
  if( m_pCurrentActor->GetRawDetail() )   // получена необработанная деталь.
    {
    Bench.SetDetail();
    return Bench.m_State;
    }
  for( int iBnch = 0; iBnch < m_BenchArray.count(); iBnch++ )
    if( m_BenchArray[iBnch].m_State == Bench::bnchWork )
      {
      Bench.StartFreeTime();
      return Bench::bnchFree;
      }
  qDebug() << "Party is developed ";
  if( !m_pCurrentActor->NextStep() )
    qDebug() << "All Step developed" << m_pCurrentActor->DetailType();
  m_pNextBlock->Evaluate( m_pCurrentActor );
  m_pCurrentActor = m_pPrevQueue->GetActor();  //берем очередную партию деталей;
  if( m_pCurrentActor == nullptr )
    {
    StartFree();
    return Bench::bnchFree;
    }
  NewParty();
  return Bench.m_State;
  }

double GroupEquipment::RepairTime() 
  {
  return m_RepairTimeNeed.GetNumber();
  }

double GroupEquipment::WorkTime()
  {
  if( m_pCurrentActor == nullptr ) 
    return Time::sc_BadTime;
  return m_pCurrentActor->DetailWorkTime(); 
  }

double GroupEquipment::ReadjustmentTime()
  {
  return m_pCurrentActor->m_PartyWorkTime.GetNumber();
  }

Time GroupEquipment::NextEventTime()
  {
  if (m_EventQueue.isEmpty()) return sc_NoTime;
//  qDebug() << m_Name << m_EventQueue.count() << m_EventQueue.begin().key().Hour();
  return m_EventQueue.begin().key();  //Возврашаем время ближайшего события
  }

bool GroupEquipment::IsTimeOff()
  {
  if( m_pCurrentActor == nullptr || m_EventQueue.isEmpty() || m_EventQueue.begin().value() != evSchiftStart ) return false;
  qDebug() << "Time is off" << m_Name << m_EventQueue.count() << m_EventQueue.begin().key().Hour();
  return true;
  }

void GroupEquipment::ShiftsReport()
  {
  for( auto pBench = m_BenchArray.begin(); pBench != m_BenchArray.end(); pBench++ )
    pBench->Report();
  qDebug() << m_Name << "Repair Time" << m_TotalRepairTime << "No Worker " << m_TimeNoWorker
    << "Work Time" << m_TotalWorkTime << "No work" << m_TotalFreeTime << "ReadjustmentTime" << m_TotalReadjustmentTime
    << "Total Time" << ( m_TotalRepairTime + m_TimeNoWorker + m_TotalReadjustmentTime + m_TotalWorkTime + m_TotalFreeTime );
  }

void GroupEquipment::Report()
  {
  IncompleteMap &IM = ProductionState::sm_pProductionState->m_IncompleteProduction;
  for( int i = 0; i < m_pPrevQueue->GetLen(); i++ )
    IM.Add( ( *m_pPrevQueue )[i].GetId(), 1, 0 );
  if( m_pCurrentActor != nullptr )
    IM.Add( m_pCurrentActor->GetId(), 1, 0 );
  qDebug() << m_Name << "Work Fund" << m_TotalWorkFund << "Repair Time" << m_TotalRepairTime << "No Worker " << m_TimeNoWorker
    << "Work Time" << m_TotalWorkTime << "No work" << m_TotalFreeTime << "ReadjustmentTime" << m_TotalReadjustmentTime
    << "Total Time" << ( m_TotalRepairTime + m_TimeNoWorker + m_TotalReadjustmentTime + m_TotalWorkTime + m_TotalFreeTime);
  }

Bench::Bench() : m_State( bnchFree ), m_NextRepairTime( sc_NoTime ),
  m_TotalRepairTime(0), m_TotalWorkTime(0), m_RepairTime(0), m_WorkTime(0), m_TimeNoWorker(0),
  m_StartFree(Time::sm_StartWork), m_TotalFreeTime(0.0), m_TotalReadjustmentTime(0)
  {
  m_NextEventTime = sc_NoTime;
  }

QString Bench::GetName() 
  { 
  return "Bench " + m_pGroup->GetName(); 
  }

void Bench::SetWorker(bool IsWorker) 
  {
  EndFreeTime();
  if( !IsWorker )
    {
    m_TimeNoWorker += Time::sm_ShiftLength;
    if( m_State == bnchRepair )
      m_StateQueue.insert( bnchRepair, m_RepairTime );
    else
      if( m_State == bnchReadjust )
        m_StateQueue.insert( bnchReadjust, m_ReadjustmentTime );
    m_State = bnchNoWorker;
    m_NextEventTime = sc_NoTime;
    return;
    }
  if( !m_StateQueue.isEmpty() )
    {
    auto First = m_StateQueue.begin();
    m_State = First.key();
    m_NextEventTime = Time::GetCurrentTime() + First.value();
    if( m_State == bnchRepair )
      m_RepairTime = First.value();
    else
      m_ReadjustmentTime = First.value();
    m_StateQueue.erase( First ); 
    return;
    }
  if( IsRepair() ) return;
  m_State = bnchFree;
  if( ( m_State = m_pGroup->GetDetail( *this ) ) == bnchFree ) m_NextEventTime = sc_NoTime;
  }

bool Bench::IsRepair()
  {
  if( m_State == bnchRepair ) return true;
  if( m_pGroup->m_RepairInterval == 0.0 || m_NextRepairTime > m_pGroup->WorkTime() ) return false;
  EndFreeTime();
  m_NextRepairTime = m_pGroup->m_RepairInterval;
  m_RepairTime = m_pGroup->RepairTime(); //группа оборудования должна вернуть случайное число с мат ожиданием, равным времени профилактики и случайным отколнением
  m_State = bnchRepair;
  m_NextEventTime = Time::GetCurrentTime() + m_RepairTime;
  qDebug() << GetName() << m_Id << "Repair to" << m_NextEventTime.Hour() << "Repair Time" << m_RepairTime <<
    "Mean" << m_pGroup->m_RepairTimeNeed.Mean() << "Deviation" << m_pGroup->m_RepairTimeNeed.Deviation();
  return true;
  }

void Bench::CorrectRepairTime()
  {
  EndFreeTime();
  if( m_State != bnchReadjust && m_State != bnchRepair ) return;
  double RestTime = m_NextEventTime - Time::GetCurrentTime();
  m_NextEventTime = Time::GetCurrentTime().NextShift() + RestTime;
  if( m_State == bnchRepair )
    {
    m_TotalRepairTime += m_RepairTime - RestTime;
    m_RepairTime = RestTime;
    qDebug() << GetName() << m_Id << "Total Repair Time" << m_TotalRepairTime << "New Repair Time " << m_RepairTime << "Next Reapair Event Time" << m_NextEventTime.Hour();
    return;
    }
  m_TotalReadjustmentTime += m_ReadjustmentTime - RestTime;
  m_ReadjustmentTime = RestTime;
  qDebug() << GetName() << m_Id << "Total ReadjustmentTime Time" << m_TotalReadjustmentTime << "New Redjustment Time " << m_ReadjustmentTime << "Next Redjustment Event Time" << m_NextEventTime.Hour();
  }

bool Bench::SetDetail()
  {
  EndFreeTime();
  if( m_State != bnchFree )  return false;
  m_WorkTime = m_pGroup->WorkTime();
  Time NextTime = Time::GetCurrentTime() + m_WorkTime;
  qDebug() << m_pGroup->m_Name << "Work Time By Set Detail" << m_WorkTime << "Next Time" << NextTime.Hour();
  if( NextTime > Time::GetCurrentTime().EndCurrentShift() )
    {
    qDebug() << "End Shift" << m_pGroup->m_Name << m_Id << "Rest Time" << NextTime - Time::GetCurrentTime().EndCurrentShift()
      << "Time End Shift" << Time::GetCurrentTime().EndCurrentShift().Hour() << "Lost WorkTime" << m_WorkTime;
    m_WorkTime = 0.0;
    m_pGroup->BackDetail();
    StartFreeTime();
    return false; //Времени до конца смены не хватит для обработки детали.
    }
  m_State = bnchWork;
  m_NextEventTime = NextTime;
  return true;
  }

void Bench::Evaluate() 
  {
  if( m_State == bnchNoWorker )
    {
    qDebug() << "Bench, Evaluate, NoWorker" << m_pGroup->GetName() << m_Id << m_TimeNoWorker;
    return;
    }
  if( m_State == bnchRepair )
    {
    if( m_NextEventTime != Time::GetCurrentTime() ) return;
    m_TotalRepairTime += m_RepairTime;
    qDebug() << "Bench, Evaluate, bnchRepair" << m_pGroup->GetName() << m_Id << m_RepairTime << "Total RT" << m_TotalRepairTime;
    m_RepairTime = 0;
    }
  if( m_State == bnchReadjust )
    {
    if( m_NextEventTime != Time::GetCurrentTime() ) return;
    m_TotalReadjustmentTime += m_ReadjustmentTime;
    qDebug() << "Bench, Evaluate, Readjustment" << m_pGroup->GetName() << m_Id << m_ReadjustmentTime << "Total RT" << m_TotalReadjustmentTime;
    m_ReadjustmentTime = 0;
    }
  if( m_State == bnchWork )
    {
    if( m_NextRepairTime != 0 ) m_NextRepairTime -= m_WorkTime;
    m_TotalWorkTime += m_WorkTime;
    qDebug() << "Bench, End Work Time" << m_pGroup->GetName() << m_Id << Time::GetCurrentTime().Hour() << "WorkTime" << m_WorkTime <<
      "Total Work Time" << m_TotalWorkTime << "Detail" << m_pGroup->m_pCurrentActor->DetailType();
    m_WorkTime = 0.0;
    if( IsRepair() ) return;
    StartFreeTime();
    }
  m_State = bnchFree;
  if( ( m_State = m_pGroup->GetDetail( *this ) ) == bnchFree )
    m_NextEventTime = sc_NoTime;
  }

void Bench::SetReadjustment(double RT)
  {
  if( m_State != bnchFree )
    {
    m_StateQueue.insert( bnchReadjust, RT );
    return;
    }
  m_ReadjustmentTime = RT;
  EndFreeTime();
  m_State = bnchReadjust;
  m_NextEventTime = Time::GetCurrentTime() + RT;
  }

void Bench::Report()
  {
  m_pGroup->m_TotalRepairTime += m_TotalRepairTime;
  m_pGroup->m_TimeNoWorker += m_TimeNoWorker;
  m_pGroup->m_TotalWorkTime += m_TotalWorkTime;
  m_pGroup->m_TotalFreeTime += m_TotalFreeTime;
  m_pGroup->m_TotalReadjustmentTime += m_TotalReadjustmentTime;
  m_TotalRepairTime = m_TimeNoWorker = m_TotalWorkTime = m_TotalFreeTime = m_TotalReadjustmentTime = 0;
  }

void Bench::EndFreeTime()
  {
  if( m_StartFree == Time::sc_BadTime ) return;
  double AddFree = Time::GetCurrentTime() - m_StartFree;
  if( AddFree > 3 )
  qDebug() << m_pGroup->GetName() << "Add Free " << AddFree;
  m_TotalFreeTime += AddFree;
  m_StartFree = Time::sc_BadTime;
  }

void Bench::StartFreeTime()
  {
  if( m_State != bnchFree ) return;
  EndFreeTime();
  m_StartFree = Time::GetCurrentTime();
  }

Delay::Delay() : NamedBlock( "Межоперационное пролеживание" ) {}

void Delay::Evaluate()
  {
  Actor *pParty = m_PartyArray.begin().value();
  int iType = pParty->DetailType();
  qDebug() << "End Delay detail type: " << pParty->DetailType();
  m_PartyArray.remove( m_PartyArray.begin().key() );
  m_pNextBlock->Evaluate( pParty );
  if( m_PartyArray.isEmpty() )
    m_NextEventTime = sc_NoTime;
  else
    m_NextEventTime = m_PartyArray.begin().key();
  }

ServiceResult Delay::Evaluate( DetailParty* pParty )
  {
  double LieTime = pParty->GetLieTime();
  if( LieTime == 0 )
    return m_pNextBlock->Evaluate( pParty );
  Time DT = Time::GetCurrentTime() + LieTime;
  m_PartyArray.insert( DT, pParty );
  m_NextEventTime = m_PartyArray.begin().key();
  qDebug() << "Delay, detail type:" << pParty->DetailType() << " Evaluate to " << DT.Hour();
  return Success;
  }

void Delay::StartWork()
  {
  m_NextEventTime = Time::sc_BadTime;
  if( m_PartyArray.isEmpty() ) return;
  QMap<Time, DetailParty *> NewPartyArray;
  Time EndWorkTime( ProductionState::sm_pProductionState->GetEndWorkTime() );
  Time CurrentTime( Time::GetCurrentTime() );
  for( auto pPair = m_PartyArray.begin(); pPair != m_PartyArray.end(); pPair++ )
    NewPartyArray.insert( CurrentTime + ( pPair.key() - EndWorkTime ), pPair.value() );
  m_PartyArray = NewPartyArray;
  m_NextEventTime = m_PartyArray.begin().key();
  NamedBlock::StartWork();
  }

void Delay::Report()
  {
  for( auto pParty = m_PartyArray.begin(); pParty != m_PartyArray.end(); pParty++ )
    ProductionState::sm_pProductionState->m_IncompleteProduction.Add( pParty.value()->GetId(), 1, 0 );
  }

Galvanic::Galvanic( ActingBlock *pPrevBlock ) : m_GroupId( 12 ) 
  { 
  m_Name = "Цех гальванопокрытий";
  pPrevBlock->SetNextBlock( this ); 
  }

ServiceResult Galvanic::Evaluate(DetailParty* pParty) 
  { 
  if (pParty->GetCurrentGroupEquipment() != m_GroupId) return BadActor;
  m_PartyArray.insert(Time::GetCurrentTime() + pParty->GetGalvanicTime(), pParty);
  m_NextEventTime = m_PartyArray.begin().key();
  qDebug() << "Galvanic, Evaluate " << m_NextEventTime.Hour();
  return Success;
  }

void Galvanic::Evaluate()
  {
  m_PartyArray.begin().value()->NextStep();
  Delay::Evaluate();
  }

WaitWindow::WaitWindow()
  {
  QVBoxLayout *pVLayout = new QVBoxLayout;
  setWindowTitle( " Ждите..." );
  pVLayout->addWidget( new QLabel( "Идет процесс, ждите..." ) );
  setLayout( pVLayout );
  adjustSize();
  }

int WaitWindow::exec()
  {
  static QTimer Timer;
  connect( &Timer, SIGNAL( timeout() ), SLOT( reject() ) );
  Timer.setSingleShot( true );
  Timer.start( 100 );
  return QDialog::exec();
  }
