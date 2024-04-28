/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) 2001-2014
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#endif

#include "quickfix/config.h"
#include "Application.h"
#include "quickfix/Session.h"
#include <iostream>
#include "quickfix/config.h"
#include "quickfix/SessionSettings.h"
#include <thread>
#include <chrono>
#include "quickfix/fix44/ResendRequest.h"
#include "quickfix/fix44/MarketDataRequest.h"
#include "quickfix/fix44/SecurityListRequest.h"
#include <fstream>
#include "time.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>

std::string hmac_sha384(const std::string& key, const std::string& data) {
    unsigned char* result;
    unsigned int len = SHA384_DIGEST_LENGTH;

    result = (unsigned char*)malloc(sizeof(char) * len);

    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, &key[0], key.length(), EVP_sha384(), NULL);
    HMAC_Update(ctx, (unsigned char*)&data[0], data.length());
    HMAC_Final(ctx, result, &len);
    HMAC_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }

    free(result);

    return ss.str();
}

FIX::SessionSettings settings_ {};

FIX::SenderCompID senderCompId{};
FIX::TargetCompID targetCompId{};

inline uint64_t getTimestamp()
{
  struct timeval curTime;

  gettimeofday(&curTime, NULL);
  return ( (uint64_t) curTime.tv_sec ) * 1000000 + curTime.tv_usec;
}

void Application::onLogon( const FIX::SessionID& sessionID )
{
  std::cout << std::endl << "Logon - " << sessionID << std::endl;
}

void Application::onLogout( const FIX::SessionID& sessionID )
{
  std::cout << std::endl << "Logout - " << sessionID << std::endl;
}

void Application::toAdmin(FIX::Message& msg, const FIX::SessionID& sid) {
  if (msg.getHeader().getField(FIX::FIELD::MsgType) == FIX::MsgType_Logon) {
        auto& settings = sessionSettings.get(sid);
        msg.setField(FIX::Username{settings.getString("Username")});
       
        if(settings.getString("RawData") != "0") {
         

        struct timeval time;
        gettimeofday(&time, NULL);
        long long microseconds = (long long)time.tv_sec * 1000000 + time.tv_usec;
        std::string data = std::to_string(microseconds*1000);
         msg.setField(FIX::RawData{data});
        // Convert microseconds to string
        std::string pass = hmac_sha384(settings.getString("Password"), data);
        std::cout << data << " " << pass;

        msg.setField(FIX::Password{pass});  
        }
        else {
        msg.setField(FIX::Password{settings.getString("Password")});  

        }

        
        msg.setField(FIX::ResetSeqNumFlag{settings.getString("ResetSeqNumFlag") == "Y" ? true : false});  
        if(settings.getString("B") != "0") msg.setField(9010, settings.getString("B"));
        if(settings.getString("AE") != "0") msg.setField(9011, settings.getString("AE"));
        if(settings.getString("PSWH") != "0") msg.setField(9501, settings.getString("PSWH"));
        //msg.setField(6065, "Y");
        //if(settings.getString("TSST") != "0") msg.setField(341, settings.getString("TSST"));
        
        senderCompId = sid.getSenderCompID();
        targetCompId = sid.getTargetCompID();
  }
}
void Application::fromApp( const FIX::Message& message, const FIX::SessionID& sessionID )
EXCEPT( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
  crack( message, sessionID );
  std::cout << std::endl << "IN: " << message << std::endl;
}

void Application::toApp( FIX::Message& message, const FIX::SessionID& sessionID )
EXCEPT( FIX::DoNotSend )
{
  try
  {
    FIX::PossDupFlag possDupFlag;
    message.getHeader().getField( possDupFlag );
    if ( possDupFlag ) throw FIX::DoNotSend();
  }
  catch ( FIX::FieldNotFound& ) {}

  std::cout << std::endl
  << "OUT: " << message << std::endl;
}

void Application::onMessage
( const FIX40::ExecutionReport&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX40::OrderCancelReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX41::ExecutionReport&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX41::OrderCancelReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX42::ExecutionReport&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX42::OrderCancelReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX43::ExecutionReport&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX43::OrderCancelReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::ExecutionReport&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::OrderCancelReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::SecurityList&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX50::ExecutionReport&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX50::OrderCancelReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::MarketDataRequestReject&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::MarketDataSnapshotFullRefresh&, const FIX::SessionID& ) {}
void Application::onMessage
( const FIX44::MarketDataIncrementalRefresh&, const FIX::SessionID& ) {}

