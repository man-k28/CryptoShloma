TEMPLATE = aux

INSTALLER = CryptoShloma-installer-v0.3

INPUT = $$PWD/config/config.xml $$PWD/packages
installer.input = INPUT
installer.output = $$INSTALLER
installer.commands = ~/Qt/Tools/QtInstallerFramework/4.0/bin/binarycreator --offline-only -c $$PWD/config/config.xml -r $$PWD/resources/cryptoshloma.qrc -p $$PWD/packages ${QMAKE_FILE_OUT}
installer.CONFIG += target_predeps no_link combine

QMAKE_EXTRA_COMPILERS += installer

OTHER_FILES += packages/cryptoshloma/meta/package.xml

RESOURCES += resources/cryptoshloma.qrc
