#pragma once

#include <quickfix/Acceptor.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FileStore.h>
#include <quickfix/FileLog.h>

class FIXapp : public FIX::Application, public FIX::MessageCracker {
public:
    void onCreate(const FIX::SessionID&) override;
    void onLogon(const FIX::SessionID&) override;
    void onLogout(const FIX::SessionID&) override;
    void toAdmin(FIX::Message&, const FIX::SessionID&) override;
    void toApp(FIX::Message&, const FIX::SessionID&) noexcept override;
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept override;
    void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept override;
};