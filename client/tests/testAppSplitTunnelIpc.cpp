#include <QTest>
#include <QJsonArray>
#include <QJsonObject>
#include "daemon/daemon.h"
#include "daemon/interfaceconfig.h"

class TestAppSplitTunnelIpc : public QObject {
    Q_OBJECT
    QJsonObject config() {
        return {{"privateKey", "test"}, {"serverPublicKey", "test"}, {"serverPort", 51820},
                {"deviceIpv4Address", "10.8.0.2"}, {"serverIpv4AddrIn", "192.0.2.1"},
                {"allowedIPAddressRanges", QJsonArray()}, {"vpnDisabledApps", QJsonArray()}};
    }
private slots:
    void legacyDefaultsToExclusion() {
        InterfaceConfig parsed;
        QVERIFY(Daemon::parseConfig(config(), parsed));
        QVERIFY(!parsed.m_onlyVpnApps);
    }
    void emptyInclusionListIsPreserved() {
        auto json = config();
        json.insert("onlyVpnApps", true);
        InterfaceConfig parsed;
        QVERIFY(Daemon::parseConfig(json, parsed));
        QVERIFY(parsed.m_onlyVpnApps);
        QVERIFY(parsed.m_vpnDisabledApps.isEmpty());
        const auto serialized = parsed.toJson();
        QVERIFY(serialized.value("onlyVpnApps").toBool());
        InterfaceConfig roundtrip;
        QVERIFY(Daemon::parseConfig(serialized, roundtrip));
        QVERIFY(roundtrip.m_onlyVpnApps);
    }
    void selectedPathsSurviveRoundtrip() {
        auto json = config();
        json.insert("onlyVpnApps", true);
        json.insert("vpnDisabledApps", QJsonArray{"C:/Apps/Browser/browser.exe"});
        InterfaceConfig parsed;
        QVERIFY(Daemon::parseConfig(json, parsed));
        InterfaceConfig roundtrip;
        QVERIFY(Daemon::parseConfig(parsed.toJson(), roundtrip));
        QCOMPARE(roundtrip.m_vpnDisabledApps, parsed.m_vpnDisabledApps);
        QVERIFY(roundtrip.m_onlyVpnApps);
    }
    void invalidModeIsRejected() {
        auto json = config();
        json.insert("onlyVpnApps", "true");
        InterfaceConfig parsed;
        QVERIFY(!Daemon::parseConfig(json, parsed));
    }
};
QTEST_GUILESS_MAIN(TestAppSplitTunnelIpc)
#include "testAppSplitTunnelIpc.moc"