void Application::onMessage
( const FIX44::TradeCaptureReport&, const FIX::SessionID& ) {}

void SetHeader(FIX::Message& msg) {
  msg.getHeader().setField(FIX::SenderCompID{senderCompId});
  msg.getHeader().setField(FIX::TargetCompID{targetCompId});
}

int ordId = 0;
void SendLimitOrder(
    FIX::Symbol&& symbol, FIX::Side&& side, FIX::OrderQty&& qty, FIX::Price&& price) {
  FIX44::NewOrderSingle order{
    FIX::ClOrdID{std::to_string(ordId++)},
    std::move(side),
    FIX::TransactTime{},
    FIX::OrdType_LIMIT
  };
  order.setField(std::move(symbol));
  order.setField(std::move(qty));
  order.setField(std::move(price));
  //order.setField(FIX::Account{"1"});
  order.setField(FIX::TimeInForce{FIX::TimeInForce_DAY});

  SetHeader(order);
  FIX::Session::sendToTarget(order);
}


void Application::run()
{
  while ( true )
  {
    try
    {
      char action = queryAction();

      if ( action == '1' )
        queryEnterOrder();
      else if ( action == '2' )
        queryCancelOrder();
      else if ( action == '3' )
        queryReplaceOrder();
      else if ( action == '4' )
        querySecurityListRequest();
      else if ( action == '5' )
        queryMarketDataRequest();
      else if ( action == '6' )
        queryResend(); 
      else if ( action == '7') {
        for (int i =0; i < 1000; i++) {
          SendLimitOrder(FIX::Symbol{"TESTBTC:TESTUSD"}, FIX::Side{'1'}, FIX::OrderQty{0.0001}, FIX::Price{1.0});  
        }
        for (int i =0; i < 1000; i++) {
          SendLimitOrder(FIX::Symbol{"TESTBTC:TESTUSD"}, FIX::Side{'2'}, FIX::OrderQty{0.0001}, FIX::Price{10000000.0});  
        }
      }
      else if ( action == '8' ) 
        break;
    }
    catch ( std::exception & e )
    {
      std::cout << "Message Not Sent: " << e.what();
    }
  }
}

void Application::queryEnterOrder()
{
  int version = queryVersion();
  std::cout << "\nNewOrderSingle\n";
  FIX::Message order;

  switch ( version ) {
  case 40:
    order = queryNewOrderSingle40();
    break;
  case 41:
    order = queryNewOrderSingle41();
    break;
  case 42:
    order = queryNewOrderSingle42();
    break;
  case 43:
    order = queryNewOrderSingle43();
    break;
  case 44:
    order = queryNewOrderSingle44();
    break;
  case 50:
    order = queryNewOrderSingle50();
    break;
  default:
    std::cerr << "No test for version " << version << std::endl;
    break;
  }

  if ( queryConfirm( "Send order" ) )
    FIX::Session::sendToTarget( order );
}

void Application::queryCancelOrder()
{
  int version = queryVersion();
  std::cout << "\nOrderCancelRequest\n";
  FIX::Message cancel;

  switch ( version ) {
  case 40:
    cancel = queryOrderCancelRequest40();
    break;
  case 41:
    cancel = queryOrderCancelRequest41();
    break;
  case 42:
    cancel = queryOrderCancelRequest42();
    break;
  case 43:
    cancel = queryOrderCancelRequest43();
    break;
  case 44:
    cancel = queryOrderCancelRequest44();
    break;
  case 50:
    cancel = queryOrderCancelRequest50();
    break;
  default:
    std::cerr << "No test for version " << version << std::endl;
    break;
  }

  if ( queryConfirm( "Send cancel" ) )
    FIX::Session::sendToTarget( cancel );
}

void Application::queryReplaceOrder()
{
  int version = queryVersion();
  std::cout << "\nCancelReplaceRequest\n";
  FIX::Message replace;

  switch ( version ) {
  case 40:
    replace = queryCancelReplaceRequest40();
    break;
  case 41:
    replace = queryCancelReplaceRequest41();
    break;
  case 42:
    replace = queryCancelReplaceRequest42();
    break;
  case 43:
    replace = queryCancelReplaceRequest43();
    break;
  case 44:
    replace = queryCancelReplaceRequest44();
    break;
  case 50:
    replace = queryCancelReplaceRequest50();
    break;
  default:
    std::cerr << "No test for version " << version << std::endl;
    break;
  }

  if ( queryConfirm( "Send replace" ) )
    FIX::Session::sendToTarget( replace );
}


void Application::queryResend()
{
  int beg, end;
  std::cout << "BeginSeqNo: ";
  std::cin >> beg;
  std::cout << "EndSeqNo: ";
  std::cin >> end;
  FIX44::ResendRequest rr {FIX::BeginSeqNo(beg), FIX::EndSeqNo(end)}; 
  SetHeader(rr);
  FIX::Session::sendToTarget( rr );
}


void Application::querySecurityListRequest()
{
  auto slrId = std::chrono::system_clock::now();
  int64_t iTime = std::chrono::duration_cast<std::chrono::seconds>(slrId.time_since_epoch()).count();
  FIX44::SecurityListRequest slr{FIX::SecurityReqID{std::to_string(iTime)}, FIX::SecurityListRequestType_ALL_SECURITIES};
  SetHeader(slr);
  FIX::Session::sendToTarget( slr );
}


void Application::queryMarketDataRequest()
{
  int version = queryVersion();
  std::cout << "\nMarketDataRequest\n";
  FIX::Message md;

  switch (version) {
  case 43:
    md = queryMarketDataRequest43();
    break;
  case 44:
    md = queryMarketDataRequest44();
    break;
  case 50:
    md = queryMarketDataRequest50();
    break;
  default:
    std::cerr << "No test for version " << version << std::endl;
    break;
  }

  FIX::Session::sendToTarget( md );
}

FIX40::NewOrderSingle Application::queryNewOrderSingle40()
{
  FIX::OrdType ordType;

  FIX40::NewOrderSingle newOrderSingle(
    queryClOrdID(), FIX::HandlInst( '1' ), querySymbol(), querySide(),
    queryOrderQty(), ordType = queryOrdType() );

  newOrderSingle.set( queryTimeInForce() );
  if ( ordType == FIX::OrdType_LIMIT || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryPrice() );
  if ( ordType == FIX::OrdType_STOP || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryStopPx() );

  queryHeader( newOrderSingle.getHeader() );
  return newOrderSingle;
}

FIX41::NewOrderSingle Application::queryNewOrderSingle41()
{
  FIX::OrdType ordType;

  FIX41::NewOrderSingle newOrderSingle(
    queryClOrdID(), FIX::HandlInst( '1' ), querySymbol(), querySide(),
    ordType = queryOrdType() );

  newOrderSingle.set( queryOrderQty() );
  newOrderSingle.set( queryTimeInForce() );
  if ( ordType == FIX::OrdType_LIMIT || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryPrice() );
  if ( ordType == FIX::OrdType_STOP || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryStopPx() );

  queryHeader( newOrderSingle.getHeader() );
  return newOrderSingle;
}

FIX42::NewOrderSingle Application::queryNewOrderSingle42()
{
  FIX::OrdType ordType;

  FIX42::NewOrderSingle newOrderSingle(
    queryClOrdID(), FIX::HandlInst( '1' ), querySymbol(), querySide(),
    FIX::TransactTime(), ordType = queryOrdType() );

  newOrderSingle.set( queryOrderQty() );
  newOrderSingle.set( queryTimeInForce() );
  if ( ordType == FIX::OrdType_LIMIT || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryPrice() );
  if ( ordType == FIX::OrdType_STOP || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryStopPx() );

  queryHeader( newOrderSingle.getHeader() );
  return newOrderSingle;
}

FIX43::NewOrderSingle Application::queryNewOrderSingle43()
{
  FIX::OrdType ordType;

  FIX43::NewOrderSingle newOrderSingle(
    queryClOrdID(), FIX::HandlInst( '1' ), querySide(),
    FIX::TransactTime(), ordType = queryOrdType() );

  newOrderSingle.set( querySymbol() );
  newOrderSingle.set( queryOrderQty() );
  newOrderSingle.set( queryTimeInForce() );
  if ( ordType == FIX::OrdType_LIMIT || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryPrice() );
  if ( ordType == FIX::OrdType_STOP || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryStopPx() );

  queryHeader( newOrderSingle.getHeader() );
  return newOrderSingle;
}

FIX44::NewOrderSingle Application::queryNewOrderSingle44()
{
  FIX::OrdType ordType;

  FIX44::NewOrderSingle newOrderSingle(
    queryClOrdID(), querySide(),
    FIX::TransactTime(), ordType = queryOrdType() );


  newOrderSingle.set( FIX::HandlInst('1') );
  std::string symbol_str;
  FIX::Symbol symbol = querySymbol();
  symbol_str = symbol.getString();
  newOrderSingle.set( symbol );
  newOrderSingle.set( queryOrderQty() );

  if( ordType != FIX::OrdType_MARKET) {
    newOrderSingle.set( queryPrice() );
    FIX::TimeInForce tif = queryTimeInForce();
    newOrderSingle.set( tif );
    if (  ordType == FIX::OrdType_LIMIT && tif == FIX::TimeInForce( FIX::TimeInForce_GOOD_TILL_DATE ) )
      {
        std::string tifDate;
        std::cout << "Tif Date";
        std::cin >> tifDate;
        newOrderSingle.setField(FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_DATE));
        newOrderSingle.setField(FIX::FIELD::ExpireTime, tifDate);
      }
    if ( ordType == FIX::OrdType_LIMIT && (tif == FIX::TimeInForce( FIX::TimeInForce_DAY ) || tif == FIX::TimeInForce( FIX::TimeInForce_GOOD_TILL_DATE ) ) )
      newOrderSingle.setField( 3927, std::to_string(queryBfxOrderFlags()) );
    
    std::string selfPro;
    std::cout << "Self Pro (Y/N): ";
    std::cin >> selfPro;
    newOrderSingle.setField( 6065, selfPro );
    
  }

  std::string BfxMarginTrading = queryBfxMarginTrading();
  newOrderSingle.setField( 6061, BfxMarginTrading );
  
  std::size_t found = symbol_str.find("F0");
  if ( BfxMarginTrading == "Y" && found!=std::string::npos)
    newOrderSingle.setField( 6070, std::to_string(queryBfxIsolatedMargin()) );

  SetHeader( newOrderSingle );
  return newOrderSingle;
}

FIX50::NewOrderSingle Application::queryNewOrderSingle50()
{
  FIX::OrdType ordType;

  FIX50::NewOrderSingle newOrderSingle(
    queryClOrdID(), querySide(),
    FIX::TransactTime(), ordType = queryOrdType() );

  newOrderSingle.set( FIX::HandlInst('1') );
  newOrderSingle.set( querySymbol() );
  newOrderSingle.set( queryOrderQty() );
  newOrderSingle.set( queryTimeInForce() );
  if ( ordType == FIX::OrdType_LIMIT || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryPrice() );
  if ( ordType == FIX::OrdType_STOP || ordType == FIX::OrdType_STOP_LIMIT )
    newOrderSingle.set( queryStopPx() );

  queryHeader( newOrderSingle.getHeader() );
  return newOrderSingle;
}

FIX40::OrderCancelRequest Application::queryOrderCancelRequest40()
{
  FIX40::OrderCancelRequest orderCancelRequest(
    queryOrigClOrdID(), queryClOrdID(), FIX::CxlType( 'F' ),
    querySymbol(), querySide(), queryOrderQty() );

  queryHeader( orderCancelRequest.getHeader() );
  return orderCancelRequest;
}

FIX41::OrderCancelRequest Application::queryOrderCancelRequest41()
{
  FIX41::OrderCancelRequest orderCancelRequest(
    queryOrigClOrdID(), queryClOrdID(), querySymbol(), querySide() );

  orderCancelRequest.set( queryOrderQty() );
  queryHeader( orderCancelRequest.getHeader() );
  return orderCancelRequest;
}

FIX42::OrderCancelRequest Application::queryOrderCancelRequest42()
{
  FIX42::OrderCancelRequest orderCancelRequest( queryOrigClOrdID(),
      queryClOrdID(), querySymbol(), querySide(), FIX::TransactTime() );

  orderCancelRequest.set( queryOrderQty() );
  queryHeader( orderCancelRequest.getHeader() );
  return orderCancelRequest;
}

FIX43::OrderCancelRequest Application::queryOrderCancelRequest43()
{
  FIX43::OrderCancelRequest orderCancelRequest( queryOrigClOrdID(),
      queryClOrdID(), querySide(), FIX::TransactTime() );

  orderCancelRequest.set( querySymbol() );
  orderCancelRequest.set( queryOrderQty() );
  queryHeader( orderCancelRequest.getHeader() );
  return orderCancelRequest;
}

FIX44::OrderCancelRequest Application::queryOrderCancelRequest44()
{
  FIX44::OrderCancelRequest orderCancelRequest( queryOrigClOrdID(),
      queryClOrdID(), FIX::Side{'1'}, FIX::TransactTime() );

  //orderCancelRequest.set( querySymbol() );
  //orderCancelRequest.set( queryOrderQty() );
  SetHeader( orderCancelRequest );
  return orderCancelRequest;
}

FIX50::OrderCancelRequest Application::queryOrderCancelRequest50()
{
  FIX50::OrderCancelRequest orderCancelRequest( queryOrigClOrdID(),
      queryClOrdID(), querySide(), FIX::TransactTime() );

  orderCancelRequest.set( querySymbol() );
  orderCancelRequest.set( queryOrderQty() );
  queryHeader( orderCancelRequest.getHeader() );
  return orderCancelRequest;
}

FIX40::OrderCancelReplaceRequest Application::queryCancelReplaceRequest40()
{
  FIX40::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(), FIX::HandlInst( '1' ),
    querySymbol(), querySide(), queryOrderQty(), queryOrdType() );

  if ( queryConfirm( "New price" ) )
    cancelReplaceRequest.set( queryPrice() );
  if ( queryConfirm( "New quantity" ) )
    cancelReplaceRequest.set( queryOrderQty() );

  queryHeader( cancelReplaceRequest.getHeader() );
  return cancelReplaceRequest;
}

FIX41::OrderCancelReplaceRequest Application::queryCancelReplaceRequest41()
{
  FIX41::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(), FIX::HandlInst( '1' ),
    querySymbol(), querySide(), queryOrdType() );

  if ( queryConfirm( "New price" ) )
    cancelReplaceRequest.set( queryPrice() );
  if ( queryConfirm( "New quantity" ) )
    cancelReplaceRequest.set( queryOrderQty() );

  queryHeader( cancelReplaceRequest.getHeader() );
  return cancelReplaceRequest;
}

FIX42::OrderCancelReplaceRequest Application::queryCancelReplaceRequest42()
{
  FIX42::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(), FIX::HandlInst( '1' ),
    querySymbol(), querySide(), FIX::TransactTime(), queryOrdType() );

  if ( queryConfirm( "New price" ) )
    cancelReplaceRequest.set( queryPrice() );
  if ( queryConfirm( "New quantity" ) )
    cancelReplaceRequest.set( queryOrderQty() );

  queryHeader( cancelReplaceRequest.getHeader() );
  return cancelReplaceRequest;
}

FIX43::OrderCancelReplaceRequest Application::queryCancelReplaceRequest43()
{
  FIX43::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(), FIX::HandlInst( '1' ),
    querySide(), FIX::TransactTime(), queryOrdType() );

  cancelReplaceRequest.set( querySymbol() );
  if ( queryConfirm( "New price" ) )
    cancelReplaceRequest.set( queryPrice() );
  if ( queryConfirm( "New quantity" ) )
    cancelReplaceRequest.set( queryOrderQty() );

  queryHeader( cancelReplaceRequest.getHeader() );
  return cancelReplaceRequest;
}

FIX44::OrderCancelReplaceRequest Application::queryCancelReplaceRequest44()
{
  FIX44::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(),
    '0', FIX::TransactTime(), '0' );

  cancelReplaceRequest.set( FIX::HandlInst('1') );
  //cancelReplaceRequest.set( querySymbol() );
   if ( queryConfirm( "New quantity" ) )
    cancelReplaceRequest.set( queryOrderQty() );
  if ( queryConfirm( "New price" ) )
    cancelReplaceRequest.set( queryPrice() );
 

  //if( ordType != FIX::OrdType_MARKET) {
    //newOrderSingle.set( queryPrice() );
    FIX::TimeInForce tif = queryTimeInForce();
    cancelReplaceRequest.set( tif );
    if ( tif == FIX::TimeInForce( FIX::TimeInForce_GOOD_TILL_DATE ) )
      {
        std::string tifDate;
        std::cout << "Tif Date";
        std::cin >> tifDate;
        cancelReplaceRequest.setField(FIX::TimeInForce(FIX::TimeInForce_GOOD_TILL_DATE));
        cancelReplaceRequest.setField(FIX::FIELD::ExpireTime, tifDate);
      }
   if ( queryConfirm( "New flag" ) )
    cancelReplaceRequest.setField( 3927, std::to_string(queryBfxOrderFlags()) );
  //}
  SetHeader( cancelReplaceRequest );
  return cancelReplaceRequest;
}

FIX50::OrderCancelReplaceRequest Application::queryCancelReplaceRequest50()
{
  FIX50::OrderCancelReplaceRequest cancelReplaceRequest(
    queryOrigClOrdID(), queryClOrdID(),
    querySide(), FIX::TransactTime(), queryOrdType() );

  cancelReplaceRequest.set( FIX::HandlInst('1') );
  cancelReplaceRequest.set( querySymbol() );
  if ( queryConfirm( "New price" ) )
    cancelReplaceRequest.set( queryPrice() );
  if ( queryConfirm( "New quantity" ) )
    cancelReplaceRequest.set( queryOrderQty() );

  SetHeader( cancelReplaceRequest );
  //queryHeader( cancelReplaceRequest.getHeader() );
  return cancelReplaceRequest;
}

FIX43::MarketDataRequest Application::queryMarketDataRequest43()
{
  FIX::MDReqID mdReqID( "MARKETDATAID" );
  FIX::SubscriptionRequestType subType( FIX::SubscriptionRequestType_SNAPSHOT );
  FIX::MarketDepth marketDepth( 0 );

  FIX43::MarketDataRequest::NoMDEntryTypes marketDataEntryGroup;
  FIX::MDEntryType mdEntryType( FIX::MDEntryType_BID );
  marketDataEntryGroup.set( mdEntryType );

  FIX43::MarketDataRequest::NoRelatedSym symbolGroup;
  FIX::Symbol symbol( "LNUX" );
  symbolGroup.set( symbol );

  FIX43::MarketDataRequest message( mdReqID, subType, marketDepth );
  message.addGroup( marketDataEntryGroup );
  message.addGroup( symbolGroup );

  queryHeader( message.getHeader() );

  std::cout << message.toXML() << std::endl;
  std::cout << message.toString() << std::endl;

  return message;
}

FIX44::MarketDataRequest Application::queryMarketDataRequest44()
{

  FIX::MDReqID mdReqID( std::to_string(time(NULL)) );
  std::cout << "SubReqType (Snapshot 0, Snapshot+Updates 1, Unsub 2): "; 
  int subReqType = -1;
  std::cin >>subReqType;
  FIX::SubscriptionRequestType subType{ (subReqType == 0) ? FIX::SubscriptionRequestType_SNAPSHOT : FIX::SubscriptionRequestType_SNAPSHOT_AND_UPDATES };

  int marketD = 0;
  std::cout << "MarketDepth (0, 1, N): ";
  std::cin >> marketD;
  FIX::MarketDepth marketDepth( marketD );

  FIX44::MarketDataRequest::NoMDEntryTypes marketDataEntryGroup;
  marketDataEntryGroup.set(FIX::MDEntryType{FIX::MDEntryType_BID});
  marketDataEntryGroup.set(FIX::MDEntryType{FIX::MDEntryType_OFFER});
  marketDataEntryGroup.set(FIX::MDEntryType{FIX::MDEntryType_TRADE});

  FIX44::MarketDataRequest::NoRelatedSym symbolGroup;
  
  symbolGroup.set( querySymbol() );

  FIX44::MarketDataRequest message( mdReqID, subType, marketDepth );
  std::cout << "MDUpdateType (FULL REFRESH 0, Incremental 1): "; 
  int mdupdtype = -1;
  std::cin >>mdupdtype;
  if(mdupdtype) {
    message.set(FIX::MDUpdateType{FIX::MDUpdateType_INCREMENTAL_REFRESH});
  }
  else {
    message.set(FIX::MDUpdateType{FIX::MDUpdateType_FULL_REFRESH});
  }
 

  message.set(FIX::AggregatedBook{queryConfirm("Aggregate (Y/N)?: ")});

  marketDataEntryGroup.set(FIX::MDEntryType{FIX::MDEntryType_BID});
  message.addGroup( marketDataEntryGroup );
  marketDataEntryGroup.set(FIX::MDEntryType{FIX::MDEntryType_OFFER});
  message.addGroup( marketDataEntryGroup );
  char trades;
  std::cout << "Include trades? (Y/N)";
  std::cin >> trades;
  if(trades == 'Y') {
   marketDataEntryGroup.set(FIX::MDEntryType{FIX::MDEntryType_TRADE});
   message.addGroup( marketDataEntryGroup );
  }
  
  message.addGroup( symbolGroup );
  SetHeader ( message );

  //std::cout << message.toXML() << std::endl;
  //std::cout << message.toString() << std::endl;

  return message;
}

FIX50::MarketDataRequest Application::queryMarketDataRequest50()
{
  FIX::MDReqID mdReqID( "MARKETDATAID" );
  FIX::SubscriptionRequestType subType( FIX::SubscriptionRequestType_SNAPSHOT );
  FIX::MarketDepth marketDepth( 0 );

  FIX50::MarketDataRequest::NoMDEntryTypes marketDataEntryGroup;
  FIX::MDEntryType mdEntryType( FIX::MDEntryType_BID );
  marketDataEntryGroup.set( mdEntryType );

  FIX50::MarketDataRequest::NoRelatedSym symbolGroup;
  FIX::Symbol symbol( "LNUX" );
  symbolGroup.set( symbol );

  FIX50::MarketDataRequest message( mdReqID, subType, marketDepth );
  message.addGroup( marketDataEntryGroup );
  message.addGroup( symbolGroup );

  queryHeader( message.getHeader() );

  std::cout << message.toXML() << std::endl;
  std::cout << message.toString() << std::endl;

  return message;
}

void Application::queryHeader( FIX::Header& header )
{
  header.setField( querySenderCompID() );
  header.setField( queryTargetCompID() );

  if ( queryConfirm( "Use a TargetSubID" ) )
    header.setField( queryTargetSubID() );
}

char Application::queryAction()
{
  char value;
  std::cout << std::endl
  << "1) Enter Order" << std::endl
  << "2) Cancel Order" << std::endl
  << "3) Replace Order" << std::endl
  << "4) Security List" << std::endl
  << "5) Market Data Request" << std::endl
  << "6) ResendReq" << std::endl
  << "7) MultiLimit" << std::endl
  << "8) Quit" << std::endl
  << "Action: ";
  std::cin >> value;
  switch ( value )
  {
    case '1': case '2': case '3': case '4': case '5': case '6' : case '7' : case '8': break;
    default: throw std::exception();
  }
  return value;
}

int Application::queryVersion()
{
  return 44;
  char value;
  std::cout << std::endl
  << "1) FIX.4.0" << std::endl
  << "2) FIX.4.1" << std::endl
  << "3) FIX.4.2" << std::endl
  << "4) FIX.4.3" << std::endl
  << "5) FIX.4.4" << std::endl
  << "6) FIXT.1.1 (FIX.5.0)" << std::endl
  << "BeginString: ";
  std::cin >> value;
  switch ( value )
  {
    case '1': return 40;
    case '2': return 41;
    case '3': return 42;
    case '4': return 43;
    case '5': return 44;
    case '6': return 50;
    default: throw std::exception();
  }
}

bool Application::queryConfirm( const std::string& query )
{
  std::string value;
  std::cout << std::endl << query << "?: ";
  std::cin >> value;
  return toupper( *value.c_str() ) == 'Y';
}

FIX::SenderCompID Application::querySenderCompID()
{
  std::string value;
  std::cout << std::endl << "SenderCompID: ";
  std::cin >> value;
  return FIX::SenderCompID( value );
}

FIX::TargetCompID Application::queryTargetCompID()
{
  std::string value;
  std::cout << std::endl << "TargetCompID: ";
  std::cin >> value;
  return FIX::TargetCompID( value );
}

FIX::TargetSubID Application::queryTargetSubID()
{
  std::string value;
  std::cout << std::endl << "TargetSubID: ";
  std::cin >> value;
  return FIX::TargetSubID( value );
}

FIX::ClOrdID Application::queryClOrdID()
{
  std::string value;
  std::cout << std::endl << "ClOrdID: ";
  std::cin >> value;
  return FIX::ClOrdID( value );
}

FIX::OrigClOrdID Application::queryOrigClOrdID()
{
  std::string value;
  std::cout << std::endl << "OrigClOrdID: ";
  std::cin >> value;
  return FIX::OrigClOrdID( value );
}

FIX::Symbol Application::querySymbol()
{
  std::string value;
  std::cout << std::endl << "Symbol: ";
  std::cin >> value;
  return FIX::Symbol( value );
}

FIX::Side Application::querySide()
{
  char value;
  std::cout << std::endl
  << "1) Buy" << std::endl
  << "2) Sell" << std::endl
  << "Side: ";

  std::cin >> value;
  switch ( value )
  {
    case '1': return FIX::Side( FIX::Side_BUY );
    case '2': return FIX::Side( FIX::Side_SELL );
    default: throw std::exception();
  }
}

FIX::OrderQty Application::queryOrderQty()
{
  double value;
  std::cout << std::endl << "OrderQty: ";
  std::cin >> value;
  return FIX::OrderQty( value );
}

FIX::OrdType Application::queryOrdType()
{
  char value;
  std::cout << std::endl
  << "1) Market" << std::endl
  << "2) Limit" << std::endl
  << "OrdType: ";

  std::cin >> value;
  switch ( value )
  {
    case '1': return FIX::OrdType( FIX::OrdType_MARKET );
    case '2': return FIX::OrdType( FIX::OrdType_LIMIT );
    default: throw std::exception();
  }
}

FIX::Price Application::queryPrice()
{
  double value;
  std::cout << std::endl << "Price: ";
  std::cin >> value;
  return FIX::Price( value );
}

FIX::StopPx Application::queryStopPx()
{
  double value;
  std::cout << std::endl << "StopPx: ";
  std::cin >> value;
  return FIX::StopPx( value );
}

FIX::TimeInForce Application::queryTimeInForce()
{
  char value;
  std::cout << std::endl
  << "0) Session" << std::endl
  << "3) IOC" << std::endl
  << "4) FOK" << std::endl
  << "6) GTD" << std::endl
  << "TimeInForce: ";

  std::cin >> value;
  switch ( value )
  {
    case '0': return FIX::TimeInForce( FIX::TimeInForce_DAY );
    case '3': return FIX::TimeInForce( FIX::TimeInForce_IMMEDIATE_OR_CANCEL );
    case '4': return FIX::TimeInForce( FIX::TimeInForce_FILL_OR_KILL );
    case '6': return FIX::TimeInForce( FIX::TimeInForce_GOOD_TILL_DATE );
    default: throw std::exception();
  }
}

int Application::queryBfxOrderFlags()
{
  std::string value;
  std::cout << std::endl
  << "0)    Skip this field" << std::endl
  << "64)   Hidden order" << std::endl
  << "4096) Post Only" << std::endl
  << "BFX Order Flag: ";

  std::cin >> value;
  return std::atoi(value.data());
}

std::string Application::queryBfxMarginTrading()
{
  std::string value;
  std::cout << std::endl
  << "N) Exchange Order" << std::endl
  << "Y) Margin Order" << std::endl
  << "BFX Margin Order: ";

  std::cin >> value;
  return value;
}

int Application::queryBfxIsolatedMargin()
{
  int value;
  std::cout << std::endl
  << "BFX Isolated Margin (Only for derivatives, Default: 10): ";

  std::cin >> value;
  return value;
}